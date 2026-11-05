#include "game.h"

#define CAM_A_CLIMBL (-30 * ONE_DEGREE)
#define CAM_E_CLIMBL (-15 * ONE_DEGREE)
#define CAM_A_CLIMBR (30 * ONE_DEGREE)
#define CAM_E_CLIMBR (-15 * ONE_DEGREE)
#define CLIMB_WIDTHR 120
#define CLIMB_WIDTHL 120
#define CLIMB_HITE 512

int LaraTestClimbPos(ITEM_INFO *item, int front, int right, int origin, int height, int *shift);
int LaraCheckForLetGo(ITEM_INFO *item, COLL_INFO *coll);
void LaraDoClimbLeftRight(ITEM_INFO *item, COLL_INFO *coll, int result, int shift);

#define CAM_E_CLIMBS (-20 * ONE_DEGREE)
#define CAM_E_CLIMBU (30 * ONE_DEGREE)
#define CAM_E_CLIMBD (-45 * ONE_DEGREE)

int LaraTestClimb(int x, int y, int z, int xfront, int zfront, int item_height, sint16 item_room, int *shift);

int LaraTestClimbPos(ITEM_INFO *item, int front, int right, int origin, int height, int *shift)
{
    int angle, x, z;
    int xfront = 0, zfront = 0;

    angle = (uint16)(item->pos.y_rot + 0x2000) / 0x4000;
    switch (angle) {
    case 0:
        z = item->pos.z_pos + front;
        x = item->pos.x_pos + right;
        zfront = 4;
        break;
    case 1:
        x = item->pos.x_pos + front;
        z = item->pos.z_pos - right;
        xfront = 4;
        break;
    case 2:
        z = item->pos.z_pos - front;
        x = item->pos.x_pos - right;
        zfront = -4;
        break;
    default:
        x = item->pos.x_pos - front;
        z = item->pos.z_pos + right;
        xfront = -4;
        break;
    }
    return LaraTestClimb(x, item->pos.y_pos + origin, z, xfront, zfront, height, item->room_number, shift);
}

void lara_as_climbleft(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->enable_hit = 0;
    coll->enable_baddie_push = 0;
    camera.target_angle = CAM_A_CLIMBL;
    camera.target_elevation = CAM_E_CLIMBL;
    if (!(input & IN_LEFT) && !(input & IN_STEPL))
        item->goal_anim_state = AS_CLIMBSTNC;
}

void lara_as_climbright(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->enable_hit = 0;
    coll->enable_baddie_push = 0;
    camera.target_angle = CAM_A_CLIMBR;
    camera.target_elevation = CAM_E_CLIMBR;
    if (!(input & IN_RIGHT) && !(input & IN_STEPR))
        item->goal_anim_state = AS_CLIMBSTNC;
}

void lara_as_climbstnc(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->enable_hit = 0;
    coll->enable_baddie_push = 0;
    camera.target_elevation = CAM_E_CLIMBS;
    if (input & IN_LOOK)
        LookUpDown();
    if (input & IN_LEFT || input & IN_STEPL)
        item->goal_anim_state = AS_CLIMBLEFT;
    else if (input & IN_RIGHT || input & IN_STEPR)
        item->goal_anim_state = AS_CLIMBRIGHT;
    else if (input & IN_JUMP)
    {
        item->goal_anim_state = AS_BACKJUMP;
        lara.move_angle = item->pos.y_rot - 32768;
        lara.gun_status = LG_ARMLESS;
    }
}

void lara_as_climbing(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->enable_hit = 0;
    coll->enable_baddie_push = 0;
    camera.target_elevation = CAM_E_CLIMBU;
}

void lara_as_climbend(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->enable_hit = 0;
    coll->enable_baddie_push = 0;
    camera.flags = FOLLOW_CENTRE;
    camera.target_angle = -45 * ONE_DEGREE;
}

void lara_as_climbdown(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->enable_hit = 0;
    coll->enable_baddie_push = 0;
    camera.target_elevation = CAM_E_CLIMBD;
}

void lara_col_climbleft(ITEM_INFO *item, COLL_INFO *coll)
{
    int result, shift;

    if (LaraCheckForLetGo(item, coll))
        return;
    lara.move_angle = item->pos.y_rot - 16384;
    result = LaraTestClimbPos(item, coll->radius, -(coll->radius + CLIMB_WIDTHL), -CLIMB_HITE, CLIMB_HITE, &shift);
    LaraDoClimbLeftRight(item, coll, result, shift);
}

int LaraCheckForLetGo(ITEM_INFO *item, COLL_INFO *coll)
{
    sint16 room_number;
    FLOOR_INFO *floor;

    item->gravity_status = item->fallspeed = 0;
    room_number = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    coll->trigger = trigger_index;
    if (!(input & IN_ACTION) || item->hit_points <= 0)
    {
        lara.head_x_rot = lara.head_y_rot = lara.torso_x_rot = lara.torso_y_rot = 0;
        item->goal_anim_state = AS_FORWARDJUMP;
        item->current_anim_state = AS_FORWARDJUMP;
        item->anim_number = FALLDOWN_A;
        item->frame_number = FALLDOWN_F;
        item->gravity_status = 1;
        item->speed = 2;
        item->fallspeed = 1;
        lara.gun_status = LG_ARMLESS;
        return 1;
    }
    return 0;
}

void AnimateLara(ITEM_INFO *item);

void LaraDoClimbLeftRight(ITEM_INFO *item, COLL_INFO *coll, int result, int shift)
{
    if (result == 1)
    {
        if (input & IN_LEFT)
            item->goal_anim_state = AS_CLIMBLEFT;
        else if (input & IN_RIGHT)
            item->goal_anim_state = AS_CLIMBRIGHT;
        else
            item->goal_anim_state = AS_CLIMBSTNC;
        item->pos.y_pos += shift;
    }
    else if (!result)
    {
        item->pos.x_pos = coll->old.x;
        item->pos.z_pos = coll->old.z;
        item->goal_anim_state = AS_CLIMBSTNC;
        item->current_anim_state = AS_CLIMBSTNC;
        if (coll->old_anim_state != AS_CLIMBSTNC)
        {
            item->frame_number = GF(164, 0);
            item->anim_number = 164;
        }
        else
        {
            item->frame_number = coll->old_frame_number;
            item->anim_number = coll->old_anim_number;
            AnimateLara(item);
        }
    }
    else
    {
        item->goal_anim_state = AS_HANG;
        do
            AnimateItem(item);
        while (item->current_anim_state != AS_HANG);
        item->pos.x_pos = coll->old.x;
        item->pos.z_pos = coll->old.z;
    }
}

void lara_col_climbright(ITEM_INFO *item, COLL_INFO *coll)
{
    int result, shift;

    if (LaraCheckForLetGo(item, coll))
        return;
    lara.move_angle = item->pos.y_rot + 16384;
    result = LaraTestClimbPos(item, coll->radius, coll->radius + CLIMB_WIDTHR, -CLIMB_HITE, CLIMB_HITE, &shift);
    LaraDoClimbLeftRight(item, coll, result, shift);
}

int LaraTestClimbUpPos(ITEM_INFO *item, int front, int right, int *shift, int *ledge);

void lara_col_climbing(ITEM_INFO *item, COLL_INFO *coll)
{
    int result_r, result_l, shift_r, shift_l, ledge_r, ledge_l;
    int yshift, frame;

    if (LaraCheckForLetGo(item, coll))
        return;
    if (item->anim_number == 161) {
        frame = item->frame_number - anims[161].frame_base;
        if (frame == 0)
            yshift = 0;
        else if (frame == 28 || frame == 29)
            yshift = -STEP_L;
        else if (frame == 57)
            yshift = -STEP_L * 2;
        else
            return;
    } else
        return;
    item->pos.y_pos += yshift - STEP_L;
    result_r = LaraTestClimbUpPos(item, coll->radius, coll->radius + CLIMB_WIDTHR, &shift_r, &ledge_r);
    result_l = LaraTestClimbUpPos(item, coll->radius, -(coll->radius + CLIMB_WIDTHL), &shift_l, &ledge_l);
    item->pos.y_pos += STEP_L;
    if (!result_r || !result_l || !(input & IN_FORWARD)) {
        item->goal_anim_state = AS_CLIMBSTNC;
        if (yshift)
            AnimateLara(item);
        return;
    }
    if (result_r < 0 || result_l < 0) {
        item->goal_anim_state = AS_CLIMBSTNC;
        AnimateLara(item);
        if (ABS(ledge_l - ledge_r) <= 120) {
            item->goal_anim_state = AS_NULL;
            item->pos.y_pos += (ledge_r + ledge_l) / 2 - STEP_L;
        }
        return;
    }
    item->goal_anim_state = AS_CLIMBING;
    item->pos.y_pos -= yshift;
}
