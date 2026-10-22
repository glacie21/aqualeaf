/**
 * @file        MoistureMath.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Konversi raw ADC ke persen kelembaban dan validasi kalibrasi.
 */

#pragma once

#include <stdint.h>

/**
 * Konversi linear raw ADC ke 0–100 %. Mengasumsikan rawDry > rawWet
 * (sensor kapasitif: makin basah makin kecil nilainya).
 */
float rawToMoisturePercent(uint16_t raw, uint16_t rawDry, uint16_t rawWet);

bool isRawInRange(uint16_t raw, uint16_t minRaw, uint16_t maxRaw);

/**
 * Kalibrasi valid jika kedua titik berada dalam rentang [minRaw, maxRaw]
 * dan rawDry lebih besar dari rawWet setidaknya minSpan.
 */
bool isValidCalibration(uint16_t rawDry, uint16_t rawWet, uint16_t minSpan,
                        uint16_t minRaw, uint16_t maxRaw);
