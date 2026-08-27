#include "eTomadaLite.h"
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
