#pragma once
#include <Arduino.h>

#define MAX_RELES 8

typedef struct
{
    const char *modelo; // Modelo do eTomadaLite
    const char *board;  // board do esp
    uint8_t ledPin;
    bool ledInvertido;
    uint8_t relePin[MAX_RELES];
    bool releInvertido;
    uint8_t botaoPin;
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
