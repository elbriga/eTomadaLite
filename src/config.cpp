#include <Arduino.h>

#if defined(ESP8266)
#include <EEPROM.h>
#else
// eTomada LN882H - User Data reservado:
// 0x1FE000 - 0x1FEFFF : Config
// 0x1FF000 - 0x1FFFFF : Recovery boot tracking
#include <Flash.h>
#endif

#include "eTomadaLite.h"
#include "loga.h"
#include "config.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga("CONFIG.", nivel, fmt, ##__VA_ARGS__)

Config config;

#if !defined(ESP8266)

// Penúltimo setor da flash de 2 MiB.
// O último setor (0x1FF000) é reservado ao recovery.
#define CONFIG_FLASH_ADDR 0x1FE000
#define CONFIG_FLASH_SIZE 0x1000

static_assert(
    sizeof(Config) <= CONFIG_FLASH_SIZE,
    "Config nao cabe no setor reservado");

#endif

static bool configStorageRead()
{
#if defined(ESP8266)
  EEPROM.begin(ETOMADA_LITE_EEPROM_SIZE);
  EEPROM.get(0, config);

  return true;
#else
  if (Flash.getSize() != 0x200000)
  {
    logaM(LOG_CRITICO, "Flash inesperada: %u bytes", Flash.getSize());
    return false;
  }

  return Flash.readBlock(CONFIG_FLASH_ADDR, (uint8_t *)&config, sizeof(config));
#endif
}

static bool configStorageWrite()
{
#if defined(ESP8266)
  EEPROM.put(0, config);
  return EEPROM.commit();
#else
  // Flash só pode mudar bits de 1 -> 0.
  // Portanto apagamos o setor antes de regravar a Config.
  if (!Flash.eraseSector(CONFIG_FLASH_ADDR))
    return false;

  return Flash.writeBlock(CONFIG_FLASH_ADDR, (const uint8_t *)&config, sizeof(config));
#endif
}

void configDefaults()
{
  memset(&config, 0, sizeof(config));

  config.magic = ETOMADA_LITE_CONFIG_MAGIC;
  strlcpy(config.deviceID, "etomada-lite", sizeof(config.deviceID));

  // TESTES
  // strlcpy(config.deviceID, "COZY", sizeof(config.deviceID));
  // strlcpy(config.mestre, "GROW", sizeof(config.mestre));
  // strlcpy(config.ssid, "GLS", sizeof(config.ssid));
  // strlcpy(config.senha, "Lola09876543*", sizeof(config.senha));

  configSave();
}

bool configLoad()
{
  if (!configStorageRead())
  {
    logaM(LOG_CRITICO, "Erro lendo configuracao");
    return false;
  }

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
  if (!configStorageWrite())
  {
    logaM(LOG_CRITICO, "Erro ao salvar configuracao!");
    return;
  }

  logaM(LOG_NORMAL, "Configuracao salva.");
}
