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

  server.on("/api/configWifi", HTTP_GET, apiConfigWifi);
  server.on("/api/configHostname", HTTP_GET, apiConfigHostname);

  server.on("/api/setRele", HTTP_GET, apiSetRele);

  server.on("/api/reset", HTTP_GET, apiReset);

  recoveryAPIRegister();

  server.on("/", HTTP_GET, httpRoot);

  server.begin();

  logaM(LOG_NORMAL, "HTTP server iniciado.");
}
