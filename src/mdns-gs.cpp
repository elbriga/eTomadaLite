#include <ESP8266mDNS.h>

#include "eTomadaLite.h"
#include "loga.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga(".MDNS..", nivel, fmt, ##__VA_ARGS__)

void mdnsInit()
{
    String hostname = eTomadaLiteDeviceID();

    if (MDNS.begin(hostname.c_str()))
    {
        logaM(LOG_NORMAL, "mDNS iniciado em [%s]", hostname.c_str());

        if (!MDNS.addService("etomada", "tcp", 80))
        {
            logaM(LOG_CRITICO, "Erro ao add servico mDNS [%s] _etomada._tcp", hostname.c_str());
            return;
        }

        MDNS.addServiceTxt("etomada", "tcp", "device", "eTomada");
        MDNS.addServiceTxt("etomada", "tcp", "id", hostname.c_str());
        MDNS.addServiceTxt("etomada", "tcp", "ssid", WiFi.SSID().c_str());
        MDNS.addServiceTxt("etomada", "tcp", "ip", WiFi.localIP().toString().c_str());
        MDNS.addServiceTxt("etomada", "tcp", "mac", WiFi.macAddress().c_str());
        MDNS.addServiceTxt("etomada", "tcp", "model", eTomadaLiteDeviceModel().c_str());
        MDNS.addServiceTxt("etomada", "tcp", "board", eTomadaLiteDeviceBoard().c_str());
        MDNS.addServiceTxt("etomada", "tcp", "fw", eTomadaLiteVersion().c_str());
        MDNS.addServiceTxt("etomada", "tcp", "api", "Lite");

        logaM(LOG_NORMAL, "Servico _etomada._tcp adicionado na porta 80");
    }
    else
        logaM(LOG_CRITICO, "Erro ao iniciar mDNS [%s]", hostname.c_str());
}

void mdnsProcessa()
{
    MDNS.update();
}
