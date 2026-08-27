#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include "eTomadaLite.h"
#include "loga.h"
#include "wifi.h"
#include "config.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga(".WIFI..", nivel, fmt, ##__VA_ARGS__)

extern Config config;
extern ESP8266WebServer server;

void wifiStartAP()
{
  String apName = "eTomada-";

  apName += String(ESP.getChipId(), HEX);

  logaM(LOG_AVISO, "Iniciando AP: %s", apName.c_str());

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(apName.c_str());

  logaM(LOG_AVISO, "AP IP: %s", WiFi.softAPIP().toString().c_str());
}

void wifiConnect()
{
  if (config.ssid[0] == '\0')
  {
    logaM(LOG_NORMAL, "Nenhuma rede WiFi configurada.");
    wifiStartAP();
    return;
  }

  logaM(LOG_NORMAL, "Conectando em: %s", config.ssid);

  WiFi.mode(WIFI_STA);
  WiFi.hostname(config.hostname);
  WiFi.begin(config.ssid, config.senha);

  unsigned long inicio = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - inicio < 15000)
  {
    delay(100);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED)
  {
    logaM(LOG_NORMAL, "WiFi conectado!");
    logaM(LOG_NORMAL, "IP: %s", WiFi.localIP().toString().c_str());
  }
  else
  {
    logaM(LOG_AVISO, "Falha ao conectar.");
    wifiStartAP();
  }
}

void apiConfigWifi()
{
  if (!server.hasArg("ssid"))
  {
    server.send(400, "application/json", R"({"ok":false,"msg":"missing ssid"})");
    return;
  }

  if (!server.hasArg("senha"))
  {
    server.send(400, "application/json", R"({"ok":false,"msg":"missing senha"})");
    return;
  }

  String ssid = server.arg("ssid");
  String senha = server.arg("senha");

  strlcpy(config.ssid, ssid.c_str(), sizeof(config.ssid));
  strlcpy(config.senha, senha.c_str(), sizeof(config.senha));

  configSave();

  server.send(200, "application/json", R"({"ok":true,"msg":"wifi configurado"})");

  delay(100);
  WiFi.disconnect();
  delay(100);

  wifiConnect();
}
