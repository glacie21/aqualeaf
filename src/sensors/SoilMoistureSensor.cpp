/**
 * @file        SoilMoistureSensor.cpp
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Implementasi pembacaan sensor kelembaban tanah.
 */

#include "SoilMoistureSensor.h"
#include <Arduino.h>

SoilMoistureSensor::SoilMoistureSensor(uint8_t analogPin,
                                       uint16_t rawDry,
                                       uint16_t rawWet,
                                       uint8_t samples,
                                       uint16_t sampleDelayMs)
    : pin(analogPin),
      rawDry(rawDry),
      rawWet(rawWet),
      samples(samples),
      sampleDelayMs(sampleDelayMs),
      lastPercentage(0.0f),
      lastRaw(0) {
}

void SoilMoistureSensor::begin() {
    // A0 tidak memerlukan konfigurasi pin tambahan.
}

float SoilMoistureSensor::read() {
    uint32_t total = 0;
    for (uint8_t i = 0; i < samples; ++i) {
        total += analogRead(pin);
        delay(sampleDelayMs);
    }

    lastRaw = static_cast<uint16_t>(total / samples);
    lastPercentage = convertRawToPercentage(lastRaw);
    return lastPercentage;
}

float SoilMoistureSensor::getLastPercentage() const {
    return lastPercentage;
}

uint16_t SoilMoistureSensor::getLastRaw() const {
    return lastRaw;
}

float SoilMoistureSensor::convertRawToPercentage(uint16_t rawValue) const {
    if (rawValue >= rawDry) {
        return 0.0f;
    }
    if (rawValue <= rawWet) {
        return 100.0f;
    }

    float normalized = static_cast<float>(rawDry - rawValue) / static_cast<float>(rawDry - rawWet);
    normalized = max(0.0f, min(1.0f, normalized));
    return normalized * 100.0f;
}
