/**
 * @file        WateringRules.cpp
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Implementasi aturan threshold dan jadwal penyiraman.
 */

#include "WateringRules.h"

static int clampInt(int value, int minimum, int maximum) {
    if (value < minimum) {
        return minimum;
    }
    if (value > maximum) {
        return maximum;
    }
    return value;
}

ThresholdPair normalizeThresholds(int low, int high, int minGap) {
    low = clampInt(low, 0, 100);
    high = clampInt(high, 0, 100);
    minGap = clampInt(minGap, 0, 100);

    if (low > high) {
        int temp = low;
        low = high;
        high = temp;
    }

    if (high - low < minGap) {
        high = clampInt(low + minGap, 0, 100);
        low = high - minGap;
    }

    ThresholdPair result;
    result.low = static_cast<uint8_t>(low);
    result.high = static_cast<uint8_t>(high);
    return result;
}

bool isHourAllowed(uint32_t allowedHoursMask, int hour) {
    if (hour < 0 || hour > 23) {
        return true;
    }
    return (allowedHoursMask & (1UL << hour)) != 0;
}
