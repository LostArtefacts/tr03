#include "game.h"

typedef struct {
    sint32 Speed;
    sint32 MidPos;
    sint32 FrontPos;
    sint32 TurnX;
    sint32 TurnZ;
    sint16 TurnLen;
    sint16 TurnRot;
    sint16 YVel;
    sint16 Gradient;
    char Flags;
    char StopDelay;
} CARTINFO;

void MineCartInitialise(sint16 item_number)
{
    ITEM_INFO *v;

    v = &items[item_number];
    v->data = game_malloc(sizeof(CARTINFO), 0);
}
