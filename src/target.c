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

#define DONT_TARGET (-16384)

void TargetControl(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];

    if (item->status != ACTIVE)
        return;

    if (item->hit_points != 8)
        assault_targets_reset = 0;

    if (item->hit_points != DONT_TARGET) {
        if (item->hit_status)
            SoundEffect(119, &item->pos, 0);

        switch (item->current_anim_state) {
        case 0:
            if (item->hit_points < 6) {
                item->hit_points = 6;
                item->goal_anim_state = 1;
            }
            break;

        case 1:
            if (item->hit_points < 4) {
                item->anim_number = objects[item->object_number].anim_index + 2;
                item->frame_number = anims[item->anim_number].frame_base;
                item->current_anim_state = 2;
                item->hit_points = 4;
            }
            break;

        case 2:
            if (item->hit_points < 2) {
                item->anim_number = objects[item->object_number].anim_index + 3;
                item->frame_number = anims[item->anim_number].frame_base;
                item->current_anim_state = 3;
                item->hit_points = 2;
            }
            break;

        case 3:
            if (item->hit_points <= 0 && item->hit_points != DONT_TARGET) {
                lara.target = NULL;
                item->hit_points = DONT_TARGET;
                item->item_flags[0] = 10 * ONE_DEGREE;
                item->item_flags[1] = 0;
                item->item_flags[2] = 1;
            }
            break;
        }

        item->timer++;
        if (item->timer > 10 * 30) {
            lara.target = NULL;
            item->hit_points = DONT_TARGET;
            item->item_flags[0] = 1 * ONE_DEGREE;
            item->item_flags[1] = 0;
            item->item_flags[2] = 0;
        }
    }

    if (item->hit_points == DONT_TARGET && item->item_flags[2] == 1) {
        item->pos.x_rot += item->item_flags[0];
        item->item_flags[0] += (ONE_DEGREE << 2) >> item->item_flags[1];

        if (item->pos.x_rot > 0x3800) {
            if (item->item_flags[1] == 2) {
                item->pos.x_rot = 0x3800;
                RemoveActiveItem(item_number);
                assault_targets--;
                return;
            } else {
                if (item->item_flags[1] == 1)
                    SoundEffect(120, &item->pos, 0);
                item->pos.x_rot = 0x3800;
                item->item_flags[0] = -item->item_flags[0] >> 2;
                item->item_flags[1]++;
            }
        }
    } else if (item->hit_points == DONT_TARGET && item->item_flags[2] == 0) {
        item->pos.x_rot -= item->item_flags[0];
        item->item_flags[0] += (ONE_DEGREE >> 1) >> item->item_flags[1];

        if (item->pos.x_rot < -0x2a00) {
            if (item->item_flags[1] == 2) {
                item->pos.x_rot = -0x2a00;
                RemoveActiveItem(item_number);
                return;
            } else {
                SoundEffect(119, &item->pos, 0x2000004);
                item->pos.x_rot = -0x2a00;
                item->item_flags[0] = -item->item_flags[0] >> 2;
                item->item_flags[1]++;
            }
        }
    }

    AnimateItem(item);
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
