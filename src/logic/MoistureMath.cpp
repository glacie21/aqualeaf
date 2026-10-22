/**
 * @file        MoistureMath.cpp
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Implementasi konversi dan validasi kalibrasi sensor.
 */

#include "MoistureMath.h"

float rawToMoisturePercent(uint16_t raw, uint16_t rawDry, uint16_t rawWet) {
    if (rawDry <= rawWet || raw >= rawDry) {
        return 0.0f;
    }
    if (raw <= rawWet) {
        return 100.0f;
    }
    float normalized = static_cast<float>(rawDry - raw) / static_cast<float>(rawDry - rawWet);
    return normalized * 100.0f;
}

bool isRawInRange(uint16_t raw, uint16_t minRaw, uint16_t maxRaw) {
    return raw >= minRaw && raw <= maxRaw;
}

bool isValidCalibration(uint16_t rawDry, uint16_t rawWet, uint16_t minSpan,
                        uint16_t minRaw, uint16_t maxRaw) {
    if (!isRawInRange(rawDry, minRaw, maxRaw) || !isRawInRange(rawWet, minRaw, maxRaw)) {
        return false;
    }
    return rawDry > rawWet && rawDry - rawWet >= minSpan;
}
