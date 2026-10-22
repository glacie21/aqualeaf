#include <unity.h>
#include "logic/WateringGuard.h"

// Kenaikan minimal 2 %, kunci setelah 4 siklus tidak efektif berturut-turut.
static const float MIN_RISE = 2.0f;
static const uint8_t MAX_INEFFECTIVE = 4;
static const uint8_t THRESHOLD_LOW = 30;

static bool tryCycle(WateringGuard &guard, float moisture) {
    if (!guard.canStartCycle(moisture)) {
        return false;
    }
    guard.onCycleStarted(moisture);
    return true;
}

void setUp(void) {}
void tearDown(void) {}

void test_locks_after_max_ineffective_cycles(void) {
    WateringGuard guard(MIN_RISE, MAX_INEFFECTIVE);
    // Siklus 1..4 berjalan, kelembaban tetap 20 % (tangki kosong).
    for (int i = 0; i < MAX_INEFFECTIVE; ++i) {
        guard.observe(20.0f, THRESHOLD_LOW);
        TEST_ASSERT_TRUE(tryCycle(guard, 20.0f));
    }
    // Siklus ke-5 ditolak dan guard terkunci.
    TEST_ASSERT_FALSE(tryCycle(guard, 20.0f));
    TEST_ASSERT_TRUE(guard.isLockedOut());
}

void test_effective_cycles_do_not_lock(void) {
    WateringGuard guard(MIN_RISE, MAX_INEFFECTIVE);
    float moisture = 10.0f;
    for (int i = 0; i < 10; ++i) {
        TEST_ASSERT_TRUE(tryCycle(guard, moisture));
        moisture += 3.0f;  // tiap siklus menaikkan kelembaban, tetap di bawah threshold
        if (moisture > 28.0f) {
            moisture = 10.0f;
        }
    }
    TEST_ASSERT_FALSE(guard.isLockedOut());
}

void test_leaving_dry_zone_resets_counter(void) {
    WateringGuard guard(MIN_RISE, MAX_INEFFECTIVE);
    for (int i = 0; i < MAX_INEFFECTIVE; ++i) {
        TEST_ASSERT_TRUE(tryCycle(guard, 20.0f));
    }
    // Tanah sempat basah (di atas threshold), lalu kering lagi esok harinya.
    guard.observe(45.0f, THRESHOLD_LOW);
    TEST_ASSERT_EQUAL_UINT8(0, guard.getIneffectiveCycles());
    TEST_ASSERT_TRUE(tryCycle(guard, 20.0f));
    TEST_ASSERT_FALSE(guard.isLockedOut());
}

void test_rejected_activation_is_not_counted(void) {
    WateringGuard guard(MIN_RISE, MAX_INEFFECTIVE);
    TEST_ASSERT_TRUE(tryCycle(guard, 20.0f));
    // Pompa menolak (mis. debounce/batas harian): canStartCycle dipanggil berulang
    // tanpa onCycleStarted, dan itu tidak boleh menambah hitungan.
    for (int i = 0; i < 20; ++i) {
        TEST_ASSERT_TRUE(guard.canStartCycle(20.0f));
    }
    TEST_ASSERT_EQUAL_UINT8(0, guard.getIneffectiveCycles());
    TEST_ASSERT_FALSE(guard.isLockedOut());
}

void test_reset_clears_lockout(void) {
    WateringGuard guard(MIN_RISE, MAX_INEFFECTIVE);
    for (int i = 0; i <= MAX_INEFFECTIVE; ++i) {
        tryCycle(guard, 20.0f);
    }
    TEST_ASSERT_TRUE(guard.isLockedOut());
    guard.reset();
    TEST_ASSERT_FALSE(guard.isLockedOut());
    TEST_ASSERT_TRUE(tryCycle(guard, 20.0f));
}

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_locks_after_max_ineffective_cycles);
    RUN_TEST(test_effective_cycles_do_not_lock);
    RUN_TEST(test_leaving_dry_zone_resets_counter);
    RUN_TEST(test_rejected_activation_is_not_counted);
    RUN_TEST(test_reset_clears_lockout);
    return UNITY_END();
}
