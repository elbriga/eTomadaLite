#include "platform.h"

#if !defined(ESP8266)
#include <time.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>

// No LN882 o modulo http/cookies usa essa função que não está disponivel
char *strptime(
    const char *s,
    const char *format,
    struct tm *tm)
{
    if (!s || !format || !tm)
        return nullptr;

    // Formato utilizado pelo HTTPClient do LibreTiny.
    if (strcmp(format, "%a, %d %b %Y %H:%M:%S") != 0)
        return nullptr;

    char semana[4] = {};
    char mes[4] = {};

    int dia;
    int ano;
    int hora;
    int minuto;
    int segundo;
    int consumidos = 0;

    int n = sscanf(
        s,
        "%3[^,], %d %3s %d %d:%d:%d%n",
        semana,
        &dia,
        mes,
        &ano,
        &hora,
        &minuto,
        &segundo,
        &consumidos);

    if (n != 7)
        return nullptr;

    static const char *meses[] = {
        "Jan", "Feb", "Mar", "Apr",
        "May", "Jun", "Jul", "Aug",
        "Sep", "Oct", "Nov", "Dec"};

    int numeroMes = -1;

    for (int i = 0; i < 12; i++)
    {
        if (strcasecmp(mes, meses[i]) == 0)
        {
            numeroMes = i;
            break;
        }
    }

    if (numeroMes < 0)
        return nullptr;

    memset(tm, 0, sizeof(*tm));

    tm->tm_year = ano - 1900;
    tm->tm_mon = numeroMes;
    tm->tm_mday = dia;
    tm->tm_hour = hora;
    tm->tm_min = minuto;
    tm->tm_sec = segundo;
    tm->tm_isdst = -1;

    return const_cast<char *>(s + consumidos);
}
#endif
