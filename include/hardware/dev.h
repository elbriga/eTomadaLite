#pragma once
#include "hardwareProfile.h"

const HardwareProfile hardwareProfile = {
    .modelo = "PROTO",
    .board = "esp12e",
    .led = {.pin = 13, .invertido = false, .pisca = true},
    .rele = {.pin = {12, 255}, .invertido = false},
    .botao = {.pin = 5, .invertido = false},
    .sensorDigital = {.pin = 14, .debounceMS = 2000},
    .backlight = {.pin = 255, .invertido = false},
};
