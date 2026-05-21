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
                         const char *username,
                         const char *password,
                         uint16_t keepalive,
                         uint32_t reconnectIntervalMs)
    : client(wifiClient),
      host(host),
      port(port),
      clientIdPrefix(clientIdPrefix),
      username(username),
      password(password),
      keepalive(keepalive),
      reconnectIntervalMs(reconnectIntervalMs),
      lastReconnectMs(0) {
}

void MQTTService::begin() {
    client.setServer(host, port);
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

bool MQTTService::connected() const {
    return client.connected();
}

bool MQTTService::publish(const char *topic, const String &payload) {
    if (!connected()) {
        return false;
    }
    return client.publish(topic, payload.c_str(), true);
}

void MQTTService::reconnect() {
    if (WiFi.status() != WL_CONNECTED) {
        return;
    }

    String clientId = String(clientIdPrefix) + WiFi.macAddress();
    if (username && username[0] != '\0') {
        client.connect(clientId.c_str(), username, password);
    } else {
        client.connect(clientId.c_str());
    }
}
