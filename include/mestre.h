#pragma once
#include "platform.h"
#include "eventos.h"

struct Mestre
{
    String deviceID;
    IPAddress ip;
};

void mestreInit();
void mestreCheckIP();

void mestreEnviaEvento(TipoEvento tipoEvento, const char *id, const char *deviceJson);
