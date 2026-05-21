/**
 * @file        WiFiService.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Abstraksi koneksi WiFi dan manajemen reconnect.
 */

#pragma once

#include <Arduino.h>
#include "Constants.h"

class WiFiService {
public:
    WiFiService(uint32_t connectTimeoutMs, uint32_t reconnectIntervalMs);
    void begin();
    void update();
    bool isConnected() const;
    NetworkStatus getStatus() const;

private:
    void connect();
    uint32_t connectTimeoutMs;
    uint32_t reconnectIntervalMs;
    uint32_t lastAttemptMs;
    NetworkStatus status;
};
