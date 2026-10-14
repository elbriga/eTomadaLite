#include "eTomadaLite.h"
#include "recovery.h"
#include "loga.h"
#include "config.h"
#include "rele.h"
#include "wifi.h"
#include "http.h"
#include "led.h"
#include "memoria.h"
#include "mestre.h"
#include "botao.h"
#include "ntp.h"
#include "sensor.h"
#include "backlight.h"

#ifdef HW_EKAZA_6TECLAS
#include "tecladoEkazaSerial.h"
#elif HW_EKAZA_3TECLAS
#include "tecladoEkazaParalelo.h"
#endif

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga("..APP..", nivel, fmt, ##__VA_ARGS__)

static bool modoAP = false;

// ============================================================
// Setup
// ============================================================
void appInit()
{
  Serial.println();
  Serial.println();

  configLoad(); // Inicializar primeiro para usar o deviceID no log
  logaInit();

  logaM(LOG_NORMAL, "==============================");
  logaM(LOG_NORMAL, "       eTomada Lite");
  logaM(LOG_NORMAL, "==============================");

  logaM(LOG_NORMAL, "Hostname: %s", eTomadaLiteDeviceID().c_str());
  logaM(LOG_NORMAL, "Versao: %s", eTomadaLiteVersion().c_str());

  logaM(LOG_NORMAL, "MAC: %s", WiFi.macAddress().c_str());
  logaM(LOG_NORMAL, "Flash: %u bytes", ESP.getFlashChipSize());
#if defined(ESP8266)
  logaM(LOG_NORMAL, "Sketch: %u bytes", ESP.getSketchSize());
  logaM(LOG_NORMAL, "Free sketch: %u bytes", ESP.getFreeSketchSpace());
#endif

  // Hardware
  ledInit();
  backlightInit();
  releInit();
  botoesInit();
  sensorDigitalInit();

#ifdef HW_EKAZA_6TECLAS
  tecladoEkazaSerialInit();
#elif HW_EKAZA_3TECLAS
  tecladoEkazaParaleloInit();
#endif

  wifiConnect();
  modoAP = wifiGetModoAP();

  httpInit();

  if (!modoAP)
  {
    ntpInit();
    mestreInit();
  }

  ledSet(false);
  logaM(LOG_NORMAL, "Inicializacao concluida.");
}

static time_t ultimoSegundo = -1;
static time_t tsTimerHora = 0;
void appLoop()
{
  struct tm timeinfo;
  time_t now = time(nullptr);
  localtime_r(&now, &timeinfo);

  // 1s/1s
  if (ultimoSegundo != timeinfo.tm_sec)
  {
    ultimoSegundo = timeinfo.tm_sec;

    sensorDigitalProcessa();

    // 1h/1h
    if (now - tsTimerHora > 60 * 60)
    {
      memoriaLog("1h/1h");
      tsTimerHora = now;
    }
  }

  // 5ms/5ms
  ledProcessa();
  botaoProcessa();

#ifdef HW_EKAZA_6TECLAS
  tecladoEkazaProcessa();
#elif HW_EKAZA_3TECLAS
  tecladoEkazaParaleloLoop();
#endif

  httpProcessa();
  if (!modoAP)
    logaProcessa();

  delay(5);
}
