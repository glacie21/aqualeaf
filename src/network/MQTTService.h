/**
 * @file        MQTTService.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Abstraksi koneksi MQTT dan publikasi telemetry.
 */

#pragma once

#include <Arduino.h>
#include <PubSubClient.h>
#include "Constants.h"

class MQTTService {
public:
    MQTTService(const char *host,
                uint16_t port,
                const char *clientIdPrefix,
                const char *username,
                const char *password,
                uint16_t keepalive,
                uint32_t reconnectIntervalMs);
    void begin();
    void update();
    bool connected() const;
    bool publish(const char *topic, const String &payload);

private:
    void reconnect();
    WiFiClient wifiClient;
    PubSubClient client;
    const char *host;
    uint16_t port;
    const char *clientIdPrefix;
    const char *username;
    const char *password;
    uint16_t keepalive;
    uint32_t reconnectIntervalMs;
    uint32_t lastReconnectMs;
};
