/**
 * @file        PumpStateMachine.cpp
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Implementasi state machine pompa.
 */

#include "PumpStateMachine.h"

PumpStateMachine::PumpStateMachine(uint32_t maxOnMs,
                                   uint32_t cooldownMs,
                                   uint32_t debounceMs,
                                   uint32_t dailyLimitMs)
    : state(PumpState::OFF),
      maxOnMs(maxOnMs),
      cooldownMs(cooldownMs),
      debounceMs(debounceMs),
      dailyLimitMs(dailyLimitMs),
      lastChangeMs(0),
      activationMs(0),
      cycleCount(0),
      windowStartMs(0),
      windowOnMs(0) {
}

void PumpStateMachine::reset(uint32_t now) {
    state = PumpState::OFF;
    lastChangeMs = now;
    windowStartMs = now;
    windowOnMs = 0;
}

void PumpStateMachine::update(uint32_t now) {
    rollDailyWindow(now);

    if (state == PumpState::ON) {
        if (now - activationMs >= maxOnMs) {
            switchOff(now, PumpState::FAULT);
            return;
        }
        if (isDailyLimitReached(now)) {
            switchOff(now, PumpState::COOLDOWN);
            return;
        }
    }

    if ((state == PumpState::COOLDOWN || state == PumpState::FAULT) && now - lastChangeMs >= cooldownMs) {
        state = PumpState::OFF;
    }
}

bool PumpStateMachine::activate(uint32_t now) {
    rollDailyWindow(now);

    if (state == PumpState::ON) {
        return true;
    }
    if (state == PumpState::COOLDOWN || state == PumpState::FAULT) {
        return false;
    }
    if (now - lastChangeMs < debounceMs) {
        return false;
    }
    if (isDailyLimitReached(now)) {
        return false;
    }

    state = PumpState::ON;
    activationMs = now;
    lastChangeMs = now;
    cycleCount++;
    return true;
}

bool PumpStateMachine::deactivate(uint32_t now) {
    if (state != PumpState::ON) {
        // Sudah mati; jangan reset COOLDOWN/FAULT agar lockout tetap berlaku.
        return false;
    }
    switchOff(now, PumpState::COOLDOWN);
    return true;
}

void PumpStateMachine::switchOff(uint32_t now, PumpState nextState) {
    windowOnMs += now - activationMs;
    state = nextState;
    lastChangeMs = now;
}

void PumpStateMachine::rollDailyWindow(uint32_t now) {
    if (now - windowStartMs < MS_PER_DAY) {
        return;
    }
    // Run yang sedang berjalan saat pergantian jendela dihitung penuh ke jendela baru.
    windowStartMs = now;
    windowOnMs = 0;
}

PumpState PumpStateMachine::getState() const {
    return state;
}

bool PumpStateMachine::isOn() const {
    return state == PumpState::ON;
}

uint32_t PumpStateMachine::getCycleCount() const {
    return cycleCount;
}

uint32_t PumpStateMachine::getOnTimeTodayMs(uint32_t now) const {
    if (state == PumpState::ON) {
        return windowOnMs + (now - activationMs);
    }
    return windowOnMs;
}

bool PumpStateMachine::isDailyLimitReached(uint32_t now) const {
    return dailyLimitMs > 0 && getOnTimeTodayMs(now) >= dailyLimitMs;
}
