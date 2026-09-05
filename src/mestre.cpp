#include <ESP8266mDNS.h>

#include "eTomadaLite.h"
#include "mestre.h"
#include "loga.h"
#include "config.h"
#include "eventos.h"
#include "apiInterna.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga("MESTRE.", nivel, fmt, ##__VA_ARGS__)

Mestre mestre;

#define MESTRE_HEARTBEAT_TIMEOUT 30000

extern Config config;

void mestreInit()
{
    mestre.deviceID = String(config.mestre);
    mestre.ip = (IPAddress)0;
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

    // Escanear
    int totND = MDNS.queryService("etomada", "tcp");

    logaM(LOG_AVISO, "totND: %d", totND);

    // Procurar nosso mestre
    String mestreFQDN = mestre.deviceID + ".local";
    IPAddress ipMestre = (IPAddress)0;
    for (int nd = 0; nd < totND; nd++)
    {
        logaM(LOG_AVISO, "[%d]: %s == %s", nd, MDNS.hostname(nd).c_str(), mestre.deviceID.c_str());
        if (MDNS.hostname(nd) == mestreFQDN)
        {
            ipMestre = MDNS.IP(nd);
            // break;
        }
    }
    if (ipMestre)
    {
        if (!mestre.online)
            logaM(LOG_AVISO, "Mestre Online!");
        mestre.online = true;

        if (mestre.ip != ipMestre)
            logaM(LOG_AVISO, "Mestre novo IP [%s]", ipMestre.toString().c_str());
        mestre.ip = ipMestre;

        mestre.ultimoHeartbeat = millis();
    }
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

void mestreEnviaEvento(TipoEvento tipoEvento, const char *json)
{
    if (!mestreAtivo()) // Sem mestre retorna
        return;

    if (!mestre.online)
    {
        logaM(LOG_AVISO, "Mestre OFFLINE. Descartando evento [%d]", tipoEvento);
        return;
    }

    apiInternaEnviaEvento(mestre.ip, json);
}

bool mestreAtivo()
{
    return (mestre.deviceID != "");
}

IPAddress mestreGetIP()
{
    return mestre.ip;
}
