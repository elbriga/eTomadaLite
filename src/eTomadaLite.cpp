#include <Arduino.h>

#include "eTomadaLite.h"
#include "loga.h"

#define ETOMADA_LITE_VERSAO "0.0.6"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga("ETOMADA", nivel, fmt, ##__VA_ARGS__)

#define ETOMADA_LITE_DEVICE_ID "DEV8266" // TODO : config
#define ETOMADA_LITE_DEVICE_MODEL "PROTO"
#define ETOMADA_LITE_DEVICE_BOARD "esp12e"

String eTomadaLiteDeviceID()
{
  return ETOMADA_LITE_DEVICE_ID;
}
String eTomadaLiteDeviceModel()
{
  return ETOMADA_LITE_DEVICE_MODEL;
}
String eTomadaLiteDeviceBoard()
{
  return ETOMADA_LITE_DEVICE_BOARD;
}
