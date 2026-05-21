/**
 * @file        SystemManager.cpp
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Implementasi orkestrator sistem.
 */

#include "SystemManager.h"
#include "Config.h"
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
             MOISTURE_SAMPLE_DELAY_MS),
      pump(PIN_RELAY,
           RELAY_ON,
           RELAY_OFF,
           PUMP_MAX_ON_TIME_MS,
           PUMP_COOLDOWN_MS,
           PUMP_RELAY_DEBOUNCE_MS),
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
      lastDashboardMs(0) {
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
    logger.begin();
    initializeHardware();
    storage.begin();
    wateringConfig = storage.loadWateringConfig();
    initializeServices();
    logger.info("AquaLeaf SystemManager started");
    deviceState.systemState = SystemState::RUNNING;
    s_instance = this;
}

void SystemManager::initializeHardware() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(100);
    logger.info("Booting hardware modules");
    pinMode(PIN_LED_STATUS, OUTPUT);
    pinMode(PIN_LED_PUMP, OUTPUT);
    digitalWrite(PIN_LED_STATUS, LOW);
    digitalWrite(PIN_LED_PUMP, LOW);
    pinMode(PIN_RELAY, OUTPUT);
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
    if (wifi.isConnected()) {
        Blynk.config(BLYNK_AUTH_TOKEN, BLYNK_SERVER, BLYNK_PORT);
        Blynk.connect();
    }
}

void SystemManager::update() {
    wifi.update();
    mqtt.update();
    pump.update();
    dashboard.handleClient();

    if (wifi.isConnected()) {
        deviceState.networkStatus = NetworkStatus::CONNECTED;
        if (!Blynk.connected()) {
            Blynk.connect();
        } else {
            Blynk.run();
        }
    } else {
        deviceState.networkStatus = wifi.getStatus();
    }

    deviceState.mqttStatus = mqtt.connected() ? MQTTStatus::CONNECTED : MQTTStatus::DISCONNECTED;
    deviceState.pumpState = pump.getState();

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

    if (deviceState.operatingMode == OperatingMode::AUTO && wateringConfig.autoMode && !wateringConfig.manualOverride) {
        if (latestSensorData.moisturePercent <= wateringConfig.thresholdLow) {
            if (pump.activate()) {
                logger.info("Automatic watering: pump ON");
            }
        } else if (latestSensorData.moisturePercent >= wateringConfig.thresholdHigh) {
            if (pump.deactivate()) {
                logger.info("Automatic watering: pump OFF");
            }
        }
    }

    deviceState.pumpState = pump.getState();
    digitalWrite(PIN_LED_PUMP, pump.isOn() ? HIGH : LOW);
}

void SystemManager::publishTelemetry() {
    DynamicJsonDocument payload(512);
    payload["moisture_pct"] = latestSensorData.moisturePercent;
    payload["moisture_raw"] = latestSensorData.rawValue;
    payload["pump_state"] = static_cast<uint8_t>(deviceState.pumpState);
    payload["auto_mode"] = wateringConfig.autoMode;
    payload["manual_override"] = wateringConfig.manualOverride;
    payload["threshold_low"] = wateringConfig.thresholdLow;
    payload["threshold_high"] = wateringConfig.thresholdHigh;
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
        Blynk.virtualWrite(BLYNK_VPIN_AUTO_MODE, wateringConfig.autoMode ? 0 : 1);
        Blynk.virtualWrite(BLYNK_VPIN_UPTIME, millis() / 60000);
        Blynk.virtualWrite(BLYNK_VPIN_RSSI, WiFi.RSSI());
        logger.debug("Updated Blynk dashboard");
    }
}

void SystemManager::updateDashboard() {
    // Dashboard polling interval maintained by scheduler.
}

void SystemManager::requestPump(bool on) {
    wateringConfig.manualOverride = true;
    deviceState.operatingMode = OperatingMode::MANUAL;

    if (on) {
        pump.activate();
        logger.info("Manual pump request: ON");
    } else {
        pump.deactivate();
        logger.info("Manual pump request: OFF");
    }
}

void SystemManager::updateWateringConfig(uint8_t thresholdLow, uint8_t thresholdHigh, bool autoMode) {
    wateringConfig.thresholdLow = thresholdLow;
    wateringConfig.thresholdHigh = thresholdHigh;
    wateringConfig.autoMode = autoMode;
    wateringConfig.manualOverride = false;
    storage.saveWateringConfig(wateringConfig);
    logger.info("Watering configuration updated");
}

String SystemManager::getStatusJson() const {
    DynamicJsonDocument status(512);
    status["moisture_pct"] = latestSensorData.moisturePercent;
    status["moisture_raw"] = latestSensorData.rawValue;
    status["pump_state"] = static_cast<uint8_t>(deviceState.pumpState);
    status["system_state"] = static_cast<uint8_t>(deviceState.systemState);
    status["network_status"] = static_cast<uint8_t>(deviceState.networkStatus);
    status["mqtt_status"] = static_cast<uint8_t>(deviceState.mqttStatus);
    status["auto_mode"] = wateringConfig.autoMode;
    status["manual_override"] = wateringConfig.manualOverride;
    status["threshold_low"] = wateringConfig.thresholdLow;
    status["threshold_high"] = wateringConfig.thresholdHigh;
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
        manager->updateWateringConfig(param.asInt(), config.thresholdHigh, config.autoMode);
    }
}

BLYNK_WRITE(BLYNK_VPIN_AUTO_MODE) {
    if (auto manager = SystemManager::getInstance()) {
        auto config = manager->getWateringConfig();
        manager->updateWateringConfig(config.thresholdLow, config.thresholdHigh, param.asInt() == 0);
    }
}
