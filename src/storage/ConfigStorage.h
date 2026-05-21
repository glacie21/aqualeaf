/**
 * @file        ConfigStorage.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Persistensi konfigurasi threshold di EEPROM.
 */

#pragma once

#include <Arduino.h>
#include "models/WateringConfig.h"

class ConfigStorage {
public:
    bool begin();
    WateringConfig loadWateringConfig() const;
    bool saveWateringConfig(const WateringConfig &config);

private:
    bool validConfig() const;
};
