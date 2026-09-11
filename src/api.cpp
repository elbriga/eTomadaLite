#include "eTomadaLite.h"
#include "platform.h"
#include "loga.h"
#include "rele.h"
#include "config.h"
#include "util.h"
#include "wifi.h"
#include "mestre.h"
#include "sensor.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga("..API..", nivel, fmt, ##__VA_ARGS__)

extern ETomadaWebServer server;
extern Config config;

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

  resposta += F(",\"mac\":\"");
  resposta += WiFi.macAddress();

  resposta += F("\",\"ip\":\"");
  char ipBuffer[16];
  utilIPToString(WiFi.localIP(), ipBuffer, 16);
  resposta += ipBuffer;

  resposta += F("\",\"ssid\":\"");
  resposta += WiFi.SSID();

  resposta += F("\",\"wifiPower\":");
  resposta += String(WiFi.RSSI());

  resposta += F(",\"recursos\":[");

  resposta += F("{\"id\":\"R1\",\"device\":{\"estado\":");
  resposta += releGetEstado() ? "1" : "0";
  resposta += F("}},");

  resposta += F("{\"id\":\"S1\",\"device\":{\"valor\":");
  resposta += sensorDigitalGetEstado() ? "1" : "0";
  resposta += F("}}");

  resposta += F("]}");

  server.send(200, "application/json", resposta);
}

void apiConfig()
{
  if (server.hasArg("hostname"))
  {
    String devID = server.arg("hostname");
    // TODO :: limpar string
    strlcpy(config.deviceID, devID.c_str(), sizeof(config.deviceID));

    configSave();

    server.send(200, "application/json", R"({"ok":true,"msg":"hostname configurado"})");
    logaM(LOG_NORMAL, "Hostname configurado. Reconectar");

    delay(100);
    wifiConnect();
  }
  else if (server.hasArg("ssid") && server.hasArg("senha"))
  {
    String ssid = server.arg("ssid");
    String senha = server.arg("senha");

    strlcpy(config.ssid, ssid.c_str(), sizeof(config.ssid));
    strlcpy(config.senha, senha.c_str(), sizeof(config.senha));

    configSave();

    server.send(200, "application/json", R"({"ok":true,"msg":"wifi configurado > reconectar"})");
    logaM(LOG_NORMAL, "WiFi configurado. Reconectar");

    delay(100);
    wifiConnect();
  }
  else if (server.hasArg("logLevel"))
  {
    int level = server.arg("logLevel").toInt();
    if (level != config.logLevel)
    {
      config.logLevel = logaSetLevel((LogLevel)level);
      configSave();
    }
    server.send(200, "application/json", R"({"ok":true,"msg":"logLevel configurado"})");
    logaM(LOG_NORMAL, "Log Level setado para [%s]", logaGetNivelTxt((LogLevel)config.logLevel));
  }
  else if (server.hasArg("mestre"))
  {
    String mestre = server.arg("mestre");
    // TODO :: limpar string
    strlcpy(config.mestre, mestre.c_str(), sizeof(config.mestre));

    configSave();

    server.send(200, "application/json", R"({"ok":true,"msg":"mestre configurado"})");
    logaM(LOG_NORMAL, "Mestre configurado. Reinit mestre");

    mestreInit();
  }
  else
  {
    server.send(400, "application/json", R"({"ok":false,"msg":"missing args"})");
    return;
  }
}

void apiReset()
{
  server.send(200, "application/json", R"({"ok":true,"msg":"vou reiniciar"})");
  utilRestart("API reset");
}