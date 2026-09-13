#include "eTomadaLite.h"
#include "platform.h"
#include "mestre.h"
#include "loga.h"
#include "config.h"
#include "eventos.h"
#include "apiInterna.h"
#include "wifi.h"
#include "util.h"
#include "sensor.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga("MESTRE.", nivel, fmt, ##__VA_ARGS__)

Mestre mestre = {};

#define MESTRE_HEARTBEAT_TIMEOUT 30000

extern Config config;

bool mestreAtivo()
{
    return (mestre.deviceID != "");
}

void mestreInit()
{
    mestre.deviceID = String(config.mestre);
    mestre.ip = IPAddress();

    if (!mestreAtivo())
        return;

    logaM(LOG_AVISO, "Nodo Mestre: %s", mestre.deviceID.c_str());
    mestreCheckIP();

    if (mestre.ip)
        sensorDigitalEnviaEvento();
}

void mestreCheckIP()
{
    if (!mestreAtivo())
        return;

    // Procurar nosso mestre
    // Suporte para queryHost e queryService adicionado na minha versão do LibreTiny
    // Suporte para queryHost adicionado na minha versão do framework-arduinoespressif8266
    IPAddress ipMestre = MDNS.queryHost(mestre.deviceID);
    if (!ipMestre)
    {
        logaM(LOG_DEBUG, "Mestre não respondeu o mDNS");
        if (mestre.ip)
        {
            // TODO : ping?
        }
        return;
    }

    if (mestre.ip != ipMestre)
    {
        char ipStr[16];
        utilIPToString(ipMestre, ipStr, 16);
        logaM(LOG_AVISO, "Mestre novo IP [%s]", ipStr);

        mestre.ip = ipMestre;
    }
}

void mestreEnviaEvento(TipoEvento tipoEvento, const char *id, const char *deviceJson)
{
    if (!mestreAtivo()) // Sem mestre retorna
        return;

    if (WiFi.status() != WL_CONNECTED || wifiGetModoAP())
    {
        logaM(LOG_AVISO, "offline ou Modo AP. Descartando evento [%d]", tipoEvento);
        return;
    }

    if (!mestre.ip)
    {
        mestreCheckIP();
        if (!mestre.ip)
        {
            logaM(LOG_AVISO, "Mestre OFFLINE. Descartando evento [%d]", tipoEvento);
            return;
        }
    }

    time_t now = 0;
    time(&now);

    // TODO :: usar *GetRecursoJSON()
    String body;
    body.reserve(256);

    body = F("{\"origem\":\"");
    body += eTomadaLiteDeviceID();

    body += F("\",\"id\":\"");
    body += id;

    body += F("\",\"timestamp\":");
    body += (unsigned long)now;

    body += F(",\"evento\":\"");
    body += eventoGetTipoTxt(tipoEvento);

    body += F("\",\"device\":{"); // TODO :: mudar device para recurso
    body += deviceJson;
    body += F("}}");

    apiInternaEnviaEvento(mestre.ip, body.c_str());
}
