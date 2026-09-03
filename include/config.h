#pragma once
#include <Arduino.h>

#define ETOMADA_LITE_CONFIG_MAGIC 0x45544F4DUL // "ETOM"

struct Config
{
  uint32_t magic;

  char ssid[32];
  char senha[32];

  char deviceID[32];
  char mestre[32];
};

bool configLoad();
void configSave();
