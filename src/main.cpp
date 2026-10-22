/**
 * @file        main.cpp
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Entry point untuk sistem irigasi cerdas.
 * @author      AquaLeaf Team
 * @version     1.0.0
 */

#include <Arduino.h>
#include "Config.h"
#include "Constants.h"
#include "Pins.h"
#include "Secrets.h"
#include "core/SystemManager.h"

// Unit test menyediakan setup()/loop() sendiri.
#ifndef PIO_UNIT_TESTING

SystemManager systemManager;

void setup() {
    systemManager.begin();
}

void loop() {
    systemManager.update();
}

#endif
