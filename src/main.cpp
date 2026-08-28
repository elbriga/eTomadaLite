#include "eTomadaLite.h"
#include "loga.h"
#include "config.h"
#include "rele.h"
#include "wifi.h"
#include "http.h"
#include "mdns-gs.h"
#include "led.h"
#include "memoria.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga(".MAIN..", nivel, fmt, ##__VA_ARGS__)

// ============================================================
// Setup
// ============================================================
void setup()
{
  Serial.begin(115200);

  delay(100);

  configLoad(); // Inicializar primeiro para usar o deviceID no log
  logaInit();

  Serial.println();
  Serial.println();
  logaM(LOG_NORMAL, "==============================");
  logaM(LOG_NORMAL, "       eTomada Lite");
  logaM(LOG_NORMAL, "==============================");

  logaM(LOG_NORMAL, "Hostname: %s", eTomadaLiteDeviceID().c_str());
  logaM(LOG_NORMAL, "Versao: %s", eTomadaLiteVersion().c_str());

  logaM(LOG_NORMAL, "Chip ID: %08X", ESP.getChipId());
  logaM(LOG_NORMAL, "Flash: %u bytes", ESP.getFlashChipSize());
  logaM(LOG_NORMAL, "Sketch: %u bytes", ESP.getSketchSize());
  logaM(LOG_NORMAL, "Free sketch: %u bytes", ESP.getFreeSketchSpace());

  // Hardware
  ledInit();
  releInit();

  wifiConnect();

  httpInit();
  mdnsInit();

  logaM(LOG_NORMAL, "Inicializacao concluida.");
}

static uint32_t tsOla = 0;
void loop()
{
  if (millis() - tsOla > 60 * 60 * 1000)
  {
    memoriaLog("1h/1h");
    tsOla = millis();
  }

  mdnsProcessa();
  httpProcessa();
  logaProcessa();
  ledProcessa();

  yield();
}
