/**
 * @file        ConfigStorage.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Persistensi pengaturan (threshold, jadwal, kalibrasi) di EEPROM.
 */

#pragma once

#include <Arduino.h>
#include "logic/SettingsCodec.h"

class ConfigStorage {
public:
    bool begin();
    /** Selalu mengisi `out` (default jika data kosong/rusak). */
    DecodeResult load(const StoredSettings &defaults, StoredSettings &out) const;
    bool save(const StoredSettings &settings);
};
