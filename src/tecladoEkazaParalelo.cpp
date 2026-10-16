
#include "eTomadaLite.h"
#include "loga.h"
#include "rele.h"
#include "backlight.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga("TECPEKZ", nivel, fmt, ##__VA_ARGS__)

#define TOUCH_K1 14
#define TOUCH_K2 16
#define TOUCH_K3 17

#define TOUCH_DEBOUNCE_MS 15
#define TOUCH_LONGPRESS_MS 800

struct TouchTecla
{
    uint8_t pin;
    bool bruto;
    bool estavel;
    bool longEnviado;
    uint32_t mudouEm;
    uint32_t pressionadoEm;
};

static TouchTecla teclas[] = {
    {.pin = TOUCH_K1},
    {.pin = TOUCH_K2},
    {.pin = TOUCH_K3},
};

static void tecladoEkazaParaleloEvento(uint8_t tecla, bool longo)
{
    logaM(LOG_NORMAL, "TOUCH K%d: %s\n", tecla, longo ? "LONG PRESS" : "TOQUE");

    if (longo)
        backlightToggle();
    else
        releToggle(tecla - 1);
}

void tecladoEkazaParaleloInit()
{
    logaM(LOG_NORMAL, "Ativando teclado EKAZA paralelo de 3 teclas");

    uint32_t agora = millis();
    for (auto &t : teclas)
    {
        pinMode(t.pin, INPUT);

        t.bruto = (digitalRead(t.pin) == LOW);
        t.estavel = t.bruto;
        t.longEnviado = false;
        t.mudouEm = agora;
        t.pressionadoEm = agora;
    }
}

void tecladoEkazaParaleloLoop()
{
    uint32_t agora = millis();

    for (uint8_t i = 0; i < 3; i++)
    {
        TouchTecla &t = teclas[i];
        bool pressionado = (digitalRead(t.pin) == LOW);

        // Detecta mudança no sinal bruto
        if (pressionado != t.bruto)
        {
            t.bruto = pressionado;
            t.mudouEm = agora;
        }

        // Debounce: aceita mudança apenas após estabilidade
        if (t.bruto != t.estavel &&
            agora - t.mudouEm >= TOUCH_DEBOUNCE_MS)
        {
            t.estavel = t.bruto;

            if (t.estavel)
            {
                // Tecla pressionada
                t.pressionadoEm = agora;
                t.longEnviado = false;
            }
            else if (!t.longEnviado)
            {
                // Tecla liberada: toque curto ou longo
                bool longo =
                    t.mudouEm - t.pressionadoEm >=
                    TOUCH_LONGPRESS_MS;

                tecladoEkazaParaleloEvento(i + 1, longo);
                t.longEnviado = true;
            }
        }

        // Long press enquanto a tecla continua pressionada
        if (t.estavel && t.bruto && !t.longEnviado &&
            agora - t.pressionadoEm >= TOUCH_LONGPRESS_MS)
        {
            t.longEnviado = true;
            tecladoEkazaParaleloEvento(i + 1, true);
        }
    }
}
