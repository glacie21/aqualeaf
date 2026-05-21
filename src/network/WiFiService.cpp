/**
 * @file        WiFiService.cpp
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Implementasi koneksi WiFi dengan retry otomatis.
 */

#include "WiFiService.h"
#include "Config.h"
#include <ESP8266WiFi.h>

WiFiService::WiFiService(uint32_t connectTimeoutMs, uint32_t reconnectIntervalMs)
    : connectTimeoutMs(connectTimeoutMs),
      reconnectIntervalMs(reconnectIntervalMs),
      lastAttemptMs(0),
      status(NetworkStatus::DISCONNECTED) {
}

void WiFiService::begin() {
    WiFi.mode(WIFI_STA);
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
    connect();
}

void WiFiService::update() {
    if (isConnected()) {
        return;
    }
    uint32_t now = millis();
    if (now - lastAttemptMs >= reconnectIntervalMs) {
        connect();
    }
}

void WiFiService::connect() {
    status = NetworkStatus::CONNECTING;
    lastAttemptMs = millis();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < connectTimeoutMs) {
        delay(200);
    }

    if (WiFi.status() == WL_CONNECTED) {
        status = NetworkStatus::CONNECTED;
    } else {
        WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);
        status = NetworkStatus::AP_MODE;
    }
}

bool WiFiService::isConnected() const {
    return status == NetworkStatus::CONNECTED && WiFi.status() == WL_CONNECTED;
}

NetworkStatus WiFiService::getStatus() const {
    return status;
}
