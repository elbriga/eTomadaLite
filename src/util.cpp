#include "eTomadaLite.h"
#include "platform.h"
#include "loga.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga(".UTIL..", nivel, fmt, ##__VA_ARGS__)

void utilRestart(const char *porque)
{
  logaM(LOG_AVISO, "RESTART! [%s]", porque);

  // Limpar a fila de logs antes de reiniciar
  logaFlush();

  delay(100);
  ESP.reset();
}

#include "util.h"

void utilIPToString(IPAddress ip, char *out, size_t maxlen)
{
  snprintf(out, maxlen, "%u.%u.%u.%u",
           ip[0], ip[1], ip[2], ip[3]);
}
