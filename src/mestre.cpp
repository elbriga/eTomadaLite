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

struct Mestre
{
    String deviceID;
    IPAddress ip;
};
Mestre mestre = {};

extern Config config;

bool mestreAtivo()
{
    return utilIPValido(mestre.ip);
}

const char *mestreGetID()
{
    return mestre.deviceID.c_str();
}

void mestreInit()
{
    mestre.deviceID = String(config.mestre);
    mestre.ip = IPAddress();

    if (mestre.deviceID == "")
    {
        logaM(LOG_AVISO, "Sem MESTRE configurado!");
        return;
    }

    String hostname = mestre.deviceID + ".etomada";
    if (!WiFi.hostByName(hostname.c_str(), mestre.ip))
    {
        logaM(LOG_CRITICO, "Falha ao resolver o IP do MESTRE");
        return;
    }
    if (!mestre.ip)
    {
        logaM(LOG_CRITICO, "Falha ao resolver o IP do MESTRE 2");
        return;
    }

    char ipStr[16];
    utilIPToString(mestre.ip, ipStr, sizeof(ipStr));
    logaM(LOG_AVISO, "Nodo Mestre: [%s] @ [%s]", mestre.deviceID.c_str(), ipStr);

    // Avisar nosso boot para o mestre
    String json = "\"tipo\":\"Lite\",\"uptime\":" + String(millis());
    mestreEnviaEvento(EVENTO_BOOT, "ETOMADA", json.c_str());

    sensorDigitalEnviaEvento();
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
