#pragma once
#include <Arduino.h>

struct Config
{
  uint32_t magic;

  char ssid[64];
  char senha[64];

  char hostname[32];
};

bool configLoad();
void configSave();
