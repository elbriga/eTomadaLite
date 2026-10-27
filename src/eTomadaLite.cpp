#include <Arduino.h>

#include "eTomadaLite.h"
#include "platform.h"
#include "loga.h"
#include "hardwareProfile.h"
#include "config.h"
#include "rele.h"
#include "sensor.h"
#include "botao.h"
#include "util.h"

#define ETOMADA_LITE_VERSAO "4.0.1"
// 0.0.6 - led por TS
// 0.0.7 - logaM e log remoto
// 0.0.8 - modularizado
// 0.0.9 - HardwareProfile
// 0.1.0 - mDNS
// 0.1.1 - memoriaLog(1h/1h)
// 0.2.0 - hostname/deviceID configuravel
// 0.2.1 - /api/restart
// 0.2.2 - SSID e MAC no mDNS
// 1.0.0 - Suporte a LN882H
// 1.0.1 - sensorDigital
// 1.0.2 - Recurso Botao
// 2.0.0 - Suporte ao BK7231N e Teclado serial EKAZA6
// 2.0.1 - Multiplos Reles
// 2.0.2 - del mDNS!
// 3.0.0 - Suporte a BK7238 e Teclado paralelo EKAZA3
// 4.0.0 - DEL mestre > Ave SERVER
// 4.0.1 - Suporte novo EKAZA3_2

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

String eTomadaLiteGetSnapshot()
{
  String resposta;
  resposta.reserve(256);

  resposta = F("{\"device\":\"eTomada\",\"api\":\"Lite\",");

  resposta += F("\"fw_version\":\"");
  resposta += eTomadaLiteVersion();

  resposta += F("\",\"device_id\":\"");
  resposta += eTomadaLiteDeviceID();
  resposta += F("\",\"device_model\":\"");
  resposta += eTomadaLiteDeviceModel();
  resposta += F("\",\"device_board\":\"");
  resposta += eTomadaLiteDeviceBoard();

  resposta += F("\",\"uptime\":");
  resposta += millis();

  resposta += F(",\"mac\":\"");
  resposta += WiFi.macAddress();

  resposta += F("\",\"ip\":\"");
  char ipBuffer[16];
  utilIPToString(WiFi.localIP(), ipBuffer, 16);
  resposta += ipBuffer;

  resposta += F("\",\"ssid\":\"");
  resposta += WiFi.SSID();

  resposta += F("\",\"wifiPower\":");
  resposta += String(WiFi.RSSI());

  resposta += F(",\"recursos\":[");
  int totReles = relesGetCount();
  for (int i = 0; i < totReles; i++)
  {
    if (i > 0)
      resposta += F(",");
    resposta += releGetRecursoJSON(i);
  }
  if (sensorDigitalAtivo())
  {
    if (totReles)
      resposta += F(",");
    resposta += sensorGetRecursoJSON();
  }
  if (botaoAtivo())
  {
    if (totReles || sensorDigitalAtivo())
      resposta += F(",");
    resposta += botaoGetRecursoJSON();
  }
  resposta += F("]}");

  return resposta;
}
