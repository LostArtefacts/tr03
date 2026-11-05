#include "game.h"

extern GAME_VECTOR ForcedFixedCamera;
extern char UseForcedFixedCamera;

void TriggerDynamic(sint32 x, sint32 y, sint32 z, sint32 falloff, sint32 r, sint32 g, sint32 b);
sint32 TestHeight_train(ITEM_INFO *item, sint32 x, sint32 z, sint16 *room_number);

void TrainControl(sint16 train_number)
{
    ITEM_INFO *t;
    sint32 s, c;
    sint32 fh, rh;
    FLOOR_INFO *floor;
    sint16 room_number;

    t = &items[train_number];

    if (!TriggerActive(t)) {
        return;
    }

    if (t->item_flags[0] == 0) {
        t->item_flags[0] = t->item_flags[1] = 260;
    }

    c = phd_cos(t->pos.y_rot);
    s = phd_sin(t->pos.y_rot);

    t->pos.x_pos += (t->item_flags[1] * s) >> W2V_SHIFT;
    t->pos.z_pos += (t->item_flags[1] * c) >> W2V_SHIFT;

    rh = TestHeight_train(t, 0, 5120, &room_number);
    t->pos.y_pos = fh = TestHeight_train(t, 0, 0, &room_number);

    if (fh == NO_HEIGHT) {
        KillItem(train_number);
        return;
    }

    t->pos.y_pos -= 32;

    room_number = t->room_number;
    GetFloor(t->pos.x_pos, t->pos.y_pos, t->pos.z_pos, &room_number);

    if (room_number != t->room_number) {
        ItemNewRoom(train_number, room_number);
    }

    t->pos.x_rot = -(rh - fh) << 1;

    TriggerDynamic(t->pos.x_pos + ((3072 * s) >> W2V_SHIFT), t->pos.y_pos, t->pos.z_pos + ((3072 * c) >> W2V_SHIFT), 16, 31, 31, 31);

    if (t->item_flags[1] != 260) {
        if ((t->item_flags[1] -= 48) < 0) {
            t->item_flags[1] = 0;
        }

        if (!UseForcedFixedCamera) {
            ForcedFixedCamera.x = t->pos.x_pos + ((8192 * s) >> W2V_SHIFT);
            ForcedFixedCamera.z = t->pos.z_pos + ((8192 * c) >> W2V_SHIFT);

            room_number = t->room_number;
            floor = GetFloor(ForcedFixedCamera.x, t->pos.y_pos - 512, ForcedFixedCamera.z, &room_number);
            ForcedFixedCamera.y = GetHeight(floor, ForcedFixedCamera.x, t->pos.y_pos - 512, ForcedFixedCamera.z);

            ForcedFixedCamera.room_number = room_number;

            UseForcedFixedCamera = 1;
        }
    } else {
        SoundEffect(148, &t->pos, 2);
    }
}

sint32 TestHeight_train(ITEM_INFO *item, sint32 x, sint32 z, sint16 *room_number)
{
    sint32 s, c;
    PHD_VECTOR pos;
    FLOOR_INFO *floor;

    c = phd_cos(item->pos.y_rot);
    s = phd_sin(item->pos.y_rot);

    pos.x = item->pos.x_pos + ((z * s + x * c) >> W2V_SHIFT);
    pos.y = item->pos.y_pos - (z * phd_sin(item->pos.x_rot) >> W2V_SHIFT) + (x * phd_sin(item->pos.z_rot) >> W2V_SHIFT);
    pos.z = item->pos.z_pos + ((z * c - x * s) >> W2V_SHIFT);

    *room_number = item->room_number;
    floor = GetFloor(pos.x, pos.y, pos.z, room_number);
    return GetHeight(floor, pos.x, pos.y, pos.z);
}


#define LARA_EXTRA 13
#define EXTRA_TRAINKILL 11

int TestBoundsCollide(ITEM_INFO *item, ITEM_INFO *laraitem, sint32 radius);
int TestCollision(ITEM_INFO *item, ITEM_INFO *laraitem);
void DoLotsOfBlood(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num, int number);

void TrainCollision(sint16 train_number, ITEM_INFO *l, COLL_INFO *coll)
{
    ITEM_INFO *t;
    sint32 s, c;
    sint32 x, z;

    t = &items[train_number];
    if (!TestBoundsCollide(t, l, coll->radius))
        return;
    if (!TestCollision(t, l))
        return;
    SoundEffect(41, &l->pos, 2);
    SoundEffect(53, &l->pos, 2);
    StopSoundEffect(148);
    l->anim_number = objects[LARA_EXTRA].anim_index;
    l->frame_number = anims[l->anim_number].frame_base;
    l->current_anim_state = EXTRA_TRAINKILL;
    l->goal_anim_state = EXTRA_TRAINKILL;
    l->hit_points = 0;
    l->pos.y_rot = t->pos.y_rot;
    l->fallspeed = 0;
    l->gravity_status = 0;
    l->speed = 0;
    AnimateItem(l);
    lara.extra_anim = 1;
    lara.gun_status = LG_HANDSBUSY;
    lara.gun_type = LG_UNARMED;
    lara.hit_direction = -1;
    lara.air = -1;
    t->item_flags[1] = 160;
    c = phd_cos(t->pos.y_rot);
    s = phd_sin(t->pos.y_rot);
    x = l->pos.x_pos + ((256 * s) >> W2V_SHIFT);
    z = l->pos.z_pos + ((256 * c) >> W2V_SHIFT);
    DoLotsOfBlood(x, l->pos.y_pos - 512, z, 1024, t->pos.y_rot, l->room_number, 15);
}
