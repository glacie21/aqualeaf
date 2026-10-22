/**
 * @file        WateringRules.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Aturan murni (tanpa Arduino) untuk threshold dan jadwal penyiraman.
 */

#pragma once

#include <stdint.h>

struct ThresholdPair {
    uint8_t low;
    uint8_t high;
};

/**
 * Clamp kedua nilai ke 0–100, tukar jika low > high, lalu jaga jarak minimal
 * minGap (low digeser turun jika high sudah mentok di 100).
 */
ThresholdPair normalizeThresholds(int low, int high, int minGap);

/**
 * True jika penyiraman otomatis boleh dimulai pada jam lokal `hour` (0–23).
 * hour < 0 berarti waktu belum diketahui (NTP belum sinkron); demi tanaman,
 * penyiraman tetap diizinkan.
 */
bool isHourAllowed(uint32_t allowedHoursMask, int hour);
