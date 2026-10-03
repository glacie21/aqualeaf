/**
 * @file        SensorData.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Struktur data sensor kelembaban.
 */

#pragma once

#include <Arduino.h>

struct SensorData {
    float moisturePercent;
    uint16_t rawValue;
    uint32_t timestamp;
};
