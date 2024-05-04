#include "PlantManipulator.hpp"

PlantManipulator::PlantManipulator(Stream* serial, Cart* cart, short tofArrayCount, TimeOfFlightArray** tofArrays, int* distanceByToF)
    : serial(serial)
    , cart(cart)
    , tofArrayCount(tofArrayCount)
    , tofArrays(tofArrays)
    , distanceByToF(distanceByToF)
    , movingToPlant(false)
{;
}

bool PlantManipulator::faceNextPlantAsync() {
    if (!cart->isHomeSet()) {
        serial->println("home not set");
        return false;
    }
    int tofIndex = 0;
    for(int i=0; i<tofArrayCount; i++) {
        for (int j=0; j<TOF_MAX_COUNT; j++) {
            if (tofArrays[i]->tofExists(j)) {
                uint16_t distance = tofArrays[i]->getDistance(j);
                if (distance >= PLANTPANIULATOR_MIN_DISTANCE_MM && distance <= PLANTPANIULATOR_MAX_DISTANCE_MM) {
                    cart->setPosition(distanceByToF[tofIndex]);
                    movingToPlant = true;
                    return true;
                }
            }
            tofIndex++;
        }
    }
    return false;
}

void PlantManipulator::acquireAndPrintLine() {
    for(int i=0; i<tofArrayCount; i++) {
        if (i>0) {
            Serial.print(";");
        }
        for (int j=0; j<TOF_MAX_COUNT; j++) {
            uint16_t d = tofArrays[i]->getDistance(j);
            if (j>0) {
                Serial.print(";");
            }
            if (d == 0xFFFF) {
                Serial.print("-      ");
            } else {
                Serial.print(d);
            if (d < 10) {
                Serial.print("     ");
            } else if (d < 100) {
                Serial.print("    ");
            } else if (d < 1000) {
                Serial.print("   ");
            } else if (d < 10000) {
                Serial.print("  ");
            } else if (d < 100000) {
                Serial.print(" ");
            }
            }
        }
    }
    Serial.println();
}
