#pragma once
#include "hardwareProfile.h"

// EKAZA 3 teclas com o BK7231N CBU
// Necessário gravar o files/bk7231n-bootloader 1.0.1 para funcionar OTA
const HardwareProfile hardwareProfile = {
    .modelo = "EKAZA3_2",
    .board = "cbu",
    .led = {.pin = PIN_P8, .invertido = false, .pisca = false},
    .rele = {
        .pin = {PIN_P15, PIN_P17, PIN_P8, PIN_P23, PIN_P22, PIN_P20, PIN_P16, PIN_P14},
        .invertido = false,
    },
    .botao = {.pin = 255},
    .sensorDigital = {.pin = 255},
    .backlight = {.pin = PIN_P7, .invertido = true},
};
