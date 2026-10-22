/**
 * @file        WateringGuard.cpp
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Implementasi proteksi dry-run.
 */

#include "WateringGuard.h"

WateringGuard::WateringGuard(float minRisePct, uint8_t maxIneffectiveCycles)
    : minRisePct(minRisePct),
      maxIneffectiveCycles(maxIneffectiveCycles),
      ineffectiveCycles(0),
      pendingIneffectiveCycles(0),
      cycleStartMoisture(-1.0f),
      lockedOut(false) {
}

void WateringGuard::observe(float moisture, uint8_t thresholdLow) {
    if (moisture > thresholdLow) {
        ineffectiveCycles = 0;
        cycleStartMoisture = -1.0f;
    }
}

bool WateringGuard::canStartCycle(float moisture) {
    if (lockedOut) {
        return false;
    }

    bool previousIneffective = cycleStartMoisture >= 0.0f &&
                               moisture - cycleStartMoisture < minRisePct;
    pendingIneffectiveCycles = previousIneffective ? ineffectiveCycles + 1 : 0;

    if (pendingIneffectiveCycles >= maxIneffectiveCycles) {
        lockedOut = true;
        return false;
    }
    return true;
}

void WateringGuard::onCycleStarted(float moisture) {
    ineffectiveCycles = pendingIneffectiveCycles;
    cycleStartMoisture = moisture;
}

bool WateringGuard::isLockedOut() const {
    return lockedOut;
}

uint8_t WateringGuard::getIneffectiveCycles() const {
    return ineffectiveCycles;
}

void WateringGuard::reset() {
    lockedOut = false;
    ineffectiveCycles = 0;
    pendingIneffectiveCycles = 0;
    cycleStartMoisture = -1.0f;
}
