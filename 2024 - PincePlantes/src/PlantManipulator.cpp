#include "PlantManipulator.hpp"

PlantManipulator::PlantManipulator(Stream* serial, Cart* cart, short tofArrayCount, TimeOfFlightArray** tofArrays, int* distanceByToF)
    : serial(serial)
    , cart(cart)
    , tofArrayCount(tofArrayCount)
    , tofArrays(tofArrays)
    , distanceByToF(distanceByToF)
    , lastClosestObjectTofIndexInArray(-1)
{;
}

bool PlantManipulator::faceNextClosestPlantAsync() {
    if (!cart->isHomeSet()) {
        serial->println("home not set");
        return false;
    }
    int tofIndex = 0;
    int targetPosition = -1;
    uint16_t smallestDistance = 10000;
    for(int i=0; i<tofArrayCount; i++) {
        for (int j=0; j<TOF_MAX_COUNT; j++) {
            if (tofArrays[i]->tofExists(j)) {
                uint16_t distance = tofArrays[i]->getDistance(j);
                if (
                    distance >= PLANTPANIULATOR_MIN_DISTANCE_MM &&
                    distance <= PLANTPANIULATOR_MAX_DISTANCE_MM &&
                    distance < smallestDistance
                ) {
                    smallestDistance = distance;
                    targetPosition = distanceByToF[tofIndex];
                }
            }
            tofIndex++;
        }
    }

    if (targetPosition != -1) {
        cart->setPosition(targetPosition);
    }
    return targetPosition != -1;
}

int PlantManipulator::getClosestObjectDistance(unsigned int sampleCount) {
    int tofCount = tofArrayCount * TOF_MAX_COUNT;
    uint16_t accumulator[tofCount];
    for (int m=0; m<tofCount; m++) {
        accumulator[m] = 0; // Par défaut 10 mètres, bien au delà des specs des ToF pour détecter facilement les abérations
    }

    for (unsigned int k=0; k<sampleCount; k++) {
        for(int i=0; i<tofArrayCount; i++) {
            for (int j=0; j<TOF_MAX_COUNT; j++) {
                if (tofArrays[i]->tofExists(j)) {
                    accumulator[i*TOF_MAX_COUNT + j] += tofArrays[i]->readTriggeredMeasure(j);
                } else {
                    accumulator[i*TOF_MAX_COUNT + j] = 10000;
                }
            }
        }
    }

    uint16_t smallestDistance = 10000; // Par défaut 10 mètres, bien au delà des specs des ToF pour détecter facilement les abérations
    for (int m=0; m<tofCount; m++) {
        uint16_t mean = accumulator[m] / sampleCount;
        if (mean < smallestDistance) {
            smallestDistance = mean;
            lastClosestObjectTofIndexInArray = m;
        }
    }

    return smallestDistance;

}

int PlantManipulator::getLastClosestObjectDistance(unsigned int sampleCount) {
    if (lastClosestObjectTofIndexInArray == -1) {
        return getClosestObjectDistance(sampleCount);
    }

    int tofArrayIndex = lastClosestObjectTofIndexInArray / TOF_MAX_COUNT;
    int tofIndex = lastClosestObjectTofIndexInArray % TOF_MAX_COUNT;

    uint16_t accumulator = 0;
    if (tofArrays[tofArrayIndex]->tofExists(tofIndex)) {
        for (unsigned int k=0; k<sampleCount; k++) {
            accumulator += tofArrays[tofArrayIndex]->readTriggeredMeasure(tofIndex);
        }
        accumulator /= sampleCount;
    } else {
        accumulator = 10000;
    }

    return accumulator;
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
