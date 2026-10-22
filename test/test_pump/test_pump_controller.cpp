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

void test_pump_daily_limit(void) {
    // Batas harian (30 ms) lebih pendek dari max on-time (50 ms).
    PumpController limited(PIN_RELAY, RELAY_ON, RELAY_OFF, 50, 10, 1, 30);
    limited.begin();
    delay(5);
    TEST_ASSERT_TRUE(limited.activate());
    delay(35);
    limited.update();
    TEST_ASSERT_EQUAL(PumpState::COOLDOWN, limited.getState());
    TEST_ASSERT_TRUE(limited.isDailyLimitReached());
    delay(15);
    limited.update();
    TEST_ASSERT_EQUAL(PumpState::OFF, limited.getState());
    // Cooldown selesai, tetapi jatah harian sudah habis.
    TEST_ASSERT_FALSE(limited.activate());
    TEST_ASSERT_FALSE(limited.isOn());
}

void test_pump_no_daily_limit_by_default(void) {
    beginPump();
    TEST_ASSERT_TRUE(pump.activate());
    delay(10);
    TEST_ASSERT_FALSE(pump.isDailyLimitReached());
    TEST_ASSERT_TRUE(pump.getOnTimeTodayMs() >= 10);
    pump.deactivate();
}

void setup() {
    delay(100);
    UNITY_BEGIN();
    RUN_TEST(test_pump_initial_state);
    RUN_TEST(test_pump_activate_deactivate);
    RUN_TEST(test_pump_timeout_fault);
    RUN_TEST(test_pump_lockout_survives_deactivate);
    RUN_TEST(test_pump_cooldown_expires);
    RUN_TEST(test_pump_daily_limit);
    RUN_TEST(test_pump_no_daily_limit_by_default);
    UNITY_END();
}

void loop() {
}
