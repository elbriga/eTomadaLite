#include "eTomadaLite.h"
#include "platform.h"
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
  String ipStr = utilIPToString(ip);
  int code = apiInterna(ipStr.c_str(), "evento", body, nullptr);
  return code == 200;
}

int apiInterna(const char *host, const char *endpoint, const char *request, char *responseOut)
{
  String url = "http://" + String(host) + "/api/" + endpoint;

  logaM(LOG_DEBUG0, "apiInterna: Acionando %s", url.c_str());
  logaM(LOG_DEBUG0, "body: [%s]", request);

  HTTPClient http;
  WiFiClient client;
  if (!http.begin(client, url))
  {
    Serial.println("Nao foi possivel iniciar HTTP para api Interna.");
    return -1;
  }

  http.setTimeout(API_INTERNA_TIMEOUT);

  // Simplifica a leitura direta do stream:
  // evita resposta HTTP chunked e encerra a conexão após a resposta.
  http.useHTTP10(true);

  if (request)
    http.addHeader("Content-Type", "application/json");

  int code = request ? http.POST(request) : http.GET();

  if (responseOut)
    responseOut[0] = '\0';

  if (code == 200)
  {
    char response[API_INTERNA_RESPONSE_MAXLEN] = {0};

    WiFiClient *stream = http.getStreamPtr();

    if (stream)
    {
      size_t pos = 0;
      int restante = http.getSize();

      uint32_t ultimoDado = millis();

      while (pos < API_INTERNA_RESPONSE_MAXLEN - 1)
      {
        int disponivel = stream->available();

        if (disponivel > 0)
        {
          size_t tamanho = min(
              (size_t)disponivel,
              (size_t)(API_INTERNA_RESPONSE_MAXLEN - 1 - pos));

          if (restante >= 0 && tamanho > (size_t)restante)
            tamanho = restante;

          if (!tamanho)
            break;

          size_t lido = stream->readBytes(response + pos, tamanho);
          if (!lido)
            break;

          pos += lido;

          if (restante >= 0)
          {
            restante -= lido;

            if (!restante)
              break;
          }

          ultimoDado = millis();
          continue;
        }

        if (restante == 0 || !http.connected())
          break;

        if (millis() - ultimoDado >= API_INTERNA_TIMEOUT)
          break;

        delay(1);
      }
      response[pos] = '\0';

      logaM(LOG_DEBUG0, " >> RESP: %s", response);

      if (responseOut)
        strlcpy(responseOut, response, API_INTERNA_RESPONSE_MAXLEN);
    }
  }

  http.end();

  return code;
}
