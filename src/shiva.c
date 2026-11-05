#include "game.h"

#define SHIVA_START_ANIM 14
#define SHIVA_START 4

sint32 GetFreeSpark(void);

void TriggerShivaSmoke(sint32 x, sint32 y, sint32 z, sint32 uw)
{
    sint32 size;
    SPARKS *sptr;
    sint32 dx, dz;

    dx = lara_item->pos.x_pos - x;
    dz = lara_item->pos.z_pos - z;
    if (dx < -0x4000 || dx > 0x4000 || dz < -0x4000 || dz > 0x4000)
        return;
    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    if (uw) {
        sptr->sR = 0;
        sptr->sG = 0;
        sptr->sB = 0;
        sptr->dR = 192;
        sptr->dG = 192;
        sptr->dB = 208;
    } else {
        sptr->sR = 144;
        sptr->sG = 144;
        sptr->sB = 144;
        sptr->dR = 64;
        sptr->dG = 64;
        sptr->dB = 64;
    }
    sptr->ColFadeSpeed = 8;
    sptr->FadeToBlack = 64;
    sptr->sLife = sptr->Life = (GetRandomControl() & 31) + 96;
    if (uw)
        sptr->TransType = 2;
    else
        sptr->TransType = 3;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = x + (GetRandomControl() & 31) - 16;
    sptr->y = y + (GetRandomControl() & 31) - 16;
    sptr->z = z + (GetRandomControl() & 31) - 16;
    sptr->Xvel = ((GetRandomControl() & 4095) - 2048) >> 2;
    sptr->Yvel = (GetRandomControl() & 255) - 128;
    sptr->Zvel = ((GetRandomControl() & 4095) - 2048) >> 2;
    if (uw) {
        sptr->Yvel >>= 4;
        sptr->y += 32;
        sptr->Friction = 4 | (1 << 4);
    } else
        sptr->Friction = 6;
    sptr->Flags = 2 | 8 | 16 | 512;
    sptr->RotAng = GetRandomControl() & 4095;
    if (GetRandomControl() & 1)
        sptr->RotAdd = -(GetRandomControl() & 15) - 16;
    else
        sptr->RotAdd = (GetRandomControl() & 15) + 16;
    sptr->Def = objects[296].mesh_index;
    sptr->Scalar = 3;
    if (uw)
        sptr->Gravity = sptr->MaxYvel = 0;
    else {
        sptr->Gravity = -(GetRandomControl() & 3) - 3;
        sptr->MaxYvel = -(GetRandomControl() & 3) - 4;
    }
    size = (GetRandomControl() & 31) + 128;
    sptr->Width = sptr->sWidth = size >> 2;
    sptr->dWidth = size;
    size += (GetRandomControl() & 31) + 32;
    sptr->Height = sptr->sHeight = size >> 3;
    sptr->dHeight = size;
}

void InitialiseShiva(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    if (item->object_number == SHIVA)
    {
        item->anim_number = objects[SHIVA].anim_index + SHIVA_START_ANIM;
        item->frame_number = anims[item->anim_number].frame_base;
        item->current_anim_state = item->goal_anim_state = SHIVA_START;
    }
    item->status = NOT_ACTIVE;
    item->mesh_bits = 0;
}

#define SHIVA_TOUCHL 0x2400
#define SHIVA_TOUCHR 0x2400000

extern BITE_INFO shiva_left;
extern BITE_INFO shiva_right;

sint16 CreatureEffect(ITEM_INFO *item, BITE_INFO *bite, sint16 (*generate)(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 yrot, sint16 room_number));
sint16 DoBloodSplat(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num);

void ShivaDamage(ITEM_INFO *item, CREATURE_INFO *shiva, int damage)
{
    if (!shiva->flags && (item->touch_bits & SHIVA_TOUCHR)) {
        lara_item->hit_points -= damage;
        lara_item->hit_status = 1;
        CreatureEffect(item, &shiva_right, DoBloodSplat);
        shiva->flags = 1;
        SoundEffect(318, &item->pos, 0);
    }
    if (!shiva->flags && (item->touch_bits & SHIVA_TOUCHL)) {
        lara_item->hit_points -= damage;
        lara_item->hit_status = 1;
        CreatureEffect(item, &shiva_left, DoBloodSplat);
        shiva->flags = 1;
        SoundEffect(318, &item->pos, 0);
    }
}
