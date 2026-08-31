#include <ESP8266WebServer.h>

#include "eTomadaLite.h"
#include "loga.h"
#include "api.h"
#include "wifi.h"
#include "rele.h"
#include "ota.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga(".HTTP..", nivel, fmt, ##__VA_ARGS__)

ESP8266WebServer server(80);

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

  server.on("/api/ota", HTTP_POST, apiOtaFlashHelper, apiOtaFlash);

  server.on("/", HTTP_GET, httpRoot);

  server.begin();

  logaM(LOG_NORMAL, "HTTP server iniciado.");
}
