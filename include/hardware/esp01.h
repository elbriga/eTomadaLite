#pragma once
#include "hardwareProfile.h"

// config do agua-quarto wemos
const HardwareProfile hardwareProfile = {
    .modelo = "R1S1",
    .board = "esp01_1m",
    .ledPin = 255,
    .ledInvertido = false,
    .relePin = 2,
    .releInvertido = false,
    .botaoPin = 255,
    .sensorDigital = {.pin = 255},
};
