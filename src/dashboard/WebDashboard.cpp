/**
 * @file        WebDashboard.cpp
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Implementasi web server untuk status dan REST API.
 */

#include "WebDashboard.h"
#include "../core/SystemManager.h"
#include "Config.h"
#include "Secrets.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

WebDashboard::WebDashboard(uint16_t port)
    : server(port), manager(nullptr) {
}

void WebDashboard::setManager(SystemManager *manager) {
    this->manager = manager;
}

void WebDashboard::begin() {
    LittleFS.begin();
    // Secara default ESP8266WebServer hanya menyimpan header Authorization; Content-Type
    // perlu didaftarkan agar bisa dicek di requireJsonBody().
    server.collectHeaders("Content-Type");
    registerRoutes();
    server.begin();
}

void WebDashboard::handleClient() {
    server.handleClient();
}

void WebDashboard::registerRoutes() {
    server.on("/", HTTP_GET, [this]() { if (authorize()) handleRoot(); });
    server.on("/style.css", HTTP_GET, [this]() { if (authorize()) handleStaticFile("/style.css"); });
    server.on("/app.js", HTTP_GET, [this]() { if (authorize()) handleStaticFile("/app.js"); });
    server.on("/api/status", HTTP_GET, [this]() { if (authorize()) handleApiStatus(); });
    server.on("/api/history", HTTP_GET, [this]() { if (authorize()) handleApiHistory(); });
    server.on("/api/pump", HTTP_POST, [this]() { if (authorize() && requireJsonBody()) handleApiPump(); });
    server.on("/api/config", HTTP_POST, [this]() { if (authorize() && requireJsonBody()) handleApiConfig(); });
    server.on("/api/calibration", HTTP_POST, [this]() { if (authorize() && requireJsonBody()) handleApiCalibration(); });
    server.on("/api/wifi", HTTP_POST, [this]() { if (authorize() && requireJsonBody()) handleApiWifi(); });
    server.on("/api/health", HTTP_GET, [this]() { handleApiHealth(); });
    server.onNotFound([this]() { handleNotFound(); });
}

bool WebDashboard::authorize() {
    if (!manager) {
        sendError(500, "manager unavailable");
        return false;
    }
    if (server.authenticate(WEB_ADMIN_USER, WEB_ADMIN_PASS)) {
        return true;
    }
    server.requestAuthentication(BASIC_AUTH, "AquaLeaf");
    return false;
}

bool WebDashboard::requireJsonBody() {
    // Proteksi CSRF: form HTML lintas-situs hanya bisa mengirim text/plain, urlencoded,
    // atau multipart tanpa preflight, dan browser tetap menyertakan kredensial Basic Auth
    // yang tersimpan. Mewajibkan application/json memaksa preflight CORS, yang tidak
    // dijawab server ini, sehingga request lintas-situs tidak pernah terkirim.
    String contentType = server.header("Content-Type");
    contentType.toLowerCase();
    if (contentType.startsWith("application/json")) {
        return true;
    }
    sendError(415, "Content-Type must be application/json");
    return false;
}

bool WebDashboard::parseJsonBody(JsonDocument &json) {
    DeserializationError error = deserializeJson(json, server.arg("plain"));
    if (error) {
        sendError(400, "invalid JSON");
        return false;
    }
    return true;
}

void WebDashboard::handleRoot() {
    handleStaticFile("/index.html");
}

void WebDashboard::handleStaticFile(const String &path) {
    String contentType = "text/plain";
    if (path.endsWith(".html")) {
        contentType = "text/html";
    } else if (path.endsWith(".css")) {
        contentType = "text/css";
    } else if (path.endsWith(".js")) {
        contentType = "application/javascript";
    }

    if (!LittleFS.exists(path)) {
        server.send(404, "text/plain", "File not found");
        return;
    }

    File file = LittleFS.open(path, "r");
    if (!file) {
        server.send(500, "text/plain", "Unable to open file");
        return;
    }

    server.streamFile(file, contentType);
    file.close();
}

void WebDashboard::handleNotFound() {
    // Captive portal: saat AP setup aktif, ponsel mengecek konektivitas ke domain lain
    // (mis. connectivitycheck.gstatic.com) yang oleh DNS diarahkan ke sini. Arahkan
    // permintaan itu ke dashboard agar halaman setup muncul otomatis.
    if (manager && manager->isAccessPointActive()) {
        String apAddress = WiFi.softAPIP().toString();
        String host = server.hostHeader();
        bool addressedToDevice = host == apAddress ||
                                 host == WiFi.localIP().toString() ||
                                 host.equalsIgnoreCase(DEVICE_HOSTNAME ".local");
        if (!addressedToDevice) {
            server.sendHeader("Location", String("http://") + apAddress + "/", true);
            server.send(302, "text/plain", "");
            return;
        }
    }
    server.send(404, "text/plain", "Resource not found");
}

void WebDashboard::handleApiStatus() {
    sendJson(manager->getStatusJson());
}

void WebDashboard::handleApiHistory() {
    sendJson(manager->getHistoryJson());
}

void WebDashboard::handleApiPump() {
    JsonDocument json;
    if (!parseJsonBody(json)) {
        return;
    }

    bool pumpOn = json["pump_on"] | false;
    manager->requestPump(pumpOn);
    sendJson(manager->getStatusJson());
}

void WebDashboard::handleApiConfig() {
    JsonDocument json;
    if (!parseJsonBody(json)) {
        return;
    }

    // Field yang tidak dikirim mempertahankan nilai aktif (partial update).
    // Dibaca sebagai int agar nilai di luar rentang di-clamp, bukan terpotong ke uint8_t.
    const WateringConfig &current = manager->getWateringConfig();
    int thresholdLow = json["threshold_low"] | static_cast<int>(current.thresholdLow);
    int thresholdHigh = json["threshold_high"] | static_cast<int>(current.thresholdHigh);
    bool autoMode = json["auto_mode"] | current.autoMode;
    uint32_t allowedHoursMask = json["allowed_hours_mask"] | current.allowedHoursMask;
    if ((allowedHoursMask & ALL_HOURS_MASK) == 0) {
        sendError(400, "allowed_hours_mask must enable at least one hour (bits 0-23)");
        return;
    }

    manager->updateWateringConfig(thresholdLow, thresholdHigh, autoMode, allowedHoursMask);
    sendJson(manager->getStatusJson());
}

void WebDashboard::handleApiCalibration() {
    JsonDocument json;
    if (!parseJsonBody(json)) {
        return;
    }

    const char *error = nullptr;
    const char *point = json["point"] | "";
    if (json["reset"] | false) {
        manager->resetCalibration();
    } else if (strcmp(point, "dry") == 0 || strcmp(point, "wet") == 0) {
        error = manager->calibrateFromCurrentReading(strcmp(point, "dry") == 0);
    } else if (json["raw_dry"].is<int>() || json["raw_wet"].is<int>()) {
        SensorCalibration current = manager->getCalibration();
        int rawDry = json["raw_dry"] | static_cast<int>(current.rawDry);
        int rawWet = json["raw_wet"] | static_cast<int>(current.rawWet);
        if (rawDry < 0 || rawDry > ADC_MAX_RAW || rawWet < 0 || rawWet > ADC_MAX_RAW) {
            sendError(400, "raw values must be between 0 and 1023");
            return;
        }
        error = manager->setCalibration(static_cast<uint16_t>(rawDry), static_cast<uint16_t>(rawWet));
    } else {
        sendError(400, "expected {\"point\":\"dry\"|\"wet\"}, {\"raw_dry\",\"raw_wet\"} or {\"reset\":true}");
        return;
    }

    if (error) {
        sendError(400, error);
        return;
    }
    sendJson(manager->getStatusJson());
}

void WebDashboard::handleApiWifi() {
    JsonDocument json;
    if (!parseJsonBody(json)) {
        return;
    }

    String ssid = json["ssid"] | "";
    String password = json["password"] | "";
    if (ssid.length() == 0 || ssid.length() > 32) {
        sendError(400, "ssid must be 1-32 characters");
        return;
    }
    if (password.length() > 0 && (password.length() < 8 || password.length() > 63)) {
        sendError(400, "password must be empty (open network) or 8-63 characters");
        return;
    }

    // Kirim respons dulu: begin() bisa memindah channel radio dan memutus klien AP.
    sendJson("{\"status\":\"connecting\"}", 202);
    manager->setWifiCredentials(ssid, password);
}

void WebDashboard::handleApiHealth() {
    server.send(200, "text/plain", "OK");
}

void WebDashboard::sendJson(const String &payload, int code) {
    server.send(code, "application/json", payload);
}

void WebDashboard::sendError(int code, const char *message) {
    JsonDocument json;
    json["error"] = message;
    String payload;
    serializeJson(json, payload);
    sendJson(payload, code);
}
