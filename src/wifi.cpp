#include "eTomadaLite.h"
#include "platform.h"
#include "loga.h"
#include "wifi.h"
#include "config.h"
#include "util.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga(".WIFI..", nivel, fmt, ##__VA_ARGS__)

extern Config config;
extern ETomadaWebServer server;

bool wifiGetModoAP()
{
  WiFiMode_t mode = WiFi.getMode();
  return mode == WIFI_AP_STA || mode == WIFI_AP;
}

void wifiStartAP()
{
  String apName = "eTomada-";

  apName += String(ESP.getChipId(), HEX);

  logaM(LOG_AVISO, "Iniciando AP: %s", apName.c_str());

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(apName.c_str());

  char ipStr[16];
  utilIPToString(WiFi.softAPIP(), ipStr, 16);
  logaM(LOG_AVISO, "AP IP: %s", ipStr);
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
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.hostname(config.deviceID);
  WiFi.begin(config.ssid, config.senha);

  unsigned long inicio = millis();
  while ((WiFi.status() != WL_CONNECTED || !wifiTemIP()) && millis() - inicio < 15000)
  {
    delay(100);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED)
  {
    logaM(LOG_NORMAL, "WiFi conectado!");
    char ipStr[16];
    utilIPToString(WiFi.localIP(), ipStr, 16);
    logaM(LOG_NORMAL, "IP: %s", ipStr);
  }
  else
  {
    logaM(LOG_AVISO, "Falha ao conectar.");
    wifiStartAP();
  }
}

bool wifiTemIP()
{
  IPAddress ip = WiFi.localIP();

  return ip[0] != 0 ||
         ip[1] != 0 ||
         ip[2] != 0 ||
         ip[3] != 0;
}
