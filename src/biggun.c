#include "game.h"

typedef struct {
    sint16 RotX;
    sint16 RotY;
    sint16 StartRotY;
    char Flags;
    char FireCount;
} BIGGUNINFO;

void BigGunInitialise(sint16 item_number)
{
    ITEM_INFO *v;
    BIGGUNINFO *gun;

    v = &items[item_number];
    gun = v->data = game_malloc(sizeof(BIGGUNINFO), 0);
    gun->Flags = gun->FireCount = 0;
    gun->RotX = 30;
    gun->RotY = 0;
    gun->StartRotY = v->pos.y_rot;
}
