#pragma once
#include <Arduino.h>
#include <ESP8266WiFi.h>

struct Mestre
{
    String deviceID;
    IPAddress ip;
    bool online;
    uint32_t ultimoHeartbeat;
};

void mestreInit();
void mestreCheckOnline();

void mestreLoop();
bool mestreAtivo();
IPAddress mestreGetIP();

void mestreEnviaEvento(TipoEvento tipoEvento, const char *json);
