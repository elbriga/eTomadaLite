#include "eTomadaLite.h"
#include "platform.h"
#include "loga.h"
#include "util.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga("..OTA..", nivel, fmt, ##__VA_ARGS__)

extern ETomadaWebServer server;

static const char *otaErroMsg = nullptr;
static size_t otaTamanhoEsperado = 0;
static int otaTamanhoAtual = 0;
static int otaDownloadUltimoPercentual = -1;

static void otaAbort()
{
#if defined(ESP8266)
  if (Update.isRunning())
    Update.end();
#else
  if (Update.isRunning())
    Update.abort();
#endif
}

void apiOtaFlash()
{
  HTTPUpload &upload = server.upload();

  switch (upload.status)
  {
  case UPLOAD_FILE_START:
  {
    otaErroMsg = nullptr;
    otaTamanhoEsperado = 0;
    otaTamanhoAtual = 0;
    otaDownloadUltimoPercentual = -1;

    if (!server.hasArg("tamanho"))
    {
      otaErroMsg = "parametro tamanho ausente";
      logaM(LOG_AVISO, "OTA: %s", otaErroMsg);
      return;
    }

    otaTamanhoEsperado = server.arg("tamanho").toInt();

    logaM(LOG_NORMAL, "OTA iniciando: %s (%u bytes)", upload.filename.c_str(), (unsigned)otaTamanhoEsperado);

    if (!otaTamanhoEsperado)
    {
      otaErroMsg = "tamanho invalido";
      return;
    }

#if defined(ESP8266)
    if (otaTamanhoEsperado > ESP.getFreeSketchSpace())
    {
      otaErroMsg = "firmware grande demais";
      logaM(LOG_AVISO, "OTA: tamanho=%u free=%u", (unsigned)otaTamanhoEsperado, (unsigned)ESP.getFreeSketchSpace());
      return;
    }
#else
    // LibreTiny OTA recebe firmware UF2.
    // UF2 é formado por blocos de 512 bytes.
    if (otaTamanhoEsperado % 512)
    {
      otaErroMsg = "arquivo nao parece UF2";
      logaM(LOG_AVISO, "OTA: tamanho nao multiplo de 512");
      return;
    }
#endif

    if (!Update.begin(otaTamanhoEsperado, U_FLASH))
    {
      otaErroMsg = "Update.begin falhou";

#if defined(ESP8266)
      logaM(LOG_AVISO, "OTA: Update.begin falhou");
      Update.printError(Serial);
#else
      logaM(LOG_AVISO, "OTA: Update.begin falhou: %s",
            Update.errorString());
#endif

      return;
    }

    break;
  }

  case UPLOAD_FILE_WRITE:
  {
    if (otaErroMsg != nullptr)
      return;

    if (!Update.isRunning())
    {
      otaErroMsg = "Update nao esta em execucao";
      return;
    }

    size_t gravado = Update.write(upload.buf, upload.currentSize);
    if (gravado != upload.currentSize)
    {
      otaErroMsg = "erro ao gravar";

#if !defined(ESP8266)
      logaM(LOG_CRITICO, "OTA: erro ao gravar: %s",
            Update.errorString());
#else
      logaM(LOG_CRITICO, "OTA: erro ao gravar");
#endif

      return;
    }

    // Loga somente a cada 10%
    otaTamanhoAtual += gravado;
    int percentual = (otaTamanhoAtual * 100) / otaTamanhoEsperado;
    if (percentual / 10 != otaDownloadUltimoPercentual / 10)
    {
      otaDownloadUltimoPercentual = percentual;

      logaM(LOG_NORMAL,
            "OTA: %d%% (%u/%u bytes)",
            percentual,
            (unsigned)otaTamanhoAtual,
            (unsigned)otaTamanhoEsperado);
    }

    break;
  }

  case UPLOAD_FILE_END:
  {
    if (otaErroMsg != nullptr)
      return;

    logaM(LOG_AVISO, "OTA recebido: %u bytes", (unsigned)upload.totalSize);

    if (upload.totalSize != otaTamanhoEsperado)
    {
      otaErroMsg = "tamanho recebido diferente";

      logaM(LOG_AVISO, "OTA: esperado=%u recebido=%u",
            (unsigned)otaTamanhoEsperado,
            (unsigned)upload.totalSize);

      if (Update.isRunning())
        otaAbort();

      return;
    }

    if (!Update.isRunning())
    {
      otaErroMsg = "Update nao esta em execucao";
      return;
    }

    if (!Update.end())
    {
      otaErroMsg = "Update.end falhou";

#if defined(ESP8266)
      logaM(LOG_AVISO, "OTA: Update.end falhou");
      Update.printError(Serial);
#else
      logaM(LOG_AVISO, "OTA: Update.end falhou: %s",
            Update.errorString());
#endif

      return;
    }

    logaM(LOG_AVISO, "OTA concluido!");

    break;
  }

  case UPLOAD_FILE_ABORTED:
  {
    logaM(LOG_AVISO, "OTA abortado");
    otaErroMsg = "upload abortado";
    if (Update.isRunning())
      otaAbort();

    break;
  }
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

  utilRestart();
}
