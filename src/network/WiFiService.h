/**
 * @file        WiFiService.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Abstraksi koneksi WiFi, manajemen reconnect, dan AP setup
 *              (captive portal) untuk mengisi kredensial dari dashboard.
 */

#pragma once

#include <Arduino.h>
#include <DNSServer.h>
#include "Constants.h"

class WiFiService {
public:
    WiFiService(uint32_t connectTimeoutMs, uint32_t reconnectIntervalMs);
    void begin();
    void update();
    bool isConnected() const;
    NetworkStatus getStatus() const;
    bool isAccessPointActive() const;
    /** Simpan kredensial baru (persisten di flash SDK) lalu mulai menyambung. */
    void setCredentials(const String &ssid, const String &password);
    /** SSID yang tersimpan, kosong jika belum pernah dikonfigurasi. */
    String getConfiguredSsid() const;

private:
    bool hasStoredCredentials() const;
    void connect();
    void startAccessPoint();
    void stopAccessPoint();

    DNSServer dnsServer;
    uint32_t connectTimeoutMs;
    uint32_t reconnectIntervalMs;
    uint32_t lastAttemptMs;
    NetworkStatus status;
    bool apActive;
};
