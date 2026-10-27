#include "eTomadaLite.h"
#include "server.h"
#include "loga.h"
#include "config.h"
#include "rele.h"
#include "wifi.h"
#include "http.h"
#include "led.h"
#include "memoria.h"
#include "botao.h"
#include "ntp.h"
#include "sensor.h"
#include "backlight.h"
#include "hardwareProfile.h"

#ifdef HW_EKAZA_6TECLAS
#include "tecladoEkazaSerial.h"
#elif HW_EKAZA_3TECLAS
#include "tecladoEkazaParalelo.h"
#elif HW_EKAZA_3TECLAS2
#include "tecladoEkazaParalelo.h"
#endif

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga("..APP..", nivel, fmt, ##__VA_ARGS__)

extern const HardwareProfile hardwareProfile;

static bool modoAP = false;

// ============================================================
// Setup
// ============================================================
void appInit()
{
  Serial.println();
  Serial.println();

  serverInit();

  configLoad(); // Inicializar primeiro para usar o deviceID no log
  logaInit();

  logaM(LOG_NORMAL, "==============================");
  logaM(LOG_NORMAL, "       eTomada Lite");
  logaM(LOG_NORMAL, "==============================");

  logaM(LOG_NORMAL, "Hostname: %s", eTomadaLiteDeviceID().c_str());
  logaM(LOG_NORMAL, "Versao: %s", eTomadaLiteVersion().c_str());
  logaM(LOG_NORMAL, "Log Level: %s", logaGetNivelAtual());

  logaM(LOG_NORMAL, "Modelo: %s", hardwareProfile.modelo);
  logaM(LOG_NORMAL, "Board: %s", hardwareProfile.board);

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
  tecladoEkazaParaleloInit(14, 16, 17);
#elif HW_EKAZA_3TECLAS2
  tecladoEkazaParaleloInit(PIN_P23, PIN_P9, PIN_P28, PIN_P14, PIN_P16, PIN_P20);
#endif

  // Rede
  wifiConnect();
  modoAP = wifiGetModoAP();

  httpInit();

  if (!modoAP)
  {
    ntpInit();

    serverEnviaSnapshot();

    sensorDigitalEnviaEvento();
  }

  ledSet(false);
  logaM(LOG_NORMAL, "Inicializacao concluida.");
}

void appLoop()
{
  static time_t ultimoSegundo = -1;
  static time_t ultimaHora = -1;

  struct tm timeinfo;
  time_t now = time(nullptr);
  localtime_r(&now, &timeinfo);

  // 1s/1s
  if (ultimoSegundo != timeinfo.tm_sec)
  {
    ultimoSegundo = timeinfo.tm_sec;

    sensorDigitalProcessa();

    // 1h/1h
    if (ultimaHora != timeinfo.tm_hour)
    {
      ultimaHora = timeinfo.tm_hour;

      memoriaLog("1h/1h");
    }
  }

  // 5ms/5ms
  ledProcessa();
  botaoProcessa();

#ifdef HW_EKAZA_6TECLAS
  tecladoEkazaSerialProcessa();
#elif HW_EKAZA_3TECLAS
  tecladoEkazaParaleloLoop();
#elif HW_EKAZA_3TECLAS2
  tecladoEkazaParaleloLoop();
#endif

  httpProcessa();

  if (!modoAP)
    logaProcessa();

  delay(5);
}
