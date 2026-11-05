#include "game.h"

extern uint32 CurrentLevel;
extern sint32 level_items;

void TriggerExplosionSparks(sint32 x, sint32 y, sint32 z, sint32 extras, sint32 dynamic, sint32 uw, sint16 roomnum);
void TriggerExplosionSmoke(sint32 x, sint32 y, sint32 z, sint32 uw);

void ControlFusebox(sint16 item_number)
{
    ITEM_INFO *item, *target_item;
    FLOOR_INFO *floor;
    sint32 x, z, height;
    sint16 room_number, i;

    item = &items[item_number];
    if (item->hit_points <= 0 && item->anim_number == objects[item->object_number].anim_index) {
        item->anim_number = objects[item->object_number].anim_index + 1;
        item->frame_number = anims[item->anim_number].frame_base;
        x = (item->pos.x_pos + 420 * phd_sin(item->pos.y_rot + 0x8000)) >> W2V_SHIFT;
        z = (item->pos.z_pos + 420 * phd_cos(item->pos.y_rot + 0x8000)) >> W2V_SHIFT;
        TriggerExplosionSparks(x, item->pos.y_pos - STEP_L * 3, z, 2, 0, 0, item->room_number);
        TriggerExplosionSmoke(x, item->pos.y_pos - STEP_L * 3, z, 0);
        if (CurrentLevel == 12) {
            room_number = item->room_number;
            floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
            height = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
            TestTriggers(trigger_index, 1);
            for (i = 0, target_item = &items[0]; i < level_items; i++, target_item++) {
                if (target_item->object_number == 57) {
                    target_item->item_flags[2] = 1;
                    break;
                }
            }
        }
    }
    AnimateItem(item);
}
