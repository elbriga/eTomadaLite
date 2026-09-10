#include "eTomadaLite.h"
#include "memoria.h"
#include "loga.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga("..MEM..", nivel, fmt, ##__VA_ARGS__)

static uint32_t memoriaMinimo = INT32_MAX;

void memoriaLog(const char *onde)
{
    uint32_t livre = ESP.getFreeHeap();
    if (livre < memoriaMinimo)
        memoriaMinimo = livre;

    logaM(LOG_AVISO, "MEMLOG [%s] : free=%u min=%u maxAlloc=%u",
          onde, livre, memoriaMinimo, ESP.getMaxFreeBlockSize());
}
