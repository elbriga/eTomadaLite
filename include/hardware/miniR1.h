#pragma once
#include "hardwareProfile.h"

const HardwareProfile hardwareProfile = {
    .modelo = "MINIR1",
    .board = "esp01_1m",
    .ledPin = 13,
    .ledInvertido = true,
    .relePin = {12, 255},
    .releInvertido = false,
    .botaoPin = 255,
    .sensorDigital = {
        .pin = 255,
        .debounceMS = 0,
    },
    .backlight = {.pin = 255, .invertido = false},
};
