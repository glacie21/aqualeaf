/**
 * @file        SystemManager.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Orkestrasi keseluruhan siklus operasi.
 */

#pragma once

#include <Arduino.h>
#include "Constants.h"
#include "models/SensorData.h"
#include "models/DeviceState.h"
#include "models/WateringConfig.h"
#include "sensors/SoilMoistureSensor.h"
#include "actuators/PumpController.h"
#include "network/WiFiService.h"
#include "network/MQTTService.h"
#include "dashboard/WebDashboard.h"
#include "storage/ConfigStorage.h"
#include "utils/Logger.h"
#include "utils/Helpers.h"
#include "core/Scheduler.h"

class SystemManager {
public:
    SystemManager();
    void begin();
    void update();
    void requestPump(bool on);
    /** Nilai di-clamp ke 0–100 dan dijaga jaraknya minimal MOISTURE_THRESHOLD_MIN_GAP. */
    void updateWateringConfig(int thresholdLow, int thresholdHigh, bool autoMode);
    String getStatusJson() const;
    const WateringConfig &getWateringConfig() const;
    static SystemManager *getInstance();

private:
    void initializeHardware();
    void initializeServices();
    void processSensorCycle();
    void runAutomaticWatering();
    void startAutomaticCycle(float moisture);
    void checkManualOverrideExpiry();
    bool isAutomaticControlActive() const;
    uint32_t getOverrideRemainingMs() const;
    void publishTelemetry();
    void updateDashboard();

    SoilMoistureSensor sensor;
    PumpController pump;
    WiFiService wifi;
    MQTTService mqtt;
    WebDashboard dashboard;
    ConfigStorage storage;
    Logger logger;

    DeviceState deviceState;
    WateringConfig wateringConfig;
    SensorData latestSensorData;
    uint32_t lastSensorReadMs;
    uint32_t lastPublishMs;
    uint32_t lastDashboardMs;
    uint32_t manualOverrideMs;
    bool blynkEnabled;
    bool sensorFault;
    bool wateringLockout;
    uint8_t ineffectiveCycles;
    float cycleStartMoisture;   ///< Kelembaban saat siklus otomatis terakhir dimulai; < 0 = belum ada
};
