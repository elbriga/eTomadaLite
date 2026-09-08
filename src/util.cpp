#include "eTomadaLite.h"
#include "platform.h"
#include "loga.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga(".UTIL..", nivel, fmt, ##__VA_ARGS__)

void utilRestart()
{
  logaM(LOG_AVISO, "RESTART!");

  // Limpar a fila de logs antes de reiniciar
  logaFlush();

  delay(100);
  ESP.reset();
}

#include "util.h"

String utilIPToString(IPAddress ip)
{
  char buffer[16];

  snprintf(
      buffer,
      sizeof(buffer),
      "%u.%u.%u.%u",
      ip[0],
      ip[1],
      ip[2],
      ip[3]);

  return String(buffer);
}
