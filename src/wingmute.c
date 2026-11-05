#include "game.h"

#define WING_START_ANIM 2
#define WING_WAIT 2

void InitialiseWingmute(sint16 item_number)
{
    ITEM_INFO *item;

    InitialiseCreature(item_number);
    item = &items[item_number];
    item->anim_number = objects[MUTANT1].anim_index + WING_START_ANIM;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = item->goal_anim_state = WING_WAIT;
    item->item_flags[1] = GetRandomControl() & 0x7f;
}

sint32 GetFreeSpark(void);

void TriggerWingMuteParticles(sint16 item_number)
{
    sint32 size;
    SPARKS *sptr;
    sint32 dx, dz;

    dx = lara_item->pos.x_pos - items[item_number].pos.x_pos;
    dz = lara_item->pos.z_pos - items[item_number].pos.z_pos;
    if (dx < -0x4000 || dx > 0x4000 || dz < -0x4000 || dz > 0x4000)
        return;
    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sG = (GetRandomControl() & 63) + 32;
    sptr->sB = sptr->sG >> 1;
    sptr->sR = sptr->sG >> 2;
    sptr->dG = (GetRandomControl() & 31) + 224;
    sptr->dB = sptr->dG >> 1;
    sptr->dR = sptr->dG >> 2;
    sptr->ColFadeSpeed = 4;
    sptr->FadeToBlack = 2;
    sptr->sLife = sptr->Life = 8;
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = (GetRandomControl() & 15) - 8;
    sptr->y = (GetRandomControl() & 15) - 8;
    sptr->z = (GetRandomControl() & 127) - 64;
    sptr->Xvel = (GetRandomControl() & 31) - 16;
    sptr->Yvel = (GetRandomControl() & 31) - 16;
    sptr->Zvel = (GetRandomControl() & 31) - 16;
    sptr->Friction = 2 | (2 << 4);
    sptr->Flags = 2 | 128 | 4096 | 8;
    sptr->Gravity = sptr->MaxYvel = 0;
    sptr->FxObj = item_number;
    sptr->NodeNumber = 1;
    sptr->Def = objects[296].mesh_index;
    sptr->Scalar = 3;
    size = (GetRandomControl() & 3) + 3;
    sptr->Width = sptr->sWidth = size;
    sptr->Height = sptr->sHeight = size;
    sptr->dWidth = size >> 1;
    sptr->dHeight = size >> 1;
}
