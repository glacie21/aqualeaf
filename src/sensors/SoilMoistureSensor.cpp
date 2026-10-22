/**
 * @file        SoilMoistureSensor.cpp
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Implementasi pembacaan sensor kelembaban tanah.
 */

#include "SoilMoistureSensor.h"
#include "logic/MoistureMath.h"
#include <Arduino.h>

SoilMoistureSensor::SoilMoistureSensor(uint8_t analogPin,
                                       uint16_t rawDry,
                                       uint16_t rawWet,
                                       uint8_t samples,
                                       uint16_t sampleDelayMs,
                                       uint16_t rawValidMin,
                                       uint16_t rawValidMax)
    : pin(analogPin),
      calibration{rawDry, rawWet},
      samples(samples),
      sampleDelayMs(sampleDelayMs),
      rawValidMin(rawValidMin),
      rawValidMax(rawValidMax),
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
    lastPercentage = rawToMoisturePercent(lastRaw, calibration.rawDry, calibration.rawWet);
    return lastPercentage;
}

float SoilMoistureSensor::getLastPercentage() const {
    return lastPercentage;
}

bool SoilMoistureSensor::isLastReadingValid() const {
    return isRawInRange(lastRaw, rawValidMin, rawValidMax);
}

uint16_t SoilMoistureSensor::getLastRaw() const {
    return lastRaw;
}

void SoilMoistureSensor::setCalibration(const SensorCalibration &calibration) {
    this->calibration = calibration;
}

SensorCalibration SoilMoistureSensor::getCalibration() const {
    return calibration;
}
