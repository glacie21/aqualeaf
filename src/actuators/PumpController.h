/**
 * @file        PumpController.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Kontrol pompa dengan timeout safety, cooldown, dan batas harian.
 */

#pragma once

#include <Arduino.h>
#include "RelayModule.h"
#include "Constants.h"

class PumpController {
public:
    /** dailyLimitMs = 0 menonaktifkan batas waktu nyala harian. */
    PumpController(uint8_t relayPin,
                   uint8_t activeState,
                   uint8_t idleState,
                   uint32_t maxOnMs,
                   uint32_t cooldownMs,
                   uint32_t debounceMs,
                   uint32_t dailyLimitMs = 0);
    void begin();
    void update();
    bool activate();
    bool deactivate();
    PumpState getState() const;
    bool isOn() const;
    uint32_t getCycleCount() const;
    /** Total waktu nyala pada jendela 24 jam saat ini (termasuk run yang sedang berjalan). */
    uint32_t getOnTimeTodayMs() const;
    bool isDailyLimitReached() const;

private:
    void switchOff(uint32_t now, PumpState nextState);
    void rollDailyWindow(uint32_t now);

    RelayModule relay;
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
