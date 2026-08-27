#include <Arduino.h>

#include "eTomadaLite.h"
#include "loga.h"
#include "hardwareProfile.h"
#include "config.h"

#define ETOMADA_LITE_VERSAO "0.2.0"
// 0.0.6 - led por TS
// 0.0.7 - logaM e log remoto
// 0.0.8 - modularizado
// 0.0.9 - HardwareProfile
// 0.1.0 - mDNS
// 0.1.1 - memoriaLog(1h/1h)
// 0.2.0 - hostname/deviceID configuravel

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga("ETOMADA", nivel, fmt, ##__VA_ARGS__)

extern Config config;

#define ETOMADA_LITE_DEVICE_ID_DEFAULT "ETOMADA"
extern const HardwareProfile hardwareProfile;

String eTomadaLiteVersion()
{
  return ETOMADA_LITE_VERSAO;
}

String eTomadaLiteDeviceID()
{
  // Verificar se já carregou o config antes de acessar
  return (config.magic == ETOMADA_LITE_CONFIG_MAGIC) ? config.deviceID : ETOMADA_LITE_DEVICE_ID_DEFAULT;
}
String eTomadaLiteDeviceModel()
{
  return hardwareProfile.modelo;
}
String eTomadaLiteDeviceBoard()
{
  return hardwareProfile.board;
}
