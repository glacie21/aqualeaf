/**
 * @file        Helpers.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Utility functions untuk AquaLeaf.
 */

#pragma once

#include <Arduino.h>

float clampFloat(float value, float minimum, float maximum);
String boolToString(bool value);
