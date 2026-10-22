/**
 * @file        PumpController.cpp
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Implementasi logika safety pompa.
 */

#include "PumpController.h"
#include <Arduino.h>

PumpController::PumpController(uint8_t relayPin,
                               uint8_t activeState,
                               uint8_t idleState,
                               uint32_t maxOnMs,
                               uint32_t cooldownMs,
                               uint32_t debounceMs,
                               uint32_t dailyLimitMs)
    : relay(relayPin, activeState, idleState),
      state(PumpState::OFF),
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

void PumpController::begin() {
    relay.begin();
    state = PumpState::OFF;
    lastChangeMs = millis();
    windowStartMs = lastChangeMs;
    windowOnMs = 0;
}

void PumpController::update() {
    uint32_t now = millis();
    rollDailyWindow(now);

    if (state == PumpState::ON) {
        if (now - activationMs >= maxOnMs) {
            switchOff(now, PumpState::FAULT);
            return;
        }
        if (isDailyLimitReached()) {
            switchOff(now, PumpState::COOLDOWN);
            return;
        }
    }

    if ((state == PumpState::COOLDOWN || state == PumpState::FAULT) && now - lastChangeMs >= cooldownMs) {
        state = PumpState::OFF;
    }
}

bool PumpController::activate() {
    uint32_t now = millis();
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
    if (isDailyLimitReached()) {
        return false;
    }

    relay.setOn(true);
    state = PumpState::ON;
    activationMs = now;
    lastChangeMs = now;
    cycleCount++;
    return true;
}

bool PumpController::deactivate() {
    if (state != PumpState::ON) {
        // Sudah mati; jangan reset COOLDOWN/FAULT agar lockout tetap berlaku.
        relay.setOn(false);
        return false;
    }
    switchOff(millis(), PumpState::COOLDOWN);
    return true;
}

void PumpController::switchOff(uint32_t now, PumpState nextState) {
    relay.setOn(false);
    windowOnMs += now - activationMs;
    state = nextState;
    lastChangeMs = now;
}

void PumpController::rollDailyWindow(uint32_t now) {
    if (now - windowStartMs < MS_PER_DAY) {
        return;
    }
    // Run yang sedang berjalan saat pergantian jendela dihitung penuh ke jendela baru.
    windowStartMs = now;
    windowOnMs = 0;
}

PumpState PumpController::getState() const {
    return state;
}

bool PumpController::isOn() const {
    return state == PumpState::ON;
}

uint32_t PumpController::getCycleCount() const {
    return cycleCount;
}

uint32_t PumpController::getOnTimeTodayMs() const {
    if (state == PumpState::ON) {
        return windowOnMs + (millis() - activationMs);
    }
    return windowOnMs;
}

bool PumpController::isDailyLimitReached() const {
    return dailyLimitMs > 0 && getOnTimeTodayMs() >= dailyLimitMs;
}
