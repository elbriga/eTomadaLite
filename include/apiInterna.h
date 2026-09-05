#pragma once
#include <ESP8266WiFi.h>

bool apiInternaEnviaEvento(IPAddress ip, const char *body);
