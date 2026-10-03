/**
 * @file        Config.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Central runtime configuration constants
 * @author      AquaLeaf Team
 * @version     1.0.0
 * @date        2025
 *
 * Edit these values to tune the system behaviour without touching
 * any other source file.  Hardware pin assignments live in Pins.h;
 * compile-time limits / magic numbers live in Constants.h.
 */

#pragma once

// ============================================================
//  WiFi & Networking
// ============================================================
/** SSID displayed when the captive-portal AP is active */
#define WIFI_AP_SSID            "AquaLeaf-Setup"
/** Password for the captive-portal AP (min 8 chars, or "" for open) */
#define WIFI_AP_PASSWORD        "aqualeaf123"
/** Timeout (ms) to wait for a WiFi connection before falling back to AP */
#define WIFI_CONNECT_TIMEOUT_MS  20000UL
/** Interval (ms) between WiFi reconnect attempts */
#define WIFI_RECONNECT_INTERVAL_MS 30000UL

// ============================================================
//  MQTT Broker
// ============================================================
/** Broker hostname or IP */
#define MQTT_BROKER             "broker.hivemq.com"
/** Broker port (1883 = plain, 8883 = TLS) */
#define MQTT_PORT               1883
/** Unique client ID prefix — MAC suffix is appended at runtime */
#define MQTT_CLIENT_ID_PREFIX   "aqualeaf-"
/** QoS level for all published messages */
#define MQTT_QOS                1
/** Keep-alive interval (s) */
#define MQTT_KEEPALIVE_S        60
/** Interval (ms) between reconnect retries */
#define MQTT_RECONNECT_INTERVAL_MS 5000UL

/** Topic root — device-unique suffix appended at runtime */
#define MQTT_TOPIC_ROOT         "aqualeaf/"
#define MQTT_TOPIC_SENSOR       MQTT_TOPIC_ROOT "sensors"
#define MQTT_TOPIC_STATUS       MQTT_TOPIC_ROOT "status"
#define MQTT_TOPIC_CMD          MQTT_TOPIC_ROOT "cmd"
#define MQTT_TOPIC_CONFIG       MQTT_TOPIC_ROOT "config"
#define MQTT_TOPIC_LWT          MQTT_TOPIC_ROOT "lwt"

// ============================================================
//  Blynk
// ============================================================
/** Blynk cloud server */
#define BLYNK_SERVER            "blynk.cloud"
#define BLYNK_PORT              80

/** Virtual pin mapping */
#define BLYNK_VPIN_MOISTURE     V0   ///< Soil moisture (%)
#define BLYNK_VPIN_PUMP_STATE   V1   ///< Pump ON/OFF indicator
#define BLYNK_VPIN_PUMP_MANUAL  V2   ///< Manual pump button
#define BLYNK_VPIN_THRESHOLD    V3   ///< Moisture threshold slider
#define BLYNK_VPIN_AUTO_MODE    V4   ///< Auto / manual toggle
#define BLYNK_VPIN_UPTIME       V5   ///< Device uptime (min)
#define BLYNK_VPIN_RSSI         V6   ///< WiFi signal strength
#define BLYNK_VPIN_PUMP_COUNT   V7   ///< Watering cycle counter
#define BLYNK_VPIN_TERMINAL     V10  ///< Remote serial terminal

// ============================================================
//  Soil Moisture Sensor
// ============================================================
/** Raw ADC value at 0 % moisture (sensor fully dry / in air) */
#define MOISTURE_RAW_DRY        820
/** Raw ADC value at 100 % moisture (sensor fully submerged) */
#define MOISTURE_RAW_WET        380
/** Percentage below which irrigation is triggered */
#define MOISTURE_THRESHOLD_LOW   30
/** Percentage above which irrigation stops */
#define MOISTURE_THRESHOLD_HIGH  60
/** Number of ADC samples averaged per reading */
#define MOISTURE_SAMPLES         10
/** Delay (ms) between individual ADC samples */
#define MOISTURE_SAMPLE_DELAY_MS 10

// ============================================================
//  Pump / Relay
// ============================================================
/** Maximum continuous pump run time (ms) — safety cut-off */
#define PUMP_MAX_ON_TIME_MS     30000UL   // 30 s
/** Minimum off time (cooldown) between watering cycles */
#define PUMP_COOLDOWN_MS        300000UL  // 5 min
/** Debounce delay (ms) after toggling relay */
#define PUMP_RELAY_DEBOUNCE_MS  200UL

// ============================================================
//  Scheduler / Timing
// ============================================================
/** How often the main sensor-read task runs */
#define TASK_SENSOR_INTERVAL_MS  2000UL
/** How often status/data is published to MQTT */
#define TASK_MQTT_PUB_INTERVAL_MS 10000UL
/** How often Blynk virtual pins are updated */
#define TASK_BLYNK_INTERVAL_MS   3000UL
/** How often the watchdog / health-check task runs */
#define TASK_WATCHDOG_INTERVAL_MS 60000UL
/** How often the Web dashboard heartbeat updates */
#define TASK_WEB_INTERVAL_MS      1000UL

// ============================================================
//  Web Dashboard
// ============================================================
/** HTTP port for the embedded web server */
#define WEB_SERVER_PORT          80
/** Maximum number of simultaneous HTTP clients */
#define WEB_MAX_CLIENTS          4

// ============================================================
//  Storage (EEPROM / SPIFFS)
// ============================================================
/** EEPROM base address for persisted configuration */
#define STORAGE_EEPROM_ADDR      0
/** EEPROM size reserved for AquaLeaf config */
#define STORAGE_EEPROM_SIZE      512
/** Magic byte to validate EEPROM content */
#define STORAGE_MAGIC_BYTE       0xAF

// ============================================================
//  Serial Logger
// ============================================================
/** Baud rate for the USB-serial monitor */
#define SERIAL_BAUD_RATE         115200
/** Minimum log level printed to serial (DEBUG=0 … FATAL=4) */
#define LOG_LEVEL                0   // DEBUG
