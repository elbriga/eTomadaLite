#pragma once
#include "hardwareProfile.h"

const HardwareProfile hardwareProfile = {
    .modelo = "MINIR1",
    .board = "esp01_1m",
    .led = {.pin = 13, .invertido = true},
    .rele = {.pin = {12, 255}, .invertido = false},
    .botao = {.pin = 255},
    .sensorDigital = {.pin = 255},
    .backlight = {.pin = 255},
};
