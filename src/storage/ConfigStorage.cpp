/**
 * @file        ConfigStorage.cpp
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Implementasi penyimpanan pengaturan di EEPROM.
 */

#include "ConfigStorage.h"
#include "Config.h"
#include <EEPROM.h>

bool ConfigStorage::begin() {
    EEPROM.begin(STORAGE_EEPROM_SIZE);
    return true;
}

DecodeResult ConfigStorage::load(const StoredSettings &defaults, StoredSettings &out) const {
    uint8_t blob[SETTINGS_BLOB_SIZE];
    for (size_t i = 0; i < SETTINGS_BLOB_SIZE; ++i) {
        blob[i] = EEPROM.read(STORAGE_EEPROM_ADDR + i);
    }
    return decodeSettings(blob, SETTINGS_BLOB_SIZE, defaults, out);
}

bool ConfigStorage::save(const StoredSettings &settings) {
    uint8_t blob[SETTINGS_BLOB_SIZE];
    encodeSettings(settings, blob);
    for (size_t i = 0; i < SETTINGS_BLOB_SIZE; ++i) {
        // EEPROM.write hanya menandai dirty jika nilainya berubah, jadi commit
        // tanpa perubahan tidak menulis ulang flash.
        EEPROM.write(STORAGE_EEPROM_ADDR + i, blob[i]);
    }
    return EEPROM.commit();
}
