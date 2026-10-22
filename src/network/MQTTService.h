/**
 * @file        MQTTService.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Abstraksi koneksi MQTT dan publikasi telemetry.
 */

#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "Constants.h"

class MQTTService {
public:
    MQTTService(const char *host,
                uint16_t port,
                const char *clientIdPrefix,
                const char *topicRoot,
                const char *username,
                const char *password,
                uint16_t keepalive,
                uint32_t reconnectIntervalMs);
    void begin();
    void update();
    bool connected();
    /** Publish ke <topicRoot><deviceId>/<topicSuffix>. */
    bool publish(const char *topicSuffix, const String &payload);
    /** Base topic unik perangkat, mis. "aqualeaf/a4cf12b3c4d5/". */
    const String &getTopicBase() const;

private:
    void reconnect();
    WiFiClient wifiClient;
    PubSubClient client;
    const char *host;
    uint16_t port;
    const char *clientIdPrefix;
    const char *topicRoot;
    String deviceId;
    String topicBase;
    const char *username;
    const char *password;
    uint16_t keepalive;
    uint32_t reconnectIntervalMs;
    uint32_t lastReconnectMs;
};
