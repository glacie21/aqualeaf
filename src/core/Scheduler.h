/**
 * @file        Scheduler.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Simple scheduling helper untuk periodic tasks.
 */

#pragma once

#include <Arduino.h>

class Scheduler {
public:
    static bool isDue(uint32_t &lastRun, uint32_t intervalMs);
};
