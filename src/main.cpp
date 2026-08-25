#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <Updater.h>

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

  Serial.printf("Rele: %s\n",
                ligado ? "ON" : "OFF");
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
void apiConfigWifi()
{
  if (!server.hasArg("ssid"))
  {
    server.send(
        400,
        "application/json",
        R"({"ok":false,"msg":"missing ssid"})");
    return;
  }

  if (!server.hasArg("senha"))
  {
    server.send(
        400,
        "application/json",
        R"({"ok":false,"msg":"missing senha"})");
    return;
  }

  String ssid = server.arg("ssid");
  String senha = server.arg("senha");

  strlcpy(config.ssid, ssid.c_str(), sizeof(config.ssid));
  strlcpy(config.senha, senha.c_str(), sizeof(config.senha));

  configSave();

  server.send(
      200,
      "application/json",
      R"({"ok":true,"msg":"wifi configurado"})");

  delay(100);
  WiFi.disconnect();
  delay(100);

  wifiConnect();
}

void apiSetRele()
{
  if (!server.hasArg("estado"))
  {
    server.send(
        400,
        "application/json",
        R"({"ok":false,"msg":"missing estado"})");
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
    Serial.printf("OTA iniciando: %s\n", upload.filename.c_str());

    uint32_t tamanho = upload.totalSize;
    if (!Update.begin(tamanho))
    {
      Update.printError(Serial);
      return;
    }
  }
  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    size_t gravado =
        Update.write(
            upload.buf,
            upload.currentSize);

    if (gravado != upload.currentSize)
      Update.printError(Serial);
  }
  else if (upload.status == UPLOAD_FILE_END)
  {
    if (Update.end(true))
      Serial.printf("OTA concluido: %u bytes\n", upload.totalSize);
    else
      Update.printError(Serial);
  }
  else if (upload.status == UPLOAD_FILE_ABORTED)
  {
    Update.end();

    Serial.println("OTA abortado.");
  }

  yield();
}

// ============================================================
// Web server
// ============================================================
void webInit()
{
  server.on("/api/configWifi", HTTP_GET, apiConfigWifi);

  server.on("/api/setRele", HTTP_GET, apiSetRele);

  server.on("/api/ota_flash", HTTP_POST, []()
            {
        if (Update.hasError())
        {
          server.send(
              500,
              "application/json",
              R"({"ok":false,"msg":"ota failed"})");
          return;
        }

        server.send(
            200,
            "application/json",
            R"({"ok":true,"msg":"ota ok"})");
        delay(1000);
        ESP.restart(); }, apiOtaFlash);

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

  Serial.printf("Chip ID: %08X\n", ESP.getChipId());
  Serial.printf("Flash: %u bytes\n", ESP.getFlashChipSize());
  Serial.printf("Sketch: %u bytes\n", ESP.getSketchSize());
  Serial.printf("Free sketch: %u bytes\n", ESP.getFreeSketchSpace());

  // Hardware
  pinMode(RELE_GPIO, OUTPUT);
  releSet(false);

  configLoad();

  wifiConnect();

  webInit();

  Serial.println("Inicializacao concluida.");
}

void loop()
{
  server.handleClient();
  yield();
}
