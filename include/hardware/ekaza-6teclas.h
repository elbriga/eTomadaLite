#pragma once
#include "hardwareProfile.h"

const HardwareProfile hardwareProfile = {
    .modelo = "EKAZA6",
    .board = "generic-bk7231n-qfn32-tuya",
    .led = {.pin = PIN_P14, .invertido = false, .piscar = false},
    .rele = {
        .pin = {PIN_P7, PIN_P23, PIN_P6, PIN_P8, PIN_P22, PIN_P24, 255},
        .invertido = false,
    },
    .botao = {.pin = 255},
    .sensorDigital = {.pin = 255},
    .backlight = {.pin = PIN_P28, .invertido = true},
};
