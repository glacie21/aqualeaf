#include <string.h>
#include <unity.h>
#include "Constants.h"
#include "logic/HistoryBuffer.h"
#include "logic/SettingsCodec.h"

static StoredSettings defaults() {
    StoredSettings s;
    s.watering.thresholdLow = 30;
    s.watering.thresholdHigh = 60;
    s.watering.autoMode = true;
    s.watering.manualOverride = false;
    s.watering.allowedHoursMask = ALL_HOURS_MASK;
    s.calibration.rawDry = 820;
    s.calibration.rawWet = 380;
    return s;
}

static StoredSettings custom() {
    StoredSettings s = defaults();
    s.watering.thresholdLow = 25;
    s.watering.thresholdHigh = 70;
    s.watering.autoMode = false;
    s.watering.allowedHoursMask = (1UL << 6) | (1UL << 18);
    s.calibration.rawDry = 790;
    s.calibration.rawWet = 410;
    return s;
}

void setUp(void) {}
void tearDown(void) {}

// ── SettingsCodec ───────────────────────────────────────────

void test_roundtrip(void) {
    uint8_t blob[SETTINGS_BLOB_SIZE];
    encodeSettings(custom(), blob);

    StoredSettings out;
    TEST_ASSERT_EQUAL(DecodeResult::OK, decodeSettings(blob, sizeof(blob), defaults(), out));
    TEST_ASSERT_EQUAL_UINT8(25, out.watering.thresholdLow);
    TEST_ASSERT_EQUAL_UINT8(70, out.watering.thresholdHigh);
    TEST_ASSERT_FALSE(out.watering.autoMode);
    TEST_ASSERT_FALSE(out.watering.manualOverride);
    TEST_ASSERT_EQUAL_HEX32((1UL << 6) | (1UL << 18), out.watering.allowedHoursMask);
    TEST_ASSERT_EQUAL_UINT16(790, out.calibration.rawDry);
    TEST_ASSERT_EQUAL_UINT16(410, out.calibration.rawWet);
}

void test_empty_flash_gives_defaults(void) {
    uint8_t blob[SETTINGS_BLOB_SIZE];
    memset(blob, 0xFF, sizeof(blob));
    StoredSettings out;
    TEST_ASSERT_EQUAL(DecodeResult::EMPTY, decodeSettings(blob, sizeof(blob), defaults(), out));
    TEST_ASSERT_EQUAL_UINT8(30, out.watering.thresholdLow);
    TEST_ASSERT_EQUAL_UINT16(820, out.calibration.rawDry);
}

void test_corrupted_byte_is_detected(void) {
    uint8_t blob[SETTINGS_BLOB_SIZE];
    encodeSettings(custom(), blob);
    blob[6] ^= 0x10;  // bit flip pada rawDry
    StoredSettings out;
    TEST_ASSERT_EQUAL(DecodeResult::CORRUPT, decodeSettings(blob, sizeof(blob), defaults(), out));
    TEST_ASSERT_EQUAL_UINT16(820, out.calibration.rawDry);
}

void test_unknown_version_is_rejected(void) {
    uint8_t blob[SETTINGS_BLOB_SIZE];
    encodeSettings(custom(), blob);
    blob[1] = 99;
    blob[SETTINGS_BLOB_SIZE - 1] = crc8(blob, SETTINGS_BLOB_SIZE - 1);
    StoredSettings out;
    TEST_ASSERT_EQUAL(DecodeResult::CORRUPT, decodeSettings(blob, sizeof(blob), defaults(), out));
}

void test_legacy_format_is_migrated(void) {
    uint8_t blob[SETTINGS_BLOB_SIZE];
    memset(blob, 0xFF, sizeof(blob));
    blob[0] = 0xAF;  // format v1: magic, low, high, autoMode
    blob[1] = 35;
    blob[2] = 65;
    blob[3] = 0;
    StoredSettings out;
    TEST_ASSERT_EQUAL(DecodeResult::MIGRATED_LEGACY, decodeSettings(blob, sizeof(blob), defaults(), out));
    TEST_ASSERT_EQUAL_UINT8(35, out.watering.thresholdLow);
    TEST_ASSERT_EQUAL_UINT8(65, out.watering.thresholdHigh);
    TEST_ASSERT_FALSE(out.watering.autoMode);
    TEST_ASSERT_EQUAL_HEX32(ALL_HOURS_MASK, out.watering.allowedHoursMask);
    TEST_ASSERT_EQUAL_UINT16(820, out.calibration.rawDry);
}

void test_implausible_values_fall_back_to_defaults(void) {
    StoredSettings bad = custom();
    bad.watering.thresholdLow = 80;   // low >= high
    bad.watering.thresholdHigh = 40;
    bad.watering.allowedHoursMask = 0;
    bad.calibration.rawDry = 300;     // dry <= wet
    bad.calibration.rawWet = 500;
    uint8_t blob[SETTINGS_BLOB_SIZE];
    encodeSettings(bad, blob);

    StoredSettings out;
    TEST_ASSERT_EQUAL(DecodeResult::OK, decodeSettings(blob, sizeof(blob), defaults(), out));
    TEST_ASSERT_EQUAL_UINT8(30, out.watering.thresholdLow);
    TEST_ASSERT_EQUAL_UINT8(60, out.watering.thresholdHigh);
    TEST_ASSERT_EQUAL_HEX32(ALL_HOURS_MASK, out.watering.allowedHoursMask);
    TEST_ASSERT_EQUAL_UINT16(820, out.calibration.rawDry);
}

void test_crc8_known_value(void) {
    // CRC-8/ATM dari "123456789" = 0xF4
    const uint8_t data[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    TEST_ASSERT_EQUAL_HEX8(0xF4, crc8(data, sizeof(data)));
}

// ── HistoryBuffer ───────────────────────────────────────────

static HistorySample reading(float pct, bool watered = false) {
    HistorySample s;
    s.moisturePercent = pct;
    s.hasReading = true;
    s.watered = watered;
    return s;
}

void test_history_keeps_order_before_wrap(void) {
    HistoryBuffer<4> history;
    history.push(reading(10.0f));
    history.push(reading(20.0f, true));
    TEST_ASSERT_EQUAL_UINT16(2, history.size());
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 10.0f, history.at(0).moisturePercent);
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 20.0f, history.at(1).moisturePercent);
    TEST_ASSERT_TRUE(history.at(1).watered);
}

void test_history_overwrites_oldest(void) {
    HistoryBuffer<3> history;
    for (int i = 1; i <= 5; ++i) {
        history.push(reading(i * 10.0f));
    }
    TEST_ASSERT_EQUAL_UINT16(3, history.size());
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 30.0f, history.at(0).moisturePercent);
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 50.0f, history.at(2).moisturePercent);
}

void test_history_preserves_one_decimal_and_gaps(void) {
    HistoryBuffer<3> history;
    history.push(reading(42.37f));
    HistorySample gap;
    gap.moisturePercent = 0.0f;
    gap.hasReading = false;
    gap.watered = true;
    history.push(gap);
    history.push(reading(150.0f));  // di-clamp ke 100

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 42.4f, history.at(0).moisturePercent);
    TEST_ASSERT_FALSE(history.at(1).hasReading);
    TEST_ASSERT_TRUE(history.at(1).watered);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 100.0f, history.at(2).moisturePercent);
}

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_roundtrip);
    RUN_TEST(test_empty_flash_gives_defaults);
    RUN_TEST(test_corrupted_byte_is_detected);
    RUN_TEST(test_unknown_version_is_rejected);
    RUN_TEST(test_legacy_format_is_migrated);
    RUN_TEST(test_implausible_values_fall_back_to_defaults);
    RUN_TEST(test_crc8_known_value);
    RUN_TEST(test_history_keeps_order_before_wrap);
    RUN_TEST(test_history_overwrites_oldest);
    RUN_TEST(test_history_preserves_one_decimal_and_gaps);
    return UNITY_END();
}
