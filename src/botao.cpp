#include <Arduino.h>

#include "eTomadaLite.h"
#include "mestre.h"
#include "hardwareProfile.h"
#include "botao.h"
#include "loga.h"
#include "http.h"
#include "eventos.h"

// Função de log para esta modulo
#define logaM(nivel, fmt, ...) loga("BOTAO", nivel, fmt, ##__VA_ARGS__)

// Hardware Profile - um para cada placa
extern const HardwareProfile hardwareProfile;

static Botao botao;

void botoesInit()
{
  // Zerar tudo
  memset(&botao, 0, sizeof(Botao));

  if (!botaoAtivo())
    return;

  botao.pino = hardwareProfile.botaoPin;

  pinMode(botao.pino, INPUT_PULLUP);

  botao.estado = !digitalRead(botao.pino);
  botao.ultimoEstado = botao.estado;
  botao.debounce = millis();
  botao.ultimoToggle = millis();

  botaoPrint();
}

bool botaoAtivo()
{
  return hardwareProfile.botaoPin != 255;
}

void botaoPrint()
{
  logaM(LOG_NORMAL, "Botao [pin:%d] (%s)",
        botao.pino, (botao.estado ? "on" : "off"));
}

void botaoProcessa()
{
  if (!botaoAtivo())
    return;

  // Debounce
  bool leitura = !digitalRead(botao.pino); // PINO LOW == BOTAO ON
  uint32_t agora = millis();

  if (leitura != botao.ultimoEstado)
  {
    botao.debounce = agora;
    botao.ultimoEstado = leitura;
  }

  if (agora - botao.debounce > BOTAO_DEBOUCE_TIME_MS)
  {
    if (botao.estado != leitura)
    {
      logaM(LOG_NORMAL, "BOTAO MUDOU [%s]", leitura ? "ON" : "OFF");

      botao.estado = leitura;
      uint32_t duracaoAnterior = agora - botao.ultimoToggle;
      botao.ultimoToggle = agora;

      time_t now = 0;
      time(&now);

      String body;
      body.reserve(512);

      body = F("{\"origem\":\"");
      body += eTomadaLiteDeviceID();
      body += F("\",\"id\":\"B1\"");

      body += F(",\"timestamp\":");
      body += (unsigned long)now;

      body += F(",\"evento\":\"TOGGLE\"");

      body += F(",\"device\":{");

      body += F("\"estado\":");
      body += leitura ? "1" : "0";

      body += F("}}");

      mestreEnviaEvento(EVENTO_TOGGLE, body.c_str());

      /*
      eventoPost(botao->estado ? EVENTO_LIGOU : EVENTO_DESLIGOU, atual[rb].rec, true, true);
      eventoPost(EVENTO_TOGGLE, atual[rb].rec, true, true);

      // Detectar CLICK, em qualquer direcao
      if (atual[rb].duracaoAnterior < BOTAO_TEMPO_CLICK_MS)
        eventoPost(EVENTO_CLICK, atual[rb].rec, true, true);

      // Detectar longPress e bigPress ao desligar
      if (!botao->estado && atual[rb].duracaoAnterior > BOTAO_TEMPO_LONGP_MS)
        eventoPost(EVENTO_LONG_PRESS, atual[rb].rec, true, true);
      */
    }
  }
}
