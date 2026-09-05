#pragma once
#include <Arduino.h>

#define BOTAO_DEBOUCE_TIME_MS 33
#define BOTAO_TEMPO_CLICK_MS 333
#define BOTAO_TEMPO_LONGP_MS 3333

struct Botao
{
    int pino;
    bool estado; // nível atual
    bool ultimoEstado;
    uint32_t debounce;
    uint32_t ultimoToggle; // para detectar CLICK
};

void botoesInit();
bool botaoAtivo();

void botaoPrint();

// JsonDocument botaoGetJSONDoc(Botao *s, bool full);

void botaoProcessa();
