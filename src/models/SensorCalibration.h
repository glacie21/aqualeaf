/**
 * @file        SensorCalibration.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Titik kalibrasi sensor kelembaban (nilai raw ADC).
 */

#pragma once

#include <stdint.h>

struct SensorCalibration {
    uint16_t rawDry;   ///< Raw ADC pada 0 % (sensor di udara)
    uint16_t rawWet;   ///< Raw ADC pada 100 % (sensor di air)
};
