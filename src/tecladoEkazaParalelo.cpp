
#include "eTomadaLite.h"
#include "loga.h"
#include "rele.h"
#include "backlight.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga("TECPEKZ", nivel, fmt, ##__VA_ARGS__)

#define EKAZA3_TOT_TECLAS 3

#define TOUCH_DEBOUNCE_MS 15
#define TOUCH_LONGPRESS_MS 800

struct TouchTecla
{
    uint8_t pin;
    uint8_t ledPin;
    bool bruto;
    bool estavel;
    bool longEnviado;
    uint32_t mudouEm;
    uint32_t pressionadoEm;
};
static TouchTecla teclas[EKAZA3_TOT_TECLAS] = {};

static void tecladoEkazaParaleloEvento(uint8_t tecla, bool longo)
{
    logaM(LOG_NORMAL, "TOUCH K%d: %s\n", tecla, longo ? "LONG PRESS" : "TOQUE");

    if (tecla < 1 || tecla > EKAZA3_TOT_TECLAS)
    {
        logaM(LOG_CRITICO, "TECLA INVALIDA! [%d]", tecla);
        return;
    }

#ifdef HW_EKAZA_3TECLAS
    if (longo)
        backlightToggle();
    else
        // Acionamento do rele controla tambem o led indicativo
        releToggle(tecla - 1);
#elif defined(HW_EKAZA_3TECLAS2)
    if (!longo)
    {
        // Acionamento dos reles e led indicativo em pinos diferentes!
        bool estado = releToggle(tecla - 1);
        if (teclas[tecla - 1].ledPin != 255)
            digitalWrite(teclas[tecla - 1].ledPin, !estado);
    }
#endif
}

void tecladoEkazaParaleloInit(int k1, int k2, int k3, int l1, int l2, int l3)
{
    teclas[0].pin = k1;
    teclas[1].pin = k2;
    teclas[2].pin = k3;

    teclas[0].ledPin = l1;
    teclas[1].ledPin = l2;
    teclas[2].ledPin = l3;

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

        if (t.ledPin != 255)
        {
            pinMode(t.ledPin, OUTPUT);
            digitalWrite(t.ledPin, HIGH); // Apagado
        }
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
