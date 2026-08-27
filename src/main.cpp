#include "eTomadaLite.h"
#include "loga.h"
#include "config.h"
#include "rele.h"
#include "wifi.h"
#include "http.h"
#include "led.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga(".MAIN..", nivel, fmt, ##__VA_ARGS__)

void logaProcessa();

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

  logaM(LOG_NORMAL, "Versao: %s", eTomadaLiteVersion().c_str());

  logaM(LOG_NORMAL, "Chip ID: %08X", ESP.getChipId());
  logaM(LOG_NORMAL, "Flash: %u bytes", ESP.getFlashChipSize());
  logaM(LOG_NORMAL, "Sketch: %u bytes", ESP.getSketchSize());
  logaM(LOG_NORMAL, "Free sketch: %u bytes", ESP.getFreeSketchSpace());

  // Hardware
  ledInit();
  releInit();

  configLoad();
  wifiConnect();
  httpInit();

  logaM(LOG_NORMAL, "Inicializacao concluida.");
}

void loop()
{
  httpProcessa();
  logaProcessa();
  ledProcessa();

  yield();
}
