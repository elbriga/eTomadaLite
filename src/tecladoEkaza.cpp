#include <Arduino.h>
#include <cstring>

#include "loga.h"
#include "tecladoEkaza.h"
#include "rele.h"
#include "backlight.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga(".EKAZA.", nivel, fmt, ##__VA_ARGS__)

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

namespace
{
    constexpr uint8_t FRAME_SIZE = 9;
    constexpr uint8_t FILA_SIZE = 16;
    constexpr uint32_t TIMEOUT_FRAME_MS = 50;

    uint8_t frame[FRAME_SIZE];
    uint8_t framePos = 0;
    uint32_t ultimoByteMs = 0;

    TecladoEkazaEvento fila[FILA_SIZE];
    uint8_t filaInicio = 0;
    uint8_t filaFim = 0;
    uint8_t filaCount = 0;

    void adicionaEvento(uint8_t tecla, TecladoEkazaEventoTipo tipo)
    {
        if (filaCount >= FILA_SIZE)
            return;

        fila[filaFim] = {tecla, tipo};
        filaFim = (filaFim + 1) % FILA_SIZE;
        filaCount++;
    }

    bool validaFrame()
    {
        if (frame[0] != 0xAA || frame[1] != 0x55)
            return false;

        if (frame[2] != 0x00 ||
            frame[3] != 0x08 ||
            frame[4] != 0x00 ||
            frame[5] != 0x10)
            return false;

        if (frame[6] > TECLADO_EKAZA_KEEP_PRESS)
            return false;

        if (frame[7] < 1 || frame[7] > 6)
            return false;

        uint8_t checksum = 0;

        for (uint8_t i = 0; i < FRAME_SIZE - 1; i++)
            checksum += frame[i];

        return checksum == frame[8];
    }

    void ressincroniza()
    {
        // Procura outro cabeçalho dentro do frame inválido.
        for (uint8_t i = 1; i < FRAME_SIZE; i++)
        {
            if (frame[i] != 0xAA)
                continue;

            if (i < FRAME_SIZE - 1 && frame[i + 1] != 0x55)
                continue;

            framePos = FRAME_SIZE - i;
            memmove(frame, frame + i, framePos);
            return;
        }

        framePos = 0;
    }

    void processaByte(uint8_t b)
    {
        if (framePos == 0)
        {
            if (b == 0xAA)
                frame[framePos++] = b;
            return;
        }

        if (framePos == 1)
        {
            if (b == 0x55)
                frame[framePos++] = b;
            else if (b != 0xAA)
                framePos = 0;

            return;
        }

        frame[framePos++] = b;

        if (framePos < FRAME_SIZE)
            return;

        if (!validaFrame())
        {
            logaM(LOG_CRITICO, "EKAZA: frame inválido recebido");
            ressincroniza();
            return;
        }

        adicionaEvento(frame[7], (TecladoEkazaEventoTipo)frame[6]);
        framePos = 0;
    }
}

void tecladoEkazaInit()
{
    framePos = 0;
    ultimoByteMs = 0;

    filaInicio = 0;
    filaFim = 0;
    filaCount = 0;

    Serial1.begin(9600);
}

void tecladoEkazaLoop()
{
    uint32_t agora;

    while (Serial1.available() > 0)
    {
        agora = millis();
        if (framePos && (uint32_t)(agora - ultimoByteMs) > TIMEOUT_FRAME_MS)
            framePos = 0;
        ultimoByteMs = agora;

        int b = Serial1.read();
        if (b >= 0)
            processaByte((uint8_t)b);
    }
}

bool tecladoEkazaProximoEvento(TecladoEkazaEvento &evento)
{
    if (!filaCount)
        return false;

    evento = fila[filaInicio];

    filaInicio = (filaInicio + 1) % FILA_SIZE;
    filaCount--;

    return true;
}

// chamado no loop principal de 5ms/5ms
void tecladoEkazaProcessa()
{
    tecladoEkazaLoop();

    TecladoEkazaEvento evento;
    while (tecladoEkazaProximoEvento(evento))
    {
        logaM(LOG_NORMAL, "EKAZA: tecla %u, evento %u", evento.tecla, evento.tipo);

        if (evento.tipo == TECLADO_EKAZA_TOQUE)
            releToggle(evento.tecla - 1);
        else if (evento.tipo == TECLADO_EKAZA_LONG_PRESS)
            backlightToggle();
    }
}
