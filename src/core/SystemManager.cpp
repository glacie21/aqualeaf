/**
 * @file        SystemManager.cpp
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Implementasi orkestrator sistem.
 */

#include "SystemManager.h"
#include "Config.h"
#include "Pins.h"
#include "Secrets.h"
#include "logic/MoistureMath.h"
#include "logic/WateringRules.h"
#include <ArduinoJson.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>
#include <time.h>

static SystemManager *s_instance = nullptr;

// Mengubah makro numerik dari Config.h menjadi string literal pada waktu kompilasi.
#define AQUA_STR_INNER(x) #x
#define AQUA_STR(x) AQUA_STR_INNER(x)

/** Waktu epoch minimum yang dianggap hasil sinkron NTP (sebelumnya jam masih di 1970). */
static const time_t MIN_VALID_EPOCH = 1700000000;

// Password dianggap lemah jika masih nilai default dari repo publik atau < 8 karakter.
static bool isWeakSecret(const char *value, const char *knownDefault) {
    return strlen(value) < 8 || strcmp(value, knownDefault) == 0;
}

static StoredSettings defaultSettings() {
    StoredSettings defaults;
    defaults.watering.thresholdLow = MOISTURE_THRESHOLD_LOW;
    defaults.watering.thresholdHigh = MOISTURE_THRESHOLD_HIGH;
    defaults.watering.autoMode = true;
    defaults.watering.manualOverride = false;
    defaults.watering.allowedHoursMask = ALL_HOURS_MASK;
    defaults.calibration.rawDry = MOISTURE_RAW_DRY;
    defaults.calibration.rawWet = MOISTURE_RAW_WET;
    return defaults;
}

static bool isAcceptableCalibration(uint16_t rawDry, uint16_t rawWet) {
    return isValidCalibration(rawDry, rawWet, MOISTURE_CALIBRATION_MIN_SPAN,
                              MOISTURE_RAW_VALID_MIN, MOISTURE_RAW_VALID_MAX);
}

SystemManager::SystemManager()
    : sensor(PIN_MOISTURE_SENSOR,
             MOISTURE_RAW_DRY,
             MOISTURE_RAW_WET,
             MOISTURE_SAMPLES,
             MOISTURE_SAMPLE_DELAY_MS,
             MOISTURE_RAW_VALID_MIN,
             MOISTURE_RAW_VALID_MAX),
      pump(PIN_RELAY,
           RELAY_ON,
           RELAY_OFF,
           PUMP_MAX_ON_TIME_MS,
           PUMP_COOLDOWN_MS,
           PUMP_RELAY_DEBOUNCE_MS,
           PUMP_DAILY_LIMIT_MS),
      wifi(WIFI_CONNECT_TIMEOUT_MS,
           WIFI_RECONNECT_INTERVAL_MS),
      mqtt(MQTT_BROKER,
           MQTT_PORT,
           MQTT_CLIENT_ID_PREFIX,
           MQTT_TOPIC_ROOT,
           MQTT_TOPIC_STATUS,
           MQTT_USERNAME,
           MQTT_PASSWORD,
           MQTT_KEEPALIVE_S,
           MQTT_RECONNECT_INTERVAL_MS),
      dashboard(WEB_SERVER_PORT),
      logger(LOG_LEVEL),
      guard(PUMP_MIN_MOISTURE_RISE_PCT, PUMP_MAX_INEFFECTIVE_CYCLES),
      lastSensorReadMs(0),
      lastPublishMs(0),
      lastBlynkMs(0),
      lastHistoryMs(0),
      manualOverrideMs(0),
      blynkEnabled(false),
      weakWebPassword(false),
      sensorFault(false),
      wateredSinceLastSample(false) {
    deviceState.systemState = SystemState::BOOT;
    deviceState.networkStatus = NetworkStatus::DISCONNECTED;
    deviceState.mqttStatus = MQTTStatus::DISCONNECTED;
    deviceState.pumpState = PumpState::OFF;
    deviceState.operatingMode = OperatingMode::AUTO;
    wateringConfig = defaultSettings().watering;
    latestSensorData.moisturePercent = 0.0f;
    latestSensorData.rawValue = 0;
    latestSensorData.timestamp = 0;
}

void SystemManager::begin() {
    s_instance = this;
    logger.begin();
    initializeHardware();
    storage.begin();
    loadSettings();
    deviceState.operatingMode = wateringConfig.autoMode ? OperatingMode::AUTO : OperatingMode::MANUAL;
    initializeServices();
    logger.info("AquaLeaf SystemManager started");
    deviceState.systemState = SystemState::RUNNING;
}

void SystemManager::initializeHardware() {
    logger.info("Booting hardware modules");
    pinMode(PIN_LED_STATUS, OUTPUT);
    pinMode(PIN_LED_PUMP, OUTPUT);
    digitalWrite(PIN_LED_STATUS, LOW);
    digitalWrite(PIN_LED_PUMP, LOW);
    pump.begin();
    sensor.begin();
    digitalWrite(PIN_LED_STATUS, HIGH);
}

void SystemManager::loadSettings() {
    StoredSettings defaults = defaultSettings();
    StoredSettings loaded;
    switch (storage.load(defaults, loaded)) {
        case DecodeResult::OK:
            logger.info("Settings loaded from EEPROM");
            break;
        case DecodeResult::MIGRATED_LEGACY:
            logger.info("Settings migrated from legacy EEPROM format");
            break;
        case DecodeResult::EMPTY:
            logger.info("No saved settings, using defaults");
            break;
        case DecodeResult::CORRUPT:
            logger.warn("Saved settings corrupt (CRC/version mismatch), using defaults");
            break;
    }

    if (!isAcceptableCalibration(loaded.calibration.rawDry, loaded.calibration.rawWet)) {
        logger.warn("Saved calibration outside valid range, using defaults");
        loaded.calibration = defaults.calibration;
    }

    wateringConfig = loaded.watering;
    sensor.setCalibration(loaded.calibration);
}

void SystemManager::saveSettings() {
    StoredSettings settings;
    settings.watering = wateringConfig;
    settings.calibration = sensor.getCalibration();
    if (!storage.save(settings)) {
        logger.error("Failed to write settings to EEPROM");
    }
}

void SystemManager::initializeServices() {
    logger.info("Initializing network and services");
    wifi.begin();
    // Jam diambil dari NTP begitu WiFi tersambung; dipakai oleh jadwal penyiraman.
    configTime(TIME_ZONE, NTP_SERVER_1, NTP_SERVER_2);
    mqtt.begin();
    logger.info(String("MQTT base topic: ") + mqtt.getTopicBase());
    dashboard.setManager(this);
    dashboard.begin();
    warnInsecureDefaults();

    // Blynk hanya diaktifkan jika token sudah diisi; reconnect ditangani Blynk.run().
    blynkEnabled = strlen(BLYNK_AUTH_TOKEN) > 0 &&
                   strcmp(BLYNK_AUTH_TOKEN, "YOUR_BLYNK_AUTH_TOKEN_HERE") != 0;
    if (blynkEnabled) {
        Blynk.config(BLYNK_AUTH_TOKEN, BLYNK_SERVER, BLYNK_PORT);
    } else {
        logger.warn("Blynk token not set, Blynk disabled");
    }
}

void SystemManager::warnInsecureDefaults() {
    weakWebPassword = isWeakSecret(WEB_ADMIN_PASS, "aqualeaf");
    if (weakWebPassword) {
        logger.warn("SECURITY: WEB_ADMIN_PASS is the default or shorter than 8 chars - change it in Secrets.h");
    }
    if (isWeakSecret(WIFI_AP_PASSWORD, "aqualeaf123")) {
        logger.warn("SECURITY: WIFI_AP_PASSWORD is the default (public in the repo) or too short - change it in Config.h");
    }
    if (isWeakSecret(OTA_PASSWORD, "aqualeaf_ota_2025")) {
        logger.warn("SECURITY: OTA_PASSWORD is the default or too short - anyone on the network could flash firmware");
    }
    if (MQTT_USERNAME[0] == '\0') {
        logger.warn(String("SECURITY: MQTT has no credentials; telemetry on ") + MQTT_BROKER +
                    " is readable by anyone subscribed to " + MQTT_TOPIC_ROOT + "#");
    }
}

void SystemManager::startOtaWhenOnline() {
    if (ota.isStarted() || !wifi.isConnected()) {
        return;
    }
    ota.begin(DEVICE_HOSTNAME, OTA_PASSWORD, WEB_SERVER_PORT,
              [this]() {
                  // Selama transfer OTA loop utama berhenti, jadi pompa wajib mati dulu.
                  pump.deactivate();
                  updatePumpLed();
                  deviceState.systemState = SystemState::OTA;
                  logger.warn("OTA update started, pump disabled");
              },
              [this](bool success) {
                  if (success) {
                      logger.info("OTA update complete, rebooting");
                  } else {
                      deviceState.systemState = SystemState::RUNNING;
                      logger.error("OTA update failed");
                  }
              });
    logger.info(String("OTA and mDNS ready: http://") + DEVICE_HOSTNAME + ".local");
}

void SystemManager::update() {
    wifi.update();
    startOtaWhenOnline();
    ota.handle();
    mqtt.update();
    pump.update();
    dashboard.handleClient();

    if (wifi.isConnected()) {
        deviceState.networkStatus = NetworkStatus::CONNECTED;
        if (blynkEnabled) {
            Blynk.run();
        }
    } else {
        deviceState.networkStatus = wifi.getStatus();
    }

    deviceState.mqttStatus = mqtt.connected() ? MQTTStatus::CONNECTED : MQTTStatus::DISCONNECTED;
    deviceState.pumpState = pump.getState();
    if (pump.isOn()) {
        wateredSinceLastSample = true;
    }
    checkManualOverrideExpiry();

    if (Scheduler::isDue(lastSensorReadMs, TASK_SENSOR_INTERVAL_MS)) {
        processSensorCycle();
    }

    recordHistory();

    if (Scheduler::isDue(lastPublishMs, TASK_MQTT_PUB_INTERVAL_MS)) {
        publishTelemetry();
    }

    if (blynkEnabled && Scheduler::isDue(lastBlynkMs, TASK_BLYNK_INTERVAL_MS)) {
        publishBlynk();
    }
}

void SystemManager::processSensorCycle() {
    float moisture = sensor.read();
    latestSensorData.moisturePercent = moisture;
    latestSensorData.rawValue = sensor.getLastRaw();
    latestSensorData.timestamp = millis();

    if (logger.isEnabled(LOG_DEBUG)) {
        logger.debug(String("Soil moisture = ") + latestSensorData.moisturePercent + "%");
    }

    bool readingValid = sensor.isLastReadingValid();
    if (sensorFault == readingValid) {
        sensorFault = !readingValid;
        if (sensorFault) {
            logger.error(String("Sensor fault: raw value ") + latestSensorData.rawValue + " out of plausible range");
        } else {
            logger.info("Sensor reading back in range");
        }
    }

    if (!sensorFault) {
        guard.observe(moisture, wateringConfig.thresholdLow);
    }

    if (isAutomaticControlActive()) {
        runAutomaticWatering();
    }

    deviceState.pumpState = pump.getState();
    updatePumpLed();
}

void SystemManager::runAutomaticWatering() {
    if (sensorFault) {
        // Pembacaan tidak bisa dipercaya: jangan pernah menyiram berdasarkan data ini.
        if (pump.deactivate()) {
            logger.warn("Automatic watering stopped due to sensor fault");
        }
        return;
    }

    float moisture = latestSensorData.moisturePercent;
    if (moisture <= wateringConfig.thresholdLow) {
        // Jadwal hanya membatasi kapan siklus boleh dimulai; siklus yang sedang
        // berjalan tetap dibatasi oleh PUMP_MAX_ON_TIME_MS.
        if (pump.getState() == PumpState::OFF &&
            isHourAllowed(wateringConfig.allowedHoursMask, getLocalHour())) {
            startAutomaticCycle(moisture);
        }
    } else if (moisture >= wateringConfig.thresholdHigh) {
        if (pump.deactivate()) {
            logger.info("Automatic watering: pump OFF");
        }
    }
}

void SystemManager::startAutomaticCycle(float moisture) {
    bool wasLockedOut = guard.isLockedOut();
    if (!guard.canStartCycle(moisture)) {
        if (!wasLockedOut) {
            logger.error(String("Moisture did not rise after ") + PUMP_MAX_INEFFECTIVE_CYCLES +
                         " watering cycles; automatic watering locked until config is saved");
        }
        return;
    }

    if (pump.activate()) {
        guard.onCycleStarted(moisture);
        logger.info("Automatic watering: pump ON");
    }
}

bool SystemManager::isAutomaticControlActive() const {
    return deviceState.operatingMode == OperatingMode::AUTO &&
           wateringConfig.autoMode &&
           !wateringConfig.manualOverride;
}

void SystemManager::checkManualOverrideExpiry() {
    if (!wateringConfig.manualOverride || millis() - manualOverrideMs < MANUAL_OVERRIDE_TIMEOUT_MS) {
        return;
    }
    wateringConfig.manualOverride = false;
    deviceState.operatingMode = wateringConfig.autoMode ? OperatingMode::AUTO : OperatingMode::MANUAL;
    logger.info("Manual override expired, returning to configured mode");
}

uint32_t SystemManager::getOverrideRemainingMs() const {
    if (!wateringConfig.manualOverride) {
        return 0;
    }
    uint32_t elapsed = millis() - manualOverrideMs;
    return elapsed >= MANUAL_OVERRIDE_TIMEOUT_MS ? 0 : MANUAL_OVERRIDE_TIMEOUT_MS - elapsed;
}

void SystemManager::recordHistory() {
    if (latestSensorData.timestamp == 0) {
        return;  // Belum ada pembacaan sensor sama sekali.
    }
    // Sampel pertama langsung dicatat; berikutnya setiap HISTORY_SAMPLE_INTERVAL_MS.
    if (history.size() == 0) {
        lastHistoryMs = millis();
    } else if (!Scheduler::isDue(lastHistoryMs, HISTORY_SAMPLE_INTERVAL_MS)) {
        return;
    }

    HistorySample sample;
    sample.moisturePercent = latestSensorData.moisturePercent;
    sample.hasReading = !sensorFault;
    sample.watered = wateredSinceLastSample;
    history.push(sample);
    wateredSinceLastSample = pump.isOn();
}

int SystemManager::getLocalHour() {
    time_t now = time(nullptr);
    if (now < MIN_VALID_EPOCH) {
        return -1;
    }
    struct tm local;
    localtime_r(&now, &local);
    return local.tm_hour;
}

void SystemManager::updatePumpLed() {
    digitalWrite(PIN_LED_PUMP, pump.isOn() ? HIGH : LOW);
}

void SystemManager::publishTelemetry() {
    if (!mqtt.connected()) {
        return;
    }

    JsonDocument payload;
    payload["moisture_pct"] = latestSensorData.moisturePercent;
    payload["moisture_raw"] = latestSensorData.rawValue;
    payload["pump_state"] = static_cast<uint8_t>(deviceState.pumpState);
    payload["pump_cycles"] = pump.getCycleCount();
    payload["auto_mode"] = wateringConfig.autoMode;
    payload["manual_override"] = wateringConfig.manualOverride;
    payload["threshold_low"] = wateringConfig.thresholdLow;
    payload["threshold_high"] = wateringConfig.thresholdHigh;
    payload["sensor_fault"] = sensorFault;
    payload["watering_lockout"] = guard.isLockedOut();
    payload["daily_limit_reached"] = pump.isDailyLimitReached();
    payload["uptime_ms"] = millis();
    payload["wifi_connected"] = wifi.isConnected();
    payload["mqtt_connected"] = true;

    String json;
    serializeJson(payload, json);

    if (mqtt.publish(MQTT_TOPIC_SENSOR, json)) {
        logger.debug("Published telemetry to MQTT");
    } else {
        logger.warn(String("MQTT publish failed (payload ") + json.length() + " bytes)");
    }
}

void SystemManager::publishBlynk() {
    if (!Blynk.connected()) {
        return;
    }
    Blynk.virtualWrite(BLYNK_VPIN_MOISTURE, latestSensorData.moisturePercent);
    Blynk.virtualWrite(BLYNK_VPIN_PUMP_STATE, pump.isOn() ? 1 : 0);
    Blynk.virtualWrite(BLYNK_VPIN_THRESHOLD, wateringConfig.thresholdLow);
    Blynk.virtualWrite(BLYNK_VPIN_AUTO_MODE, wateringConfig.autoMode ? 1 : 0);
    Blynk.virtualWrite(BLYNK_VPIN_UPTIME, millis() / MS_PER_MINUTE);
    Blynk.virtualWrite(BLYNK_VPIN_RSSI, WiFi.RSSI());
    Blynk.virtualWrite(BLYNK_VPIN_PUMP_COUNT, pump.getCycleCount());
    logger.debug("Updated Blynk dashboard");
}

void SystemManager::requestPump(bool on) {
    // Override berakhir otomatis setelah MANUAL_OVERRIDE_TIMEOUT_MS (lihat checkManualOverrideExpiry).
    wateringConfig.manualOverride = true;
    manualOverrideMs = millis();
    deviceState.operatingMode = OperatingMode::MANUAL;

    if (on) {
        if (pump.activate()) {
            logger.info("Manual pump request: ON");
        } else if (pump.isDailyLimitReached()) {
            logger.warn("Manual pump request rejected (daily run-time limit reached)");
        } else {
            logger.warn("Manual pump request rejected (cooldown/fault lockout)");
        }
    } else {
        pump.deactivate();
        logger.info("Manual pump request: OFF");
    }

    deviceState.pumpState = pump.getState();
    updatePumpLed();
}

void SystemManager::updateWateringConfig(int thresholdLow, int thresholdHigh, bool autoMode, uint32_t allowedHoursMask) {
    ThresholdPair thresholds = normalizeThresholds(thresholdLow, thresholdHigh, MOISTURE_THRESHOLD_MIN_GAP);
    allowedHoursMask &= ALL_HOURS_MASK;

    wateringConfig.thresholdLow = thresholds.low;
    wateringConfig.thresholdHigh = thresholds.high;
    wateringConfig.autoMode = autoMode;
    if (allowedHoursMask != 0) {
        wateringConfig.allowedHoursMask = allowedHoursMask;
    }
    wateringConfig.manualOverride = false;
    // Menyimpan konfigurasi mengakhiri override manual dan mengembalikan mode sesuai autoMode.
    deviceState.operatingMode = autoMode ? OperatingMode::AUTO : OperatingMode::MANUAL;
    // Menyimpan konfigurasi juga menjadi cara pengguna me-reset lockout dry-run.
    if (guard.isLockedOut()) {
        logger.info("Watering lockout cleared by configuration save");
    }
    guard.reset();
    saveSettings();
    logger.info("Watering configuration updated");
}

const char *SystemManager::setCalibration(uint16_t rawDry, uint16_t rawWet) {
    if (!isAcceptableCalibration(rawDry, rawWet)) {
        return "Calibration rejected: both raw values must be between " AQUA_STR(MOISTURE_RAW_VALID_MIN)
               " and " AQUA_STR(MOISTURE_RAW_VALID_MAX) ", and dry must be at least "
               AQUA_STR(MOISTURE_CALIBRATION_MIN_SPAN) " higher than wet";
    }
    SensorCalibration calibration;
    calibration.rawDry = rawDry;
    calibration.rawWet = rawWet;
    sensor.setCalibration(calibration);
    // Skala persen berubah, jadi perbandingan kelembaban dry-run sebelumnya tidak berlaku lagi.
    guard.reset();
    saveSettings();
    logger.info(String("Sensor calibrated: dry=") + rawDry + " wet=" + rawWet);
    return nullptr;
}

const char *SystemManager::calibrateFromCurrentReading(bool dry) {
    if (latestSensorData.timestamp == 0) {
        return "No sensor reading yet";
    }
    SensorCalibration current = sensor.getCalibration();
    uint16_t raw = latestSensorData.rawValue;
    return dry ? setCalibration(raw, current.rawWet) : setCalibration(current.rawDry, raw);
}

void SystemManager::resetCalibration() {
    StoredSettings defaults = defaultSettings();
    setCalibration(defaults.calibration.rawDry, defaults.calibration.rawWet);
}

SensorCalibration SystemManager::getCalibration() const {
    return sensor.getCalibration();
}

void SystemManager::setWifiCredentials(const String &ssid, const String &password) {
    logger.info(String("Connecting to new WiFi network: ") + ssid);
    wifi.setCredentials(ssid, password);
}

bool SystemManager::isAccessPointActive() const {
    return wifi.isAccessPointActive();
}

String SystemManager::getStatusJson() const {
    JsonDocument status;
    status["moisture_pct"] = latestSensorData.moisturePercent;
    status["moisture_raw"] = latestSensorData.rawValue;
    status["pump_state"] = static_cast<uint8_t>(deviceState.pumpState);
    status["pump_cycles"] = pump.getCycleCount();
    status["operating_mode"] = static_cast<uint8_t>(deviceState.operatingMode);
    status["system_state"] = static_cast<uint8_t>(deviceState.systemState);
    status["network_status"] = static_cast<uint8_t>(deviceState.networkStatus);
    status["mqtt_status"] = static_cast<uint8_t>(deviceState.mqttStatus);
    status["auto_mode"] = wateringConfig.autoMode;
    status["manual_override"] = wateringConfig.manualOverride;
    status["threshold_low"] = wateringConfig.thresholdLow;
    status["threshold_high"] = wateringConfig.thresholdHigh;
    status["allowed_hours_mask"] = wateringConfig.allowedHoursMask;
    status["sensor_fault"] = sensorFault;
    status["watering_lockout"] = guard.isLockedOut();
    status["daily_limit_reached"] = pump.isDailyLimitReached();
    status["pump_on_today_s"] = pump.getOnTimeTodayMs() / MS_PER_SECOND;
    status["override_remaining_s"] = getOverrideRemainingMs() / MS_PER_SECOND;
    status["weak_web_password"] = weakWebPassword;

    SensorCalibration calibration = sensor.getCalibration();
    status["raw_dry"] = calibration.rawDry;
    status["raw_wet"] = calibration.rawWet;

    int hour = getLocalHour();
    status["time_synced"] = hour >= 0;
    if (hour >= 0) {
        time_t now = time(nullptr);
        struct tm local;
        localtime_r(&now, &local);
        char clock[6];
        snprintf(clock, sizeof(clock), "%02d:%02d", local.tm_hour, local.tm_min);
        status["local_time"] = clock;
    }
    status["schedule_allows_now"] = isHourAllowed(wateringConfig.allowedHoursMask, hour);

    status["wifi_ssid"] = wifi.getConfiguredSsid();
    status["wifi_ip"] = wifi.isConnected() ? WiFi.localIP().toString() : String();
    status["wifi_rssi"] = wifi.isConnected() ? WiFi.RSSI() : 0;
    status["ap_active"] = wifi.isAccessPointActive();
    status["hostname"] = String(DEVICE_HOSTNAME) + ".local";
    status["firmware"] = AQUA_VERSION;
    status["uptime_ms"] = millis();
    String json;
    serializeJson(status, json);
    return json;
}

String SystemManager::getHistoryJson() const {
    // Dibangun manual (bukan JsonDocument) agar tidak perlu ~5 KB heap untuk 288 elemen.
    uint16_t count = history.size();
    String json;
    json.reserve(80 + count * 7);
    json += "{\"interval_s\":";
    json += HISTORY_SAMPLE_INTERVAL_MS / MS_PER_SECOND;
    json += ",\"age_s\":";
    json += count > 0 ? (millis() - lastHistoryMs) / MS_PER_SECOND : 0;
    json += ",\"moisture\":[";
    for (uint16_t i = 0; i < count; ++i) {
        HistorySample sample = history.at(i);
        if (i > 0) {
            json += ',';
        }
        if (sample.hasReading) {
            json += String(sample.moisturePercent, 1);
        } else {
            json += "null";
        }
    }
    json += "],\"watered\":\"";
    for (uint16_t i = 0; i < count; ++i) {
        json += history.at(i).watered ? '1' : '0';
    }
    json += "\"}";
    return json;
}

const WateringConfig &SystemManager::getWateringConfig() const {
    return wateringConfig;
}

SystemManager *SystemManager::getInstance() {
    return s_instance;
}

BLYNK_WRITE(BLYNK_VPIN_PUMP_MANUAL) {
    if (auto manager = SystemManager::getInstance()) {
        manager->requestPump(param.asInt() > 0);
    }
}

BLYNK_WRITE(BLYNK_VPIN_THRESHOLD) {
    if (auto manager = SystemManager::getInstance()) {
        auto config = manager->getWateringConfig();
        // Slider hanya mengatur low; high dipertahankan dengan jarak minimum.
        int requestedLow = constrain(param.asInt(), 0, config.thresholdHigh - MOISTURE_THRESHOLD_MIN_GAP);
        manager->updateWateringConfig(requestedLow, config.thresholdHigh, config.autoMode, config.allowedHoursMask);
    }
}

BLYNK_WRITE(BLYNK_VPIN_AUTO_MODE) {
    if (auto manager = SystemManager::getInstance()) {
        auto config = manager->getWateringConfig();
        manager->updateWateringConfig(config.thresholdLow, config.thresholdHigh, param.asInt() == 1,
                                      config.allowedHoursMask);
    }
}
