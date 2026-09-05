#include "eventos.h"

const char *eventoGetTipoTxt(TipoEvento tipo)
{
    switch (tipo)
    {
    case EVENTO_NENHUM:
        return "NENHUM";
    case EVENTO_LIGOU:
        return "LIGOU";
    case EVENTO_DESLIGOU:
        return "DESLIGOU";
    case EVENTO_TOGGLE:
        return "TOGGLE";
    case EVENTO_CLICK:
        return "CLICK";
    case EVENTO_DOUBLE_CLICK:
        return "DUPCLICK";
    case EVENTO_LONG_PRESS:
        return "LONG_PRESS";
    case EVENTO_VALOR_MUDOU:
        return "CHANGED";
    case EVENTO_HORARIO:
        return "HORARIO";
    default:
        return "?*?";
    }
}