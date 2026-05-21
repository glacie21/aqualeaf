/**
 * @file        Scheduler.cpp
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Implementasi penjadwalan sederhana.
 */

#include "Scheduler.h"

bool Scheduler::isDue(uint32_t &lastRun, uint32_t intervalMs) {
    uint32_t now = millis();
    if (now - lastRun >= intervalMs) {
        lastRun = now;
        return true;
    }
    return false;
}
