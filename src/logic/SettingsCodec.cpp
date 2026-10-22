/**
 * @file        SettingsCodec.cpp
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Implementasi encode/decode pengaturan EEPROM.
 */

#include "SettingsCodec.h"
#include "Constants.h"

static const uint8_t MAGIC_V2 = 0xA5;
static const uint8_t FORMAT_VERSION = 2;
static const uint8_t MAGIC_LEGACY = 0xAF;
static const size_t LEGACY_SIZE = 4;
static const uint8_t FLAG_AUTO_MODE = 0x01;

uint8_t crc8(const uint8_t *data, size_t length) {
    // CRC-8/ATM (polinomial 0x07), cukup untuk mendeteksi flash yang rusak.
    uint8_t crc = 0;
    for (size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; ++bit) {
            crc = (crc & 0x80) ? static_cast<uint8_t>((crc << 1) ^ 0x07) : static_cast<uint8_t>(crc << 1);
        }
    }
    return crc;
}

static void writeU16(uint8_t *out, uint16_t value) {
    out[0] = static_cast<uint8_t>(value & 0xFF);
    out[1] = static_cast<uint8_t>(value >> 8);
}

static uint16_t readU16(const uint8_t *in) {
    return static_cast<uint16_t>(in[0] | (in[1] << 8));
}

static void writeU32(uint8_t *out, uint32_t value) {
    for (uint8_t i = 0; i < 4; ++i) {
        out[i] = static_cast<uint8_t>((value >> (8 * i)) & 0xFF);
    }
}

static uint32_t readU32(const uint8_t *in) {
    uint32_t value = 0;
    for (uint8_t i = 0; i < 4; ++i) {
        value |= static_cast<uint32_t>(in[i]) << (8 * i);
    }
    return value;
}

void encodeSettings(const StoredSettings &settings, uint8_t *out) {
    out[0] = MAGIC_V2;
    out[1] = FORMAT_VERSION;
    out[2] = settings.watering.thresholdLow;
    out[3] = settings.watering.thresholdHigh;
    out[4] = settings.watering.autoMode ? FLAG_AUTO_MODE : 0;
    writeU16(&out[5], settings.calibration.rawDry);
    writeU16(&out[7], settings.calibration.rawWet);
    writeU32(&out[9], settings.watering.allowedHoursMask);
    out[13] = crc8(out, SETTINGS_BLOB_SIZE - 1);
}

// Ganti nilai yang tidak masuk akal dengan default.
static void sanitize(StoredSettings &settings, const StoredSettings &defaults) {
    WateringConfig &w = settings.watering;
    if (w.thresholdHigh > 100 || w.thresholdLow >= w.thresholdHigh) {
        w.thresholdLow = defaults.watering.thresholdLow;
        w.thresholdHigh = defaults.watering.thresholdHigh;
    }
    w.manualOverride = false;
    w.allowedHoursMask &= ALL_HOURS_MASK;
    if (w.allowedHoursMask == 0) {
        w.allowedHoursMask = ALL_HOURS_MASK;
    }

    SensorCalibration &c = settings.calibration;
    if (c.rawDry <= c.rawWet || c.rawDry > ADC_MAX_RAW) {
        c = defaults.calibration;
    }
}

DecodeResult decodeSettings(const uint8_t *data, size_t length,
                            const StoredSettings &defaults, StoredSettings &out) {
    out = defaults;
    if (length < 1) {
        return DecodeResult::EMPTY;
    }

    if (data[0] == MAGIC_LEGACY && length >= LEGACY_SIZE) {
        out.watering.thresholdLow = data[1];
        out.watering.thresholdHigh = data[2];
        out.watering.autoMode = data[3] == 1;
        sanitize(out, defaults);
        return DecodeResult::MIGRATED_LEGACY;
    }

    if (data[0] != MAGIC_V2) {
        return DecodeResult::EMPTY;
    }
    if (length < SETTINGS_BLOB_SIZE ||
        data[1] != FORMAT_VERSION ||
        crc8(data, SETTINGS_BLOB_SIZE - 1) != data[SETTINGS_BLOB_SIZE - 1]) {
        return DecodeResult::CORRUPT;
    }

    out.watering.thresholdLow = data[2];
    out.watering.thresholdHigh = data[3];
    out.watering.autoMode = (data[4] & FLAG_AUTO_MODE) != 0;
    out.calibration.rawDry = readU16(&data[5]);
    out.calibration.rawWet = readU16(&data[7]);
    out.watering.allowedHoursMask = readU32(&data[9]);
    sanitize(out, defaults);
    return DecodeResult::OK;
}
