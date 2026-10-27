#pragma once

void releInit();
int relesGetCount();

void releSet(int index, bool ligado, bool enviaEvento = true);
bool releToggle(int index);
bool releGetEstado(int index);

String releGetRecursoJSON(int index);

void apiReleSet();
void apiReleToggle();
