#pragma once
#include "hardwareProfile.h"

const HardwareProfile hardwareProfile = {
    .modelo = "COZY",
    .board = "generic-ln882h",
    .led = {.pin = PIN_PB04, .invertido = true, .pisca = true},
    .rele = {.pin = {PIN_PB03, 255}, .invertido = false},
    .botao = {.pin = 255},
    .sensorDigital = {.pin = 255},
    .backlight = {.pin = 255},
};
