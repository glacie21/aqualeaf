/**
 * @file        WebDashboard.cpp
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Implementasi web server untuk status dan REST API.
 */

#include "WebDashboard.h"
#include "SystemManager.h"
#include "Config.h"
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

WebDashboard::WebDashboard(uint16_t port)
    : server(port), manager(nullptr) {
}

void WebDashboard::setManager(SystemManager *manager) {
    this->manager = manager;
}

void WebDashboard::begin() {
    SPIFFS.begin();
    registerRoutes();
    server.begin();
}

void WebDashboard::handleClient() {
    server.handleClient();
}

void WebDashboard::registerRoutes() {
    server.on("/", HTTP_GET, [this]() { handleRoot(); });
    server.on("/style.css", HTTP_GET, [this]() { handleStaticFile("/style.css"); });
    server.on("/app.js", HTTP_GET, [this]() { handleStaticFile("/app.js"); });
    server.on("/api/status", HTTP_GET, [this]() { handleApiStatus(); });
    server.on("/api/pump", HTTP_POST, [this]() { handleApiPump(); });
    server.on("/api/config", HTTP_POST, [this]() { handleApiConfig(); });
    server.on("/api/health", HTTP_GET, [this]() { handleApiHealth(); });
    server.onNotFound([this]() { handleNotFound(); });
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

    if (!SPIFFS.exists(path)) {
        server.send(404, "text/plain", "File not found");
        return;
    }

    File file = SPIFFS.open(path, "r");
    if (!file) {
        server.send(500, "text/plain", "Unable to open file");
        return;
    }

    server.streamFile(file, contentType);
    file.close();
}

void WebDashboard::handleNotFound() {
    server.send(404, "text/plain", "Resource not found");
}

void WebDashboard::handleApiStatus() {
    if (!manager) {
        server.send(500, "application/json", "{\"error\":\"manager unavailable\"}");
        return;
    }
    sendJson(manager->getStatusJson());
}

void WebDashboard::handleApiPump() {
    if (!manager) {
        server.send(500, "application/json", "{\"error\":\"manager unavailable\"}");
        return;
    }

    String body = server.arg("plain");
    DynamicJsonDocument json(256);
    DeserializationError error = deserializeJson(json, body);
    if (error) {
        server.send(400, "application/json", "{\"error\":\"invalid JSON\"}");
        return;
    }

    bool pumpOn = json["pump_on"] | false;
    manager->requestPump(pumpOn);
    sendJson(manager->getStatusJson());
}

void WebDashboard::handleApiConfig() {
    if (!manager) {
        server.send(500, "application/json", "{\"error\":\"manager unavailable\"}");
        return;
    }

    String body = server.arg("plain");
    DynamicJsonDocument json(256);
    DeserializationError error = deserializeJson(json, body);
    if (error) {
        server.send(400, "application/json", "{\"error\":\"invalid JSON\"}");
        return;
    }

    uint8_t thresholdLow = json["threshold_low"] | MOISTURE_THRESHOLD_LOW;
    uint8_t thresholdHigh = json["threshold_high"] | MOISTURE_THRESHOLD_HIGH;
    bool autoMode = json["auto_mode"] | true;
    manager->updateWateringConfig(thresholdLow, thresholdHigh, autoMode);
    sendJson(manager->getStatusJson());
}

void WebDashboard::handleApiHealth() {
    server.send(200, "text/plain", "OK");
}

void WebDashboard::sendJson(const String &payload, int code) {
    server.send(code, "application/json", payload);
}
