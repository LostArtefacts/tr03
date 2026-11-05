#include "game.h"

#define FIREHEAD_REAR 1

void InitialiseFireHead(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    item->anim_number = objects[item->object_number].anim_index + 1;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = FIREHEAD_REAR;
    item->goal_anim_state = FIREHEAD_REAR;
}

extern sint16 rcossin_tbl[];
sint32 GetFreeSpark(void);

void TriggerFireHeadFlame(sint32 x, sint32 y, sint32 z, sint32 angle, sint32 speed)
{
    sint32 size, zv, xv;
    SPARKS *sptr;
    sint32 dx, dz;

    dx = lara_item->pos.x_pos - x;
    dz = lara_item->pos.z_pos - z;
    if (dx < -0x4000 || dx > 0x4000 || dz < -0x4000 || dz > 0x4000)
        return;
    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sR = 48 + (GetRandomControl() & 31);
    sptr->sG = sptr->sR;
    sptr->sB = 192 + (GetRandomControl() & 63);
    sptr->dR = 192 + (GetRandomControl() & 63);
    sptr->dG = 128 + (GetRandomControl() & 63);
    sptr->dB = 32;
    sptr->ColFadeSpeed = 12 + (GetRandomControl() & 3);
    sptr->FadeToBlack = 8;
    sptr->sLife = sptr->Life = (GetRandomControl() & 7) + 28;
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = x + ((GetRandomControl() & 31) - 16);
    sptr->y = y + ((GetRandomControl() & 31) - 16);
    sptr->z = z + ((GetRandomControl() & 31) - 16);
    speed -= GetRandomControl() % ((speed >> 3) + 1);
    zv = (rcossin_tbl[angle + 1] * speed) >> 11;
    xv = (rcossin_tbl[angle] * speed) >> 11;
    sptr->Xvel = xv + ((GetRandomControl() & 127) - 64);
    sptr->Yvel = (GetRandomControl() & 7) + 6;
    sptr->Zvel = zv + ((GetRandomControl() & 127) - 64);
    sptr->Friction = 4;
    sptr->Flags = 2 | 8 | 512;
    if (GetRandomControl() & 1) {
        sptr->Flags |= 16;
        sptr->RotAng = GetRandomControl() & 4095;
        sptr->RotAdd = (GetRandomControl() & 63) - 32;
    }
    sptr->Gravity = -(GetRandomControl() & 15) - 8;
    sptr->MaxYvel = -(GetRandomControl() & 7) - 8;
    sptr->Def = objects[296].mesh_index;
    sptr->Scalar = 3;
    size = (GetRandomControl() & 15) + (speed >> 4);
    sptr->Width = sptr->sWidth = size >> 2;
    sptr->Height = sptr->sHeight = size >> 2;
    sptr->dWidth = size;
    sptr->dHeight = size;
}

void ControlRotateyThing(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    if (TriggerActive(item))
        AnimateItem(item);
    else
        KillItem(item_number);
}
