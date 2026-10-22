/**
 * @file        SoilMoistureSensor.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Soil moisture sensor abstraction.
 */

#pragma once

#include <Arduino.h>
#include "models/SensorCalibration.h"

class SoilMoistureSensor {
public:
    SoilMoistureSensor(uint8_t analogPin,
                       uint16_t rawDry,
                       uint16_t rawWet,
                       uint8_t samples,
                       uint16_t sampleDelayMs,
                       uint16_t rawValidMin = 0,
                       uint16_t rawValidMax = 1023);
    void begin();
    float read();
    float getLastPercentage() const;
    uint16_t getLastRaw() const;
    /** False when the last raw reading is outside the plausible range. */
    bool isLastReadingValid() const;
    void setCalibration(const SensorCalibration &calibration);
    SensorCalibration getCalibration() const;

private:
    uint8_t pin;
    SensorCalibration calibration;
    uint8_t samples;
    uint16_t sampleDelayMs;
    uint16_t rawValidMin;
    uint16_t rawValidMax;
    float lastPercentage;
    uint16_t lastRaw;
};
