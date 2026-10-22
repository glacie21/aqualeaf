#include <Arduino.h>
#include <unity.h>
#include "actuators/PumpController.h"
#include "Constants.h"
#include "Pins.h"

PumpController pump(PIN_RELAY, RELAY_ON, RELAY_OFF, 50, 100, 1);

// begin() mencatat waktu perubahan terakhir; tunggu melewati debounce (1 ms)
// sebelum activate() agar request tidak ditolak.
static void beginPump() {
    pump.begin();
    delay(5);
}

void test_pump_initial_state(void) {
    pump.begin();
    TEST_ASSERT_EQUAL(PumpState::OFF, pump.getState());
    TEST_ASSERT_FALSE(pump.isOn());
}

void test_pump_activate_deactivate(void) {
    beginPump();
    TEST_ASSERT_TRUE(pump.activate());
    TEST_ASSERT_TRUE(pump.isOn());
    delay(10);
    TEST_ASSERT_TRUE(pump.deactivate());
    TEST_ASSERT_FALSE(pump.isOn());
    TEST_ASSERT_EQUAL(PumpState::COOLDOWN, pump.getState());
}

void test_pump_timeout_fault(void) {
    beginPump();
    TEST_ASSERT_TRUE(pump.activate());
    delay(60);
    pump.update();
    TEST_ASSERT_EQUAL(PumpState::FAULT, pump.getState());
    TEST_ASSERT_FALSE(pump.isOn());
}

void test_pump_lockout_survives_deactivate(void) {
    beginPump();
    TEST_ASSERT_TRUE(pump.activate());
    TEST_ASSERT_TRUE(pump.deactivate());
    // Deactivate kedua tidak boleh menghapus cooldown.
    TEST_ASSERT_FALSE(pump.deactivate());
    TEST_ASSERT_EQUAL(PumpState::COOLDOWN, pump.getState());
    TEST_ASSERT_FALSE(pump.activate());
}

void test_pump_cooldown_expires(void) {
    beginPump();
    TEST_ASSERT_TRUE(pump.activate());
    TEST_ASSERT_TRUE(pump.deactivate());
    delay(110);
    pump.update();
    TEST_ASSERT_EQUAL(PumpState::OFF, pump.getState());
    TEST_ASSERT_TRUE(pump.activate());
}

void setup() {
    delay(100);
    UNITY_BEGIN();
    RUN_TEST(test_pump_initial_state);
    RUN_TEST(test_pump_activate_deactivate);
    RUN_TEST(test_pump_timeout_fault);
    RUN_TEST(test_pump_lockout_survives_deactivate);
    RUN_TEST(test_pump_cooldown_expires);
    UNITY_END();
}

void loop() {
}
