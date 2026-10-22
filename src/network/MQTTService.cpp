/**
 * @file        MQTTService.cpp
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Implementasi koneksi MQTT.
 */

#include "MQTTService.h"
#include <ESP8266WiFi.h>
#include <Arduino.h>

MQTTService::MQTTService(const char *host,
                         uint16_t port,
                         const char *clientIdPrefix,
                         const char *topicRoot,
                         const char *statusTopicSuffix,
                         const char *username,
                         const char *password,
                         uint16_t keepalive,
                         uint32_t reconnectIntervalMs)
    : client(wifiClient),
      host(host),
      port(port),
      clientIdPrefix(clientIdPrefix),
      topicRoot(topicRoot),
      statusTopicSuffix(statusTopicSuffix),
      username(username),
      password(password),
      keepalive(keepalive),
      reconnectIntervalMs(reconnectIntervalMs),
      lastReconnectMs(0) {
}

void MQTTService::begin() {
    // ID perangkat dari MAC (tanpa ':'), dipakai untuk client ID dan topic agar
    // beberapa perangkat di broker yang sama tidak saling menimpa data.
    deviceId = WiFi.macAddress();
    deviceId.replace(":", "");
    deviceId.toLowerCase();
    topicBase = String(topicRoot) + deviceId + "/";

    client.setServer(host, port);
    client.setKeepAlive(keepalive);
    client.setBufferSize(PAYLOAD_MAX_LEN + TOPIC_MAX_LEN);
}

void MQTTService::update() {
    if (connected()) {
        client.loop();
        return;
    }

    uint32_t now = millis();
    if (now - lastReconnectMs < reconnectIntervalMs) {
        return;
    }
    lastReconnectMs = now;
    reconnect();
}

bool MQTTService::connected() {
    return client.connected();
}

bool MQTTService::publish(const char *topicSuffix, const String &payload) {
    if (!connected()) {
        return false;
    }
    String topic = topicBase + topicSuffix;
    return client.publish(topic.c_str(), payload.c_str(), true);
}

const String &MQTTService::getTopicBase() const {
    return topicBase;
}

void MQTTService::reconnect() {
    if (WiFi.status() != WL_CONNECTED) {
        return;
    }

    String clientId = String(clientIdPrefix) + deviceId;
    bool hasCredentials = username && username[0] != '\0';
    // Last Will: broker mempublikasikan "offline" (retained) jika koneksi putus tanpa disconnect.
    String statusTopic = topicBase + statusTopicSuffix;
    bool ok = client.connect(clientId.c_str(),
                             hasCredentials ? username : nullptr,
                             hasCredentials ? password : nullptr,
                             statusTopic.c_str(), 0, true, "offline");
    if (ok) {
        client.publish(statusTopic.c_str(), "online", true);
    }
}
