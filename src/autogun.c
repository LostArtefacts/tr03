#include "game.h"

#define AUTOGUN_STILL_ANIM 1
#define AUTOGUN_STILL 1

void InitialiseAutogun(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    item->anim_number = objects[ROBOT_SENTRY_GUN].anim_index + AUTOGUN_STILL_ANIM;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = item->goal_anim_state = AUTOGUN_STILL;
    item->item_flags[0] = 0;
}

typedef struct {
    sint16 zone_number;
    sint16 enemy_zone;
    sint32 distance;
    sint32 ahead;
    sint32 bite;
    sint16 angle;
    sint16 x_angle;
    sint16 enemy_facing;
} AI_INFO;

extern BITE_INFO autogun_left;
extern BITE_INFO autogun_right;

void phd_PushMatrix(void);
void TriggerDynamic(sint32 x, sint32 y, sint32 z, sint32 falloff, sint32 r, sint32 g, sint32 b);
int CreatureActive(sint16 item_number);
void CreatureAIInfo(ITEM_INFO *item, AI_INFO *info);
void CreatureJoint(ITEM_INFO *item, sint16 joint, sint16 required);
int ExplodingDeath(sint16 item_number, sint32 mesh_bits, sint16 damage);
void DisableBaddieAI(sint16 item_number);
int Targetable(ITEM_INFO *item, AI_INFO *info);
int ShotLara(ITEM_INFO *item, AI_INFO *info, BITE_INFO *gun, sint16 extra_rotation, int damage);
void AnimateItem(ITEM_INFO *item);

void AutogunControl(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *autogun;
    sint16 tilt_x, tilt_y, change;
    AI_INFO info;

    item = &items[item_number];
    tilt_x = tilt_y = 0;
    if (item->fired_weapon > 1) {
        PHD_VECTOR pos;

        item->fired_weapon--;
        if (GetRandomControl() & 1) {
            objects[item->object_number].bite_offset = 4;
            phd_PushMatrix();
            pos.x = autogun_left.x;
            pos.y = autogun_left.y;
            pos.z = autogun_left.z;
            GetJointAbsPosition(item, &pos, autogun_left.mesh_num);
            TriggerDynamic(pos.x, pos.y, pos.z, (item->fired_weapon << 1) + 8, 24, 16, 4);
            phd_PopMatrix();
        } else {
            objects[item->object_number].bite_offset = 5;
            phd_PushMatrix();
            pos.x = autogun_right.x;
            pos.y = autogun_right.y;
            pos.z = autogun_right.z;
            GetJointAbsPosition(item, &pos, autogun_right.mesh_num);
            TriggerDynamic(pos.x, pos.y, pos.z, (item->fired_weapon << 1) + 8, 24, 16, 4);
            phd_PopMatrix();
        }
    }

    if (!CreatureActive(item_number) || item->data == NULL)
        return;
    if (item->hit_status)
        item->really_active = 1;
    autogun = (CREATURE_INFO *)item->data;
    if (item->hit_points <= 0) {
        ExplodingDeath(item_number, 0xffffffff, 0);
        DisableBaddieAI(item_number);
        KillItem(item_number);
        item->status = DEACTIVATED;
        item->flags |= ONESHOT;
    }
    if (!items[item_number].really_active)
        return;

    CreatureAIInfo(item, &info);
    tilt_x = -info.x_angle;
    switch (item->current_anim_state) {
    case 0:
        if (Targetable(item, &info)) {
            if (item->frame_number == anims[item->anim_number].frame_base) {
                item->item_flags[0] = 1;
                if (objects[item->object_number].bite_offset == 5)
                    ShotLara(item, &info, &autogun_left, autogun->joint_rotation[0], 10);
                else
                    ShotLara(item, &info, &autogun_right, autogun->joint_rotation[0], 10);
                item->fired_weapon = 10;
                SoundEffect(44, &item->pos, 0);
            }
        } else
            item->goal_anim_state = AUTOGUN_STILL;
        break;
    case AUTOGUN_STILL:
        if (Targetable(item, &info) && item->item_flags[0] == 0)
            item->goal_anim_state = 0;
        else if (item->item_flags[0]) {
            if (item->ai_bits == 8) {
                item->item_flags[0] = 1;
                item->goal_anim_state = 0;
            } else {
                item->item_flags[0] = 0;
                item->really_active = 0;
            }
        }
        break;
    }

    change = info.angle - autogun->joint_rotation[0];
    if (change > ONE_DEGREE * 10)
        change = ONE_DEGREE * 10;
    else if (change < -ONE_DEGREE * 10)
        change = -ONE_DEGREE * 10;
    autogun->joint_rotation[0] += change;
    CreatureJoint(item, 1, tilt_x);
    AnimateItem(item);
    if (info.angle > 0x4000) {
        item->pos.y_rot -= 0x8000;
        if (info.angle > 0)
            autogun->joint_rotation[0] -= 0x8000;
        else if (info.angle < 0)
            autogun->joint_rotation[0] += 0x8000;
    } else if (info.angle < -0x4000) {
        item->pos.y_rot += 0x8000;
        if (info.angle > 0)
            autogun->joint_rotation[0] -= 0x8000;
        else if (info.angle < 0)
            autogun->joint_rotation[0] += 0x8000;
    }
}

#ifdef TR3_EXE
BITE_INFO autogun_left = { 110, -30, -530, 2 };
BITE_INFO autogun_right = { -110, -30, -530, 2 };
#endif
