#pragma once
#include <Arduino.h>

#define ETOMADA_LITE_CONFIG_MAGIC 0x45544F4DUL // "ETOM"

#define ETOMADA_LITE_EEPROM_SIZE 256

// 0..239 = config da aplicacao
// 240..255 = recovery
#define ETOMADA_LITE_EEPROM_RECOVERY_OFFSET 240

struct Config
{
  uint32_t magic;

  char ssid[32];
  char senha[32];

  char deviceID[32];
  char mestre[32];
}; // 132 bytes

bool configLoad();
void configSave();
