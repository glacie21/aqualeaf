/**
 * @file        SystemManager.cpp
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Implementasi orkestrator sistem.
 */

#include "SystemManager.h"
#include "Config.h"
#include "Pins.h"
#include "Secrets.h"
#include <ArduinoJson.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>

static SystemManager *s_instance = nullptr;

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
           MQTT_USERNAME,
           MQTT_PASSWORD,
           MQTT_KEEPALIVE_S,
           MQTT_RECONNECT_INTERVAL_MS),
      dashboard(WEB_SERVER_PORT),
      logger(LOG_LEVEL),
      lastSensorReadMs(0),
      lastPublishMs(0),
      lastDashboardMs(0),
      manualOverrideMs(0),
      blynkEnabled(false),
      sensorFault(false),
      wateringLockout(false),
      ineffectiveCycles(0),
      cycleStartMoisture(-1.0f) {
    deviceState.systemState = SystemState::BOOT;
    deviceState.networkStatus = NetworkStatus::DISCONNECTED;
    deviceState.mqttStatus = MQTTStatus::DISCONNECTED;
    deviceState.pumpState = PumpState::OFF;
    deviceState.operatingMode = OperatingMode::AUTO;
    wateringConfig.thresholdLow = MOISTURE_THRESHOLD_LOW;
    wateringConfig.thresholdHigh = MOISTURE_THRESHOLD_HIGH;
    wateringConfig.autoMode = true;
    wateringConfig.manualOverride = false;
    latestSensorData.moisturePercent = 0.0f;
    latestSensorData.rawValue = 0;
    latestSensorData.timestamp = 0;
}

void SystemManager::begin() {
    s_instance = this;
    logger.begin();
    initializeHardware();
    storage.begin();
    wateringConfig = storage.loadWateringConfig();
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

void SystemManager::initializeServices() {
    logger.info("Initializing network and services");
    wifi.begin();
    mqtt.begin();
    dashboard.setManager(this);
    dashboard.begin();

    // Blynk hanya diaktifkan jika token sudah diisi; reconnect ditangani Blynk.run().
    blynkEnabled = strlen(BLYNK_AUTH_TOKEN) > 0 &&
                   strcmp(BLYNK_AUTH_TOKEN, "YOUR_BLYNK_AUTH_TOKEN_HERE") != 0;
    if (blynkEnabled) {
        Blynk.config(BLYNK_AUTH_TOKEN, BLYNK_SERVER, BLYNK_PORT);
    } else {
        logger.warn("Blynk token not set, Blynk disabled");
    }
}

void SystemManager::update() {
    wifi.update();
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
    checkManualOverrideExpiry();

    if (Scheduler::isDue(lastSensorReadMs, TASK_SENSOR_INTERVAL_MS)) {
        processSensorCycle();
    }

    if (Scheduler::isDue(lastPublishMs, TASK_MQTT_PUB_INTERVAL_MS)) {
        publishTelemetry();
    }

    if (Scheduler::isDue(lastDashboardMs, TASK_BLYNK_INTERVAL_MS)) {
        updateDashboard();
    }
}

void SystemManager::processSensorCycle() {
    float moisture = sensor.read();
    latestSensorData.moisturePercent = moisture;
    latestSensorData.rawValue = sensor.getLastRaw();
    latestSensorData.timestamp = millis();

    logger.debug(String("Soil moisture = ") + latestSensorData.moisturePercent + "%");

    bool readingValid = sensor.isLastReadingValid();
    if (sensorFault == readingValid) {
        sensorFault = !readingValid;
        if (sensorFault) {
            logger.error(String("Sensor fault: raw value ") + latestSensorData.rawValue + " out of plausible range");
        } else {
            logger.info("Sensor reading back in range");
        }
    }

    if (isAutomaticControlActive()) {
        runAutomaticWatering();
    }

    deviceState.pumpState = pump.getState();
    digitalWrite(PIN_LED_PUMP, pump.isOn() ? HIGH : LOW);
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
    if (moisture > wateringConfig.thresholdLow) {
        // Tanah sudah keluar dari zona kering, jadi penyiraman sebelumnya efektif.
        ineffectiveCycles = 0;
        cycleStartMoisture = -1.0f;
    }

    if (moisture <= wateringConfig.thresholdLow) {
        if (pump.getState() == PumpState::OFF) {
            startAutomaticCycle(moisture);
        }
    } else if (moisture >= wateringConfig.thresholdHigh) {
        if (pump.deactivate()) {
            logger.info("Automatic watering: pump OFF");
        }
    }
}

void SystemManager::startAutomaticCycle(float moisture) {
    if (wateringLockout) {
        return;
    }

    // Siklus sebelumnya dianggap tidak efektif jika kelembaban hampir tidak naik
    // (tangki kosong, selang lepas, atau sensor macet di nilai kering).
    bool previousIneffective = cycleStartMoisture >= 0.0f &&
                               moisture - cycleStartMoisture < PUMP_MIN_MOISTURE_RISE_PCT;
    uint8_t nextIneffective = previousIneffective ? ineffectiveCycles + 1 : 0;

    if (nextIneffective >= PUMP_MAX_INEFFECTIVE_CYCLES) {
        wateringLockout = true;
        logger.error(String("Moisture did not rise after ") + nextIneffective +
                     " watering cycles; automatic watering locked until config is saved");
        return;
    }

    // Counter hanya dimajukan jika siklus benar-benar berjalan, supaya penolakan
    // (debounce / batas harian) tidak terhitung berulang setiap pembacaan sensor.
    if (pump.activate()) {
        ineffectiveCycles = nextIneffective;
        cycleStartMoisture = moisture;
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

void SystemManager::publishTelemetry() {
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
    payload["watering_lockout"] = wateringLockout;
    payload["daily_limit_reached"] = pump.isDailyLimitReached();
    payload["uptime_ms"] = millis();
    payload["wifi_connected"] = wifi.isConnected();
    payload["mqtt_connected"] = mqtt.connected();

    String json;
    serializeJson(payload, json);

    if (mqtt.connected()) {
        mqtt.publish(MQTT_TOPIC_SENSOR, json);
        logger.debug("Published telemetry to MQTT");
    }

    if (Blynk.connected()) {
        Blynk.virtualWrite(BLYNK_VPIN_MOISTURE, latestSensorData.moisturePercent);
        Blynk.virtualWrite(BLYNK_VPIN_PUMP_STATE, pump.isOn() ? 1 : 0);
        Blynk.virtualWrite(BLYNK_VPIN_THRESHOLD, wateringConfig.thresholdLow);
        Blynk.virtualWrite(BLYNK_VPIN_AUTO_MODE, wateringConfig.autoMode ? 1 : 0);
        Blynk.virtualWrite(BLYNK_VPIN_UPTIME, millis() / 60000);
        Blynk.virtualWrite(BLYNK_VPIN_RSSI, WiFi.RSSI());
        Blynk.virtualWrite(BLYNK_VPIN_PUMP_COUNT, pump.getCycleCount());
        logger.debug("Updated Blynk dashboard");
    }
}

void SystemManager::updateDashboard() {
    // Dashboard polling interval maintained by scheduler.
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
    digitalWrite(PIN_LED_PUMP, pump.isOn() ? HIGH : LOW);
}

void SystemManager::updateWateringConfig(int thresholdLow, int thresholdHigh, bool autoMode) {
    int low = constrain(thresholdLow, 0, 100);
    int high = constrain(thresholdHigh, 0, 100);

    if (low > high) {
        int temp = low;
        low = high;
        high = temp;
    }

    // Jaga jarak minimum; geser low ke bawah jika high sudah mentok di 100.
    if (high - low < MOISTURE_THRESHOLD_MIN_GAP) {
        high = min(100, low + MOISTURE_THRESHOLD_MIN_GAP);
        low = high - MOISTURE_THRESHOLD_MIN_GAP;
    }

    wateringConfig.thresholdLow = static_cast<uint8_t>(low);
    wateringConfig.thresholdHigh = static_cast<uint8_t>(high);
    wateringConfig.autoMode = autoMode;
    wateringConfig.manualOverride = false;
    // Menyimpan konfigurasi mengakhiri override manual dan mengembalikan mode sesuai autoMode.
    deviceState.operatingMode = autoMode ? OperatingMode::AUTO : OperatingMode::MANUAL;
    // Menyimpan konfigurasi juga menjadi cara pengguna me-reset lockout dry-run.
    if (wateringLockout) {
        logger.info("Watering lockout cleared by configuration save");
    }
    wateringLockout = false;
    ineffectiveCycles = 0;
    cycleStartMoisture = -1.0f;
    storage.saveWateringConfig(wateringConfig);
    logger.info("Watering configuration updated");
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
    status["sensor_fault"] = sensorFault;
    status["watering_lockout"] = wateringLockout;
    status["daily_limit_reached"] = pump.isDailyLimitReached();
    status["pump_on_today_s"] = pump.getOnTimeTodayMs() / MS_PER_SECOND;
    status["override_remaining_s"] = getOverrideRemainingMs() / MS_PER_SECOND;
    status["uptime_ms"] = millis();
    String json;
    serializeJson(status, json);
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
        manager->updateWateringConfig(requestedLow, config.thresholdHigh, config.autoMode);
    }
}

BLYNK_WRITE(BLYNK_VPIN_AUTO_MODE) {
    if (auto manager = SystemManager::getInstance()) {
        auto config = manager->getWateringConfig();
        manager->updateWateringConfig(config.thresholdLow, config.thresholdHigh, param.asInt() == 1);
    }
}
