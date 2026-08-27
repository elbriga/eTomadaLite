#pragma once
#include <Arduino.h>

#define ETOMADA_LITE_CONFIG_MAGIC 0x45544F4DUL // "ETOM"

struct Config
{
  uint32_t magic;

  char ssid[64];
  char senha[64];

  char deviceID[32];
};

bool configLoad();
void configSave();
