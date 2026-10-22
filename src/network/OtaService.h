/**
 * @file        OtaService.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Update firmware over-the-air (ArduinoOTA) dan mDNS
 *              (<hostname>.local) — keduanya memakai responder mDNS yang sama.
 */

#pragma once

#include <Arduino.h>
#include <functional>

class OtaService {
public:
    using StartCallback = std::function<void()>;
    using EndCallback = std::function<void(bool success)>;

    /** Panggil setelah WiFi STA tersambung; aman dipanggil berulang. */
    void begin(const char *hostname, const char *password, uint16_t httpPort,
               StartCallback onStart, EndCallback onEnd);
    void handle();
    bool isStarted() const;

private:
    bool started = false;
};
