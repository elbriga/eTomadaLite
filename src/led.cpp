#include <sys/time.h>

#include "eTomadaLite.h"

bool ledState;

void ledInit()
{
  pinMode(LED_GPIO, OUTPUT);   // Led Azul do MINI
  digitalWrite(LED_GPIO, LOW); // LOW = Aceso!
}

void ledProcessa()
{
  // Sincronizado com o segundo!
  struct timeval tv;
  gettimeofday(&tv, nullptr);

  bool estado = tv.tv_usec > 100000;

  if (estado != ledState)
  {
    ledState = estado;
    digitalWrite(LED_GPIO, ledState);
  }
}
