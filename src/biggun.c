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

int CanUseGun(ITEM_INFO *v, ITEM_INFO *l)
{
    int dist;
    sint32 x, z;

    if (!(input & IN_ACTION) || lara.gun_status != LG_ARMLESS || l->gravity_status)
        return 0;

    x = l->pos.x_pos - v->pos.x_pos;
    z = l->pos.z_pos - v->pos.z_pos;
    dist = x * x + z * z;
    if (dist > 30000)
        return 0;
    else
        return 1;
}
