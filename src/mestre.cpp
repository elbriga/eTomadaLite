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

#if defined(ESP8266)
    // Escanear
    int totND = MDNS.queryService("etomada", "tcp");

    // logaM(LOG_AVISO, "totND: %d", totND);

    // Procurar nosso mestre
    String mestreFQDN = mestre.deviceID + ".local";
    IPAddress ipMestre = IPAddress(0, 0, 0, 0);
    for (int nd = 0; nd < totND; nd++)
    {
        // logaM(LOG_AVISO, "[%d]: %s == %s", nd, MDNS.hostname(nd).c_str(), mestre.deviceID.c_str());
        if (MDNS.hostname(nd) == mestreFQDN)
        {
            ipMestre = MDNS.IP(nd);
            break;
        }
    }
    if (ipMestre)
    {
        if (!mestre.online)
            logaM(LOG_AVISO, "Mestre Online!");
        mestre.online = true;

        if (mestre.ip != ipMestre)
        {
            String ipStr = utilIPToString(ipMestre);
            logaM(LOG_AVISO, "Mestre novo IP [%s]", ipStr.c_str());
        }
        mestre.ip = ipMestre;

        mestre.ultimoHeartbeat = millis();
    }
#else
    // LibreTiny/LN882H atualmente não implementa
    // mDNS service discovery.
#endif
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

void mestreEnviaEvento(TipoEvento tipoEvento, const char *device)
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
    body += F("\",\"id\":\"B1\"");

    body += F(",\"timestamp\":");
    body += (unsigned long)now;

    body += F(",\"evento\":\"");
    body += eventoGetTipoTxt(tipoEvento);

    body += F("\",\"device\":{");

    body += device;

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
