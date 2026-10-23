#pragma once
#include "platform.h"
#include "eventos.h"

void mestreInit();
bool mestreAtivo();

const char *mestreGetID();
IPAddress mestreGetIP();

void mestreEnviaEvento(TipoEvento tipoEvento, const char *id, const char *deviceJson);
