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
}

void test_sensor_linear_conversion(void) {
    SoilMoistureSensor sensor(PIN_MOISTURE_SENSOR, 1023, 0, 5, 1);
    float pct = sensor.read();
    float expected = (1023.0f - sensor.getLastRaw()) / 1023.0f * 100.0f;
    TEST_ASSERT_FLOAT_WITHIN(0.01f, expected, pct);
}

void test_sensor_clamps_dry(void) {
    // Setiap nilai raw >= rawDry dianggap kering total.
    SoilMoistureSensor sensor(PIN_MOISTURE_SENSOR, 0, 0, 1, 1);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, sensor.read());
}

void test_sensor_clamps_wet(void) {
    // Setiap nilai raw <= rawWet dianggap basah total.
    SoilMoistureSensor sensor(PIN_MOISTURE_SENSOR, 1024, 1023, 1, 1);
    TEST_ASSERT_EQUAL_FLOAT(100.0f, sensor.read());
}

void setup() {
    delay(100);
    UNITY_BEGIN();
    RUN_TEST(test_sensor_reading_in_range);
    RUN_TEST(test_sensor_linear_conversion);
    RUN_TEST(test_sensor_clamps_dry);
    RUN_TEST(test_sensor_clamps_wet);
    UNITY_END();
}

void loop() {
}
