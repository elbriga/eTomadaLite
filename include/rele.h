#pragma once

void releInit();
int relesGetCount();

void releSet(int index, bool ligado);
void releToggle(int index);
bool releGetEstado(int index);

String releGetRecursoJSON(int index);

void apiReleSet();
void apiReleToggle();
