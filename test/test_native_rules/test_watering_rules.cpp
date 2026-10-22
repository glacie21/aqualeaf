#include <unity.h>
#include "Constants.h"
#include "logic/MoistureMath.h"
#include "logic/WateringRules.h"

void setUp(void) {}
void tearDown(void) {}

// ── normalizeThresholds ─────────────────────────────────────

void test_thresholds_valid_pair_unchanged(void) {
    ThresholdPair t = normalizeThresholds(30, 60, 5);
    TEST_ASSERT_EQUAL_UINT8(30, t.low);
    TEST_ASSERT_EQUAL_UINT8(60, t.high);
}

void test_thresholds_out_of_range_are_clamped(void) {
    // Dulu 300 terpotong ke 44 karena parameter uint8_t.
    ThresholdPair t = normalizeThresholds(-5, 300, 5);
    TEST_ASSERT_EQUAL_UINT8(0, t.low);
    TEST_ASSERT_EQUAL_UINT8(100, t.high);
}

void test_thresholds_swapped_when_reversed(void) {
    ThresholdPair t = normalizeThresholds(70, 20, 5);
    TEST_ASSERT_EQUAL_UINT8(20, t.low);
    TEST_ASSERT_EQUAL_UINT8(70, t.high);
}

void test_thresholds_min_gap_enforced(void) {
    ThresholdPair t = normalizeThresholds(40, 42, 5);
    TEST_ASSERT_EQUAL_UINT8(40, t.low);
    TEST_ASSERT_EQUAL_UINT8(45, t.high);
}

void test_thresholds_min_gap_at_upper_bound(void) {
    // Dulu menghasilkan low = high = 100.
    ThresholdPair t = normalizeThresholds(100, 100, 5);
    TEST_ASSERT_EQUAL_UINT8(95, t.low);
    TEST_ASSERT_EQUAL_UINT8(100, t.high);
}

// ── isHourAllowed ───────────────────────────────────────────

void test_all_hours_mask_allows_every_hour(void) {
    for (int hour = 0; hour < 24; ++hour) {
        TEST_ASSERT_TRUE(isHourAllowed(ALL_HOURS_MASK, hour));
    }
}

void test_morning_and_evening_window(void) {
    uint32_t mask = (1UL << 6) | (1UL << 7) | (1UL << 17) | (1UL << 18);
    TEST_ASSERT_TRUE(isHourAllowed(mask, 6));
    TEST_ASSERT_TRUE(isHourAllowed(mask, 18));
    TEST_ASSERT_FALSE(isHourAllowed(mask, 12));
    TEST_ASSERT_FALSE(isHourAllowed(mask, 0));
}

void test_unknown_time_always_allowed(void) {
    TEST_ASSERT_TRUE(isHourAllowed(0, -1));
    TEST_ASSERT_TRUE(isHourAllowed(1UL << 6, 24));
}

// ── MoistureMath ────────────────────────────────────────────

void test_moisture_endpoints(void) {
    TEST_ASSERT_EQUAL_FLOAT(0.0f, rawToMoisturePercent(820, 820, 380));
    TEST_ASSERT_EQUAL_FLOAT(100.0f, rawToMoisturePercent(380, 820, 380));
}

void test_moisture_midpoint(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 50.0f, rawToMoisturePercent(600, 820, 380));
}

void test_moisture_clamped_outside_calibration(void) {
    TEST_ASSERT_EQUAL_FLOAT(0.0f, rawToMoisturePercent(1000, 820, 380));
    TEST_ASSERT_EQUAL_FLOAT(100.0f, rawToMoisturePercent(100, 820, 380));
}

void test_moisture_invalid_calibration_reads_zero(void) {
    TEST_ASSERT_EQUAL_FLOAT(0.0f, rawToMoisturePercent(500, 380, 820));
}

void test_raw_range_check(void) {
    TEST_ASSERT_TRUE(isRawInRange(100, 100, 1000));
    TEST_ASSERT_TRUE(isRawInRange(1000, 100, 1000));
    TEST_ASSERT_FALSE(isRawInRange(99, 100, 1000));
    TEST_ASSERT_FALSE(isRawInRange(1023, 100, 1000));
}

void test_calibration_validation(void) {
    TEST_ASSERT_TRUE(isValidCalibration(820, 380, 50, 100, 1000));
    TEST_ASSERT_FALSE(isValidCalibration(380, 820, 50, 100, 1000));   // terbalik
    TEST_ASSERT_FALSE(isValidCalibration(420, 380, 50, 100, 1000));   // span < 50
    TEST_ASSERT_FALSE(isValidCalibration(1010, 380, 50, 100, 1000));  // dry di luar rentang
    TEST_ASSERT_FALSE(isValidCalibration(820, 50, 50, 100, 1000));    // wet di luar rentang
}

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_thresholds_valid_pair_unchanged);
    RUN_TEST(test_thresholds_out_of_range_are_clamped);
    RUN_TEST(test_thresholds_swapped_when_reversed);
    RUN_TEST(test_thresholds_min_gap_enforced);
    RUN_TEST(test_thresholds_min_gap_at_upper_bound);
    RUN_TEST(test_all_hours_mask_allows_every_hour);
    RUN_TEST(test_morning_and_evening_window);
    RUN_TEST(test_unknown_time_always_allowed);
    RUN_TEST(test_moisture_endpoints);
    RUN_TEST(test_moisture_midpoint);
    RUN_TEST(test_moisture_clamped_outside_calibration);
    RUN_TEST(test_moisture_invalid_calibration_reads_zero);
    RUN_TEST(test_raw_range_check);
    RUN_TEST(test_calibration_validation);
    return UNITY_END();
}
