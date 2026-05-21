/**
 * @file        DeviceState.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Struktur status perangkat.
 */

#pragma once

#include "Constants.h"

struct DeviceState {
    SystemState systemState;
    NetworkStatus networkStatus;
    MQTTStatus mqttStatus;
    PumpState pumpState;
    OperatingMode operatingMode;
};
