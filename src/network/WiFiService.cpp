/**
 * @file        WiFiService.cpp
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Implementasi koneksi WiFi non-blocking dengan retry otomatis.
 */

#include "WiFiService.h"
#include "Config.h"
#include "Secrets.h"
#include <ESP8266WiFi.h>

WiFiService::WiFiService(uint32_t connectTimeoutMs, uint32_t reconnectIntervalMs)
    : connectTimeoutMs(connectTimeoutMs),
      reconnectIntervalMs(reconnectIntervalMs),
      lastAttemptMs(0),
      status(NetworkStatus::DISCONNECTED),
      apActive(false) {
}

void WiFiService::begin() {
    WiFi.mode(WIFI_STA);
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
    connect();
}

void WiFiService::update() {
    uint32_t now = millis();

    if (WiFi.status() == WL_CONNECTED) {
        if (status != NetworkStatus::CONNECTED) {
            status = NetworkStatus::CONNECTED;
            stopAccessPoint();
        }
        return;
    }

    if (status == NetworkStatus::CONNECTED) {
        // Koneksi baru saja putus; beri waktu auto-reconnect bawaan SDK.
        status = NetworkStatus::DISCONNECTED;
        lastAttemptMs = now;
        return;
    }

    if (status == NetworkStatus::CONNECTING) {
        if (now - lastAttemptMs >= connectTimeoutMs) {
            startAccessPoint();
        }
        return;
    }

    if (now - lastAttemptMs >= reconnectIntervalMs) {
        connect();
    }
}

void WiFiService::connect() {
    status = NetworkStatus::CONNECTING;
    lastAttemptMs = millis();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void WiFiService::startAccessPoint() {
    if (!apActive) {
        WiFi.mode(WIFI_AP_STA);
        WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);
        apActive = true;
    }
    status = NetworkStatus::AP_MODE;
    lastAttemptMs = millis();
}

void WiFiService::stopAccessPoint() {
    if (apActive) {
        WiFi.softAPdisconnect(true);
        WiFi.mode(WIFI_STA);
        apActive = false;
    }
}

bool WiFiService::isConnected() const {
    return status == NetworkStatus::CONNECTED && WiFi.status() == WL_CONNECTED;
}

NetworkStatus WiFiService::getStatus() const {
    return status;
}
