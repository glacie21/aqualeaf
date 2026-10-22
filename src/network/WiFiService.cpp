/**
 * @file        WiFiService.cpp
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Implementasi koneksi WiFi non-blocking dengan retry otomatis.
 *
 * Kredensial disimpan oleh SDK ESP8266 di flash (WiFi.persistent). Nilai di
 * Secrets.h hanya dipakai sebagai bekal awal jika belum ada yang tersimpan;
 * setelah itu kredensial diganti lewat dashboard (POST /api/wifi).
 */

#include "WiFiService.h"
#include "Config.h"
#include "Secrets.h"
#include <ESP8266WiFi.h>

static const uint8_t DNS_PORT = 53;

WiFiService::WiFiService(uint32_t connectTimeoutMs, uint32_t reconnectIntervalMs)
    : connectTimeoutMs(connectTimeoutMs),
      reconnectIntervalMs(reconnectIntervalMs),
      lastAttemptMs(0),
      status(NetworkStatus::DISCONNECTED),
      apActive(false) {
}

void WiFiService::begin() {
    WiFi.persistent(true);
    WiFi.mode(WIFI_STA);
    WiFi.hostname(DEVICE_HOSTNAME);
    WiFi.setAutoReconnect(true);

    bool secretsConfigured = strlen(WIFI_SSID) > 0 && strcmp(WIFI_SSID, "YOUR_WIFI_SSID") != 0;
    if (!hasStoredCredentials() && secretsConfigured) {
        setCredentials(WIFI_SSID, WIFI_PASSWORD);
        return;
    }

    if (hasStoredCredentials()) {
        connect();
    } else {
        // Belum ada jaringan sama sekali: langsung buka AP setup.
        startAccessPoint();
    }
}

void WiFiService::update() {
    uint32_t now = millis();

    if (apActive) {
        dnsServer.processNextRequest();
    }

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

    if (hasStoredCredentials() && now - lastAttemptMs >= reconnectIntervalMs) {
        connect();
    }
}

void WiFiService::setCredentials(const String &ssid, const String &password) {
    status = NetworkStatus::CONNECTING;
    lastAttemptMs = millis();
    // Dengan WiFi.persistent(true), SDK menyimpan kredensial ini ke flash.
    WiFi.begin(ssid.c_str(), password.c_str());
}

String WiFiService::getConfiguredSsid() const {
    return WiFi.SSID();
}

bool WiFiService::hasStoredCredentials() const {
    return WiFi.SSID().length() > 0;
}

void WiFiService::connect() {
    status = NetworkStatus::CONNECTING;
    lastAttemptMs = millis();
    WiFi.begin();
}

void WiFiService::startAccessPoint() {
    if (!apActive) {
        WiFi.mode(WIFI_AP_STA);
        WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);
        // Captive portal: semua nama domain diarahkan ke IP AP, sehingga ponsel
        // menampilkan notifikasi "masuk ke jaringan" yang membuka dashboard.
        dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
        apActive = true;
    }
    status = NetworkStatus::AP_MODE;
    lastAttemptMs = millis();
}

void WiFiService::stopAccessPoint() {
    if (apActive) {
        dnsServer.stop();
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

bool WiFiService::isAccessPointActive() const {
    return apActive;
}
