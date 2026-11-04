#include "game.h"

extern sint32 assault_targets_reset;
extern sint32 level_items;

void InitialiseTarget(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    if (item->active)
        RemoveActiveItem(item_number);
    item->anim_number = objects[item->object_number].anim_index;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = item->goal_anim_state = anims[item->anim_number].current_anim_state;
    item->required_anim_state = 0;
    item->pos.x_rot = item->pos.z_rot = 0;
    item->status = NOT_ACTIVE;
    item->active = item->flags = item->timer = item->item_flags[2] = 0;
    item->hit_points = objects[item->object_number].hit_points;
    item->data = NULL;
}

void ResetTargets(void)
{
    sint32 lp;

    if (!assault_targets_reset)
    {
        assault_targets = 0;
        for (lp = 0; lp < level_items; lp++)
        {
            if (items[lp].object_number == TARGETS)
            {
                InitialiseTarget(lp);
                assault_targets++;
            }
        }
        assault_targets_reset = 1;
    }
}
