#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <Updater.h>

#include "eTomadaLite.h"
#include "loga.h"
#include "config.h"
#include "rele.h"

#define ETOMADA_LITE_VERSAO "0.0.7"
// 0.0.6 - led por TS
// 0.0.7 - logaM e log remoto

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga(".MAIN..", nivel, fmt, ##__VA_ARGS__)

void logaProcessa();

extern Config config;

ESP8266WebServer server(80);

const char *otaErroMsg = nullptr;
size_t otaTamanhoEsperado = 0;

// ============================================================
// WiFi
// ============================================================
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

// ============================================================
// API
// ============================================================
void apiGetSnapshot()
{
  String resposta;

  resposta.reserve(256);

  resposta = F("{\"device\":\"eTomada\",");

  resposta += F("\"fw_version\":\"");
  resposta += ETOMADA_LITE_VERSAO;

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

// ============================================================
// API OTA
// ============================================================
void apiOtaFlash()
{
  HTTPUpload &upload = server.upload();

  if (upload.status == UPLOAD_FILE_START)
  {
    otaErroMsg = nullptr;
    otaTamanhoEsperado = 0;

    if (!server.hasArg("tamanho"))
    {
      logaM(LOG_AVISO, "OTA: parametro tamanho ausente");
      otaErroMsg = "parametro tamanho ausente";
      return;
    }

    otaTamanhoEsperado = server.arg("tamanho").toInt();

    logaM(LOG_NORMAL, "OTA iniciando: %s (%u bytes)", upload.filename.c_str(), otaTamanhoEsperado);

    if (otaTamanhoEsperado == 0)
    {
      otaErroMsg = "tamanho invalido";
      return;
    }

    if (otaTamanhoEsperado > ESP.getFreeSketchSpace())
    {
      otaErroMsg = "firmware grande demais";
      logaM(LOG_NORMAL, "OTA: tamanho=%u free=%u\n", otaTamanhoEsperado, ESP.getFreeSketchSpace());
      return;
    }

    if (!Update.begin(otaTamanhoEsperado))
    {
      otaErroMsg = "Update.begin falhou";
      logaM(LOG_AVISO, "OTA: Update.begin falhou");
      Update.printError(Serial);
      return;
    }
  }

  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    // Se já ocorreu algum erro, simplesmente ignora os chunks restantes.
    if (otaErroMsg != nullptr)
      return;

    size_t gravado =
        Update.write(upload.buf, upload.currentSize);

    Serial.print(".");

    if (gravado != upload.currentSize)
    {
      otaErroMsg = "erro ao gravar";
      logaM(LOG_CRITICO, "OTA: erro ao gravar");
      Update.printError(Serial);
    }
  }

  else if (upload.status == UPLOAD_FILE_END)
  {
    if (otaErroMsg != nullptr)
      return;

    logaM(LOG_AVISO, "OTA recebido: %u bytes", upload.totalSize);

    if (upload.totalSize != otaTamanhoEsperado)
    {
      logaM(LOG_AVISO, "OTA: esperado=%u recebido=%u\n", otaTamanhoEsperado, upload.totalSize);
      otaErroMsg = "tamanho recebido diferente";
      Update.end();
      return;
    }

    if (!Update.end(true))
    {
      logaM(LOG_AVISO, "OTA: Update.end falhou");
      otaErroMsg = "Update.end falhou";
      Update.printError(Serial);
      return;
    }

    logaM(LOG_AVISO, "OTA concluido!");
  }

  else if (upload.status == UPLOAD_FILE_ABORTED)
  {
    logaM(LOG_AVISO, "OTA abortado");
    otaErroMsg = "upload abortado";
    if (Update.isRunning())
      Update.end();
  }
}

// ============================================================
// Web server
// ============================================================
void apiOtaFlashHelper()
{
  if (otaErroMsg != nullptr)
  {
    String resposta = "{\"ok\":false,\"msg\":\"";
    resposta += otaErroMsg;
    resposta += "\"}";

    server.send(400, "application/json", resposta);
    return;
  }

  server.send(200, "application/json", R"({"ok":true,"msg":"ota ok > restart"})");
  delay(500);
  ESP.restart();
}

void webRoot()
{
  server.send(200, "text/plain", "eTomada Lite");
}

void webInit()
{
  server.on("/api/getSnapshot", HTTP_GET, apiGetSnapshot);

  server.on("/api/configWifi", HTTP_GET, apiConfigWifi);

  server.on("/api/setRele", HTTP_GET, apiSetRele);

  server.on("/api/ota", HTTP_POST, apiOtaFlashHelper, apiOtaFlash);

  server.on("/", HTTP_GET, webRoot);

  server.begin();

  logaM(LOG_NORMAL, "HTTP server iniciado.");
}

// ============================================================
// Setup
// ============================================================
void setup()
{
  Serial.begin(115200);

  delay(100);

  logaInit();

  Serial.println();
  Serial.println();
  logaM(LOG_NORMAL, "==============================");
  logaM(LOG_NORMAL, "       eTomada Lite");
  logaM(LOG_NORMAL, "==============================");

  logaM(LOG_NORMAL, "Versao: %s", ETOMADA_LITE_VERSAO);

  logaM(LOG_NORMAL, "Chip ID: %08X", ESP.getChipId());
  logaM(LOG_NORMAL, "Flash: %u bytes", ESP.getFlashChipSize());
  logaM(LOG_NORMAL, "Sketch: %u bytes", ESP.getSketchSize());
  logaM(LOG_NORMAL, "Free sketch: %u bytes", ESP.getFreeSketchSpace());

  // Hardware
  pinMode(LED_GPIO, OUTPUT);   // Led Azul do MINI
  digitalWrite(LED_GPIO, LOW); // LOW = Aceso!
  releInit();

  configLoad();
  wifiConnect();
  webInit();

  logaM(LOG_NORMAL, "Inicializacao concluida.");
}

bool ledState;
void ledProcessa()
{
  // Sincronizado com o segundo!
  struct timeval tv;
  gettimeofday(&tv, nullptr);

  bool estado = tv.tv_usec > 100000;

  if (estado != ledState)
  {
    ledState = estado;
    digitalWrite(LED_GPIO, ledState);
  }
}

void loop()
{
  server.handleClient();

  logaProcessa();

  ledProcessa();

  yield();
}
