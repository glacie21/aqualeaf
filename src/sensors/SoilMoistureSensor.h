/**
 * @file        SoilMoistureSensor.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Soil moisture sensor abstraction.
 */

#pragma once

#include <Arduino.h>

class SoilMoistureSensor {
public:
    SoilMoistureSensor(uint8_t analogPin,
                       uint16_t rawDry,
                       uint16_t rawWet,
                       uint8_t samples,
                       uint16_t sampleDelayMs);
    void begin();
    float read();
    float getLastPercentage() const;
    uint16_t getLastRaw() const;

private:
    uint8_t pin;
    uint16_t rawDry;
    uint16_t rawWet;
    uint8_t samples;
    uint16_t sampleDelayMs;
    float lastPercentage;
    uint16_t lastRaw;
    float convertRawToPercentage(uint16_t rawValue) const;
};
