/**
 * @file        OtaService.cpp
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Implementasi OTA dan mDNS.
 */

#include "OtaService.h"
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>

void OtaService::begin(const char *hostname, const char *password, uint16_t httpPort,
                       StartCallback onStart, EndCallback onEnd) {
    if (started) {
        return;
    }

    ArduinoOTA.setHostname(hostname);
    if (password && password[0] != '\0') {
        ArduinoOTA.setPassword(password);
    }
    ArduinoOTA.onStart([onStart]() { onStart(); });
    ArduinoOTA.onEnd([onEnd]() { onEnd(true); });
    ArduinoOTA.onError([onEnd](ota_error_t) { onEnd(false); });

    // ArduinoOTA.begin() juga menjalankan MDNS.begin(hostname).
    ArduinoOTA.begin();
    MDNS.addService("http", "tcp", httpPort);
    started = true;
}

void OtaService::handle() {
    if (started) {
        // Juga memanggil MDNS.update().
        ArduinoOTA.handle();
    }
}

bool OtaService::isStarted() const {
    return started;
}
