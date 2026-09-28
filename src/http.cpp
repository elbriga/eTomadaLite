#include "eTomadaLite.h"
#include "platform.h"
#include "loga.h"
#include "api.h"
#include "wifi.h"
#include "rele.h"
#include "recovery.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga(".HTTP..", nivel, fmt, ##__VA_ARGS__)

ETomadaWebServer server(80);

void httpProcessa()
{
  server.handleClient();
}

void httpRoot()
{
  server.send(200, "text/plain", "eTomada Lite");
}

void httpInit()
{
  server.on("/api/getSnapshot", HTTP_GET, apiGetSnapshot);

  server.on("/api/config", HTTP_GET, apiConfig);

  server.on("/api/setRele", HTTP_GET, apiReleSet);
  server.on("/api/toggle", HTTP_GET, apiReleToggle);

  server.on("/api/reset", HTTP_GET, apiReset);

  recoveryAPIRegister();

  server.on("/", HTTP_GET, httpRoot);

  server.begin();

  logaM(LOG_NORMAL, "HTTP server iniciado.");
}

const char *httpMethodToString()
{
  switch (server.method())
  {
  case HTTP_GET:
    return "GET";

  case HTTP_POST:
    return "POST";

  case HTTP_PUT:
    return "PUT";

  case HTTP_DELETE:
    return "DELETE";

  case HTTP_PATCH:
    return "PATCH";

  case HTTP_OPTIONS:
    return "OPTIONS";

  default:
    return "UNKNOWN";
  }
}

void httpLogaRequest(String msg)
{
  logaM(LOG_NORMAL, "[org:%s] %s %s => [%s]",
        server.client().remoteIP().toString().c_str(),
        httpMethodToString(),
        server.uri().c_str(),
        msg.c_str());
}
