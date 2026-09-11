#include "eTomadaLite.h"
#include "loga.h"
#include "hardwareProfile.h"
#include "sensor.h"
#include "mestre.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga("SENSOR.", nivel, fmt, ##__VA_ARGS__)

extern const HardwareProfile hardwareProfile;

struct SensorDigital
{
    bool estado;
    bool ultimoEstado;
    uint32_t debounce;
};

static SensorDigital sensorDigital = {};

void sensorDigitalInit()
{
    if (!sensorDigitalAtivo())
        return;

    logaM(LOG_NORMAL, "Inicializando sensor Digital em [%d]",
          hardwareProfile.sensorDigital.pin);

    pinMode(hardwareProfile.sensorDigital.pin, INPUT);

    sensorDigital.estado = digitalRead(hardwareProfile.sensorDigital.pin);
    sensorDigital.ultimoEstado = sensorDigital.estado;
    sensorDigital.debounce = millis();
}

bool sensorDigitalAtivo()
{
    return hardwareProfile.sensorDigital.pin != 255;
}

// Chamado a cada segundo
void sensorDigitalProcessa()
{
    if (!sensorDigitalAtivo())
        return;

    // Debounce
    bool leitura = digitalRead(hardwareProfile.sensorDigital.pin); // PINO LOW == BOTAO ON
    uint32_t agora = millis();

    if (leitura != sensorDigital.ultimoEstado)
    {
        sensorDigital.debounce = agora;
        sensorDigital.ultimoEstado = leitura;
    }

    if (agora - sensorDigital.debounce > (uint32_t)hardwareProfile.sensorDigital.debounceMS)
    {
        if (sensorDigital.estado != leitura)
        {
            logaM(LOG_NORMAL, "SENSOR MUDOU [%s]", leitura ? "ON" : "OFF");

            sensorDigital.estado = leitura;

            String device = F("\"valor\":");
            device += sensorDigital.estado ? "1" : "0";

            mestreEnviaEvento(EVENTO_VALOR_MUDOU, "S1", device.c_str());
        }
    }
}