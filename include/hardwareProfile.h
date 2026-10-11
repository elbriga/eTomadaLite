#pragma once
#include <Arduino.h>

#define MAX_RELES 8

typedef struct
{
    const char *modelo; // Modelo do eTomadaLite
    const char *board;  // board do esp
    struct
    {
        uint8_t pin;
        bool invertido;
    } led;
    struct
    {
        uint8_t pin[MAX_RELES];
        bool invertido;
    } rele;
    struct
    {
        uint8_t pin;
        bool invertido;
    } botao;
    struct
    {
        uint8_t pin;
        int debounceMS;
    } sensorDigital;
    struct
    {
        uint8_t pin;
        bool invertido;
    } backlight;

} HardwareProfile;

#ifdef HW_DEV
#include "hardware/dev.h"
#elif defined(HW_WEMOS)
#include "hardware/wemos.h"
#elif defined(HW_SONOFF_MINIR1)
#include "hardware/miniR1.h"
#elif defined(HW_SONOFF_MINID1)
#include "hardware/miniD1.h"
#elif defined(HW_COZY)
#include "hardware/cozy.h"
#elif defined(HW_ESP01)
#include "hardware/esp01.h"
#elif defined(HW_EKAZA_6TECLAS)
#include "hardware/ekaza-6teclas.h"
#else
#error "Nenhum Hardware Profile definido."
#endif
