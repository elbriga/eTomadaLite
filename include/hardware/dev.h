#pragma once
#include "hardwareProfile.h"

const HardwareProfile hardwareProfile = {
    .modelo = "PROTO",
    .board = "esp12e",
    .ledPin = 13,
    .ledInvertido = false,
    .relePin = 12,
    .releInvertido = false,
    .botaoPin = 5,
    .sensorDigital = {
        .pin = 14,
        .debounceMS = 2000,
    },
};
