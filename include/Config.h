/**
 * @file        Config.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Konfigurasi runtime utama sistem
 * @author      AquaLeaf Team
 * @version     1.0.0
 * @date        2026-10-07
 *
 * Edit these values to tune the system behaviour without touching
 * any other source file.  Hardware pin assignments live in Pins.h;
 * compile-time limits / magic numbers live in Constants.h.
 */

#pragma once

// ============================================================
//  WiFi & Networking
// ============================================================
/**
 * Setup access point, started when no WiFi network can be joined.
 * Connect to it and open http://192.168.4.1 (any URL is redirected there)
 * to enter the WiFi credentials from the dashboard.
 */
#define WIFI_AP_SSID            "AquaLeaf-Setup"
/** Password for the setup AP (min 8 chars, or "" for open) */
#define WIFI_AP_PASSWORD        "aqualeaf123"
/** Timeout (ms) to wait for a WiFi connection before falling back to AP */
#define WIFI_CONNECT_TIMEOUT_MS  20000UL
/** Interval (ms) between WiFi reconnect attempts */
#define WIFI_RECONNECT_INTERVAL_MS 30000UL
/**
 * mDNS / OTA hostname — the dashboard is reachable at http://<name>.local.
 * Give every device on the same network a different name.
 */
#define DEVICE_HOSTNAME         "aqualeaf"

// ============================================================
//  Time (NTP) — used by the watering schedule
// ============================================================
/** POSIX TZ string. WIB = "WIB-7", WITA = "WITA-8", WIT = "WIT-9" */
#define TIME_ZONE               "WIB-7"
#define NTP_SERVER_1            "pool.ntp.org"
#define NTP_SERVER_2            "time.google.com"

// ============================================================
//  MQTT Broker
// ============================================================
/** Broker hostname or IP */
#define MQTT_BROKER             "broker.hivemq.com"
/** Broker port (1883 = plain, 8883 = TLS) */
#define MQTT_PORT               1883
/** Unique client ID prefix — MAC suffix is appended at runtime */
#define MQTT_CLIENT_ID_PREFIX   "aqualeaf-"
/** Keep-alive interval (s) */
#define MQTT_KEEPALIVE_S        60
/** Interval (ms) between reconnect retries */
#define MQTT_RECONNECT_INTERVAL_MS 5000UL

/**
 * Topics are built at runtime as <ROOT><device-id>/<suffix>, where device-id
 * is the lowercase MAC without colons, e.g. "aqualeaf/a4cf12b3c4d5/sensors".
 * The full base topic is printed to the serial log at boot.
 *
 * NOTE: a unique topic only keeps devices from mixing their data. On a public
 * broker anyone subscribed to "aqualeaf/#" can still read it — use a private
 * broker with MQTT_USERNAME / MQTT_PASSWORD for anything sensitive.
 */
#define MQTT_TOPIC_ROOT         "aqualeaf/"
/** Telemetry JSON (retained) */
#define MQTT_TOPIC_SENSOR       "sensors"
/** "online" / "offline" (retained; "offline" is the Last Will message) */
#define MQTT_TOPIC_STATUS       "status"

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

// ============================================================
//  Soil Moisture Sensor
// ============================================================
/**
 * Default calibration: raw ADC value at 0 % (sensor in air) and 100 %
 * (sensor in water). Can be re-calibrated from the dashboard; the
 * calibrated values are stored in EEPROM and take precedence.
 */
#define MOISTURE_RAW_DRY        820
#define MOISTURE_RAW_WET        380
/** Minimum distance between the dry and wet calibration points */
#define MOISTURE_CALIBRATION_MIN_SPAN 50
/** Percentage below which irrigation is triggered */
#define MOISTURE_THRESHOLD_LOW   30
/** Percentage above which irrigation stops */
#define MOISTURE_THRESHOLD_HIGH  60
/** Minimum gap (%) enforced between the low and high thresholds */
#define MOISTURE_THRESHOLD_MIN_GAP 5
/** Number of ADC samples averaged per reading */
#define MOISTURE_SAMPLES         10
/** Delay (ms) between individual ADC samples */
#define MOISTURE_SAMPLE_DELAY_MS 10
/**
 * Plausible raw ADC range. Readings outside it are treated as a sensor
 * fault (disconnected wire, shorted probe) and automatic watering is
 * suspended. Calibration points must also lie inside this range.
 */
#define MOISTURE_RAW_VALID_MIN   100
#define MOISTURE_RAW_VALID_MAX   1000

// ============================================================
//  Pump / Relay
// ============================================================
/** Maximum continuous pump run time (ms) — safety cut-off */
#define PUMP_MAX_ON_TIME_MS     30000UL   // 30 s
/** Minimum off time (cooldown) between watering cycles */
#define PUMP_COOLDOWN_MS        300000UL  // 5 min
/** Debounce delay (ms) after toggling relay */
#define PUMP_RELAY_DEBOUNCE_MS  200UL
/** Maximum total pump run time per 24 h window (ms), manual included */
#define PUMP_DAILY_LIMIT_MS     600000UL  // 10 min
/**
 * Dry-run / stuck-sensor protection: an automatic cycle counts as
 * ineffective when moisture rose less than PUMP_MIN_MOISTURE_RISE_PCT since
 * the previous cycle started. After PUMP_MAX_INEFFECTIVE_CYCLES in a row,
 * automatic watering is locked out until the configuration is saved again.
 */
#define PUMP_MIN_MOISTURE_RISE_PCT   2.0f
#define PUMP_MAX_INEFFECTIVE_CYCLES  4
/** A manual pump command suspends automatic mode for this long */
#define MANUAL_OVERRIDE_TIMEOUT_MS   1800000UL  // 30 min

// ============================================================
//  Scheduler / Timing
// ============================================================
/** How often the main sensor-read task runs */
#define TASK_SENSOR_INTERVAL_MS  2000UL
/** How often status/data is published to MQTT */
#define TASK_MQTT_PUB_INTERVAL_MS 10000UL
/** How often Blynk virtual pins are updated */
#define TASK_BLYNK_INTERVAL_MS   10000UL
/** How often a moisture sample is added to the history chart */
#define HISTORY_SAMPLE_INTERVAL_MS 300000UL  // 5 min
/** Number of history samples kept in RAM (288 × 5 min = 24 h) */
#define HISTORY_CAPACITY         288

// ============================================================
//  Web Dashboard
// ============================================================
/** HTTP port for the embedded web server */
#define WEB_SERVER_PORT          80

// ============================================================
//  Storage (EEPROM emulation in flash)
// ============================================================
/** EEPROM base address for persisted configuration */
#define STORAGE_EEPROM_ADDR      0
/** EEPROM size reserved for AquaLeaf config */
#define STORAGE_EEPROM_SIZE      64

// ============================================================
//  Serial Logger
// ============================================================
/** Baud rate for the USB-serial monitor */
#define SERIAL_BAUD_RATE         115200
/** Minimum log level printed to serial (DEBUG=0 … FATAL=4) */
#define LOG_LEVEL                1   // INFO
