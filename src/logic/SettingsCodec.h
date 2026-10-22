/**
 * @file        SettingsCodec.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Format biner pengaturan di EEPROM (versi + CRC8) beserta migrasi
 *              dari format lama. Murni byte-in / byte-out agar bisa dites.
 *
 * Layout v2 (little-endian):
 *   [0]     magic 0xA5
 *   [1]     versi format (2)
 *   [2]     threshold low
 *   [3]     threshold high
 *   [4]     flags (bit0 = autoMode)
 *   [5..6]  rawDry
 *   [7..8]  rawWet
 *   [9..12] allowedHoursMask
 *   [13]    CRC8 dari byte 0..12
 *
 * Layout lama (v1): [0] magic 0xAF, [1] low, [2] high, [3] autoMode.
 */

#pragma once

#include <stddef.h>
#include <stdint.h>
#include "models/WateringConfig.h"
#include "models/SensorCalibration.h"

struct StoredSettings {
    WateringConfig watering;
    SensorCalibration calibration;
};

enum class DecodeResult : uint8_t {
    OK,               ///< Data v2 valid
    MIGRATED_LEGACY,  ///< Data v1 dibaca; field baru memakai default
    EMPTY,            ///< Belum pernah disimpan (flash kosong)
    CORRUPT,          ///< Magic cocok tetapi versi/CRC salah; default dipakai
};

constexpr size_t SETTINGS_BLOB_SIZE = 14;

uint8_t crc8(const uint8_t *data, size_t length);

void encodeSettings(const StoredSettings &settings, uint8_t *out);

/**
 * Decode `data` ke `out`. Field yang hilang atau tidak masuk akal diganti
 * dengan nilai dari `defaults`, jadi `out` selalu bisa langsung dipakai.
 */
DecodeResult decodeSettings(const uint8_t *data, size_t length,
                            const StoredSettings &defaults, StoredSettings &out);
