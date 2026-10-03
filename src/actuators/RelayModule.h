/**
 * @file        RelayModule.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Abstraksi relay untuk pengendalian pompa.
 */

#pragma once

#include <Arduino.h>

class RelayModule {
public:
    RelayModule(uint8_t pin, uint8_t activeState, uint8_t idleState);
    void begin();
    void setOn(bool on);
    bool isOn() const;

private:
    uint8_t pin;
    uint8_t activeState;
    uint8_t idleState;
    bool currentState;
};
