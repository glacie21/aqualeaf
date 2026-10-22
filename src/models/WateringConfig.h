/**
 * @file        WateringConfig.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Konfigurasi threshold, mode otomatis, dan jadwal.
 */

#pragma once

#include <stdint.h>

struct WateringConfig {
    uint8_t thresholdLow;
    uint8_t thresholdHigh;
    bool autoMode;
    bool manualOverride;        ///< Tidak disimpan ke EEPROM
    uint32_t allowedHoursMask;  ///< Bit n = penyiraman otomatis boleh dimulai pada jam n
};
