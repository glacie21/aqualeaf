/**
 * @file        Helpers.cpp
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Implementasi helper sederhana.
 */

#include "Helpers.h"

float clampFloat(float value, float minimum, float maximum) {
    if (value < minimum) {
        return minimum;
    }
    if (value > maximum) {
        return maximum;
    }
    return value;
}

String boolToString(bool value) {
    return value ? "true" : "false";
}
