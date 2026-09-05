#include <ESP8266HTTPClient.h>

#include "eTomadaLite.h"
#include "loga.h"
#include "util.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga("APIINT.", nivel, fmt, ##__VA_ARGS__)

#define API_INTERNA_TIMEOUT 1000
#define API_INTERNA_RESPONSE_MAXLEN 256

int apiInterna(const char *host, const char *endpoint, const char *request, char *response);
/*
String apiInternaGetSnapshot(NodoRemoto *nodo, JsonDocument &doc)
{
  int code = apiInterna(nodo->ip, "getSnapshot", "GET", nullptr, &doc);

  return code == 200 ? "OK" : String(code);
}

String apiInternaSetRecurso(Recurso *recurso, String estado)
{
  if (!recurso->remoto)
  {
    return "Recurso nao Remoto";
  }

  RecursoRemoto *rr = recurso->recursoRemoto;
  JsonDocument request, resposta;

  request["id"] = String(rr->idRemoto);
  request["estado"] = estado;

  int code = apiInterna(rr->nodo->ip, "setRecurso", "PUT", &request, &resposta);
  if (code != 200)
  {
    logaM(LOG_CRITICO, "Erro API Interna: %d", code);
    // TODO ??
  }

  // String out;
  // serializeJson(resposta, out);
  // logaM("ATUALIZAR RECURSO REMOTO com Resposta :::::::: [%s]", out.c_str());

  switch (recurso->tipo)
  {
  case RECURSO_RELE:
    Rele *rele = &rr->rele;
    rele->estado = resposta["recurso"]["device"]["estado"].as<bool>();
    break;
  }

  // TODO localizar a msg para os params locais
  return resposta["msg"].as<String>();
}
*/
bool apiInternaEnviaEvento(IPAddress ip, const char *body)
{
  int code = apiInterna(ip.toString().c_str(), "evento", body, nullptr);
  return code == 200;
}

int apiInterna(const char *host, const char *endpoint, const char *request, char *response)
{
  String url = "http://" + String(host) + "/api/" + endpoint;

  logaM(LOG_DEBUG0, "apiInterna: Acionando %s", url.c_str());

  HTTPClient http;
  WiFiClient client;
  if (!http.begin(client, url))
  {
    Serial.println("Nao foi possivel iniciar HTTP para api Interna.");
    return false;
  }

  http.setTimeout(API_INTERNA_TIMEOUT);
  if (request)
    http.addHeader("Content-Type", "application/json");

  int code = request ? http.POST(request) : http.GET();

  if (code == 200)
  {
    // TODO http.getString() é perigoso !!! usar o stream
    String respBody = http.getString();
    logaM(LOG_DEBUG0, " >> RESP: %s", respBody.c_str());

    if (response)
      strlcpy(response, respBody.c_str(), API_INTERNA_RESPONSE_MAXLEN);
  }

  http.end();

  return code;
}
