#pragma once
#include "hardwareProfile.h"

// config do agua-quarto wemos
const HardwareProfile hardwareProfile = {
    .modelo = "R1S1",
    .board = "d1_mini",
    .led = {.pin = 13, .invertido = false},
    .rele = {.pin = {D2, 255}, .invertido = true},
    .botao = {.pin = 255},
    .sensorDigital = {.pin = D5, .debounceMS = 5000},
    .backlight = {.pin = 255},
};
