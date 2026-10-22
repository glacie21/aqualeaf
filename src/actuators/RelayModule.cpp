/**
 * @file        RelayModule.cpp
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Implementasi driver relay untuk kontrol pompa.
 */

#include "RelayModule.h"

RelayModule::RelayModule(uint8_t pin, uint8_t activeState, uint8_t idleState)
    : pin(pin),
      activeState(activeState),
      idleState(idleState),
      currentState(false) {
}

void RelayModule::begin() {
    // Set level idle sebelum pin menjadi output agar relay tidak sempat aktif saat boot.
    digitalWrite(pin, idleState);
    pinMode(pin, OUTPUT);
    currentState = false;
}

void RelayModule::setOn(bool on) {
    currentState = on;
    digitalWrite(pin, on ? activeState : idleState);
}

bool RelayModule::isOn() const {
    return currentState;
}
