#include "eTomadaLite.h"
#include "memoria.h"
#include "loga.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga("MEM", nivel, fmt, ##__VA_ARGS__)

void memoriaLog(const char *onde)
{
    logaM(LOG_AVISO,
          "MEMLOG [%s] : free=%u maxAlloc=%u",
          onde,
          ESP.getFreeHeap(),
          ESP.getMaxFreeBlockSize());
}
