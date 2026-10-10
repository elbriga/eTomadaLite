
#pragma once

#include <Arduino.h>

enum TecladoEkazaEventoTipo : uint8_t
{
    TECLADO_EKAZA_TOQUE = 0,
    TECLADO_EKAZA_LONG_PRESS = 1,
    TECLADO_EKAZA_KEEP_PRESS = 2,
};

struct TecladoEkazaEvento
{
    uint8_t tecla; // 1..6
    TecladoEkazaEventoTipo tipo;
};

void tecladoEkazaInit();
void tecladoEkazaLoop();

bool tecladoEkazaProximoEvento(TecladoEkazaEvento &evento);
