/**
 * @file        WateringConfig.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Konfigurasi threshold dan mode otomatis.
 */

#pragma once

struct WateringConfig {
    uint8_t thresholdLow;
    uint8_t thresholdHigh;
    bool autoMode;
    bool manualOverride;
};
