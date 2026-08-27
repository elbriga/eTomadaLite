#pragma once
#include <ESP8266WebServer.h>

void releInit();
void releSet(bool ligado);
bool releGetEstado();
void apiSetRele();
