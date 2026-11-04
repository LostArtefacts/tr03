#include "game.h"

#define SHIVA_START_ANIM 14
#define SHIVA_START 4

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
