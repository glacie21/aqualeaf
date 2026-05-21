/**
 * @file        Pins.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       GPIO / ADC pin assignments for NodeMCU v2 (ESP-12E)
 * @author      AquaLeaf Team
 * @version     1.0.0
 *
 * NodeMCU v2 pinout reference:
 *
 *   NodeMCU Label │ GPIO │ Notes
 *   ─────────────────────────────────────────────────────────
 *   D0            │  16  │ Wake from deep sleep; no interrupt
 *   D1            │   5  │ SCL (I²C)
 *   D2            │   4  │ SDA (I²C)
 *   D3            │   0  │ Flash button; boot-mode; pull-up
 *   D4            │   2  │ Built-in LED; TX1; pull-up
 *   D5            │  14  │ SPI CLK / HSCLK
 *   D6            │  12  │ SPI MISO / HMISO
 *   D7            │  13  │ SPI MOSI / HMOSI / RXD2
 *   D8            │  15  │ SPI CS / HCS; must be LOW at boot
 *   RX            │   3  │ UART0 RX
 *   TX            │   1  │ UART0 TX
 *   A0            │ ADC  │ Analog in (0–1 V / 0–1023)
 *   ─────────────────────────────────────────────────────────
 *
 * IMPORTANT: D3, D4, D8 have boot-mode constraints — avoid using
 * them as outputs that are HIGH at power-on.
 */

#pragma once

// ============================================================
//  Analog Input
// ============================================================
/**
 * Soil moisture sensor analog output → NodeMCU A0.
 * The ESP8266 ADC reads 0–1 V, so use a voltage divider if the
 * sensor outputs 0–3.3 V (R1 = 100 kΩ, R2 = 220 kΩ → ×0.31).
 */
#define PIN_MOISTURE_SENSOR     A0

// ============================================================
//  Relay / Pump
// ============================================================
/**
 * Relay IN pin — active LOW (most relay modules).
 * D1 (GPIO5): safe to drive HIGH at boot; no special constraints.
 */
#define PIN_RELAY               D1   // GPIO5

// ============================================================
//  Onboard / Status LEDs
// ============================================================
/** NodeMCU built-in LED (active LOW) — used as general status */
#define PIN_LED_STATUS          LED_BUILTIN   // D4 / GPIO2

/** Optional external LED for pump activity (active HIGH) */
#define PIN_LED_PUMP            D2            // GPIO4

// ============================================================
//  I²C Bus (optional display / external EEPROM)
// ============================================================
#define PIN_I2C_SCL             D1   // GPIO5  (shared, use with care)
#define PIN_I2C_SDA             D2   // GPIO4

// ============================================================
//  UART
// ============================================================
#define PIN_UART_TX             TX   // GPIO1
#define PIN_UART_RX             RX   // GPIO3

// ============================================================
//  Reserved / Do Not Use
// ============================================================
// D0 (GPIO16) — deep-sleep wake only; avoid general output
// D3 (GPIO0)  — flash / boot mode; avoid driving LOW at startup
// D8 (GPIO15) — boot mode; must be LOW at startup
