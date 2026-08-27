#include <Arduino.h>
#include <EEPROM.h>

#include "eTomadaLite.h"
#include "loga.h"
#include "config.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga("CONFIG.", nivel, fmt, ##__VA_ARGS__)

#define EEPROM_SIZE 256
#define CONFIG_MAGIC 0x45544F4DUL // "ETOM"

Config config;

void configDefaults()
{
  memset(&config, 0, sizeof(config));

  config.magic = CONFIG_MAGIC;
  strlcpy(config.hostname, "etomada-lite", sizeof(config.hostname));
}

bool configLoad()
{
  EEPROM.begin(EEPROM_SIZE);

  EEPROM.get(0, config);
  if (config.magic != CONFIG_MAGIC)
  {
    logaM(LOG_AVISO, "Configuracao inexistente.");
    configDefaults();
    return false;
  }

  logaM(LOG_NORMAL, "Configuracao carregada.");
  logaM(LOG_NORMAL, "SSID: %s", config.ssid);
  logaM(LOG_NORMAL, "Hostname: %s", config.hostname);

  return true;
}

void configSave()
{
  EEPROM.put(0, config);
  if (!EEPROM.commit())
  {
    logaM(LOG_CRITICO, "Erro ao salvar configuracao!");
    return;
  }

  logaM(LOG_NORMAL, "Configuracao salva.");
}
