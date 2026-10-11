#include <sys/time.h>

#include "eTomadaLite.h"
#include "loga.h"
#include "led.h"
#include "hardwareProfile.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga(".LED...", nivel, fmt, ##__VA_ARGS__)

extern const HardwareProfile hardwareProfile;

static bool ledState;
static bool piscaAtivo;

void ledInit()
{
  if (!ledAtivo())
    return;

  logaM(LOG_NORMAL, "Ativando led de status em [%d]", hardwareProfile.led.pin);
  pinMode(hardwareProfile.led.pin, OUTPUT);                              // Led Azul do MINI
  digitalWrite(hardwareProfile.led.pin, !hardwareProfile.led.invertido); // No MINI LOW = Aceso!

  piscaAtivo = hardwareProfile.led.pisca;
}

bool ledAtivo()
{
  return hardwareProfile.led.pin != 255;
}

void ledSet(bool estado)
{
  if (!ledAtivo())
    return;

  ledState = estado;
  digitalWrite(hardwareProfile.led.pin, hardwareProfile.led.invertido ? !estado : estado);
}

void ledProcessa()
{
  if (!piscaAtivo)
    return;

  // Sincronizado com o segundo!
  struct timeval tv;
  gettimeofday(&tv, nullptr);

  bool estado = tv.tv_usec < 100000; // && !((tv.tv_usec % 200000) % 2);

  if (estado != ledState)
  {
    ledState = estado;
    digitalWrite(hardwareProfile.led.pin, hardwareProfile.led.invertido ? !ledState : ledState);
  }
}
