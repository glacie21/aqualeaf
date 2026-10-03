/**
 * @file        PumpController.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Kontrol pompa dengan timeout safety dan cooldown.
 */

#pragma once

#include <Arduino.h>
#include "RelayModule.h"
#include "Constants.h"

class PumpController {
public:
    PumpController(uint8_t relayPin,
                   uint8_t activeState,
                   uint8_t idleState,
                   uint32_t maxOnMs,
                   uint32_t cooldownMs,
                   uint32_t debounceMs);
    void begin();
    void update();
    bool activate();
    bool deactivate();
    PumpState getState() const;
    bool isOn() const;
    uint32_t getCycleCount() const;

private:
    RelayModule relay;
    PumpState state;
    uint32_t maxOnMs;
    uint32_t cooldownMs;
    uint32_t debounceMs;
    uint32_t lastChangeMs;
    uint32_t activationMs;
    uint32_t cycleCount;
};
