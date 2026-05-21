#include <Arduino.h>
#include <unity.h>
#include "actuators/PumpController.h"
#include "Constants.h"
#include "Pins.h"

PumpController pump(PIN_RELAY, RELAY_ON, RELAY_OFF, 50, 100, 1);

void test_pump_initial_state(void) {
    pump.begin();
    TEST_ASSERT_EQUAL(PumpState::OFF, pump.getState());
    TEST_ASSERT_FALSE(pump.isOn());
}

void test_pump_activate_deactivate(void) {
    pump.begin();
    TEST_ASSERT_TRUE(pump.activate());
    TEST_ASSERT_TRUE(pump.isOn());
    delay(10);
    TEST_ASSERT_TRUE(pump.deactivate());
    TEST_ASSERT_FALSE(pump.isOn());
    TEST_ASSERT_EQUAL(PumpState::COOLDOWN, pump.getState());
}

void test_pump_timeout_fault(void) {
    pump.begin();
    TEST_ASSERT_TRUE(pump.activate());
    delay(60);
    pump.update();
    TEST_ASSERT_EQUAL(PumpState::FAULT, pump.getState());
}

void setup() {
    delay(100);
    UNITY_BEGIN();
    RUN_TEST(test_pump_initial_state);
    RUN_TEST(test_pump_activate_deactivate);
    RUN_TEST(test_pump_timeout_fault);
    UNITY_END();
}

void loop() {
}
