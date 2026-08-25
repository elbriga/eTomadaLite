#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <Updater.h>

#define ETOMADA_LITE_VERSAO "0.0.4"

#define ETOMADA_LITE_DEVICE_ID "MINI01" // TODO : config
#define ETOMADA_LITE_DEVICE_MODEL "MINI"
#define ETOMADA_LITE_DEVICE_BOARD "esp01_1m"

// ============================================================
// Configuração
// ============================================================

#define EEPROM_SIZE 256
#define CONFIG_MAGIC 0x45544F4DUL // "ETOM"

struct Config
{
  uint32_t magic;

  char ssid[64];
  char senha[64];

  char hostname[32];
};

Config config;

ESP8266WebServer server(80);

bool releLigado = false;

const char *otaErroMsg = nullptr;
size_t otaTamanhoEsperado = 0;

// ============================================================
// Configuração
// ============================================================
void configDefaults()
{
  memset(&config, 0, sizeof(config));

  config.magic = CONFIG_MAGIC;
  strlcpy(config.hostname,
          "etomada-lite",
          sizeof(config.hostname));
}

bool configLoad()
{
  EEPROM.begin(EEPROM_SIZE);

  EEPROM.get(0, config);
  if (config.magic != CONFIG_MAGIC)
  {
    Serial.println("Configuracao inexistente.");
    configDefaults();
    return false;
  }

  Serial.println("Configuracao carregada.");
  Serial.printf("SSID: %s\n", config.ssid);
  Serial.printf("Hostname: %s\n", config.hostname);

  return true;
}

void configSave()
{
  EEPROM.put(0, config);
  EEPROM.commit();

  Serial.println("Configuracao salva.");
}

// ============================================================
// Relé
// ============================================================

void releSet(bool ligado)
{
  releLigado = ligado;

  digitalWrite(RELE_GPIO, ligado);

  Serial.printf("Rele: %s\n", ligado ? "ON" : "OFF");
}

// ============================================================
// WiFi
// ============================================================

void wifiStartAP()
{
  String apName = "eTomada-";

  apName += String(ESP.getChipId(), HEX);

  Serial.printf("Iniciando AP: %s\n", apName.c_str());

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(apName.c_str());

  Serial.printf("AP IP: %s\n", WiFi.softAPIP().toString().c_str());
}

void wifiConnect()
{
  if (config.ssid[0] == '\0')
  {
    Serial.println("Nenhuma rede WiFi configurada.");
    wifiStartAP();
    return;
  }

  Serial.printf("Conectando em: %s\n", config.ssid);

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
    Serial.printf("WiFi conectado!\n");
    Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
  }
  else
  {
    Serial.println("Falha ao conectar.");
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

  resposta = F("{\"fw_version\":\"");
  resposta += ETOMADA_LITE_VERSAO;

  resposta += F("\",\"device_id\":\"");
  resposta += String(ETOMADA_LITE_DEVICE_ID);
  resposta += F("\",\"device_model\":\"");
  resposta += String(ETOMADA_LITE_DEVICE_MODEL);
  resposta += F("\",\"device_board\":\"");
  resposta += String(ETOMADA_LITE_DEVICE_BOARD);

  resposta += F("\",\"uptime\":");
  resposta += millis();

  resposta += F(",\"rele\":");
  resposta += releLigado ? "1" : "0";

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

void apiSetRele()
{
  if (!server.hasArg("estado"))
  {
    server.send(400, "application/json", R"({"ok":false,"msg":"missing estado"})");
    return;
  }

  String estado = server.arg("estado");

  bool ligado =
      estado == "1" ||
      estado == "on" ||
      estado == "ON" ||
      estado == "true";

  releSet(ligado);

  String resposta;

  resposta.reserve(64);

  resposta = F("{\"ok\":true,\"rele\":");
  resposta += releLigado ? "1" : "0";
  resposta += "}";

  server.send(200, "application/json", resposta);
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
      Serial.println("OTA: parametro tamanho ausente");
      otaErroMsg = "parametro tamanho ausente";
      return;
    }

    otaTamanhoEsperado = server.arg("tamanho").toInt();

    Serial.printf(
        "OTA iniciando: %s (%u bytes)\n",
        upload.filename.c_str(),
        otaTamanhoEsperado);

    if (otaTamanhoEsperado == 0)
    {
      otaErroMsg = "tamanho invalido";
      return;
    }

    if (otaTamanhoEsperado > ESP.getFreeSketchSpace())
    {
      otaErroMsg = "firmware grande demais";
      Serial.printf(
          "OTA: tamanho=%u free=%u\n",
          otaTamanhoEsperado,
          ESP.getFreeSketchSpace());

      return;
    }

    if (!Update.begin(otaTamanhoEsperado))
    {
      otaErroMsg = "Update.begin falhou";
      Serial.println("OTA: Update.begin falhou");
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
      Serial.println("OTA: erro ao gravar");
      Update.printError(Serial);
    }
  }

  else if (upload.status == UPLOAD_FILE_END)
  {
    if (otaErroMsg != nullptr)
      return;

    Serial.printf("\nOTA recebido: %u bytes\n", upload.totalSize);

    if (upload.totalSize != otaTamanhoEsperado)
    {
      otaErroMsg = "tamanho recebido diferente";

      Serial.printf(
          "OTA: esperado=%u recebido=%u\n",
          otaTamanhoEsperado,
          upload.totalSize);

      Update.end();
      return;
    }

    if (!Update.end(true))
    {
      otaErroMsg = "Update.end falhou";

      Serial.println("OTA: Update.end falhou");
      Update.printError(Serial);
      return;
    }

    Serial.println("OTA concluido!");
  }

  else if (upload.status == UPLOAD_FILE_ABORTED)
  {
    Serial.println("\nOTA abortado");
    otaErroMsg = "upload abortado";

    if (Update.isRunning())
      Update.end();
  }
}

// ============================================================
// Web server
// ============================================================
void webInit()
{
  server.on("/api/getSnapshot", HTTP_GET, apiGetSnapshot);

  server.on("/api/configWifi", HTTP_GET, apiConfigWifi);

  server.on("/api/setRele", HTTP_GET, apiSetRele);

  server.on("/api/ota_flash", HTTP_POST, []()
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
        ESP.restart(); },

            apiOtaFlash);

  server.on("/", HTTP_GET, []()
            { server.send(200, "text/plain", "eTomada Lite"); });

  server.begin();

  Serial.println("HTTP server iniciado.");
}

// ============================================================
// Setup
// ============================================================
void setup()
{
  Serial.begin(115200);

  delay(100);

  Serial.println();
  Serial.println();
  Serial.println("==============================");
  Serial.println("       eTomada Lite");
  Serial.println("==============================");

  Serial.printf("Versao: %s\n", ETOMADA_LITE_VERSAO);

  Serial.printf("Chip ID: %08X\n", ESP.getChipId());
  Serial.printf("Flash: %u bytes\n", ESP.getFlashChipSize());
  Serial.printf("Sketch: %u bytes\n", ESP.getSketchSize());
  Serial.printf("Free sketch: %u bytes\n", ESP.getFreeSketchSpace());

  // Hardware
  pinMode(LED_GPIO, OUTPUT);
  pinMode(RELE_GPIO, OUTPUT);
  releSet(false);

  configLoad();
  wifiConnect();
  webInit();

  Serial.println("Inicializacao concluida.");
}

bool ledState;
unsigned long ledTimer = 0;
void ledProcessa()
{
  if (millis() - ledTimer < 500)
    return;

  ledTimer = millis();
  ledState = !ledState;
  digitalWrite(LED_GPIO, ledState);
}

void loop()
{
  server.handleClient();

  ledProcessa();

  yield();
}
