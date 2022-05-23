/*
 * EsialRobotik CDR 2022 esialrobotik.fr
 * Sketch Arduino dédié au pilotage de l'ascenseur et la détection des carrés de fouille du robot principal
 * 
 * Utilise la biblothèque ArduPID 0.1.4 https://github.com/PowerBroker2/ArduPID pour contrôler le moteur de l'ascenseur
 * 
 * Ecrit en franglais avec amour <3
 */
#include "ArduPID.h"

// Entrées/sorties
#define PIN_ODO_CODEUR_A 2
#define PIN_ODO_CODEUR_B 3
#define PIN_PID_MONTER 5
#define PIN_PID_DESCENDRE 6
#define PIN_SONDE_ANALOG 0
#define PIN_SONDE_LED_RED 11
#define PIN_SONDE_LED_GREEN 10
#define PIN_SONDE_LED_BLUE 9

// Constantes de l'odométrie
#define ODO_TICKS_PAR_MILLIMETRE 30         // Nombre de ticks de codeur par millimètre parcourus
#define ODO_HAUTEUR_MAX_MILLIMETRES 210     // Hauteur de l'ascenseur
#define ODO_TICKS_OFFSET 1000               // Marge pour éviter les under/overflow en butées
#define ODO_SEUIL_TICKS_DEPLACEMENT_NUL 5   // Seuil de variation du nombre de ticks en dessous duquel on considère l'ascenseur immobile 
#define ODO_MARGE_COMMANDE 2                // Marge en dessous de laquelle on considère la consigne atteinte pour éviter les osciallations, plus la marge est élevée plus l'erreur de position est grande
#define ODO_ZERO_TIMEOUT_MS 10000           // Durée max en ms du zéro de l'ascenseur au delà de laquelle on considère la manoeuvre en échec

// Constantes du PID 
#define PID_MAX_MOTOR_POWER_ZERO 100  // Puissance max du moteur lors de la phase de zéro, ne pas mettre trop de jus car cherche volontairement à se caler contre la butée inférieure
#define PID_MAX_MOTOR_POWER 255       // Puissance max du moteur lors d'une commande de position
#define PID_P 10
#define PID_I 10
#define PID_D 0.01

// Commandes utilisateur disponibles sur la liaison série
#define USER_CMD_ASCENSEUR_ZERO 'z'              // zReset   place l'ascenseur tout en bas de son axe et reset à 0 sa hauteur
#define USER_CMD_ASCENSEUR_SET_POSITION 'g'      // go       place l'ascenseur à la position demandée et renvoie 'ok' ou 'err' sur la liaison série
#define USER_CMD_ASCENSEUR_READ_POSITION 'a'     // altitude renvoie la position de l'ascenseur sur la liaison série
#define USER_CMD_ASCENSEUR_EMERGENCY_STOP 'h'    // halt     arrêt d'urgence de l'ascenseur : désactive la puissance dans le moteurs jusqu'au prochain 'z' ou 'g'
#define USER_CMD_ASCENSEUR_READ_HAUTEUR_MAX 'm'  // max      renvoie la hauteur max de l'ascenseur sur la liaison série
#define USER_CMD_LECTEUR_CARRE 's'               // sonde    renvoie la valeur de la sonde du carré de fouille sur la liaison série

// Constantes de la sonde
#define PROBE_THRESHOLD_NOTHING 925     // pas d'échantillon présent
#define PROBE_THRESHOLD_TRAP 750        // échantillon piégé
#define PROBE_THRESHOLD_YELLOW 450      // échantillon équipe jaune
#define PROBE_THRESHOLD_PURPLE 250      // échantillon équipe violette
#define PROBE_SAMPLES 5                 // fenêtre d'échantillonage 

// Variables de la sonde
enum PROBE_RESULT {
  PROBE_NOTHING,
  PROBE_TRAP,
  PROBE_YELLOW,
  PROBE_PURPLE,
  PROBE_UNDEFINED
};
PROBE_RESULT probeCurrentResult;
const char probeValue2Letter[] = {
  'n',
  't',
  'y',
  'p',
  'u'
};

// Variables de l'odométrie
unsigned int odoTicks = ODO_TICKS_OFFSET;
unsigned int odoTicksMax = ODO_TICKS_OFFSET + (ODO_TICKS_PAR_MILLIMETRE * ODO_HAUTEUR_MAX_MILLIMETRES);
bool odoPrevA;
bool odoPrevB;
bool odoCurrA;
bool odoCurrB;
bool odoHomeSet = false;

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

/**
 * Routine de comptage des ticks codeur 
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
 */
void commanderAscenseur(double commande) {
  analogWrite(PIN_PID_MONTER, commande > 0 ? (int) commande : 0);
  analogWrite(PIN_PID_DESCENDRE, commande < 0 ? (int) -commande : 0);
}

/**
 * Calibre l'ascenseur : cale la nacelle tout en haut
 * Renvoie "ok" en cas de succès ou "err" + la raison en cas d'erreur
 */
void handleAscenseurZero() {
  pidEnabled = false;
  commanderAscenseur((double) PID_MAX_MOTOR_POWER_ZERO);
  unsigned long timeout = millis() + ODO_ZERO_TIMEOUT_MS;
  unsigned int prevTicks = 0;
  while (true) {
    prevTicks = odoTicks; 
    delay(250);
    Serial.print(" "); // Pour une raison obscure, ce print permet d'obtenir une valeur actualisée d'odoTicks, sinon elle ne varie jamais
    /*
     * Les autres tentatives infructueuses ont été :
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
      Serial.print("err: home timeout reached in");
      Serial.print(ODO_ZERO_TIMEOUT_MS);
      Serial.println("ms");
      commanderAscenseur(0.);
      return;
    }
  }
  commanderAscenseur(0.);
  odoTicks = ODO_TICKS_OFFSET + ODO_TICKS_PAR_MILLIMETRE * ODO_HAUTEUR_MAX_MILLIMETRES;
  odoHomeSet = true;
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
  if (position > ODO_HAUTEUR_MAX_MILLIMETRES) {
    Serial.print("err: position >");
    Serial.println(ODO_HAUTEUR_MAX_MILLIMETRES);
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
        handleAscenseurZero();
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
      case USER_CMD_ASCENSEUR_EMERGENCY_STOP:
        handleEmergencyStop();
        break;
      case USER_CMD_ASCENSEUR_READ_HAUTEUR_MAX:
        Serial.println(ODO_HAUTEUR_MAX_MILLIMETRES);
        break;
      case USER_CMD_LECTEUR_CARRE:
        Serial.println(probeValue2Letter[probeCurrentResult]);
        break;
    }
  }
}

void handleProbe() {
  unsigned int measures = 0;
  for(int i=0; i<PROBE_SAMPLES; i++) {
    measures += analogRead(PIN_SONDE_ANALOG);
  }
  measures /= PROBE_SAMPLES;
  if (measures > PROBE_THRESHOLD_NOTHING) {
    probeCurrentResult = PROBE_NOTHING;
  } else if (measures > PROBE_THRESHOLD_TRAP) {
    probeCurrentResult = PROBE_TRAP;
  } else if (measures > PROBE_THRESHOLD_YELLOW) {
    probeCurrentResult = PROBE_YELLOW;
  } else if (measures > PROBE_THRESHOLD_PURPLE) {
    probeCurrentResult = PROBE_PURPLE;
  } else {
    probeCurrentResult = PROBE_UNDEFINED;
  }

  if (probeCurrentResult != PROBE_NOTHING) {
    digitalWrite(PIN_SONDE_LED_RED, probeCurrentResult != PROBE_NOTHING && probeCurrentResult != PROBE_UNDEFINED);
    digitalWrite(PIN_SONDE_LED_GREEN, probeCurrentResult == PROBE_YELLOW);
    digitalWrite(PIN_SONDE_LED_BLUE, probeCurrentResult == PROBE_PURPLE);
  } else {
    digitalWrite(PIN_SONDE_LED_RED, !pidEnabled && odoHomeSet);
    digitalWrite(PIN_SONDE_LED_GREEN, false);
    digitalWrite(PIN_SONDE_LED_BLUE, !odoHomeSet);
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
      commanderAscenseur(pidCommandeMoteur);
    }
  }
}

/**
 * Préparation des E/S et démarrage du PID
 */
void setup() {
  Serial.begin(115200);
  delay(10);
  
  pinMode(PIN_ODO_CODEUR_A, INPUT);
  pinMode(PIN_ODO_CODEUR_B, INPUT);
  pinMode(PIN_PID_MONTER, OUTPUT);
  pinMode(PIN_PID_DESCENDRE, OUTPUT);
  pinMode(PIN_SONDE_LED_RED, OUTPUT);
  pinMode(PIN_SONDE_LED_GREEN, OUTPUT);
  pinMode(PIN_SONDE_LED_BLUE, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(PIN_ODO_CODEUR_A), odoComptageTicks, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ODO_CODEUR_B), odoComptageTicks, CHANGE);

  ascenseurPID.setOutputLimits((double) -PID_MAX_MOTOR_POWER, (double) PID_MAX_MOTOR_POWER);
  ascenseurPID.begin(&pidAltitudeCourante, &pidCommandeMoteur, &pidAltitudeCible, pidP, pidI, pidD);
}

/**
 * Boucle principale du programme 
 */
void loop() {
  handleUserCommands();
  handlePID();
  handleProbe();
}
