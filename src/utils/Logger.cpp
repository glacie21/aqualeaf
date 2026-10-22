/**
 * @file        Logger.cpp
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Implementasi logging serial.
 */

#include "Logger.h"
#include "Config.h"
#include <Arduino.h>

Logger::Logger(uint8_t minLevel)
    : minLevel(minLevel) {
}

void Logger::begin() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(100);
    info("Logger initialized");
}

bool Logger::isEnabled(uint8_t level) const {
    return level >= minLevel;
}

void Logger::debug(const char *message) {
    log(LOG_DEBUG, message);
}

void Logger::debug(const String &message) {
    log(LOG_DEBUG, message.c_str());
}

void Logger::info(const char *message) {
    log(LOG_INFO, message);
}

void Logger::info(const String &message) {
    log(LOG_INFO, message.c_str());
}

void Logger::warn(const char *message) {
    log(LOG_WARNING, message);
}

void Logger::warn(const String &message) {
    log(LOG_WARNING, message.c_str());
}

void Logger::error(const char *message) {
    log(LOG_ERROR, message);
}

void Logger::error(const String &message) {
    log(LOG_ERROR, message.c_str());
}

void Logger::fatal(const char *message) {
    log(LOG_FATAL, message);
}

void Logger::fatal(const String &message) {
    log(LOG_FATAL, message.c_str());
}

void Logger::log(uint8_t level, const char *message) {
    if (!isEnabled(level)) {
        return;
    }

    static const char *const tags[] = {"DEBUG", "INFO", "WARN", "ERROR", "FATAL"};
    const char *tag = level <= LOG_FATAL ? tags[level] : "?";
    Serial.print('[');
    Serial.print(tag);
    Serial.print("] ");
    Serial.println(message);
}
