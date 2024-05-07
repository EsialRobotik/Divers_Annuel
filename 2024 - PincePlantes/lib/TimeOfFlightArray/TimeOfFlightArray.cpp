#include "TimeOfFlightArray.hpp"

TimeOfFlightArray::TimeOfFlightArray(unsigned char pcf8574Address, unsigned char firstTimeOfFlightAddress)
: pcf8574Address(pcf8574Address)
, firstTimeOfFlightAddress(firstTimeOfFlightAddress)
, samplePeriod(0)
{
    pcf8574IsPresent = false;
    for (unsigned short i = 0; i < TOF_MAX_COUNT; i++) {
        tofs[i] = NULL;
        distances[i] = 0xFFFF;
    }
}

bool TimeOfFlightArray::init() {
    if (!checkIfPcf8574IsPresent()) {
        return false;
    }
    enableAllPcfSlots(false);

    for (int i=0; i<TOF_MAX_COUNT; i++) {
        tofs[i] = instanciateTofIfPresentInSlot(i);
    }
    return true;
}

bool TimeOfFlightArray::tofExists(unsigned short id) {
    return id < TOF_MAX_COUNT && tofs[id] != NULL;
}

uint16_t TimeOfFlightArray::getDistance(unsigned short id) {
    return id < TOF_MAX_COUNT ? distances[id] : 0xFFFF;
}

void TimeOfFlightArray::enableAllPcfSlots(bool enable) {
  pcf8574_state = enable ? 0b11111111 : 0b00000000;
  Wire.beginTransmission(pcf8574Address);
  Wire.write(pcf8574_state);
  Wire.endTransmission();
}

void TimeOfFlightArray::enableSinglePcfSlot(char slot, bool enable) {
  if (enable) {
    pcf8574_state |= (0b00000001 << slot);
  } else {
    pcf8574_state &= 0b11111111 ^ (0b00000001 << slot);
  }
  Wire.beginTransmission(pcf8574Address);
  Wire.write(pcf8574_state);
  Wire.endTransmission();
}

bool TimeOfFlightArray::checkIfPcf8574IsPresent() {
   Wire.beginTransmission(pcf8574Address);
   pcf8574IsPresent = (Wire.endTransmission() == 0);
   return pcf8574IsPresent;
}

/**
 * @brief Teste la présence du TimeOfLight sur le Bus I2C en activation le slot concerné avant le test
 * 
 * @param slot 
 * @return VL53L1X* 
 */
VL53L1X* TimeOfFlightArray::instanciateTofIfPresentInSlot(int slot) {
    enableSinglePcfSlot(slot, true);
    delay(2); // On laisse le temps en ToF de booter : max 1.2ms d'après la doc
    Wire.beginTransmission(TOF_DEFAULT_ADDRESS);
    int error = Wire.endTransmission();

    if (error != 0) {
      enableSinglePcfSlot(slot, false);
      return NULL;
    }

    VL53L1X* tof = new VL53L1X();
    if (!(tof->init(true))) {
        delete tof;
        return NULL;
    }
    tof->setAddress(firstTimeOfFlightAddress + slot + 1);
    tof->setDistanceMode(VL53L1X::DistanceMode::Short);
    tof->setMeasurementTimingBudget(60 * 1000);
    return tof;
}

void TimeOfFlightArray::triggerMeasuresNonBlocking() {
    for (int i=0; i<TOF_MAX_COUNT; i++) {
        if (tofs[i] == NULL) {
            distances[i] = 0xFFFF;
        } else {
            distances[i] = tofs[i]->read(true);
        }
    }
}

uint16_t TimeOfFlightArray::readTriggeredMeasure(unsigned short id) {
    if (id >= TOF_MAX_COUNT) {
        return 0xFFFF;
    }
    return tofs[id]->read(true);
}

void TimeOfFlightArray::triggerMeasuresBlocking() {
    for (int i=0; i<TOF_MAX_COUNT; i++) {
        if (tofs[i] == NULL) {
        distances[i] = 0xFFFF;
        } else {
        distances[i] = tofs[i]->readSingle(true);
        }
    }
}

void TimeOfFlightArray::startContinuous(uint32_t period_ms) {
    samplePeriod = period_ms;
    for (int i=0; i<TOF_MAX_COUNT; i++) {
        if (tofs[i] != NULL) {
            tofs[i]->startContinuous(period_ms);
        }
    }
}

unsigned short TimeOfFlightArray::getConnectedTofCount() {
    unsigned short count = 0;
    for (int i=0; i<TOF_MAX_COUNT; i++) {
        if (tofs[i] != NULL) {
            count++;
        }
    }
    return count;
}

uint32_t TimeOfFlightArray::getCurrentSamplePeriod() {
    return samplePeriod;
}