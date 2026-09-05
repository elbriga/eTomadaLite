#include <Arduino.h>
#include <EEPROM.h>

#include "eTomadaLite.h"
#include "loga.h"
#include "config.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga("CONFIG.", nivel, fmt, ##__VA_ARGS__)

#define EEPROM_SIZE 256

Config config;

void configDefaults()
{
  memset(&config, 0, sizeof(config));

  config.magic = ETOMADA_LITE_CONFIG_MAGIC;
  strlcpy(config.deviceID, "etomada-lite", sizeof(config.deviceID));

  // TESTES
  // strlcpy(config.deviceID, "DEV", sizeof(config.deviceID));
  // strlcpy(config.mestre, "GROW", sizeof(config.mestre));
  // strlcpy(config.ssid, "GLS", sizeof(config.mestre));
  // strlcpy(config.senha, "Lola09876543*", sizeof(config.mestre));

  configSave();
}

bool configLoad()
{
  EEPROM.begin(EEPROM_SIZE);

  EEPROM.get(0, config);

  // Testes
  // config.magic = 0;

  if (config.magic != ETOMADA_LITE_CONFIG_MAGIC)
  {
    logaM(LOG_AVISO, "Configuracao inexistente. Carregando Defaults");
    configDefaults();
  }

  logaM(LOG_NORMAL, "Configuracao carregada.");
  logaM(LOG_NORMAL, "ID: %s", config.deviceID);
  logaM(LOG_NORMAL, "SSID: %s", config.ssid);

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
