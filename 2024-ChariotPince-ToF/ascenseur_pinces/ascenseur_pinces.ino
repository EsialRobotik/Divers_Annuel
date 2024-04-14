/*
 * EsialRobotik CDR 2024 esialrobotik.fr
 * Sketch Arduino dédié au pilotage de la pince sur courroie et des ToF
 * 
 * Utilise la biblothèque ArduPID 0.1.4 https://github.com/PowerBroker2/ArduPID pour contrôler le moteur de l'ascenseur
 * 
 * Ecrit en franglais avec amour <3
 * Reprise du projet de l'an dernier, en adaptant pour l'utilisation par décalage gauche-droite et intégration des ToF
 * afin de déterminer où positionner la pince
 */
#include "ArduPID.h"
#include <Wire.h>
#include <VL53L1X.h>

// Entrées/sorties Ascenseur
#define PIN_ODO_CODEUR_A 2
#define PIN_ODO_CODEUR_B 3
#define PIN_PID_MONTER 5
#define PIN_PID_DESCENDRE 6
// Entrées/sorties ToF
// A4(SDA), A5(SCL) for i2c

// Constantes de l'odométrie
#define ODO_TICKS_PAR_MILLIMETRE 30         // Nombre de ticks de codeur par millimètre parcourus
#define ODO_TICKS_OFFSET 15000               // Marge pour éviter les under/overflow en butées
#define ODO_SEUIL_TICKS_DEPLACEMENT_NUL 5   // Seuil de variation du nombre de ticks en dessous duquel on considère l'ascenseur immobile 
#define ODO_MARGE_COMMANDE 2                // Marge en dessous de laquelle on considère la consigne atteinte pour éviter les osciallations, plus la marge est élevée plus l'erreur de position est grande
#define ODO_ZERO_TIMEOUT_MS 6000            // Durée max en ms du zéro de l'ascenseur au delà de laquelle on considère la manoeuvre en échec

// Constantes du PID 
#define PID_MAX_MOTOR_POWER_ZERO 140  // Puissance max du moteur lors de la phase de zéro, ne pas mettre trop de jus car cherche volontairement à se caler contre la butée inférieure
#define PID_MAX_MOTOR_POWER 255       // Puissance max du moteur lors d'une commande de position
#define PID_P 10
#define PID_I 10
#define PID_D 0.01

// Gestion de la vitesse en approche de limites
#define ARRIVAL_BOTTOM_POWER_RATIO 0.7   // Ratio appliqué à la puissance du moteur en approche butée inférieur
#define ARRIVAL_BOTTOM_THRESHOLD_MM 10.  // Distance en millimètre en dessous de laquelle on réduit la puissance du moteur avec le coef ci dessus

// Commandes utilisateur disponibles sur la liaison série
#define USER_CMD_ASCENSEUR_ZERO 'z'              // bottom home   fais le zéro inférieur de l'ascenseur
#define USER_CMD_ASCENSEUR_MAX_HEIGHT 'Z'        // top home      sonde l'altitude maximum de l'ascenseur
#define USER_CMD_ASCENSEUR_SET_POSITION 'g'      // go            place l'ascenseur à la position demandée et renvoie 'ok' ou 'err' sur la liaison série
#define USER_CMD_ASCENSEUR_READ_POSITION 'a'     // altitude      renvoie la position de l'ascenseur sur la liaison série
#define USER_CMD_ASCENSEUR_READ_MAX_POSITION 'A' // altitude max  renvoie la hauteur max de l'ascenseur. Renvoie -1 si pas sondée.
#define USER_CMD_ASCENSEUR_EMERGENCY_STOP 'h'    // halt          arrêt d'urgence de l'ascenseur : désactive la puissance dans le moteurs jusqu'au prochain 'z' ou 'g'

// Constantes des ToF
// Note: Cette adresse doit être différente de 0x29 qui est la valeur de démarrage
// d'un ToF (et les adresses utilisables par les ToF)
#define MULTIPLEXER_I2C_ADDRESS 0x20
#define TOF_COUNT 8
#define TOF_I2C_START_ADDRESS 0x2A
#define TOF_GET_I2C_ADDRESS(i) (TOF_I2C_START_ADDRESS + i)

// Variables de l'odométrie
unsigned int odoTicks = ODO_TICKS_OFFSET;       // Altimètre de l'ascenseur en unité odométrique
int hauteurMaxMm = -1;                          // Hauteur max en millimètres
bool odoPrevA;
bool odoPrevB;
bool odoCurrA;
bool odoCurrB;
bool odoHomeSet = false;                        // Indique si le zéro de l'ascenseur a été fait

// Variables du PID
ArduPID ascenseurPID;
double pidAltitudeCible = 0;
double pidAltitudeCourante = 0;
double pidCommandeMoteur = 0;
int pidConsigneCibleMillimetres = -1;
double pidP = PID_P;
double pidI = PID_I;
double pidD = PID_D;
bool pidEnabled = false;
bool pidTargetReached = false;

// Variables des ToF
VL53L1X tof_sensors[TOF_COUNT];

/**
 * Routine de comptage des ticks codeur incrément/décrément
 */
void odoComptageTicks() {
  odoCurrA = digitalRead(PIN_ODO_CODEUR_A);
  odoCurrB = digitalRead(PIN_ODO_CODEUR_B);

  if (odoCurrA) {
    if (odoCurrB) {
      odoTicks += odoPrevA && !odoPrevB ? -1 : 1;
    } else {
      odoTicks += odoPrevA && odoPrevB ? 1 : -1;
    }
  } else {
    if (odoCurrB) {
      odoTicks += odoPrevA && odoPrevB ? -1 : 1;
    } else {
      odoTicks += odoPrevA && !odoPrevB ? 1 : -1;
    }
  }
  odoPrevA = odoCurrA;
  odoPrevB = odoCurrB;
}

/**
 * Pilote le moteur de l'ascenseur
 * @param commande entre -255 et 255
 * Valeur négative fait descendre l'ascenseur, une valeur positive le fait monter. Une valeur nulle l'arrête
 */
void commanderAscenseur(double commande) {
  analogWrite(PIN_PID_MONTER, commande > 0 ? (int) commande : 0);
  analogWrite(PIN_PID_DESCENDRE, commande < 0 ? (int) -commande : 0);
}

/**
 * Calibre l'ascenseur en déplacment le chariot à l'une ou l'autre de ses extrrêmités
 * Calibration du zéro : déplace le chariot tout en bas.
 * Calibration hauteur max : déplace le chariot tout en haut de l'ascenseur
 * Les déplacements de chariots sont réalisés à faible vitesse. La calibration s'arrête dès que ce dernier est bloqué (plus ticks codeur)
 * Renvoie "ok" en cas de succès ou "err" + la raison en cas d'erreur
 * @param altMax si true effectue une calibration de la hauteur max, sinon du zéro
 */
void handleAscenseurZero(bool altMax) {
  pidEnabled = false;
  commanderAscenseur((double) (altMax ? 1 : -1) * PID_MAX_MOTOR_POWER_ZERO);
  unsigned long timeout = millis() + ODO_ZERO_TIMEOUT_MS;
  unsigned int prevTicks = 0;
  while (true) {
    prevTicks = odoTicks; 
    delay(250);
    Serial.print(" "); // Pour une raison obscure, ce print permet d'obtenir une valeur actualisée d'odoTicks, sinon elle ne varie jamais
    /*
     * Avant de pondre le fix ci-dessus, les autres tentatives infructueuses ont été :
     *  Utiliser une fonction d'accès à odoTicks => unsigned int getOdoTicks() { return odoTicks; }
     *  Utiliser une boucle pour l'attente => while (millis() < timetarget);
     *  Ecrire/lire sur une I/O => digitalWrite() + digitalRead() 
     */
    if (prevTicks > odoTicks) {
      if (prevTicks - odoTicks < ODO_SEUIL_TICKS_DEPLACEMENT_NUL) {
        break;
      }
    } else {
      if (odoTicks - prevTicks < ODO_SEUIL_TICKS_DEPLACEMENT_NUL) {
        break;
      }
    }
    if (millis() > timeout) {
      Serial.print("err: home timeout reached in ");
      Serial.print(ODO_ZERO_TIMEOUT_MS);
      Serial.println("ms");
      commanderAscenseur(0.);
      return;
    }
  }
  commanderAscenseur(0.);
  if (altMax) {
    pidAltitudeCourante = ((double) (odoTicks - ODO_TICKS_OFFSET)) / ((double) ODO_TICKS_PAR_MILLIMETRE);
    hauteurMaxMm = (int) pidAltitudeCourante;
  } else {
    odoTicks = ODO_TICKS_OFFSET;
    odoHomeSet = true;
    altMax = -1;
  }
  Serial.println("ok");
}

/**
 * Vérifie la position demandée et l'applique à l'ascenseur
 * Bloque jusqu'à l'atteinte de la position et renvoie "ok" ou "err" sur la liaison série
 */
void handleAscenseurSetPosition(int position) {
  if (!odoHomeSet) {
    Serial.println("err: home not set");
  }
  if (position < 0) {
    Serial.println("err: position < 0");
    return;
  }
  if (!pidEnabled) {
    pidEnabled = true;
  }
  pidAltitudeCible = (double) position;
  pidConsigneCibleMillimetres = position;
  pidTargetReached = false;
}

void handleEmergencyStop() {
    Serial.println("ok");
    pidEnabled = false;
    commanderAscenseur(0.);
}

/**
 * Gère les commandes de l'utilisateur
 */
void handleUserCommands() {
  if (Serial.available() > 0) {
    switch (Serial.read()) {
      case USER_CMD_ASCENSEUR_ZERO:
        handleAscenseurZero(false);
        break;
      case USER_CMD_ASCENSEUR_MAX_HEIGHT:
        handleAscenseurZero(true);
        break;
      case USER_CMD_ASCENSEUR_SET_POSITION:
        handleAscenseurSetPosition(Serial.parseInt());
        break;
      case USER_CMD_ASCENSEUR_READ_POSITION:
        if (odoHomeSet) {
          Serial.println((int) pidAltitudeCourante);
        } else {
          Serial.println("err: home not set");
        }
        break;
      case USER_CMD_ASCENSEUR_READ_MAX_POSITION:
        Serial.println(hauteurMaxMm);
        break;
      case USER_CMD_ASCENSEUR_EMERGENCY_STOP:
        handleEmergencyStop();
        break;
    }
  }
}

/**
 * Gère l'ascenseur
 * Renvoie true si la consigne est atteinte, false sinon
 */
void handlePID() {
  if (pidEnabled) {
    pidAltitudeCourante = ((double) (odoTicks - ODO_TICKS_OFFSET)) / ((double) ODO_TICKS_PAR_MILLIMETRE);
    ascenseurPID.compute();
    if (abs(pidAltitudeCourante - pidAltitudeCible) < ODO_MARGE_COMMANDE) {
      commanderAscenseur(0.);
      if (!pidTargetReached) {
        pidTargetReached = true;
        Serial.println("ok");
      }
    } else {
      // Si on est en descente et qu'on est en dessous de 10mm, on y va molo sinon on perd l'odométrie quand on tape la butée
      if (pidCommandeMoteur < 0 && pidAltitudeCourante < ARRIVAL_BOTTOM_THRESHOLD_MM) {
        commanderAscenseur(pidCommandeMoteur * ARRIVAL_BOTTOM_POWER_RATIO);
      } else {
        commanderAscenseur(pidCommandeMoteur);
      }
    }
  }
}

/*
* Initialise les ToF, en associant à chacun une adresse I2C unique.
*/
void initToF() {
  // Réinitialisons nos TOF
  Wire.beginTransmission(MULTIPLEXER_I2C_ADDRESS);
  Wire.write(0b00000000);                         
  Wire.endTransmission();
  delay(10);

  // Adressons nos TOF
  for (int i = 0; i < TOF_COUNT; i++) {
    // Acctivons le i-eme ToF
    Wire.beginTransmission(MULTIPLEXER_I2C_ADDRESS);
    Wire.write(1 << i);                         
    Wire.endTransmission();
    delay(10);

    // Initialisation du ToF
    tof_sensors[i].setTimeout(500);
    if (!tof_sensors[i].init())
    {
      Serial.print("Failed to detect and initialize sensor ");
      Serial.println(i);
      while (1);
    }

    tof_sensors[i].setAddress(TOF_GET_I2C_ADDRESS(i));
    tof_sensors[i].startContinuous(50);
  }

  // La configuration de nos ToF est terminée, on peut tout allumer !
  Wire.beginTransmission(MULTIPLEXER_I2C_ADDRESS);
  Wire.write(0b11111111);                         
  Wire.endTransmission();
  delay(10);
}

/**
 * Préparation des E/S et démarrage du PID
 */
void setup() {
  Serial.begin(115200);
  delay(10);

  // Setup I2C
  Wire.begin();
  Wire.setClock(100000); // 100kHz i2c - le PCF8574 ne supporte pas plus
  
  pinMode(PIN_ODO_CODEUR_A, INPUT);
  pinMode(PIN_ODO_CODEUR_B, INPUT);
  pinMode(PIN_PID_MONTER, OUTPUT);
  pinMode(PIN_PID_DESCENDRE, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(PIN_ODO_CODEUR_A), odoComptageTicks, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ODO_CODEUR_B), odoComptageTicks, CHANGE);

  ascenseurPID.setOutputLimits((double) -PID_MAX_MOTOR_POWER, (double) PID_MAX_MOTOR_POWER);
  ascenseurPID.begin(&pidAltitudeCourante, &pidCommandeMoteur, &pidAltitudeCible, pidP, pidI, pidD);

  // Vérifions que notre PCF 8574 est bien présent
  Wire.beginTransmission(MULTIPLEXER_I2C_ADDRESS);
  if(Wire.endTransmission() != 0) {
    Serial.print(F("Le PCF8574 ne répond pas à l'adresse 0x"));
    Serial.println(MULTIPLEXER_I2C_ADDRESS, HEX);
    return;
  }

  initToF();
}

/**
 * Boucle principale du programme 
 */
void loop() {
  handleUserCommands();
  handlePID();
}
