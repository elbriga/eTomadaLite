#include <Arduino.h>
#include <time.h>

#include "ntp.h"
#include "loga.h"

#define logaM(nivel, fmt, ...) loga("..NTP..", nivel, fmt, ##__VA_ARGS__)

void ntpInit()
{
    // Horário de Brasília: UTC-3, sem horário de verão
    configTime(
        -3 * 3600,
        0,
        "a.ntp.br",
        "pool.ntp.org");

    logaM(LOG_NORMAL, "NTP iniciado");
}

bool ntpSincronizado()
{
    time_t agora = time(nullptr);

    // Evita considerar epoch/valores inválidos como sincronizados
    return agora > 1700000000;
}
