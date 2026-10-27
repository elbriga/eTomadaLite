#pragma once
#include "hardwareProfile.h"

// EKAZA 3 teclas com o BK7231N CBU
// Necessário gravar o files/bk7231n-bootloader 1.0.1 para funcionar OTA
const HardwareProfile hardwareProfile = {
    .modelo = "EKAZA3_2",
    .board = "cbu",
    .led = {.pin = 255},
    .rele = {
        .pin = {PIN_P24, PIN_P6, PIN_P26, 255},
        .invertido = false,
    },
    .botao = {.pin = 255},
    .sensorDigital = {.pin = 255},
    .backlight = {.pin = 255},
};

// LED1 - P14 invertido
// LED2 - P16 invertido
// LED3 - P20 invertido