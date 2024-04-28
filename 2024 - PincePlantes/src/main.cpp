/*
 * EsialRobotik CDR 2024 esialrobotik.fr
 * Sketch Arduino dédié au pilotage de la pince à plantes
 * 
 * Utilise les biblothèque :
 *  - ArduPID 0.2.1 https://registry.platformio.org/libraries/powerbroker2/ArduPID pour contrôler le moteur du chariot
 *  - pololu/VL53L1X 1.3.1 https://registry.platformio.org/libraries/pololu/VL53L1X pour contrôler les Time OF Flight (ToF)
 * 
 * Ecrit en franglais avec amour <3
 * Reprise du projet https://github.com/EsialRobotik/Divers_Annuel/tree/master/2023%20-%20AscenseurPinces en enlevant la partie sonde + quelques améliorations 
 */

#include <Arduino.h>
#include <Wire.h>
#include <TimeOfFlightArray.hpp>
#include <Cart.hpp>
#include "PlantManipulator.hpp"

// Commandes utilisateur disponibles sur la liaison série
#define USER_CMD_CHARIOT_ZERO 'z'              // bottom home   fais le zéro du chariot
#define USER_CMD_CHARIOT_MAX_DISTANCE 'Z'      // top home      sonde la distance max que le chriot peut parcourir
#define USER_CMD_CHARIOT_SET_POSITION 'g'      // go            place le chariot à la position demandée et renvoie 'ok' ou 'err' sur la liaison série
#define USER_CMD_CHARIOT_READ_POSITION 'a'     // altitude      renvoie la position du chariot sur la liaison série
#define USER_CMD_CHARIOT_READ_MAX_POSITION 'A' // altitude max  renvoie la distance max que peut parcourir le chariot si elle a été sondée. Renvoie -1 si pas sondée.
#define USER_CMD_CHARIOT_EMERGENCY_STOP 'h'    // halt          arrêt d'urgence du chariot : désactive la puissance dans le moteurs jusqu'au prochain 'z' ou 'g'

#define USER_CMD_TOF_ACQUIRE_LINE 'l'         // line           Effectue une lecture de la ligne de Time Of Flight

#define USER_CMD_FACE_FIRST_VEGETABLE 'f'

#define PCF8574ADDRESS_1 0x3F
#define PCF8574ADDRESS_2 0x3B

/**
 * @brief Mapping entre l'index d'un ToF et sa position physique en mm du point de vue du chariot
 * 
 */
int mappingDistances[] = {
  230,
  212,
  195,
  172,
  160,
  142,
  125,
  120,
  113,
  90,
  72,
  53,
  35,
  18,
  0,  
};

Cart chariot;
TimeOfFlightArray tofArray1 = TimeOfFlightArray(PCF8574ADDRESS_1);
TimeOfFlightArray tofArray2 = TimeOfFlightArray(PCF8574ADDRESS_2, TOF_DEFAULT_ADDRESS + 0x08);

TimeOfFlightArray* tofs[] = {
  &tofArray1,
  &tofArray2
};

PlantManipulator plantManipulator(&Serial, &chariot, sizeof(tofs) / sizeof(TimeOfFlightArray*), &(tofs[0]), &(mappingDistances[0]));

/**
 * Gère les commandes de l'utilisateur en provenance de la liaison série
 */
void handleUserCommands() {
  if (Serial.available() > 0) {
    switch (Serial.read()) {
      case USER_CMD_CHARIOT_ZERO:
        chariot.zero(false);
        break;
      case USER_CMD_CHARIOT_MAX_DISTANCE:
        chariot.zero(true);
        break;
      case USER_CMD_CHARIOT_SET_POSITION:
        chariot.setPosition(Serial.parseInt());
        break;
      case USER_CMD_CHARIOT_READ_POSITION:
        if (chariot.isHomeSet()) {
          Serial.println((int) chariot.getPositionCourante());
        } else {
          Serial.println("err: home not set");
        }
        break;
      case USER_CMD_CHARIOT_READ_MAX_POSITION:
        Serial.println(chariot.getDistanceMax());
        break;
      case USER_CMD_CHARIOT_EMERGENCY_STOP:
        chariot.emergencyStop();
        break;
      case USER_CMD_TOF_ACQUIRE_LINE:
        plantManipulator.acquireAndPrintLine();
        break;
      case USER_CMD_FACE_FIRST_VEGETABLE:
        Serial.println(plantManipulator.faceNextPlantAsync() ? "ok" : "ko");
    }
  }
}

void setup() {
  Wire.begin();
  Serial.begin(115200);
  chariot.init();
  tofArray1.init();
  tofArray2.init();
  tofArray1.startContinuous(20);
  tofArray2.startContinuous(20);
}

void loop() {
  handleUserCommands();
  chariot.heartBeat();
  tofArray1.triggerMeasuresNonBlocking();
  tofArray2.triggerMeasuresNonBlocking();
  plantManipulator.heartBeat();
}