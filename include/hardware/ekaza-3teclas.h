#pragma once
#include "hardwareProfile.h"

// EKAZA 3 teclas com o BK7238
// precisa do meu patch na libretiny para funcionar
// precisa de "build_flags = -Wl,--undefined=lt_init_family" no platformio.ini+
const HardwareProfile hardwareProfile = {
    .modelo = "EKAZA3",
    .board = "t1-u",
    .led = {.pin = PIN_P8, .invertido = false, .pisca = false},
    .rele = {
        .pin = {PIN_P24, PIN_P9, PIN_P26, 255},
        .invertido = false,
    },
    .botao = {.pin = 255},
    .sensorDigital = {.pin = 255},
    .backlight = {.pin = PIN_P28, .invertido = true},
};
