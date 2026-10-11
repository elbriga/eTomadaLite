#pragma once
#include "hardwareProfile.h"

// config do agua-quarto wemos
const HardwareProfile hardwareProfile = {
    .modelo = "R1S1",
    .board = "esp01_1m",
    .led = {.pin = 255},
    .rele = {.pin = {2, 255}, .invertido = false},
    .botao = {.pin = 255},
    .sensorDigital = {.pin = 255},
    .backlight = {.pin = 255},
};
