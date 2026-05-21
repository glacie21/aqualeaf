#include <Arduino.h>
#include <unity.h>
#include "utils/Helpers.h"

void test_clamp_float(void) {
    TEST_ASSERT_EQUAL_FLOAT(0.0f, clampFloat(-5.0f, 0.0f, 10.0f));
    TEST_ASSERT_EQUAL_FLOAT(10.0f, clampFloat(15.0f, 0.0f, 10.0f));
    TEST_ASSERT_EQUAL_FLOAT(5.0f, clampFloat(5.0f, 0.0f, 10.0f));
}

void test_bool_to_string(void) {
    TEST_ASSERT_EQUAL_STRING("true", boolToString(true).c_str());
    TEST_ASSERT_EQUAL_STRING("false", boolToString(false).c_str());
}

void setup() {
    delay(100);
    UNITY_BEGIN();
    RUN_TEST(test_clamp_float);
    RUN_TEST(test_bool_to_string);
    UNITY_END();
}

void loop() {
}
