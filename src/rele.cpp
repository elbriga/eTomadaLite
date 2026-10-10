#include "platform.h"
#include "loga.h"
#include "rele.h"
#include "hardwareProfile.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga(".RELE..", nivel, fmt, ##__VA_ARGS__)

extern ETomadaWebServer server;
extern const HardwareProfile hardwareProfile;

int totReles = 0;
bool releLigado[MAX_RELES];

int relesGetCount()
{
  return totReles;
}

void releInit()
{
  for (int i = 0; i < MAX_RELES; i++)
  {
    if (hardwareProfile.relePin[i] == 255)
      break;

    totReles++;
    pinMode(hardwareProfile.relePin[i], OUTPUT);

    logaM(LOG_NORMAL, "Ativando rele [%d] no pino [%d]", i + 1, hardwareProfile.relePin[i]);
    releSet(i, false);
  }
}

void releSet(int index, bool ligado)
{
  if (index < 0 || index >= totReles)
  {
    logaM(LOG_AVISO, "releSet() > Rele invalido: %d", index);
    return;
  }

  releLigado[index] = ligado;

  digitalWrite(hardwareProfile.relePin[index],
               hardwareProfile.releInvertido ? !ligado : ligado);

  logaM(LOG_DEBUG0, "Rele [%d]: %s", index + 1, ligado ? "ON" : "OFF");
}

void releToggle(int index)
{
  releSet(index, !releLigado[index]);
}

bool releGetEstado(int index)
{
  if (index < 0 || index >= totReles)
  {
    logaM(LOG_AVISO, "releGetEstado() > Rele invalido: %d", index);
    return false;
  }

  return releLigado[index];
}

String releGetRecursoJSON(int index)
{
  String resposta;
  resposta.reserve(50);

  if (index < 0 || index >= totReles)
  {
    logaM(LOG_AVISO, "releGetRecursoJSON() > Rele invalido: %d", index);
    resposta = F("{}");
    return resposta;
  }

  resposta = F("{\"id\":\"R");
  resposta += String(index + 1);
  resposta += F("\",\"tipo\":\"RELE\",\"device\":{\"estado\":");
  resposta += releLigado[index] ? "1" : "0";
  resposta += F("}}");

  return resposta;
}

void apiRele(int index, bool estado)
{
  logaM(LOG_NORMAL, "API set rele: %s", (estado ? "ON" : "OFF"));

  releSet(index, estado);

  String resposta;
  resposta.reserve(64);
  resposta = F("{\"msg\":\"OK\",\"recurso\":");
  resposta += releGetRecursoJSON(index);
  resposta += "}";

  server.send(200, "application/json", resposta);
}

void apiReleSet()
{
  int index = server.hasArg("id") ? server.arg("id").toInt() - 1 : 0;
  if (index < 0 || index >= totReles)
  {
    server.send(400, "application/json", R"({"ok":false,"msg":"invalid id"})");
    return;
  }

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

  apiRele(index, ligado);
}

void apiReleToggle()
{
  int index = server.hasArg("id") ? server.arg("id").toInt() - 1 : 0;
  if (index < 0 || index >= totReles)
  {
    server.send(400, "application/json", R"({"ok":false,"msg":"invalid id"})");
    return;
  }
  releToggle(index);
}
