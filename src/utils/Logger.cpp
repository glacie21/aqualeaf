/**
 * @file        Logger.cpp
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Implementasi logging serial.
 */

#include "Logger.h"
#include <Arduino.h>

Logger::Logger(uint8_t minLevel)
    : minLevel(minLevel) {
}

void Logger::begin() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(100);
    info("Logger initialized");
}

void Logger::debug(const String &message) {
    log(LOG_DEBUG, message);
}

void Logger::info(const String &message) {
    log(LOG_INFO, message);
}

void Logger::warn(const String &message) {
    log(LOG_WARNING, message);
}

void Logger::error(const String &message) {
    log(LOG_ERROR, message);
}

void Logger::fatal(const String &message) {
    log(LOG_FATAL, message);
}

void Logger::log(uint8_t level, const String &message) {
    if (level < minLevel) {
        return;
    }

    const char *tags[] = {"DEBUG", "INFO", "WARN", "ERROR", "FATAL"};
    String output = String("[") + tags[level] + "] ";
    output += message;
    Serial.println(output);
}
