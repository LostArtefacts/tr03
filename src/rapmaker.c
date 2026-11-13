#include "game.h"

#define KILLED_ITEM (-0x8000)

extern sint16 RaptorItem[3];
extern sint32 level_items;

int EnableBaddieAI(sint16 item_number, int Always);

void RaptorEmitterControl(sint16 item_num)
{
    ITEM_INFO *item, *raptor, *target;
    sint16 raptor_num, target_number, i;
    sint32 x, z, distance;

    item = &items[item_num];
    if (!item->active || item->timer <= 0) {
        return;
    } else if (RaptorItem[0] == NO_ITEM) {
        for (target_number = 0, i = 0; target_number < level_items; target_number++) {
            target = &items[target_number];
            if (target->object_number == 288 && target->ai_bits == 8) {
                RaptorItem[i] = target_number;
                i++;
            }
            if (i > 2)
                return;
        }
        return;
    } else if (items[RaptorItem[0]].data != NULL && items[RaptorItem[1]].data != NULL && items[RaptorItem[2]].data != NULL) {
        return;
    } else {
        z = lara_item->pos.z_pos - item->pos.z_pos;
        x = lara_item->pos.x_pos - item->pos.x_pos;
        distance = z * z + x * x;

        if (distance < (WALL_L * 4) * (WALL_L * 4)) {
            return;
        } else if (item->item_flags[0] <= 0) {
            item->item_flags[0] = 255;
            item->timer -= 30;
        } else {
            item->item_flags[0]--;
            return;
        }
    }

    for (raptor_num = 0; raptor_num < 3; raptor_num++) {
        if (items[RaptorItem[raptor_num]].data == NULL)
            break;
    }

    raptor = &items[RaptorItem[raptor_num]];

    raptor->pos = item->pos;
    raptor->anim_number = objects[raptor->object_number].anim_index;
    raptor->frame_number = anims[raptor->anim_number].frame_base;
    raptor->current_anim_state = raptor->goal_anim_state = anims[raptor->anim_number].current_anim_state;
    raptor->required_anim_state = 0;
    raptor->flags &= ~(ONESHOT | KILLED_ITEM | INVISIBLE);
    raptor->data = 0;
    raptor->status = ACTIVE;
    raptor->mesh_bits = 0xffffffff;
    raptor->hit_points = objects[raptor->object_number].hit_points;
    raptor->collidable = 1;
    if (raptor->active)
        RemoveActiveItem(RaptorItem[raptor_num]);
    AddActiveItem(RaptorItem[raptor_num]);
    ItemNewRoom(RaptorItem[raptor_num], item->room_number);
    EnableBaddieAI(RaptorItem[raptor_num], 1);
}

void InitialiseRaptorEmitter(sint16 item_number)
{
    items[item_number].item_flags[0] = (item_number & 3) * 96;
}

#ifdef TR3_EXE
sint16 RaptorItem[3] = { NO_ITEM, NO_ITEM, NO_ITEM };
#endif
