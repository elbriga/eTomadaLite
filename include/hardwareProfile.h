#pragma once
#include <Arduino.h>

typedef struct
{
    const char *modelo; // Modelo do eTomadaLite
    const char *board;  // board do esp
    int ledPin;
    bool ledInvertido;
    int relePin;
    int botaoPin;
} HardwareProfile;

#ifdef HW_DEV
#include "hardware/dev.h"
#elif defined(HW_WEMOS)
#include "hardware/wemos.h"
#elif defined(HW_SONOFF_MINIR1)
#include "hardware/miniR1.h"
#elif defined(HW_SONOFF_MINID1)
#include "hardware/miniD1.h"
#else
#error "Nenhum Hardware Profile definido."
#endif
