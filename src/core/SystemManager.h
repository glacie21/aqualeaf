/**
 * @file        SystemManager.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Orkestrasi keseluruhan siklus operasi.
 */

#pragma once

#include <Arduino.h>
#include "Config.h"
#include "Constants.h"
#include "models/SensorData.h"
#include "models/DeviceState.h"
#include "models/WateringConfig.h"
#include "models/SensorCalibration.h"
#include "sensors/SoilMoistureSensor.h"
#include "actuators/PumpController.h"
#include "network/WiFiService.h"
#include "network/MQTTService.h"
#include "network/OtaService.h"
#include "dashboard/WebDashboard.h"
#include "storage/ConfigStorage.h"
#include "logic/WateringGuard.h"
#include "logic/HistoryBuffer.h"
#include "utils/Logger.h"
#include "core/Scheduler.h"

class SystemManager {
public:
    SystemManager();
    void begin();
    void update();
    void requestPump(bool on);
    /**
     * Threshold di-clamp ke 0–100 dan dijaga jaraknya minimal MOISTURE_THRESHOLD_MIN_GAP.
     * allowedHoursMask harus berisi minimal satu jam (bit 0–23).
     */
    void updateWateringConfig(int thresholdLow, int thresholdHigh, bool autoMode, uint32_t allowedHoursMask);
    /** Mengembalikan nullptr jika berhasil, atau pesan error. */
    const char *setCalibration(uint16_t rawDry, uint16_t rawWet);
    /** Pakai pembacaan raw terakhir sebagai titik kering (dry = true) atau basah. */
    const char *calibrateFromCurrentReading(bool dry);
    void resetCalibration();
    SensorCalibration getCalibration() const;
    void setWifiCredentials(const String &ssid, const String &password);
    bool isAccessPointActive() const;
    String getStatusJson() const;
    String getHistoryJson() const;
    const WateringConfig &getWateringConfig() const;
    static SystemManager *getInstance();

private:
    void initializeHardware();
    void initializeServices();
    void loadSettings();
    void saveSettings();
    void warnInsecureDefaults();
    void startOtaWhenOnline();
    void processSensorCycle();
    void runAutomaticWatering();
    void startAutomaticCycle(float moisture);
    void checkManualOverrideExpiry();
    void recordHistory();
    bool isAutomaticControlActive() const;
    uint32_t getOverrideRemainingMs() const;
    void publishTelemetry();
    void publishBlynk();
    void updatePumpLed();
    /** Jam lokal 0–23, atau -1 jika waktu belum tersinkron NTP. */
    static int getLocalHour();

    SoilMoistureSensor sensor;
    PumpController pump;
    WiFiService wifi;
    MQTTService mqtt;
    OtaService ota;
    WebDashboard dashboard;
    ConfigStorage storage;
    Logger logger;
    WateringGuard guard;
    HistoryBuffer<HISTORY_CAPACITY> history;

    DeviceState deviceState;
    WateringConfig wateringConfig;
    SensorData latestSensorData;
    uint32_t lastSensorReadMs;
    uint32_t lastPublishMs;
    uint32_t lastBlynkMs;
    uint32_t lastHistoryMs;
    uint32_t manualOverrideMs;
    bool blynkEnabled;
    bool weakWebPassword;
    bool sensorFault;
    bool wateredSinceLastSample;
};
