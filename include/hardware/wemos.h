#pragma once
#include "hardwareProfile.h"

const HardwareProfile hardwareProfile = {
    .modelo = "R1S1",
    .board = "d1_mini",
    .ledPin = 13,
    .ledInvertido = false,
    .relePin = D2, // 4
    .releInvertido = true,
    .botaoPin = 255,
    .sensorDigital = {
        .pin = D5, // 14
        .debounceMS = 5000,
    },
};
