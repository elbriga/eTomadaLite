#pragma once
#include "platform.h"
#include "eventos.h"

void serverInit();

IPAddress serverGetIP();
char *serverGetIPPortStr();

void serverEnviaEvento(TipoEvento tipoEvento, const char *id, const char *deviceJson);
void serverEnviaSnapshot();
