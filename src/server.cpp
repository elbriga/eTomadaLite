#include "eTomadaLite.h"
#include "platform.h"
#include "server.h"
#include "loga.h"
#include "eventos.h"
#include "apiInterna.h"
#include "wifi.h"
#include "util.h"
#include "sensor.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga("SERVER.", nivel, fmt, ##__VA_ARGS__)

IPAddress serverIP;
int serverPort;
char serverHost[32] = {};

IPAddress serverGetIP()
{
    return serverIP;
}

char *serverGetIPPortStr()
{
    return serverHost;
}

void serverInit()
{
    serverIP = IPAddress(10, 0, 0, 1);
    serverPort = 8080;

    char ipStr[16];
    utilIPToString(serverIP, ipStr, sizeof(ipStr));
    sprintf(serverHost, "%s:%d", ipStr, serverPort);

    logaM(LOG_AVISO, "eTomada Server: %s", serverHost);
}

void serverEnviaEvento(TipoEvento tipoEvento, const char *id, const char *deviceJson)
{
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

    apiInterna(serverHost, "evento", body.c_str());
}

void serverEnviaSnapshot()
{
    if (WiFi.status() != WL_CONNECTED || wifiGetModoAP())
    {
        logaM(LOG_AVISO, "offline ou Modo AP. Cancelado envio de snapshot!");
        // TODO :: agendar re-envio
        return;
    }

    String body = eTomadaLiteGetSnapshot();
    apiInterna(serverHost, "snapshot", body.c_str());
}
