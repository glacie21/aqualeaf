#include <Arduino.h>
#include <unity.h>
#include "core/Scheduler.h"

void test_scheduler_due(void) {
    uint32_t lastRun = 0;
    TEST_ASSERT_TRUE(Scheduler::isDue(lastRun, 1));
}

void test_scheduler_not_due(void) {
    uint32_t lastRun = millis();
    TEST_ASSERT_FALSE(Scheduler::isDue(lastRun, 10000));
}

void setup() {
    delay(100);
    UNITY_BEGIN();
    RUN_TEST(test_scheduler_due);
    RUN_TEST(test_scheduler_not_due);
    UNITY_END();
}

void loop() {
}
