/**
 * @file        PumpController.cpp
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Implementasi kontrol pompa; logika safety ada di PumpStateMachine.
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
      machine(maxOnMs, cooldownMs, debounceMs, dailyLimitMs) {
}

void PumpController::begin() {
    relay.begin();
    machine.reset(millis());
}

void PumpController::update() {
    machine.update(millis());
    applyRelay();
}

bool PumpController::activate() {
    bool on = machine.activate(millis());
    applyRelay();
    return on;
}

bool PumpController::deactivate() {
    bool changed = machine.deactivate(millis());
    // Relay selalu dipaksa mati, bahkan jika state machine sudah OFF.
    relay.setOn(false);
    return changed;
}

void PumpController::applyRelay() {
    if (relay.isOn() != machine.isOn()) {
        relay.setOn(machine.isOn());
    }
}

PumpState PumpController::getState() const {
    return machine.getState();
}

bool PumpController::isOn() const {
    return machine.isOn();
}

uint32_t PumpController::getCycleCount() const {
    return machine.getCycleCount();
}

uint32_t PumpController::getOnTimeTodayMs() const {
    return machine.getOnTimeTodayMs(millis());
}

bool PumpController::isDailyLimitReached() const {
    return machine.isDailyLimitReached(millis());
}
