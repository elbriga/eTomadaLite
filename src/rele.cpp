#include "eTomadaLite.h"
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
    if (hardwareProfile.rele.pin[i] == 255)
      break;

    totReles++;
    pinMode(hardwareProfile.rele.pin[i], OUTPUT);

    logaM(LOG_NORMAL, "Ativando rele [%d] no pino [%d]", i + 1, hardwareProfile.rele.pin[i]);
    releSet(i, false);
  }
}

void releEnviaEvento(int index)
{
  String id = String("R") + String(index + 1);

  String device = F("\"estado\":");
  device += releLigado[index] ? "1" : "0";

  serverEnviaEvento(EVENTO_TOGGLE, id.c_str(), device.c_str());
}

void releSet(int index, bool ligado, bool enviaEvento)
{
  if (index < 0 || index >= totReles)
  {
    logaM(LOG_AVISO, "releSet() > Rele invalido: %d", index);
    return;
  }

  releLigado[index] = ligado;

  digitalWrite(hardwareProfile.rele.pin[index],
               hardwareProfile.rele.invertido ? !ligado : ligado);

  logaM(LOG_DEBUG0, "Rele [%d]: %s", index + 1, ligado ? "ON" : "OFF");

  if (enviaEvento)
    releEnviaEvento(index);
}

bool releToggle(int index)
{
  bool estado = !releLigado[index];
  releSet(index, estado);
  return estado;
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
  resposta += releLigado[index] ? "true" : "false";
  resposta += F("}}");

  return resposta;
}

void apiRele(int index, bool estado)
{
  logaM(LOG_NORMAL, "API set rele[%d]: %s", index + 1, (estado ? "ON" : "OFF"));

  releSet(index, estado, false);

  String resposta;
  resposta.reserve(64);
  resposta = F("{\"msg\":\"OK\",\"recurso\":");
  resposta += releGetRecursoJSON(index);
  resposta += "}";

  bool fromServer = server.client().remoteIP() == serverGetIP();
  server.send(200, "application/json", resposta);

  if (!fromServer)
    releEnviaEvento(index);
}

void apiReleSet()
{
  int index = 0; // Default
  if (server.hasArg("id"))
  {
    String id = server.arg("id");
    if (id.startsWith("R"))
      id = id.substring(1); // Remover o "R"
    index = id.toInt() - 1;
  }
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

  apiRele(index, !releLigado[index]);
}

void apiRelesSet()
{
  if (!server.hasArg("estados"))
  {
    server.send(400, "application/json",
                R"({"ok":false,"msg":"missing estados"})");
    return;
  }

  String comandos = server.arg("estados");

  if (comandos.isEmpty() || comandos.length() > 80)
  {
    server.send(400, "application/json",
                R"({"ok":false,"msg":"invalid estados"})");
    return;
  }

  int indices[MAX_RELES];
  bool estados[MAX_RELES];
  bool vistos[MAX_RELES] = {};
  int total = 0;

  auto erro = [&]()
  {
    server.send(400, "application/json",
                R"({"ok":false,"msg":"invalid estados"})");
  };

  // Validar o lote inteiro antes de alterar qualquer rele.
  int inicio = 0;
  while (inicio < (int)comandos.length())
  {
    int fim = comandos.indexOf('-', inicio);
    if (fim < 0)
      fim = comandos.length();

    String item = comandos.substring(inicio, fim);

    // Formatos aceitos: R1:ON e R1:OFF (ate R8).
    if ((item.length() != 5 && item.length() != 6) ||
        item[0] != 'R' ||
        item[1] < '1' || item[1] > '8' ||
        item[2] != ':')
    {
      erro();
      return;
    }

    int index = item[1] - '1';
    String estado = item.substring(3);

    if (index >= totReles ||
        vistos[index] ||
        total >= MAX_RELES ||
        (estado != "ON" && estado != "OFF"))
    {
      erro();
      return;
    }

    vistos[index] = true;
    indices[total] = index;
    estados[total] = estado == "ON";
    total++;

    if (fim == (int)comandos.length())
      break;

    inicio = fim + 1;

    if (inicio == (int)comandos.length())
    {
      erro();
      return;
    }
  }

  bool fromServer = server.client().remoteIP() == serverGetIP();

  String resposta;
  resposta.reserve(70 + total * 110);

  resposta = F("{\"ok\":true,\"total\":");
  resposta += total;
  resposta += F(",\"aplicados\":");
  resposta += total;
  resposta += F(",\"resultados\":[");

  // Aplicar os estados e montar as confirmacoes individuais.
  for (int i = 0; i < total; i++)
  {
    int index = indices[i];

    releSet(index, estados[i], false);

    if (i > 0)
      resposta += ',';

    resposta += F("{\"id\":\"R");
    resposta += index + 1;
    resposta += F("\",\"ok\":true,\"recurso\":");
    resposta += releGetRecursoJSON(index);
    resposta += '}';
  }

  resposta += F("]}");

  logaM(LOG_NORMAL, "API setReles: %d recursos", total);
  server.send(200, "application/json", resposta);

  // Preservar o comportamento atual para comandos externos.
  if (!fromServer)
  {
    for (int i = 0; i < total; i++)
      releEnviaEvento(indices[i]);
  }
}