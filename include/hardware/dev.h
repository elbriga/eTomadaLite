#pragma once
#include "hardwareProfile.h"

const HardwareProfile hardwareProfile = {
    .modelo = "PROTO",
    .board = "esp12e",
    .led = {.pin = 15, .invertido = false, .pisca = true},
    .rele = {.pin = {2, 255}, .invertido = false},
    .botao = {.pin = 5, .invertido = false},
    .sensorDigital = {.pin = 4, .debounceMS = 2000},
    .backlight = {.pin = 255, .invertido = false},
};
