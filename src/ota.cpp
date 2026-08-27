#include <Updater.h>
#include <ESP8266WebServer.h>

#include "eTomadaLite.h"
#include "loga.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga("..OTA..", nivel, fmt, ##__VA_ARGS__)

extern ESP8266WebServer server;

static const char *otaErroMsg = nullptr;
static size_t otaTamanhoEsperado = 0;

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
