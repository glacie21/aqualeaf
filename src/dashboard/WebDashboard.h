/**
 * @file        WebDashboard.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Web dashboard dan REST API sederhana.
 */

#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>

class SystemManager;

class WebDashboard {
public:
    explicit WebDashboard(uint16_t port);
    void begin();
    void handleClient();
    void setManager(SystemManager *manager);

private:
    void registerRoutes();
    bool authorize();
    bool requireJsonBody();
    bool parseJsonBody(JsonDocument &json);
    void handleRoot();
    void handleStaticFile(const String &path);
    void handleNotFound();
    void handleApiStatus();
    void handleApiHistory();
    void handleApiPump();
    void handleApiConfig();
    void handleApiCalibration();
    void handleApiWifi();
    void handleApiHealth();
    void sendJson(const String &payload, int code = 200);
    void sendError(int code, const char *message);

    ESP8266WebServer server;
    SystemManager *manager;
};
