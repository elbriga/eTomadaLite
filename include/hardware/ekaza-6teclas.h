#pragma once
#include "hardwareProfile.h"

const HardwareProfile hardwareProfile = {
    .modelo = "EKAZA6",
    .board = "generic-bk7231n-qfn32-tuya",
    .ledPin = PIN_P14,
    .ledInvertido = false,
    .relePin = PIN_P7,
    .releInvertido = false,
    .botaoPin = 255, // TODO
    .sensorDigital = {
        .pin = 255,
        .debounceMS = 0,
    },
};
