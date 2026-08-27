#include <Arduino.h>
#include <ESP8266WebServer.h>

#include "loga.h"
#include "rele.h"
#include "hardwareProfile.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga(".RELE..", nivel, fmt, ##__VA_ARGS__)

extern ESP8266WebServer server;
extern const HardwareProfile hardwareProfile;

bool releLigado = false;

void releInit()
{
  pinMode(hardwareProfile.relePin, OUTPUT);
  releSet(false);
}

void releSet(bool ligado)
{
  releLigado = ligado;

  digitalWrite(hardwareProfile.relePin, ligado);

  logaM(LOG_NORMAL, "Rele: %s", ligado ? "ON" : "OFF");
}

bool releGetEstado()
{
  return releLigado;
}

void apiSetRele()
{
  if (!server.hasArg("estado"))
  {
    server.send(400, "application/json", R"({"ok":false,"msg":"missing estado"})");
    return;
  }

  String estado = server.arg("estado");

  bool ligado =
      estado == "1" ||
      estado == "on" ||
      estado == "ON" ||
      estado == "true";

  releSet(ligado);

  String resposta;

  resposta.reserve(64);

  resposta = F("{\"ok\":true,\"rele\":");
  resposta += releLigado ? "1" : "0";
  resposta += "}";

  server.send(200, "application/json", resposta);
}
