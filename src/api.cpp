#include "eTomadaLite.h"
#include "platform.h"
#include "loga.h"
#include "config.h"
#include "util.h"
#include "wifi.h"
#include "http.h"
#include "botao.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga("..API..", nivel, fmt, ##__VA_ARGS__)

extern ETomadaWebServer server;
extern Config config;

void apiGetSnapshot()
{
  String resposta = eTomadaLiteGetSnapshot();

  server.sendHeader("X-eTomada", "Lite");
  server.sendHeader("X-Versao", eTomadaLiteVersion());
  server.send(200, "application/json", resposta);

  // TODO if ip != SERVER
  // httpLogaRequest("OK");
}

void apiConfig()
{
  if (server.hasArg("hostname"))
  {
    String devID = server.arg("hostname");
    // TODO :: limpar string
    strlcpy(config.deviceID, devID.c_str(), sizeof(config.deviceID));

    configSave();

    server.send(200, "application/json", R"({"ok":true,"msg":"hostname configurado > reset WiFi!"})");
    httpLogaRequest("hostname configurado");

    logaM(LOG_NORMAL, "Hostname configurado. Reconectar");

    logaFlush();

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
    httpLogaRequest("WiFi configurado");

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
    httpLogaRequest("Log Level configurado");

    logaM(LOG_NORMAL, "Log Level setado para [%s]", logaGetNivelTxt((LogLevel)config.logLevel));
  }
  else
  {
    server.send(400, "application/json", R"({"ok":false,"msg":"missing args"})");
    httpLogaRequest("MISSING ARGS");
    return;
  }
}

void apiReset()
{
  server.send(200, "application/json", R"({"ok":true,"msg":"vou reiniciar"})");
  httpLogaRequest("API Reset");
  utilRestart("API reset");
}