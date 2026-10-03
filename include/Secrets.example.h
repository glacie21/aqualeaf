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
//  WiFi Credentials
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
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN_HERE"

// ============================================================
//  OTA (Over-The-Air Update) Password
// ============================================================
#define OTA_PASSWORD     "aqualeaf_ota_2025"

// ============================================================
//  Web Dashboard Basic Auth (optional)
// ============================================================
#define WEB_ADMIN_USER   "admin"
#define WEB_ADMIN_PASS   "aqualeaf"
