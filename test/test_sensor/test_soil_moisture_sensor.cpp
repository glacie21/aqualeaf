// Smoke test hardware: membaca ADC sungguhan. Logika konversi dan validasi
// dites secara deterministik di test/test_native_rules (pio test -e native).
#include <Arduino.h>
#include <unity.h>
#include "sensors/SoilMoistureSensor.h"
#include "Pins.h"

void test_sensor_reading_in_range(void) {
    SoilMoistureSensor sensor(PIN_MOISTURE_SENSOR, 1023, 0, 5, 1);
    sensor.begin();
    float pct = sensor.read();
    TEST_ASSERT_TRUE(pct >= 0.0f && pct <= 100.0f);
    TEST_ASSERT_EQUAL_FLOAT(pct, sensor.getLastPercentage());
    TEST_ASSERT_TRUE(sensor.getLastRaw() <= 1023);
}

void test_sensor_valid_range(void) {
    // Rentang 0–1023 mencakup semua nilai ADC: selalu valid.
    SoilMoistureSensor wide(PIN_MOISTURE_SENSOR, 1023, 0, 1, 1, 0, 1023);
    wide.read();
    TEST_ASSERT_TRUE(wide.isLastReadingValid());

    // Rentang di atas nilai ADC maksimum tidak mungkin tercapai: selalu fault.
    SoilMoistureSensor impossible(PIN_MOISTURE_SENSOR, 1023, 0, 1, 1, 1024, 1024);
    impossible.read();
    TEST_ASSERT_FALSE(impossible.isLastReadingValid());
}

void test_sensor_calibration_is_applied(void) {
    SoilMoistureSensor sensor(PIN_MOISTURE_SENSOR, 1023, 0, 1, 1);
    SensorCalibration calibration;
    calibration.rawDry = 820;
    calibration.rawWet = 380;
    sensor.setCalibration(calibration);
    TEST_ASSERT_EQUAL_UINT16(820, sensor.getCalibration().rawDry);
    TEST_ASSERT_EQUAL_UINT16(380, sensor.getCalibration().rawWet);
}

void setup() {
    delay(100);
    UNITY_BEGIN();
    RUN_TEST(test_sensor_reading_in_range);
    RUN_TEST(test_sensor_valid_range);
    RUN_TEST(test_sensor_calibration_is_applied);
    UNITY_END();
}

void loop() {
}
