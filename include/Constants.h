/**
 * @file        Constants.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Compile-time symbolic constants, enumerations, and
 *              type aliases shared across the entire codebase.
 * @author      AquaLeaf Team
 * @version     1.0.0
 */

#pragma once

#include <Arduino.h>

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
// ============================================================
enum class SystemState : uint8_t {
    BOOT          = 0,   ///< Cold start / hardware init
    CONNECTING    = 1,   ///< Establishing WiFi connection
    RUNNING       = 2,   ///< Normal operation
    OFFLINE       = 3,   ///< WiFi lost; operating in local mode
    ERROR         = 4,   ///< Unrecoverable hardware fault
    OTA           = 5,   ///< Over-the-air firmware update in progress
    DEEP_SLEEP    = 6,   ///< Low-power sleep mode
};

// ============================================================
//  Pump state
// ============================================================
enum class PumpState : uint8_t {
    OFF       = 0,   ///< Pump relay open
    ON        = 1,   ///< Pump relay closed
    COOLDOWN  = 2,   ///< Post-watering lockout
    MANUAL    = 3,   ///< Manually overridden by user
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
    CONNECTING   = 1,
    CONNECTED    = 2,
    ERROR        = 3,
};

// ============================================================
//  Numeric limits
// ============================================================
constexpr float   MOISTURE_MIN_PCT   = 0.0f;
constexpr float   MOISTURE_MAX_PCT   = 100.0f;
constexpr uint8_t ADC_RESOLUTION     = 10;          ///< bits
constexpr int     ADC_MAX_RAW        = 1023;
constexpr uint8_t TOPIC_MAX_LEN      = 128;
constexpr uint8_t PAYLOAD_MAX_LEN    = 256;
constexpr uint8_t DEVICE_ID_LEN      = 12;          ///< MAC-derived hex string

// ============================================================
//  Timing helpers
// ============================================================
constexpr uint32_t MS_PER_SECOND   = 1000UL;
constexpr uint32_t MS_PER_MINUTE   = 60UL * MS_PER_SECOND;
constexpr uint32_t MS_PER_HOUR     = 60UL * MS_PER_MINUTE;

// ============================================================
//  Error codes
// ============================================================
enum class ErrorCode : int8_t {
    OK                   =  0,
    ERR_WIFI_TIMEOUT     = -1,
    ERR_MQTT_CONNECT     = -2,
    ERR_SENSOR_READ      = -3,
    ERR_PUMP_TIMEOUT     = -4,
    ERR_STORAGE_CORRUPT  = -5,
    ERR_JSON_PARSE       = -6,
};
