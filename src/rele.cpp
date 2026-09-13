#include "platform.h"
#include "loga.h"
#include "rele.h"
#include "hardwareProfile.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga(".RELE..", nivel, fmt, ##__VA_ARGS__)

extern ETomadaWebServer server;
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

  digitalWrite(hardwareProfile.relePin,
               hardwareProfile.releInvertido ? !ligado : ligado);

  logaM(LOG_DEBUG0, "Rele: %s", ligado ? "ON" : "OFF");
}

bool releGetEstado()
{
  return releLigado;
}

String releGetRecursoJSON()
{
  String resposta;
  resposta.reserve(50);
  resposta = F("{\"id\":\"R1\",\"tipo\":\"RELE\",\"device\":{\"estado\":");
  resposta += releLigado ? "1" : "0";
  resposta += F("}}");
  return resposta;
}

void apiRele(bool estado)
{
  logaM(LOG_NORMAL, "API set rele: %s", (estado ? "ON" : "OFF"));

  releSet(estado);

  String resposta;
  resposta.reserve(64);
  resposta = F("{\"msg\":\"OK\",\"recurso\":");
  resposta += releGetRecursoJSON();
  resposta += "}";

  server.send(200, "application/json", resposta);
}

void apiReleSet()
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

  apiRele(ligado);
}

void apiReleToggle()
{
  apiRele(!releLigado);
}
