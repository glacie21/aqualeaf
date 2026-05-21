/**
 * @file        Logger.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Logger serial profesional dengan level.
 */

#pragma once

#include <Arduino.h>
#include "Constants.h"

class Logger {
public:
    explicit Logger(uint8_t minLevel = LOG_INFO);
    void begin();
    void debug(const String &message);
    void info(const String &message);
    void warn(const String &message);
    void error(const String &message);
    void fatal(const String &message);

private:
    void log(uint8_t level, const String &message);
    uint8_t minLevel;
};
