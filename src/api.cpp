#include <ESP8266WebServer.h>

#include "eTomadaLite.h"
#include "loga.h"
#include "rele.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga("..API..", nivel, fmt, ##__VA_ARGS__)

extern ESP8266WebServer server;

void apiGetSnapshot()
{
  String resposta;

  resposta.reserve(256);

  resposta = F("{\"device\":\"eTomada\",");

  resposta += F("\"fw_version\":\"");
  resposta += eTomadaLiteVersion();

  resposta += F("\",\"device_id\":\"");
  resposta += eTomadaLiteDeviceID();
  resposta += F("\",\"device_model\":\"");
  resposta += eTomadaLiteDeviceModel();
  resposta += F("\",\"device_board\":\"");
  resposta += eTomadaLiteDeviceBoard();

  resposta += F("\",\"uptime\":");
  resposta += millis();

  resposta += F(",\"rele\":");
  resposta += releGetEstado() ? "1" : "0";

  resposta += F(",\"mac\":\"");
  resposta += WiFi.macAddress();

  resposta += F("\",\"ip\":\"");
  resposta += WiFi.localIP().toString();

  resposta += F("\",\"ssid\":\"");
  resposta += WiFi.SSID();

  resposta += F("\",\"wifiPower\":");
  resposta += String(WiFi.RSSI());

  resposta += F("}");

  server.send(200, "application/json", resposta);
}
