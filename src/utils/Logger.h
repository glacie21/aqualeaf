/**
 * @file        Logger.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Logger serial profesional dengan level.
 *
 * Overload `const char *` menghindari alokasi String untuk pesan literal.
 * Untuk pesan hasil penggabungan String di jalur yang sering dipanggil,
 * cek isEnabled() dulu agar String tidak dibangun saat level-nya difilter.
 */

#pragma once

#include <Arduino.h>
#include "Constants.h"

class Logger {
public:
    explicit Logger(uint8_t minLevel = LOG_INFO);
    void begin();
    bool isEnabled(uint8_t level) const;
    void debug(const char *message);
    void debug(const String &message);
    void info(const char *message);
    void info(const String &message);
    void warn(const char *message);
    void warn(const String &message);
    void error(const char *message);
    void error(const String &message);
    void fatal(const char *message);
    void fatal(const String &message);

private:
    void log(uint8_t level, const char *message);
    uint8_t minLevel;
};
