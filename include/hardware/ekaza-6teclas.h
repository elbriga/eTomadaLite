#pragma once
#include "hardwareProfile.h"

const HardwareProfile hardwareProfile = {
    .modelo = "EKAZA6",
    .board = "generic-bk7231n-qfn32-tuya",
    .ledPin = PIN_P14,
    .ledInvertido = false,
    .relePin = {PIN_P7, PIN_P23, PIN_P6, PIN_P8, PIN_P22, PIN_P24, 255},
    .releInvertido = false,
    .botaoPin = 255, // TODO
    .sensorDigital = {
        .pin = 255,
        .debounceMS = 0,
    },
};
