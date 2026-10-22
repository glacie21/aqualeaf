/**
 * @file        PumpStateMachine.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       State machine pompa murni: timeout safety, cooldown, debounce,
 *              dan batas waktu nyala harian. Waktu (`now`, ms) disuntikkan
 *              pemanggil sehingga bisa dites tanpa hardware.
 */

#pragma once

#include <stdint.h>
#include "Constants.h"

class PumpStateMachine {
public:
    /** dailyLimitMs = 0 menonaktifkan batas waktu nyala harian. */
    PumpStateMachine(uint32_t maxOnMs,
                     uint32_t cooldownMs,
                     uint32_t debounceMs,
                     uint32_t dailyLimitMs = 0);

    void reset(uint32_t now);
    /** Jalankan timeout / cooldown; panggil sesering mungkin. */
    void update(uint32_t now);
    /** True jika pompa menyala setelah pemanggilan (termasuk jika sudah menyala). */
    bool activate(uint32_t now);
    /** True jika pompa sebelumnya menyala dan kini dimatikan. */
    bool deactivate(uint32_t now);

    PumpState getState() const;
    bool isOn() const;
    uint32_t getCycleCount() const;
    /** Total waktu nyala pada jendela 24 jam saat ini (termasuk run yang sedang berjalan). */
    uint32_t getOnTimeTodayMs(uint32_t now) const;
    bool isDailyLimitReached(uint32_t now) const;

private:
    void switchOff(uint32_t now, PumpState nextState);
    void rollDailyWindow(uint32_t now);

    PumpState state;
    uint32_t maxOnMs;
    uint32_t cooldownMs;
    uint32_t debounceMs;
    uint32_t dailyLimitMs;
    uint32_t lastChangeMs;
    uint32_t activationMs;
    uint32_t cycleCount;
    uint32_t windowStartMs;
    uint32_t windowOnMs;
};
