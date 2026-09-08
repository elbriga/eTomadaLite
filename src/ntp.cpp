#include <Arduino.h>
#include <time.h>

#if !defined(ESP8266)
extern "C"
{
#include <lwip/apps/sntp.h>
}
#endif

#include "ntp.h"
#include "loga.h"

#define logaM(nivel, fmt, ...) loga("..NTP..", nivel, fmt, ##__VA_ARGS__)

void ntpInit()
{
#if defined(ESP8266)
    // Horário de Brasília: UTC-3, sem horário de verão
    configTime(
        -3 * 3600,
        0,
        "a.ntp.br",
        "pool.ntp.org");
#else
    // LibreTiny/LN882H usa diretamente o SNTP do lwIP.
    sntp_stop();

    sntp_setoperatingmode(SNTP_OPMODE_POLL);

    sntp_setservername(0, (char *)"a.ntp.br");
    sntp_setservername(1, (char *)"pool.ntp.org");

    sntp_init();
#endif

    logaM(LOG_NORMAL, "NTP iniciado");
}

bool ntpSincronizado()
{
    time_t agora = time(nullptr);

    // Evita considerar epoch/valores inválidos como sincronizados
    return agora > 1700000000;
}
