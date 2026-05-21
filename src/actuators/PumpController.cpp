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
                               uint32_t debounceMs)
    : relay(relayPin, activeState, idleState),
      state(PumpState::OFF),
      maxOnMs(maxOnMs),
      cooldownMs(cooldownMs),
      debounceMs(debounceMs),
      lastChangeMs(0),
      activationMs(0),
      cycleCount(0) {
}

void PumpController::begin() {
    relay.begin();
    state = PumpState::OFF;
    lastChangeMs = millis();
}

void PumpController::update() {
    uint32_t now = millis();

    if (state == PumpState::ON && now - activationMs >= maxOnMs) {
        relay.setOn(false);
        state = PumpState::FAULT;
        lastChangeMs = now;
        return;
    }

    if ((state == PumpState::COOLDOWN || state == PumpState::FAULT) && now - lastChangeMs >= cooldownMs) {
        state = PumpState::OFF;
    }
}

bool PumpController::activate() {
    uint32_t now = millis();
    if (state == PumpState::ON) {
        return true;
    }
    if (state == PumpState::COOLDOWN || state == PumpState::FAULT) {
        return false;
    }
    if (now - lastChangeMs < debounceMs) {
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
    if (!relay.isOn() && state != PumpState::ON) {
        state = PumpState::OFF;
        return true;
    }
    relay.setOn(false);
    state = PumpState::COOLDOWN;
    lastChangeMs = millis();
    return true;
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
