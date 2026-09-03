#include "eTomadaLite.h"
#include "loga.h"
#include "config.h"
#include "rele.h"
#include "wifi.h"
#include "http.h"
#include "mdns-gs.h"
#include "led.h"
#include "memoria.h"
#include "mestre.h"

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

  mestreInit();

  logaM(LOG_NORMAL, "Inicializacao concluida.");
}

static uint32_t tsMemLog = 0;
static uint32_t tsSensorLog = 0;
void loop()
{
  if (millis() - tsMemLog > 60 * 60 * 1000)
  {
    memoriaLog("1h/1h");
    tsMemLog = millis();
  }

  // if (millis() - tsSensorLog > 1000)
  // {
  //   int sensor = analogRead(A0);
  //   logaM(LOG_NORMAL, "======================");
  //   logaM(LOG_NORMAL, "A0: %d", sensor);
  //   logaM(LOG_NORMAL, "D5: %d", digitalRead(D5));
  //   logaM(LOG_NORMAL, "======================");

  //   if (sensor < 400)
  //     digitalWrite(15, 1);
  //   else
  //     digitalWrite(15, 0);

  //   tsSensorLog = millis();
  // }

  // TODO :: 1s/1s
  mestreLoop();

  mdnsProcessa();
  httpProcessa();
  logaProcessa();
  ledProcessa();

  yield();
}
