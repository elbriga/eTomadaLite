#pragma once
#include "hardwareProfile.h"

const HardwareProfile hardwareProfile = {
    .modelo = "COZY",
    .board = "generic-ln882h",
    .ledPin = PIN_PB04, // 20
    .ledInvertido = true,
    .relePin = PIN_PB03, // 19
    .releInvertido = false,
    .botaoPin = 255, // TODO
    .sensorDigital = {
        .pin = 255,
        .debounceMS = 0,
    },
};
