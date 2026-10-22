#include <unity.h>
#include "logic/PumpStateMachine.h"

// maxOn 1000 ms, cooldown 5000 ms, debounce 200 ms, tanpa batas harian.
static PumpStateMachine makePump(uint32_t dailyLimitMs = 0) {
    PumpStateMachine pump(1000, 5000, 200, dailyLimitMs);
    pump.reset(0);
    return pump;
}

void setUp(void) {}
void tearDown(void) {}

void test_starts_off(void) {
    PumpStateMachine pump = makePump();
    TEST_ASSERT_EQUAL(PumpState::OFF, pump.getState());
    TEST_ASSERT_FALSE(pump.isOn());
}

void test_debounce_rejects_early_activation(void) {
    PumpStateMachine pump = makePump();
    TEST_ASSERT_FALSE(pump.activate(100));
    TEST_ASSERT_TRUE(pump.activate(200));
    TEST_ASSERT_EQUAL_UINT32(1, pump.getCycleCount());
}

void test_activate_is_idempotent(void) {
    PumpStateMachine pump = makePump();
    TEST_ASSERT_TRUE(pump.activate(300));
    TEST_ASSERT_TRUE(pump.activate(400));
    TEST_ASSERT_EQUAL_UINT32(1, pump.getCycleCount());
}

void test_deactivate_enters_cooldown(void) {
    PumpStateMachine pump = makePump();
    pump.activate(300);
    TEST_ASSERT_TRUE(pump.deactivate(500));
    TEST_ASSERT_EQUAL(PumpState::COOLDOWN, pump.getState());
    TEST_ASSERT_FALSE(pump.activate(600));
}

void test_second_deactivate_keeps_lockout(void) {
    PumpStateMachine pump = makePump();
    pump.activate(300);
    pump.deactivate(500);
    TEST_ASSERT_FALSE(pump.deactivate(600));
    TEST_ASSERT_EQUAL(PumpState::COOLDOWN, pump.getState());
}

void test_max_on_time_triggers_fault(void) {
    PumpStateMachine pump = makePump();
    pump.activate(300);
    pump.update(1299);
    TEST_ASSERT_TRUE(pump.isOn());
    pump.update(1300);
    TEST_ASSERT_EQUAL(PumpState::FAULT, pump.getState());
    TEST_ASSERT_FALSE(pump.activate(1400));
}

void test_cooldown_expires(void) {
    PumpStateMachine pump = makePump();
    pump.activate(300);
    pump.deactivate(500);
    pump.update(5499);
    TEST_ASSERT_EQUAL(PumpState::COOLDOWN, pump.getState());
    pump.update(5500);
    TEST_ASSERT_EQUAL(PumpState::OFF, pump.getState());
    TEST_ASSERT_TRUE(pump.activate(5500));
}

void test_fault_clears_after_cooldown(void) {
    PumpStateMachine pump = makePump();
    pump.activate(300);
    pump.update(1300);  // FAULT pada t=1300
    pump.update(6300);
    TEST_ASSERT_EQUAL(PumpState::OFF, pump.getState());
}

void test_on_time_is_accumulated(void) {
    PumpStateMachine pump = makePump();
    pump.activate(300);
    TEST_ASSERT_EQUAL_UINT32(200, pump.getOnTimeTodayMs(500));
    pump.deactivate(700);
    TEST_ASSERT_EQUAL_UINT32(400, pump.getOnTimeTodayMs(800));
}

void test_daily_limit_stops_running_pump(void) {
    PumpStateMachine pump = makePump(500);
    pump.activate(300);
    pump.update(799);
    TEST_ASSERT_TRUE(pump.isOn());
    pump.update(800);
    TEST_ASSERT_EQUAL(PumpState::COOLDOWN, pump.getState());
    TEST_ASSERT_TRUE(pump.isDailyLimitReached(800));
}

void test_daily_limit_blocks_activation_after_cooldown(void) {
    PumpStateMachine pump = makePump(500);
    pump.activate(300);
    pump.update(800);
    pump.update(5800);
    TEST_ASSERT_EQUAL(PumpState::OFF, pump.getState());
    TEST_ASSERT_FALSE(pump.activate(5800));
}

void test_daily_window_resets_after_24h(void) {
    PumpStateMachine pump = makePump(500);
    pump.activate(300);
    pump.update(800);
    uint32_t nextDay = MS_PER_DAY + 1000;
    pump.update(nextDay);
    TEST_ASSERT_FALSE(pump.isDailyLimitReached(nextDay));
    TEST_ASSERT_TRUE(pump.activate(nextDay));
}

void test_millis_overflow_is_handled(void) {
    PumpStateMachine pump(1000, 5000, 200, 0);
    uint32_t start = 0xFFFFFF00UL;
    pump.reset(start);
    TEST_ASSERT_TRUE(pump.activate(start + 300));  // melewati 0xFFFFFFFF
    pump.update(start + 1299);
    TEST_ASSERT_TRUE(pump.isOn());
    pump.update(start + 1300);
    TEST_ASSERT_EQUAL(PumpState::FAULT, pump.getState());
}

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_starts_off);
    RUN_TEST(test_debounce_rejects_early_activation);
    RUN_TEST(test_activate_is_idempotent);
    RUN_TEST(test_deactivate_enters_cooldown);
    RUN_TEST(test_second_deactivate_keeps_lockout);
    RUN_TEST(test_max_on_time_triggers_fault);
    RUN_TEST(test_cooldown_expires);
    RUN_TEST(test_fault_clears_after_cooldown);
    RUN_TEST(test_on_time_is_accumulated);
    RUN_TEST(test_daily_limit_stops_running_pump);
    RUN_TEST(test_daily_limit_blocks_activation_after_cooldown);
    RUN_TEST(test_daily_window_resets_after_24h);
    RUN_TEST(test_millis_overflow_is_handled);
    return UNITY_END();
}
