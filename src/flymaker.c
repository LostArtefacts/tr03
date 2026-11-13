#include "game.h"

#define KILLED_ITEM (-0x8000)

extern sint16 FlyItem[3];
extern sint32 level_items;

int EnableBaddieAI(sint16 item_number, int Always);

void FlyEmitterControl(sint16 item_num)
{
    ITEM_INFO *item, *fly, *target;
    sint16 fly_num, target_number, i;
    sint32 x, z, distance;

    item = &items[item_num];
    if (!item->active || item->hit_points <= 0 || item->timer <= 0) {
        return;
    } else if (FlyItem[0] == NO_ITEM) {
        for (target_number = 0, i = 0; target_number < level_items; target_number++) {
            target = &items[target_number];
            if (target->object_number == 44 && target->ai_bits == 8) {
                FlyItem[i] = target_number;
                KillItem(target_number);
                i++;
            }
            if (i > 2)
                return;
        }
        return;
    } else if (items[FlyItem[0]].status == ACTIVE && items[FlyItem[1]].status == ACTIVE && items[FlyItem[2]].status == ACTIVE) {
        return;
    } else {
        x = lara_item->pos.x_pos - item->pos.x_pos;
        z = lara_item->pos.z_pos - item->pos.z_pos;
        if (z > 32000 || z < -32000 || x > 32000 || x < -32000)
            return;

        distance = z * z + x * x;

        if (distance > (WALL_L * 12) * (WALL_L * 12)) {
            return;
        } else if (item->item_flags[0] <= 0) {
            item->item_flags[0] = 255;
            item->timer -= 30;
        } else {
            item->item_flags[0]--;
            return;
        }
    }

    for (fly_num = 0; fly_num < 3; fly_num++) {
        if (items[FlyItem[fly_num]].data == NULL && item[FlyItem[fly_num]].status != ACTIVE)
            break;
    }

    if (fly_num > 2)
        return;

    fly = &items[FlyItem[fly_num]];

    fly->pos = item->pos;
    fly->anim_number = objects[fly->object_number].anim_index;
    fly->frame_number = anims[fly->anim_number].frame_base;
    fly->current_anim_state = fly->goal_anim_state = anims[fly->anim_number].current_anim_state;
    fly->required_anim_state = 0;
    fly->flags &= ~(ONESHOT | KILLED_ITEM | INVISIBLE);
    fly->ai_bits = 8;
    fly->data = 0;
    fly->status = ACTIVE;
    fly->mesh_bits = 0xffffffff;
    fly->hit_points = objects[fly->object_number].hit_points;
    fly->collidable = 1;
    if (fly->active)
        RemoveActiveItem(FlyItem[fly_num]);
    AddActiveItem(FlyItem[fly_num]);
    ItemNewRoom(FlyItem[fly_num], item->room_number);
    EnableBaddieAI(FlyItem[fly_num], 1);
}

#ifdef TR3_EXE
sint16 FlyItem[3] = { NO_ITEM, NO_ITEM, NO_ITEM };
#endif
