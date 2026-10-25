#include "eTomadaLite.h"
#include "platform.h"
#include "loga.h"
#include "util.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga("APIINT.", nivel, fmt, ##__VA_ARGS__)

#define API_INTERNA_TIMEOUT 1500
#define API_INTERNA_RESPONSE_MAXLEN 256

int apiInterna(char *hostPort, const char *endpoint, const char *request)
{
  String url = String("http://") + hostPort + "/api/" + endpoint;

  logaM(LOG_DEBUG0, "apiInterna: Acionando %s", url.c_str());
  logaM(LOG_DEBUG0, "body: [%s]", request ? request : "");

  HTTPClient http;
  WiFiClient client;
  if (!http.begin(client, url))
  {
    logaM(LOG_AVISO, "Nao foi possivel iniciar HTTP para api Interna.");
    return -1;
  }

  http.setTimeout(API_INTERNA_TIMEOUT);

  // Simplifica a leitura direta do stream:
  // evita resposta HTTP chunked e encerra a conexão após a resposta.
  http.useHTTP10(true);

  if (request)
    http.addHeader("Content-Type", "application/json");

  int code = request ? http.POST(request) : http.GET();

  if (code != 200)
    logaM(LOG_AVISO, " >> RESP nOK: %d", code);

  http.end();

  return code;
}
