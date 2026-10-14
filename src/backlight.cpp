#include <sys/time.h>

#include "eTomadaLite.h"
#include "loga.h"
#include "led.h"
#include "hardwareProfile.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga(".BKLIT.", nivel, fmt, ##__VA_ARGS__)

extern const HardwareProfile hardwareProfile;

bool backlightState;

bool backlightAtivo()
{
  return hardwareProfile.backlight.pin != 255;
}

void backlightInit()
{
  if (!backlightAtivo())
    return;

  logaM(LOG_NORMAL, "Ativando backlight em [%d]", hardwareProfile.backlight.pin);

  backlightState = true;
  pinMode(hardwareProfile.backlight.pin, OUTPUT);                                    // Led Azul do MINI
  digitalWrite(hardwareProfile.backlight.pin, !hardwareProfile.backlight.invertido); // No MINI LOW = Aceso!
}

void backlightSet(bool ativo)
{
  if (!backlightAtivo())
    return;

  backlightState = ativo;
  digitalWrite(hardwareProfile.backlight.pin, hardwareProfile.backlight.invertido ? !ativo : ativo);
}

void backlightToggle()
{
  backlightSet(!backlightState);
}
