/**
 * @file        Constants.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Compile-time symbolic constants, enumerations, and
 *              type aliases shared across the entire codebase.
 * @author      AquaLeaf Team
 * @version     1.0.0
 *
 * This header is also used by the host-side unit tests (env:native),
 * so it must not depend on the Arduino core outside the ARDUINO guard.
 */

#pragma once

#ifdef ARDUINO
  #include <Arduino.h>
#else
  #include <stdint.h>
#endif

// ============================================================
//  Version string
// ============================================================
#ifndef AQUA_VERSION
  #define AQUA_VERSION "1.0.0"
#endif

// ============================================================
//  Boolean convenience
// ============================================================
#ifndef RELAY_ON
  #define RELAY_ON   LOW   ///< Active-LOW relay module: LOW = coil energised
  #define RELAY_OFF  HIGH
#endif

// ============================================================
//  Log levels
// ============================================================
constexpr uint8_t LOG_DEBUG   = 0;
constexpr uint8_t LOG_INFO    = 1;
constexpr uint8_t LOG_WARNING = 2;
constexpr uint8_t LOG_ERROR   = 3;
constexpr uint8_t LOG_FATAL   = 4;

// ============================================================
//  System state machine
//  Numeric values are part of the REST/MQTT API — keep them stable.
// ============================================================
enum class SystemState : uint8_t {
    BOOT          = 0,   ///< Cold start / hardware init
    RUNNING       = 2,   ///< Normal operation
    OTA           = 5,   ///< Over-the-air firmware update in progress
};

// ============================================================
//  Pump state
//  Numeric values are part of the REST/MQTT API — keep them stable.
// ============================================================
enum class PumpState : uint8_t {
    OFF       = 0,   ///< Pump relay open
    ON        = 1,   ///< Pump relay closed
    COOLDOWN  = 2,   ///< Post-watering lockout
    FAULT     = 4,   ///< Safety timeout triggered
};

// ============================================================
//  Operating mode
// ============================================================
enum class OperatingMode : uint8_t {
    AUTO   = 0,   ///< Automated threshold-based control
    MANUAL = 1,   ///< User-controlled via Blynk / web
};

// ============================================================
//  Network connection status
// ============================================================
enum class NetworkStatus : uint8_t {
    DISCONNECTED = 0,
    CONNECTING   = 1,
    CONNECTED    = 2,
    AP_MODE      = 3,
};

// ============================================================
//  MQTT connection status
// ============================================================
enum class MQTTStatus : uint8_t {
    DISCONNECTED = 0,
    CONNECTED    = 2,
};

// ============================================================
//  Numeric limits
// ============================================================
constexpr uint16_t ADC_MAX_RAW       = 1023;
constexpr uint8_t TOPIC_MAX_LEN      = 128;
constexpr uint16_t PAYLOAD_MAX_LEN   = 512;
/** Bitmask with one bit per hour of the day (bit 0 = 00:00–00:59). */
constexpr uint32_t ALL_HOURS_MASK    = 0xFFFFFFUL;

// ============================================================
//  Timing helpers
// ============================================================
constexpr uint32_t MS_PER_SECOND   = 1000UL;
constexpr uint32_t MS_PER_MINUTE   = 60UL * MS_PER_SECOND;
constexpr uint32_t MS_PER_HOUR     = 60UL * MS_PER_MINUTE;
constexpr uint32_t MS_PER_DAY      = 24UL * MS_PER_HOUR;
