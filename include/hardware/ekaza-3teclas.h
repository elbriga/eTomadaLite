#pragma once
#include "hardwareProfile.h"

const HardwareProfile hardwareProfile = {
    .modelo = "EKAZA3",
    .board = "t1-u",
    .led = {.pin = PIN_P6, .invertido = false, .pisca = false},
    .rele = {
        .pin = {PIN_P24, PIN_P9, PIN_P26, 255},
        .invertido = false,
    },
    .botao = {.pin = 255},
    .sensorDigital = {.pin = 255},
    .backlight = {.pin = PIN_P8, .invertido = true},
};
