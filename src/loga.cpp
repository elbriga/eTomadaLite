#include <Arduino.h>
#include <stdarg.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include "eTomadaLite.h"
#include "loga.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga("LOGS", nivel, fmt, ##__VA_ARGS__)

LogLevel logLevel = LOG_DEBUG;

#define LOG_MESSAGE_SIZE 256

#define LOG_SERVER "10.0.0.1:8080"
#define LOG_QUEUE_SIZE 32

struct LogRemoto
{
  time_t timestamp;
  uint32_t uptime; // em segundos -> TODO :: mudar para ms?
  int level;
  char modulo[16];
  char message[LOG_MESSAGE_SIZE];
};

static LogRemoto logQueue[LOG_QUEUE_SIZE];

static uint8_t logQueueInicio = 0;
static uint8_t logQueueFim = 0;
static uint8_t logQueueQuantidade = 0;

static String logaServerURL = "http://" + String(LOG_SERVER) + "/api/log"; // TODO :: configuravel

const char *logaGetNivelTxt(LogLevel nivel);
void logaInit()
{
  logaM(LOG_NORMAL, "Nivel de log: %s", logaGetNivelTxt(logLevel));
  logaM(LOG_NORMAL, "Log Server: %s", logaServerURL.c_str());
}

bool logaRemotoAtivo()
{
  return logaServerURL != "";
}

void loga(const char *modulo, LogLevel nivel, const char *fmt, ...)
{
  va_list args;

  va_start(args, fmt);

  logaV(modulo, nivel, fmt, args);

  va_end(args);
}

static bool logQueuePush(const LogRemoto &log)
{
  if (logQueueQuantidade >= LOG_QUEUE_SIZE)
  {
    // Fila cheia: descarta o log mais antigo.
    logQueueInicio = (logQueueInicio + 1) % LOG_QUEUE_SIZE;
    logQueueQuantidade--;

    Serial.println("Fila de logs cheia, descartando log antigo.");
  }

  logQueue[logQueueFim] = log;

  logQueueFim = (logQueueFim + 1) % LOG_QUEUE_SIZE;
  logQueueQuantidade++;

  return true;
}

static bool logQueuePop(LogRemoto &log)
{
  if (logQueueQuantidade == 0)
    return false;

  log = logQueue[logQueueInicio];

  logQueueInicio = (logQueueInicio + 1) % LOG_QUEUE_SIZE;
  logQueueQuantidade--;

  return true;
}

const char *logaGetNivelTxt(LogLevel nivel)
{
  switch (nivel)
  {
  case LOG_DESATIVADO:
    return "!OFF!!";
  case LOG_CRITICO:
    return "!CRIT!";
  case LOG_AVISO:
    return "AVISO!";
  case LOG_NORMAL:
    return "NORMAL";
  case LOG_DEBUG0:
    return "DEBUG0";
  case LOG_DEBUG:
    return "DEBUG!";
  case LOG_TESTE:
    return "TESTE!";
  default:
    return "??????";
  }
}

void logaV(const char *modulo, LogLevel nivel, const char *fmt, va_list args)
{
  //  esp_task_wdt_reset(); // alimenta o watchdog

  if (nivel == LOG_DESATIVADO || nivel > logLevel)
    // ignorar
    return;

  // Gerar o log
  char msg[LOG_MESSAGE_SIZE];
  vsnprintf(msg, sizeof(msg), fmt, args);

  // Obter horario
  struct tm timeinfo;
  time_t now = time(nullptr);
  localtime_r(&now, &timeinfo);

  char formattedTime[32] = {0};
  strftime(formattedTime, sizeof(formattedTime), "%d/%m/%Y %H:%M:%S", &timeinfo);

  char formattedUptime[32] = {0};
  uint32_t uptime = millis() / 1000;
  int dias = uptime / 86400;
  int horas = (uptime % 86400) / 3600;
  int minutos = (uptime % 3600) / 60;
  int segundos = uptime % 60;
  if (dias > 0)
  {
    snprintf(formattedUptime, sizeof(formattedUptime), "%dd %s%d:%s%d:%s%d",
             dias,
             (horas < 10 ? "0" : ""), horas,
             (minutos < 10 ? "0" : ""), minutos,
             (segundos < 10 ? "0" : ""), segundos);
  }
  else
  {
    snprintf(formattedUptime, sizeof(formattedUptime), "%s%d:%s%d:%s%d",
             (horas < 10 ? "0" : ""), horas,
             (minutos < 10 ? "0" : ""), minutos,
             (segundos < 10 ? "0" : ""), segundos);
  }

  Serial.printf("[%s][%s][%s][%s] %s\n",
                formattedTime, formattedUptime,
                logaGetNivelTxt(nivel), modulo,
                msg);

  // Enviar para fila de log remoto
  if (logaRemotoAtivo())
  {
    LogRemoto log;

    log.timestamp = now;
    log.uptime = uptime;
    log.level = nivel;

    strlcpy(log.modulo, modulo, sizeof(log.modulo));
    strlcpy(log.message, msg, sizeof(log.message));

    logQueuePush(log);
  }
}

void logaTitulo(const char *msg)
{
  loga("eTomada", LOG_AVISO, "\n====\n== %s ==\n====\n", msg);
}

bool logaProcessaPop();
static uint32_t logaProximoLogRemoto = 0;
void logaProcessa()
{
  if (!logaRemotoAtivo())
    return;

  if ((int32_t)(millis() - logaProximoLogRemoto) < 0) // Nao martelar o servidor!
    return;

  if (WiFi.status() != WL_CONNECTED)
    return;

  logaProcessaPop();
}

bool logaProcessaPop()
{
  LogRemoto log;
  if (!logQueuePop(log))
    return false;

  // TODO :: Usar apiInterna
  HTTPClient http;
  WiFiClient client;
  if (!http.begin(client, logaServerURL))
  {
    Serial.println("Nao foi possivel iniciar HTTP para log remoto.");
    return false;
  }

  http.setTimeout(1000);
  http.addHeader("Content-Type", "application/json");

  String body;
  body.reserve(512);

  body = F("{\"deviceID\":\"");
  body += eTomadaLiteDeviceID();
  body += F("\",\"timestamp\":");
  body += log.timestamp;

  body += F(",\"uptime\":");
  body += log.uptime;

  body += F(",\"level\":");
  body += log.level;

  body += F(",\"module\":\"");
  body += log.modulo;

  body += F("\",\"message\":\"");
  body += log.message;

  body += F("\"}");

  yield();

  int status = http.POST(body);
  http.end();

  if (status == 200)
    logaProximoLogRemoto = millis() + 10;
  else
  {
    Serial.printf("POST de log remoto falhou: %d\n", status);
    logaProximoLogRemoto = millis() + 5 * 1000;
    return false;
  }

  return true;
}

void logaFlush()
{
  logaM(LOG_DEBUG0, "Flush dos logs");
  while (logaProcessaPop())
    delay(10);
}