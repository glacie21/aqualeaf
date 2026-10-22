/**
 * @file        WateringGuard.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Proteksi dry-run: mengunci penyiraman otomatis jika beberapa
 *              siklus berturut-turut tidak menaikkan kelembaban (tangki kosong,
 *              selang lepas, atau sensor macet di nilai kering).
 */

#pragma once

#include <stdint.h>

class WateringGuard {
public:
    WateringGuard(float minRisePct, uint8_t maxIneffectiveCycles);

    /**
     * Panggil setiap pembacaan sensor. Kelembaban di atas threshold low berarti
     * tanah sudah keluar dari zona kering, jadi penyiraman sebelumnya efektif.
     */
    void observe(float moisture, uint8_t thresholdLow);

    /**
     * Cek sebelum memulai siklus. Mengembalikan false (dan mengunci) jika siklus
     * ini akan menjadi siklus tidak efektif ke-N berturut-turut.
     */
    bool canStartCycle(float moisture);

    /**
     * Panggil hanya jika pompa benar-benar menyala, supaya penolakan (debounce /
     * batas harian) tidak terhitung berulang setiap pembacaan sensor.
     */
    void onCycleStarted(float moisture);

    bool isLockedOut() const;
    uint8_t getIneffectiveCycles() const;
    void reset();

private:
    float minRisePct;
    uint8_t maxIneffectiveCycles;
    uint8_t ineffectiveCycles;
    uint8_t pendingIneffectiveCycles;
    float cycleStartMoisture;   ///< < 0 = belum ada siklus yang dicatat
    bool lockedOut;
};
