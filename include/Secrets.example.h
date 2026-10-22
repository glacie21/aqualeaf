/**
 * @file        Secrets.example.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Template for credentials — copy to Secrets.h and fill in.
 *
 * ┌─────────────────────────────────────────────────────────┐
 * │  NEVER commit Secrets.h to version control!             │
 * │  Secrets.h is listed in .gitignore.                     │
 * │  This file (Secrets.example.h) is safe to commit.       │
 * └─────────────────────────────────────────────────────────┘
 *
 * Setup:
 *   cp include/Secrets.example.h include/Secrets.h
 *   # Edit include/Secrets.h with your real credentials
 */

#pragma once

// ============================================================
//  WiFi Credentials (optional)
//  Only used on first boot when the device has no saved network.
//  Leave the placeholder to set up WiFi from the dashboard instead:
//  join the "AquaLeaf-Setup" access point and open http://192.168.4.1.
//  Later changes are made from the dashboard (Network panel).
// ============================================================
#define WIFI_SSID        "YOUR_WIFI_SSID"
#define WIFI_PASSWORD    "YOUR_WIFI_PASSWORD"

// ============================================================
//  MQTT Broker Credentials (leave empty if no auth required)
// ============================================================
#define MQTT_USERNAME    ""
#define MQTT_PASSWORD    ""

// ============================================================
//  Blynk Auth Token
// ============================================================
// Get your token at: https://blynk.cloud → Templates → Device Info
// Leave the placeholder token to disable Blynk entirely.
#define BLYNK_TEMPLATE_ID   "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "AquaLeaf"
#define BLYNK_AUTH_TOKEN    "YOUR_BLYNK_AUTH_TOKEN_HERE"

// ============================================================
//  OTA (Over-The-Air Update) Password
//  Required to flash firmware over WiFi (env:nodemcu_v2_ota).
//  Change it — anyone on your network who knows it can flash the device.
// ============================================================
#define OTA_PASSWORD     "aqualeaf_ota_2025"

// ============================================================
//  Web Dashboard Basic Auth (required for dashboard & REST API)
//  Change these defaults before deploying!
// ============================================================
#define WEB_ADMIN_USER   "admin"
#define WEB_ADMIN_PASS   "aqualeaf"
