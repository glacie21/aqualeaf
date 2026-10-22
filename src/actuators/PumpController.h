/**
 * @file        PumpController.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Kontrol pompa: menghubungkan PumpStateMachine dengan relay fisik.
 */

#pragma once

#include <Arduino.h>
#include "RelayModule.h"
#include "Constants.h"
#include "logic/PumpStateMachine.h"

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
    void applyRelay();

    RelayModule relay;
    PumpStateMachine machine;
};
