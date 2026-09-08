#include "eTomadaLite.h"
#include "app.h"
#include "recovery.h"
#include "loga.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga(".MAIN..", nivel, fmt, ##__VA_ARGS__)

static bool modoRecovery = false;

// ============================================================
// Setup
// ============================================================
void setup()
{
  Serial.begin(115200);
  delay(100);

  Serial.println();
  Serial.println();

  modoRecovery = recoveryBoot();
  if (modoRecovery)
  {
    recoveryInit();
    return;
  }

  appInit();
}

void loop()
{
  if (modoRecovery)
  {
    recoveryLoop();
    delay(5);
    return;
  }
  recoveryBootTick();

  appLoop();

  yield();
}
