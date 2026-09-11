#include "eTomadaLite.h"
#include "platform.h"
#include "mestre.h"
#include "loga.h"
#include "config.h"
#include "eventos.h"
#include "apiInterna.h"
#include "wifi.h"
#include "util.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga("MESTRE.", nivel, fmt, ##__VA_ARGS__)

Mestre mestre;

#define MESTRE_HEARTBEAT_TIMEOUT 30000

extern Config config;

void mestreInit()
{
    mestre.deviceID = String(config.mestre);
    mestre.ip = IPAddress(0, 0, 0, 0);
    mestre.ultimoHeartbeat = 0;
    mestre.online = false;

    if (mestreAtivo())
        logaM(LOG_AVISO, "Nodo Mestre: %s", mestre.deviceID.c_str());

    mestreCheckOnline();
}

void mestreCheckOnline()
{
    if (!mestreAtivo())
        return;

    // Procurar nosso mestre
    // Suporte para queryHost e queryService adicionado na minha versão do LibreTiny
    // Suporte para queryHost adicionado na minha versão do framework-arduinoespressif8266
    IPAddress ipMestre = MDNS.queryHost(mestre.deviceID);

    if (!ipMestre)
    {
        logaM(LOG_AVISO, "Mestre não respondeu o mDNS");
        if (mestre.ip)
        {
            // TODO : ping?
        }
        return;
    }

    if (!mestre.online)
        logaM(LOG_AVISO, "Mestre Online!");
    mestre.online = true;

    if (mestre.ip != ipMestre)
    {
        char ipStr[16];
        utilIPToString(ipMestre, ipStr, 16);
        logaM(LOG_AVISO, "Mestre novo IP [%s]", ipStr);
    }
    mestre.ip = ipMestre;

    mestre.ultimoHeartbeat = millis();
}

void mestreLoop()
{
    if (!mestreAtivo()) // Sem mestre retorna
        return;

    if (millis() - mestre.ultimoHeartbeat > MESTRE_HEARTBEAT_TIMEOUT)
    {
        if (mestre.online)
            logaM(LOG_AVISO, "Mestre - OFFLINE!");
        mestre.online = false;
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

    if (!mestre.online)
    {
        logaM(LOG_AVISO, "Mestre OFFLINE. Descartando evento [%d]", tipoEvento);
        return;
    }

    time_t now = 0;
    time(&now);

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

    body += F("\",\"device\":{");
    body += deviceJson;
    body += F("}}");

    apiInternaEnviaEvento(mestre.ip, body.c_str());
}

bool mestreAtivo()
{
    return (mestre.deviceID != "");
}

IPAddress mestreGetIP()
{
    return mestre.ip;
}
