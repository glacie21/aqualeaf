/**
 * @file        ConfigStorage.cpp
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Implementasi penyimpanan konfigurasi di EEPROM.
 */

#include "ConfigStorage.h"
#include "Constants.h"
#include <EEPROM.h>

bool ConfigStorage::begin() {
    return EEPROM.begin(STORAGE_EEPROM_SIZE);
}

WateringConfig ConfigStorage::loadWateringConfig() const {
    WateringConfig config;
    if (!validConfig()) {
        config.thresholdLow = MOISTURE_THRESHOLD_LOW;
        config.thresholdHigh = MOISTURE_THRESHOLD_HIGH;
        config.autoMode = true;
        config.manualOverride = false;
        return config;
    }

    uint16_t base = STORAGE_EEPROM_ADDR;
    config.thresholdLow = EEPROM.read(base + 1);
    config.thresholdHigh = EEPROM.read(base + 2);
    config.autoMode = EEPROM.read(base + 3) == 1;
    config.manualOverride = false;
    return config;
}

bool ConfigStorage::saveWateringConfig(const WateringConfig &config) {
    uint16_t base = STORAGE_EEPROM_ADDR;
    EEPROM.write(base, STORAGE_MAGIC_BYTE);
    EEPROM.write(base + 1, config.thresholdLow);
    EEPROM.write(base + 2, config.thresholdHigh);
    EEPROM.write(base + 3, config.autoMode ? 1 : 0);
    EEPROM.write(base + 4, config.manualOverride ? 1 : 0);
    return EEPROM.commit();
}

bool ConfigStorage::validConfig() const {
    return EEPROM.read(STORAGE_EEPROM_ADDR) == STORAGE_MAGIC_BYTE;
}
