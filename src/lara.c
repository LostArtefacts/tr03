#include "game.h"
#include <stdlib.h>

#define DAMAGE_START 140
#define DAMAGE_LENGTH 14
#define FASTFALL_FRICTION item->speed = (item->speed * 95) / 100
#define STEPUP_HEIGHT ((STEP_L * 3) / 2)
#define CAM_A_HANG 0
#define CAM_E_HANG (-60 * ONE_DEGREE)
extern sint32 wibble;

sint32 GetWaterHeight(sint32 x, sint32 y, sint32 z, sint16 room_number);
void TriggerWaterfallMist(sint32 x, sint32 y, sint32 z, sint32 angle);
int LaraTestHangJump(ITEM_INFO *item, COLL_INFO *coll);
sint16 TestMonkeyLeft(ITEM_INFO *item, COLL_INFO *coll);
sint16 TestMonkeyRight(ITEM_INFO *item, COLL_INFO *coll);

#define MONKEYHANG_A 234
#define MONKEYHANG_F GF(MONKEYHANG_A, 0)
#define BAD_JUMP_CEILING ((STEP_L * 3) / 4)
#define NO_BAD_POS (-NO_HEIGHT)
#define LARA_DUCK_DEFLECT (2 * ONE_DEGREE)
#define LARA_DEF_ADD_EDGE (5 * ONE_DEGREE)
#define HEAD_TURN (2 * ONE_DEGREE)
#define MAX_HEAD_TILT (30 * ONE_DEGREE)
#define MIN_HEAD_TILT (-35 * ONE_DEGREE)
#define MAX_HEAD_ROTATION (44 * ONE_DEGREE)
#define HANG_HEIGHT 600
#define NO_BAD_NEG NO_HEIGHT
#define LANDFAR_A 24
#define LANDFAR_F GF(LANDFAR_A, 0)
#define SWAMP 128
#define FALLBACK_A 93
#define FALLBACK_F GF(FALLBACK_A, 0)
#define SWAMP_GRAVITY 2

int LaraTestClimbPos(ITEM_INFO *item, int front, int right, int origin, int height, int *shift);

#define SLIDE_A 70
#define SLIDE_F GF(SLIDE_A, 0)
#define SLIDEBACK_A 104
#define SLIDEBACK_F GF(SLIDEBACK_A, 0)

extern PHD_ANGLE old_ang;

int TestLaraSlide(ITEM_INFO *item, COLL_INFO *coll)
{
    PHD_ANGLE adif, ang;

    if (ABS(coll->tilt_x) <= 2 && ABS(coll->tilt_z) <= 2)
        return 0;

    ang = 0;
    if (coll->tilt_x > 2)
        ang = -16384;
    else if (coll->tilt_x < -2)
        ang = 16384;
    if (coll->tilt_z > 2 && coll->tilt_z > ABS(coll->tilt_x))
        ang = -32768;
    else if (coll->tilt_z < -2 && -coll->tilt_z > ABS(coll->tilt_x))
        ang = 0;

    adif = ang - item->pos.y_rot;
    ShiftItem(item, coll);
    if (adif >= -16384 && adif <= 16384) {
        if (item->current_anim_state != AS_SLIDE || old_ang != ang) {
            item->anim_number = SLIDE_A;
            item->frame_number = SLIDE_F;
            item->goal_anim_state = AS_SLIDE;
            item->current_anim_state = AS_SLIDE;
            item->pos.y_rot = ang;
            lara.move_angle = ang;
            old_ang = ang;
        }
    } else {
        if (item->current_anim_state != AS_SLIDEBACK || old_ang != ang) {
            item->anim_number = SLIDEBACK_A;
            item->frame_number = SLIDEBACK_F;
            item->goal_anim_state = AS_SLIDEBACK;
            item->current_anim_state = AS_SLIDEBACK;
            item->pos.y_rot = ang - 32768;
            lara.move_angle = ang;
            old_ang = ang;
        }
    }
    return 1;
}

int LaraTestClimbStance(ITEM_INFO *item, COLL_INFO *coll)
{
    int shift_l, result_r, shift_r, result_l;

    result_r = LaraTestClimbPos(item, coll->radius, coll->radius + 120, -700, STEP_L * 2, &shift_r);
    if (result_r != 1)
        return 0;
    result_l = LaraTestClimbPos(item, coll->radius, -(coll->radius + 120), -700, STEP_L * 2, &shift_l);
    if (result_l != 1)
        return 0;
    if (shift_r) {
        if (shift_l) {
            if ((shift_l < 0) ^ (shift_r < 0))
                return 0;
            if (shift_r < 0 && shift_l < shift_r)
                shift_r = shift_l;
            else if (shift_r > 0 && shift_l > shift_r)
                shift_r = shift_l;
        }
        item->pos.y_pos += shift_r;
    } else if (shift_l) {
        item->pos.y_pos += shift_l;
    }
    return 1;
}

void AnimateLara(ITEM_INFO *item);

void lara_slide_slope(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->bad_pos = NO_BAD_POS;
    coll->bad_neg = -512;
    coll->bad_ceiling = 0;
    GetLaraCollisionInfo(item, coll);
    if (LaraHitCeiling(item, coll))
        return;

    LaraDeflectEdge(item, coll);
    if (coll->mid_floor > 200) {
        if (item->current_anim_state == AS_SLIDE) {
            item->anim_number = FALLDOWN_A;
            item->frame_number = FALLDOWN_F;
            item->current_anim_state = AS_FORWARDJUMP;
            item->goal_anim_state = AS_FORWARDJUMP;
        } else {
            item->anim_number = FALLBACK_A;
            item->frame_number = FALLBACK_F;
            item->current_anim_state = AS_FALLBACK;
            item->goal_anim_state = AS_FALLBACK;
        }
        StopSoundEffect(3);
        item->gravity_status = 1;
        item->fallspeed = 0;
        return;
    }

    TestLaraSlide(item, coll);
    item->pos.y_pos += coll->mid_floor;
    if (ABS(coll->tilt_x) <= 2 && ABS(coll->tilt_z) <= 2) {
        item->goal_anim_state = AS_STOP;
        StopSoundEffect(3);
    }
}

void GetLaraCollisionInfo(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->facing = lara.move_angle;
    GetCollisionInfo(coll, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, LARA_HITE);
}

#define FASTSPLAT_A 32
#define FASTSPLAT_F GF(FASTSPLAT_A, 1)
#define LAND_A 31
#define LAND_F GF(LAND_A, 0)

void LaraSlideEdgeJump(ITEM_INFO *item, COLL_INFO *coll)
{
    ShiftItem(item, coll);
    switch (coll->coll_type)
    {
    case COLL_LEFT:
        item->pos.y_rot += LARA_DEF_ADD_EDGE;
        break;
    case COLL_RIGHT:
        item->pos.y_rot -= LARA_DEF_ADD_EDGE;
        break;
    case COLL_TOP:
    case COLL_TOPFRONT:
        if (item->fallspeed <= 0)
            item->fallspeed = 1;
        break;
    case COLL_CLAMP:
        item->pos.z_pos -= (phd_cos(coll->facing) * 100) >> W2V_SHIFT;
        item->pos.x_pos -= (phd_sin(coll->facing) * 100) >> W2V_SHIFT;
        item->speed = 0;
        coll->mid_floor = 0;
        if (item->fallspeed <= 0)
            item->fallspeed = 16;
        break;
    default:
        break;
    }
}

int LaraTestHangOnClimbWall(ITEM_INFO *item, COLL_INFO *coll)
{
    int shift, result;
    sint16 *bounds;

    if (!lara.climb_status || item->fallspeed < 0)
        return 0;
    switch ((uint16)(item->pos.y_rot + 0x2000) / 0x4000)
    {
    case 0:
    case 2:
        item->pos.z_pos += coll->shift.z;
        break;
    case 1:
    case 3:
        item->pos.x_pos += coll->shift.x;
        break;
    }
    bounds = GetBoundsAccurate(item);
    if (!LaraTestClimbPos(item, coll->radius, coll->radius, bounds[2], bounds[3] - bounds[2], &shift) || !LaraTestClimbPos(item, coll->radius, -coll->radius, bounds[2], bounds[3] - bounds[2], &shift))
        return 0;
    result = LaraTestClimbPos(item, coll->radius, 0, bounds[2], bounds[3] - bounds[2], &shift);
    if (!result)
        return 0;
    else if (result == 1)
        return 1;
    item->pos.y_pos += shift;
    return 1;
}

int TestHangSwingIn(ITEM_INFO *item, PHD_ANGLE angle)
{
    int x, y, z;
    int h, c;
    FLOOR_INFO *floor;
    sint16 room_num;

    x = item->pos.x_pos;
    y = item->pos.y_pos;
    z = item->pos.z_pos;
    room_num = item->room_number;
    switch (angle) {
    case 0:
        z += 256;
        break;
    case 16384:
        x += 256;
        break;
    case -16384:
        x -= 256;
        break;
    case -32768:
        z -= 256;
        break;
    }
    floor = GetFloor(x, y, z, &room_num);
    h = GetHeight(floor, x, y, z);
    c = GetCeiling(floor, x, y, z);
    if (h != NO_HEIGHT) {
        if (h - y > 0 && c - y < -400 && y - 819 - c > -72)
            return 1;
    }
    return 0;
}

void lara_col_duck(ITEM_INFO *item, COLL_INFO *coll)
{
    item->gravity_status = item->fallspeed = 0;
    coll->facing = lara.move_angle = item->pos.y_rot;
    coll->radius = LARA_RAD * 2;
    coll->bad_pos = STEPUP_HEIGHT;
    coll->bad_neg = -STEPUP_HEIGHT;
    coll->bad_ceiling = 0;
    coll->slopes_are_walls = 1;
    GetCollisionInfo(coll, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, LARA_DUCK_HEIGHT);
    if (LaraFallen(item, coll))
    {
        lara.gun_status = LG_ARMLESS;
        return;
    }
    if (coll->mid_ceiling >= -(LARA_HITE - LARA_DUCK_HEIGHT))
        lara.keep_ducked = 1;
    else
        lara.keep_ducked = 0;
    ShiftItem(item, coll);
    item->pos.y_pos += coll->mid_floor;
    if ((!(input & IN_DUCK) || lara.water_status == LARA_WADE) && !lara.keep_ducked && item->anim_number == 222)
        item->goal_anim_state = AS_STOP;
}

void lara_as_all4s(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_DEATH;
        return;
    }

    if (input & IN_LOOK)
        LookUpDown();

    lara.torso_x_rot = lara.torso_y_rot = 0;
    coll->enable_hit = 0;
    coll->enable_baddie_push = 1;
    if (item->anim_number == 258)
        lara.gun_status = LG_HANDSBUSY;

    if (TestLaraSlide(item, coll))
        return;

    camera.target_elevation = -23 * ONE_DEGREE;
}

void lara_as_crawl(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_ALL4S;
        return;
    }

    if (input & IN_LOOK)
        LookUpDown();
    lara.torso_x_rot = lara.torso_y_rot = 0;
    coll->enable_hit = 0;
    coll->enable_baddie_push = 1;
    camera.target_elevation = -23 * ONE_DEGREE;
    if (TestLaraSlide(item, coll))
        return;

    if (!(input & IN_FORWARD) || (!(input & IN_DUCK) && !lara.keep_ducked))
        item->goal_anim_state = AS_ALL4S;
    else if (input & IN_LEFT)
    {
        lara.turn_rate -= LARA_TURN_RATE;
        if (lara.turn_rate < -LARA_JUMP_TURN)
            lara.turn_rate = -LARA_JUMP_TURN;
    }
    else if (input & IN_RIGHT)
    {
        lara.turn_rate += LARA_TURN_RATE;
        if (lara.turn_rate > LARA_JUMP_TURN)
            lara.turn_rate = LARA_JUMP_TURN;
    }
}

void lara_col_crawl(ITEM_INFO *item, COLL_INFO *coll)
{
    item->gravity_status = item->fallspeed = 0;
    lara.move_angle = item->pos.y_rot;
    coll->radius = LARA_RAD * 2;
    coll->bad_pos = STEP_L - 1;
    coll->bad_neg = -(STEP_L - 1);
    coll->bad_ceiling = LARA_DUCK_HEIGHT;
    coll->slopes_are_walls = 1;
    coll->slopes_are_pits = 1;
    coll->facing = lara.move_angle;
    GetCollisionInfo(coll, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, LARA_DUCK_HEIGHT);
    if (LaraDeflectEdgeDuck(item, coll))
    {
        item->current_anim_state = AS_ALL4S;
        item->goal_anim_state = AS_ALL4S;
        if (item->anim_number != 263)
        {
            item->anim_number = 263;
            item->frame_number = GF(263, 0);
        }
        return;
    }
    if (LaraFallen(item, coll))
    {
        lara.gun_status = LG_ARMLESS;
        return;
    }
    ShiftItem(item, coll);
    item->pos.y_pos += coll->mid_floor;
}

int LaraDeflectEdgeDuck(ITEM_INFO *item, COLL_INFO *coll)
{
    if (coll->coll_type == COLL_FRONT || coll->coll_type == COLL_TOPFRONT)
    {
        ShiftItem(item, coll);
        item->gravity_status = 0;
        item->speed = 0;
        return 1;
    }
    else if (coll->coll_type == COLL_LEFT)
    {
        ShiftItem(item, coll);
        item->pos.y_rot += LARA_DUCK_DEFLECT;
    }
    else if (coll->coll_type == COLL_RIGHT)
    {
        ShiftItem(item, coll);
        item->pos.y_rot -= LARA_DUCK_DEFLECT;
    }
    return 0;
}

void lara_as_all4turnl(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_ALL4S;
        return;
    }

    coll->enable_hit = 0;
    coll->enable_baddie_push = 1;
    lara.torso_x_rot = lara.torso_y_rot = 0;
    camera.target_elevation = -23 * ONE_DEGREE;
    if (TestLaraSlide(item, coll))
        return;

    item->pos.y_rot -= ONE_DEGREE + ONE_DEGREE / 2;
    if (!(input & IN_LEFT))
        item->goal_anim_state = AS_ALL4S;
}

void lara_col_all4turnl(ITEM_INFO *item, COLL_INFO *coll)
{
    GetCollisionInfo(coll, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, LARA_DUCK_HEIGHT);
}

void lara_as_all4turnr(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_ALL4S;
        return;
    }

    coll->enable_hit = 0;
    coll->enable_baddie_push = 1;
    lara.torso_x_rot = lara.torso_y_rot = 0;
    camera.target_elevation = -23 * ONE_DEGREE;
    if (TestLaraSlide(item, coll))
        return;

    item->pos.y_rot += ONE_DEGREE + ONE_DEGREE / 2;
    if (!(input & IN_RIGHT))
        item->goal_anim_state = AS_ALL4S;
}

void lara_as_crawlb(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_ALL4S;
        return;
    }

    if (input & IN_LOOK)
        LookUpDown();
    coll->enable_hit = 0;
    coll->enable_baddie_push = 1;
    lara.torso_x_rot = lara.torso_y_rot = 0;
    camera.target_elevation = -23 * ONE_DEGREE;
    if (TestLaraSlide(item, coll))
        return;

    if (!(input & IN_BACK))
        item->goal_anim_state = AS_ALL4S;
    else if (input & IN_RIGHT)
    {
        lara.turn_rate -= LARA_TURN_RATE;
        if (lara.turn_rate < -LARA_JUMP_TURN)
            lara.turn_rate = -LARA_JUMP_TURN;
    }
    else if (input & IN_LEFT)
    {
        lara.turn_rate += LARA_TURN_RATE;
        if (lara.turn_rate > LARA_JUMP_TURN)
            lara.turn_rate = LARA_JUMP_TURN;
    }
}

void lara_col_crawlb(ITEM_INFO *item, COLL_INFO *coll)
{
    item->gravity_status = item->fallspeed = 0;
    coll->radius = 250;
    coll->bad_pos = STEP_L - 1;
    coll->bad_neg = -(STEP_L - 1);
    coll->bad_ceiling = LARA_DUCK_HEIGHT;
    coll->slopes_are_walls = 1;
    coll->slopes_are_pits = 1;
    coll->facing = lara.move_angle = item->pos.y_rot - 32768;
    GetCollisionInfo(coll, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, LARA_DUCK_HEIGHT);
    if (LaraDeflectEdgeDuck(item, coll))
    {
        item->current_anim_state = AS_ALL4S;
        item->goal_anim_state = AS_ALL4S;
        if (item->anim_number != 263)
        {
            item->anim_number = 263;
            item->frame_number = GF(263, 0);
        }
        return;
    }
    if (LaraFallen(item, coll))
    {
        lara.gun_status = LG_ARMLESS;
        return;
    }
    ShiftItem(item, coll);
    item->pos.y_pos += coll->mid_floor;
    lara.move_angle = item->pos.y_rot;
}

extern uint16 DashTimer;
int TestLaraVault(ITEM_INFO *item, COLL_INFO *coll);
int TestWall(ITEM_INFO *item, sint32 front, sint32 right, sint32 down);
void LaraCollideStop(ITEM_INFO *item, COLL_INFO *coll);

#define HITWALLLEFT_A 53
#define HITWALLLEFT_F GF(HITWALLLEFT_A, 0)
#define RUNSTEPUP_LEFT_A 56
#define RUNSTEPUP_LEFT_F GF(RUNSTEPUP_LEFT_A, 0)
#define RUNSTEPUP_RIGHT_A 55
#define RUNSTEPUP_RIGHT_F GF(RUNSTEPUP_RIGHT_A, 0)
#define LARA_DASH_LEAN_RATE ((ONE_DEGREE / 2) + LARA_LEAN_UNDO)
#define LARA_DASH_LEAN_MAX ((ONE_DEGREE * 15) + LARA_LEAN_UNDO)
#define LARA_DASH_TURN_RATE ((ONE_DEGREE / 4) + LARA_TURN_UNDO)
#define LARA_DASH_TURN_MAX ((ONE_DEGREE * 2) + LARA_TURN_UNDO)

int TestWall(ITEM_INFO *item, sint32 front, sint32 right, sint32 down)
{
    int x, y, z;
    int h, c;
    FLOOR_INFO *floor;
    sint16 room_num;
    PHD_ANGLE angle;

    x = item->pos.x_pos;
    y = item->pos.y_pos + down;
    z = item->pos.z_pos;
    room_num = item->room_number;
    angle = (uint16)(item->pos.y_rot + 0x2000) / 0x4000;
    switch (angle)
    {
    case 0:
        x -= right;
        break;
    case 3:
        z += right;
        break;
    case 2:
        x += right;
        break;
    case 1:
        z -= right;
        break;
    }
    floor = GetFloor(x, y, z, &room_num);
    switch (angle)
    {
    case 0:
        z += front;
        break;
    case 3:
        x -= front;
        break;
    case 2:
        z -= front;
        break;
    case 1:
        x += front;
        break;
    }
    floor = GetFloor(x, y, z, &room_num);
    h = GetHeight(floor, x, y, z);
    c = GetCeiling(floor, x, y, z);
    if (h != NO_HEIGHT)
    {
        if (h - y > 0 && c - y < 0)
            return 0;
        return 2;
    }
    return 1;
}

void LaraCollideStop(ITEM_INFO *item, COLL_INFO *coll)
{
    switch (coll->old_anim_state)
    {
    case AS_STOP:
    case AS_TURN_L:
    case AS_TURN_R:
    case AS_FASTTURN:
        item->current_anim_state = coll->old_anim_state;
        item->anim_number = coll->old_anim_number;
        item->frame_number = coll->old_frame_number;
        if (input & IN_LEFT)
            item->goal_anim_state = AS_TURN_L;
        else if (input & IN_RIGHT)
            item->goal_anim_state = AS_TURN_R;
        else
            item->goal_anim_state = AS_STOP;
        AnimateLara(item);
        break;
    default:
        item->anim_number = STOP_A;
        item->frame_number = STOP_F;
    }
}

void lara_as_dashdive(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->goal_anim_state != AS_DEATH && item->goal_anim_state != AS_STOP && item->goal_anim_state != AS_RUN)
    {
        if (item->fallspeed > LARA_FASTFALL_SPEED)
            item->goal_anim_state = AS_FASTFALL;
    }
}

void lara_as_hang2(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_STOP;
        return;
    }

    coll->enable_hit = 0;
    coll->enable_baddie_push = 0;
    lara.torso_x_rot = lara.torso_y_rot = 0;
    if (lara.CanMonkeySwing)
    {
        if (!(input & IN_ACTION) || item->hit_points <= 0)
        {
            MonkeySwingFall(item);
            return;
        }
        camera.target_angle = CAM_A_HANG;
        camera.target_elevation = CAM_E_HANG;
    }
    else if (input & IN_LOOK)
        LookUpDown();
}

void MonkeySwingFall(ITEM_INFO *item)
{
    item->goal_anim_state = AS_UPJUMP;
    item->current_anim_state = AS_UPJUMP;
    item->anim_number = STOPHANG_A;
    item->frame_number = STOPHANG_F;
    item->gravity_status = 1;
    item->speed = 2;
    item->fallspeed = 1;
    item->pos.y_pos += 256;
    lara.gun_status = LG_ARMLESS;
}

void MonkeySwingSnap(ITEM_INFO *item, COLL_INFO *coll)
{
    sint16 room_num;
    FLOOR_INFO *floor;

    room_num = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_num);
    item->pos.y_pos = GetCeiling(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos) + (768 - 64);
}

sint16 TestMonkeyLeft(ITEM_INFO *item, COLL_INFO *coll)
{
    sint16 oct;

    coll->bad_pos = NO_BAD_POS;
    coll->bad_neg = NO_BAD_NEG;
    coll->bad_ceiling = 0;
    coll->facing = lara.move_angle = item->pos.y_rot - 16384;
    coll->radius = LARA_RAD;
    coll->slopes_are_walls = 0;
    GetCollisionInfo(coll, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, HANG_HEIGHT);
    if (abs(coll->mid_ceiling - coll->front_ceiling) > 50)
        return 0;
    if (coll->coll_type)
    {
        oct = GetDirOctant(item->pos.y_rot);
        if ((oct == 0 && coll->coll_type == COLL_FRONT) || (oct == 0 && coll->coll_type == COLL_LEFT) || (oct == 1 && coll->coll_type == COLL_RIGHT) || (oct == 1 && coll->coll_type == COLL_LEFT))
            return 0;
    }
    return 1;
}

sint16 GetDirOctant(sint32 rot)
{
    if (abs(rot) < 8192 || abs(rot) > 24576)
        return 0;
    else
        return 1;
}

sint16 TestMonkeyRight(ITEM_INFO *item, COLL_INFO *coll)
{
    sint16 oct;

    coll->bad_pos = NO_BAD_POS;
    coll->bad_neg = -STEPUP_HEIGHT;
    coll->bad_ceiling = 0;
    coll->facing = lara.move_angle = item->pos.y_rot + 16384;
    coll->radius = LARA_RAD;
    coll->slopes_are_walls = 0;
    GetCollisionInfo(coll, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, HANG_HEIGHT);
    if (abs(coll->mid_ceiling - coll->front_ceiling) > 50)
        return 0;
    if (coll->coll_type)
    {
        oct = GetDirOctant(item->pos.y_rot);
        if ((oct == 0 && coll->coll_type == COLL_FRONT) || (oct == 1 && coll->coll_type == COLL_FRONT) || (oct == 1 && coll->coll_type == COLL_RIGHT) || (oct == 1 && coll->coll_type == COLL_LEFT))
            return 0;
    }
    return 1;
}

void lara_as_monkeyswing(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_HANG2;
        return;
    }
    coll->enable_hit = 0;
    coll->enable_baddie_push = 0;
    lara.torso_x_rot = lara.torso_y_rot = 0;
    if (input & IN_FORWARD)
        item->goal_anim_state = AS_MONKEYSWING;
    else
        item->goal_anim_state = AS_HANG2;
    if (input & IN_LEFT)
    {
        lara.turn_rate -= LARA_TURN_RATE;
        if (lara.turn_rate < -LARA_JUMP_TURN)
            lara.turn_rate = -LARA_JUMP_TURN;
    }
    else if (input & IN_RIGHT)
    {
        lara.turn_rate += LARA_TURN_RATE;
        if (lara.turn_rate > LARA_JUMP_TURN)
            lara.turn_rate = LARA_JUMP_TURN;
    }
}

void lara_col_monkeyswing(ITEM_INFO *item, COLL_INFO *coll)
{
    if (!(input & IN_ACTION) || !lara.CanMonkeySwing)
    {
        MonkeySwingFall(item);
        return;
    }
    coll->bad_pos = NO_BAD_POS;
    coll->bad_neg = NO_BAD_NEG;
    coll->bad_ceiling = 0;
    coll->facing = lara.move_angle = item->pos.y_rot;
    coll->radius = LARA_RAD;
    coll->slopes_are_walls = 0;
    GetCollisionInfo(coll, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, HANG_HEIGHT);
    if (coll->coll_type == COLL_FRONT || abs(coll->mid_ceiling - coll->front_ceiling) > 50)
    {
        item->current_anim_state = AS_HANG2;
        item->goal_anim_state = AS_HANG2;
        item->anim_number = MONKEYHANG_A;
        item->frame_number = MONKEYHANG_F;
        return;
    }
    camera.target_elevation = 10 * ONE_DEGREE;
    MonkeySwingSnap(item, coll);
}

void lara_as_monkeyl(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_HANG2;
        return;
    }

    coll->enable_hit = 0;
    coll->enable_baddie_push = 0;
    lara.torso_x_rot = lara.torso_y_rot = 0;
    if (input & IN_STEPL)
        item->goal_anim_state = AS_MONKEYL;
    else
        item->goal_anim_state = AS_HANG2;
    camera.target_elevation = 10 * ONE_DEGREE;
}

void lara_col_monkeyl(ITEM_INFO *item, COLL_INFO *coll)
{
    if (!(input & IN_ACTION) || !lara.CanMonkeySwing)
    {
        MonkeySwingFall(item);
        return;
    }
    if (!TestMonkeyLeft(item, coll))
    {
        item->current_anim_state = AS_HANG2;
        item->goal_anim_state = AS_HANG2;
        item->anim_number = MONKEYHANG_A;
        item->frame_number = MONKEYHANG_F;
    }
    else
        MonkeySwingSnap(item, coll);
}

void lara_as_monkeyr(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_HANG2;
        return;
    }

    lara.torso_x_rot = lara.torso_y_rot = 0;
    coll->enable_hit = 0;
    coll->enable_baddie_push = 0;
    if (input & IN_STEPR)
        item->goal_anim_state = AS_MONKEYR;
    else
        item->goal_anim_state = AS_HANG2;
    camera.target_elevation = 10 * ONE_DEGREE;
}

void lara_col_monkeyr(ITEM_INFO *item, COLL_INFO *coll)
{
    if (!(input & IN_ACTION) || !lara.CanMonkeySwing)
    {
        MonkeySwingFall(item);
        return;
    }
    if (!TestMonkeyRight(item, coll))
    {
        item->current_anim_state = AS_HANG2;
        item->goal_anim_state = AS_HANG2;
        item->anim_number = MONKEYHANG_A;
        item->frame_number = MONKEYHANG_F;
    }
    else
        MonkeySwingSnap(item, coll);
}

void lara_as_hangturnl(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_HANG2;
        return;
    }

    camera.target_elevation = 10 * ONE_DEGREE;
    lara.torso_x_rot = lara.torso_y_rot = 0;
    item->pos.y_rot -= ONE_DEGREE + ONE_DEGREE / 2;
    if (!(input & IN_LEFT))
        item->goal_anim_state = AS_HANG2;
}

void lara_col_hangturnlr(ITEM_INFO *item, COLL_INFO *coll)
{
    if (!(input & IN_ACTION) || !lara.CanMonkeySwing)
    {
        MonkeySwingFall(item);
        return;
    }
    coll->bad_pos = NO_BAD_POS;
    coll->bad_neg = -STEPUP_HEIGHT;
    coll->bad_ceiling = 0;
    coll->facing = lara.move_angle = item->pos.y_rot;
    coll->radius = LARA_RAD;
    coll->slopes_are_walls = 1;
    GetCollisionInfo(coll, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, HANG_HEIGHT);
    MonkeySwingSnap(item, coll);
}

void lara_as_hangturnr(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_HANG2;
        return;
    }

    lara.torso_x_rot = lara.torso_y_rot = 0;
    camera.target_elevation = 10 * ONE_DEGREE;
    item->pos.y_rot += ONE_DEGREE + ONE_DEGREE / 2;
    if (!(input & IN_RIGHT))
        item->goal_anim_state = AS_HANG2;
}

void lara_as_monkey180(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->enable_hit = 0;
    coll->enable_baddie_push = 0;
    item->goal_anim_state = AS_HANG2;
}

void lara_col_monkey180(ITEM_INFO *item, COLL_INFO *coll)
{
    lara_col_monkeyswing(item, coll);
}

void lara_col_stop(ITEM_INFO *item, COLL_INFO *coll)
{
    item->gravity_status = item->fallspeed = 0;
    lara.move_angle = item->pos.y_rot;
    coll->bad_pos = STEPUP_HEIGHT;
    coll->bad_neg = -STEPUP_HEIGHT;
    coll->bad_ceiling = 0;
    coll->slopes_are_pits = 1;
    coll->slopes_are_walls = 1;
    GetLaraCollisionInfo(item, coll);
    if (LaraHitCeiling(item, coll))
        return;
    if (LaraFallen(item, coll))
        return;
    if (TestLaraSlide(item, coll))
        return;
    ShiftItem(item, coll);
    if (!(room[item->room_number].flags & SWAMP) || coll->mid_floor < 0)
        item->pos.y_pos += coll->mid_floor;
    else if ((room[item->room_number].flags & SWAMP) && coll->mid_floor)
        item->pos.y_pos += SWAMP_GRAVITY;
}

void lara_as_upjump(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->fallspeed > LARA_FASTFALL_SPEED)
        item->goal_anim_state = AS_FASTFALL;
}

int LaraTestHangJumpUp(ITEM_INFO *item, COLL_INFO *coll);

void lara_as_forwardjump(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->goal_anim_state == AS_SWANDIVE || item->goal_anim_state == AS_REACH)
        item->goal_anim_state = AS_FORWARDJUMP;
    if (item->goal_anim_state != AS_DEATH && item->goal_anim_state != AS_STOP && item->goal_anim_state != AS_RUN)
    {
        if ((input & IN_ACTION) && lara.gun_status == LG_ARMLESS)
            item->goal_anim_state = AS_REACH;
        if ((input & IN_ROLL) || (input & IN_BACK))
            item->goal_anim_state = AS_TWIST;
        if ((input & IN_SLOW) && lara.gun_status == LG_ARMLESS)
            item->goal_anim_state = AS_SWANDIVE;
        if (item->fallspeed > LARA_FASTFALL_SPEED)
            item->goal_anim_state = AS_FASTFALL;
    }
    if (input & IN_LEFT)
    {
        lara.turn_rate -= LARA_TURN_RATE;
        if (lara.turn_rate < -LARA_JUMP_TURN)
            lara.turn_rate = -LARA_JUMP_TURN;
    }
    else if (input & IN_RIGHT)
    {
        lara.turn_rate += LARA_TURN_RATE;
        if (lara.turn_rate > LARA_JUMP_TURN)
            lara.turn_rate = LARA_JUMP_TURN;
    }
}

void lara_as_walk(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_STOP;
        return;
    }
    if (input & IN_LEFT)
    {
        lara.turn_rate -= LARA_TURN_RATE;
        if (lara.turn_rate < -LARA_SLOW_TURN)
            lara.turn_rate = -LARA_SLOW_TURN;
    }
    else if (input & IN_RIGHT)
    {
        lara.turn_rate += LARA_TURN_RATE;
        if (lara.turn_rate > LARA_SLOW_TURN)
            lara.turn_rate = LARA_SLOW_TURN;
    }
    if (input & IN_FORWARD)
    {
        if (lara.water_status == LARA_WADE)
            item->goal_anim_state = AS_WADE;
        else if (!(input & IN_SLOW))
            item->goal_anim_state = AS_RUN;
        else
            item->goal_anim_state = AS_WALK;
    }
    else
        item->goal_anim_state = AS_STOP;
}

void lara_as_fastback(ITEM_INFO *item, COLL_INFO *coll)
{
    item->goal_anim_state = AS_STOP;
    if (input & IN_LEFT)
    {
        lara.turn_rate -= LARA_TURN_RATE;
        if (lara.turn_rate < -LARA_MED_TURN)
            lara.turn_rate = -LARA_MED_TURN;
    }
    else if (input & IN_RIGHT)
    {
        lara.turn_rate += LARA_TURN_RATE;
        if (lara.turn_rate > LARA_MED_TURN)
            lara.turn_rate = LARA_MED_TURN;
    }
}

void lara_as_turn_r(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_STOP;
        return;
    }
    lara.turn_rate += LARA_TURN_RATE;
    if (lara.gun_status == LG_READY && lara.water_status != LARA_WADE)
        item->goal_anim_state = AS_FASTTURN;
    else if (lara.turn_rate > LARA_SLOW_TURN)
    {
        if (!(input & IN_SLOW) && lara.water_status != LARA_WADE)
            item->goal_anim_state = AS_FASTTURN;
        else
            lara.turn_rate = LARA_SLOW_TURN;
    }
    if (input & IN_FORWARD)
    {
        if (lara.water_status == LARA_WADE)
            item->goal_anim_state = AS_WADE;
        else if (input & IN_SLOW)
            item->goal_anim_state = AS_WALK;
        else
            item->goal_anim_state = AS_RUN;
    }
    else if (!(input & IN_RIGHT))
        item->goal_anim_state = AS_STOP;
}

void lara_as_turn_l(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_STOP;
        return;
    }
    lara.turn_rate -= LARA_TURN_RATE;
    if (lara.gun_status == LG_READY && lara.water_status != LARA_WADE)
        item->goal_anim_state = AS_FASTTURN;
    else if (lara.turn_rate < -LARA_SLOW_TURN)
    {
        if (!(input & IN_SLOW) && lara.water_status != LARA_WADE)
            item->goal_anim_state = AS_FASTTURN;
        else
            lara.turn_rate = -LARA_SLOW_TURN;
    }
    if (input & IN_FORWARD)
    {
        if (lara.water_status == LARA_WADE)
            item->goal_anim_state = AS_WADE;
        else if (input & IN_SLOW)
            item->goal_anim_state = AS_WALK;
        else
            item->goal_anim_state = AS_RUN;
    }
    else if (!(input & IN_LEFT))
        item->goal_anim_state = AS_STOP;
}

void lara_as_death(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.look = 0;
    coll->enable_hit = 0;
    coll->enable_baddie_push = 0;
}

void lara_as_fastfall(ITEM_INFO *item, COLL_INFO *coll)
{
    FASTFALL_FRICTION;
    if (item->fallspeed == DAMAGE_START + DAMAGE_LENGTH)
        SoundEffect(30, &item->pos, 0);
}

void lara_as_hang(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_STOP;
        return;
    }

    if (input & IN_LOOK)
        LookUpDown();

    coll->enable_hit = 0;
    coll->enable_baddie_push = 0;
    camera.target_angle = CAM_A_HANG;
    camera.target_elevation = CAM_E_HANG;
    if (input & (IN_LEFT | IN_STEPL))
        item->goal_anim_state = AS_HANGLEFT;
    else if (input & (IN_RIGHT | IN_STEPR))
        item->goal_anim_state = AS_HANGRIGHT;
}

void lara_as_reach(ITEM_INFO *item, COLL_INFO *coll)
{
    camera.target_angle = 85 * ONE_DEGREE;
    if (item->fallspeed > LARA_FASTFALL_SPEED)
        item->goal_anim_state = AS_FASTFALL;
}

void lara_as_splat(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.look = 0;
}

sint16 LaraFloorFront(ITEM_INFO *item, PHD_ANGLE ang, sint32 dist);

void lara_as_back(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_STOP;
        return;
    }
    if (!(input & IN_BACK) || (!(input & IN_SLOW) && lara.water_status != LARA_WADE))
        item->goal_anim_state = AS_STOP;
    else
        item->goal_anim_state = AS_BACK;
    if (input & IN_LEFT)
    {
        lara.turn_rate -= LARA_TURN_RATE;
        if (lara.turn_rate < -LARA_SLOW_TURN)
            lara.turn_rate = -LARA_SLOW_TURN;
    }
    else if (input & IN_RIGHT)
    {
        lara.turn_rate += LARA_TURN_RATE;
        if (lara.turn_rate > LARA_SLOW_TURN)
            lara.turn_rate = LARA_SLOW_TURN;
    }
}

void lara_as_fastturn(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_STOP;
        return;
    }
    if (lara.turn_rate < 0)
    {
        lara.turn_rate = -LARA_FAST_TURN;
        if (!(input & IN_LEFT))
            item->goal_anim_state = AS_STOP;
    }
    else
    {
        lara.turn_rate = LARA_FAST_TURN;
        if (!(input & IN_RIGHT))
            item->goal_anim_state = AS_STOP;
    }
}

void lara_as_stepright(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.look = 0;
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_STOP;
        return;
    }

    if (!(input & IN_STEPR))
        item->goal_anim_state = AS_STOP;

    if (input & IN_LEFT)
    {
        lara.turn_rate -= LARA_TURN_RATE;
        if (lara.turn_rate < -LARA_SLOW_TURN)
            lara.turn_rate = -LARA_SLOW_TURN;
    }
    else if (input & IN_RIGHT)
    {
        lara.turn_rate += LARA_TURN_RATE;
        if (lara.turn_rate > LARA_SLOW_TURN)
            lara.turn_rate = LARA_SLOW_TURN;
    }
}

void lara_as_stepleft(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.look = 0;
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_STOP;
        return;
    }

    if (!(input & IN_STEPL))
        item->goal_anim_state = AS_STOP;

    if (input & IN_LEFT)
    {
        lara.turn_rate -= LARA_TURN_RATE;
        if (lara.turn_rate < -LARA_SLOW_TURN)
            lara.turn_rate = -LARA_SLOW_TURN;
    }
    else if (input & IN_RIGHT)
    {
        lara.turn_rate += LARA_TURN_RATE;
        if (lara.turn_rate > LARA_SLOW_TURN)
            lara.turn_rate = LARA_SLOW_TURN;
    }
}

void lara_as_slide(ITEM_INFO *item, COLL_INFO *coll)
{
    camera.flags = NO_CHUNKY;
    camera.target_elevation = -45 * ONE_DEGREE;
    if ((input & IN_JUMP) && !(input & IN_BACK))
        item->goal_anim_state = AS_FORWARDJUMP;
}

void lara_as_backjump(ITEM_INFO *item, COLL_INFO *coll)
{
    camera.target_angle = ONE_DEGREE * 135;
    if (item->fallspeed > LARA_FASTFALL_SPEED)
        item->goal_anim_state = AS_FASTFALL;
    else if (item->goal_anim_state == AS_RUN)
        item->goal_anim_state = AS_STOP;
    else if ((input & IN_FORWARD || input & IN_ROLL) && item->goal_anim_state != AS_STOP)
        item->goal_anim_state = AS_TWIST;
}

void lara_as_rightjump(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.look = 0;
    if (item->fallspeed > LARA_FASTFALL_SPEED)
        item->goal_anim_state = AS_FASTFALL;
    else if (input & IN_LEFT && item->goal_anim_state != AS_STOP)
        item->goal_anim_state = AS_TWIST;
}

void lara_as_leftjump(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.look = 0;
    if (item->fallspeed > LARA_FASTFALL_SPEED)
        item->goal_anim_state = AS_FASTFALL;
    else if (input & IN_RIGHT && item->goal_anim_state != AS_STOP)
        item->goal_anim_state = AS_TWIST;
}

void lara_as_fallback(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->fallspeed > LARA_FASTFALL_SPEED)
        item->goal_anim_state = AS_FASTFALL;
    if ((input & IN_ACTION) && lara.gun_status == LG_ARMLESS)
        item->goal_anim_state = AS_REACH;
}

void lara_as_hangleft(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->enable_hit = 0;
    coll->enable_baddie_push = 0;
    camera.target_angle = CAM_A_HANG;
    camera.target_elevation = CAM_E_HANG;
    if (!(input & IN_LEFT) && !(input & IN_STEPL))
        item->goal_anim_state = AS_HANG;
}

void lara_as_hangright(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->enable_hit = 0;
    coll->enable_baddie_push = 0;
    camera.target_angle = CAM_A_HANG;
    camera.target_elevation = CAM_E_HANG;
    if (!(input & IN_RIGHT) && !(input & IN_STEPR))
        item->goal_anim_state = AS_HANG;
}

void lara_as_slideback(ITEM_INFO *item, COLL_INFO *coll)
{
    if ((input & IN_JUMP) && !(input & IN_FORWARD))
        item->goal_anim_state = AS_BACKJUMP;
}

void lara_as_pushblock(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.look = 0;
    coll->enable_hit = 0;
    coll->enable_baddie_push = 0;
    camera.flags = FOLLOW_CENTRE;
    camera.target_angle = 35 * ONE_DEGREE;
    camera.target_elevation = -25 * ONE_DEGREE;
}

void lara_as_ppready(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->enable_hit = 0;
    coll->enable_baddie_push = 0;
    camera.target_angle = 75 * ONE_DEGREE;
    if (!(input & IN_ACTION))
        item->goal_anim_state = AS_STOP;
}

void lara_as_pickup(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.look = 0;
    coll->enable_hit = 0;
    coll->enable_baddie_push = 0;
    camera.target_angle = -130 * ONE_DEGREE;
    camera.target_elevation = -15 * ONE_DEGREE;
    camera.target_distance = WALL_L;
}

void lara_as_pickupflare(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.look = 0;
    coll->enable_hit = 0;
    coll->enable_baddie_push = 0;
    camera.target_angle = 130 * ONE_DEGREE;
    camera.target_elevation = -15 * ONE_DEGREE;
    camera.target_distance = WALL_L;
    if (item->frame_number == anims[item->anim_number].frame_end - 1)
        lara.gun_status = LG_ARMLESS;
}

void lara_as_switchon(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.look = 0;
    coll->enable_hit = 0;
    coll->enable_baddie_push = 0;
    camera.target_angle = 80 * ONE_DEGREE;
    camera.target_elevation = -25 * ONE_DEGREE;
    camera.target_distance = WALL_L;
    camera.speed = 6;
}

void lara_as_usekey(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.look = 0;
    coll->enable_hit = 0;
    coll->enable_baddie_push = 0;
    camera.target_angle = -80 * ONE_DEGREE;
    camera.target_elevation = -25 * ONE_DEGREE;
    camera.target_distance = WALL_L;
}

void lara_as_special(ITEM_INFO *item, COLL_INFO *coll)
{
    camera.flags = FOLLOW_CENTRE;
    camera.target_angle = 170 * ONE_DEGREE;
    camera.target_elevation = -25 * ONE_DEGREE;
}

void lara_as_swandive(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->enable_hit = 0;
    coll->enable_baddie_push = 1;
    if (item->fallspeed > LARA_FASTFALL_SPEED && item->goal_anim_state != AS_DIVE)
        item->goal_anim_state = AS_FASTDIVE;
}

void lara_as_fastdive(ITEM_INFO *item, COLL_INFO *coll)
{
    if ((input & IN_ROLL) && item->goal_anim_state == AS_FASTDIVE)
        item->goal_anim_state = AS_TWIST;
    coll->enable_hit = 0;
    coll->enable_baddie_push = 1;
    FASTFALL_FRICTION;
}

void lara_as_gymnast(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->enable_hit = 0;
    coll->enable_baddie_push = 0;
}

void lara_as_waterout(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->enable_hit = 0;
    coll->enable_baddie_push = 0;
    camera.flags = FOLLOW_CENTRE;
}

void lara_as_deathslide(ITEM_INFO *item, COLL_INFO *coll)
{
    sint16 room_number;
    FLOOR_INFO *floor;

    camera.target_angle = ONE_DEGREE * 70;
    room_number = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    coll->trigger = trigger_index;
    if (!(input & IN_ACTION))
    {
        item->goal_anim_state = AS_FORWARDJUMP;
        AnimateLara(item);
        lara_item->gravity_status = 1;
        lara_item->speed = 100;
        lara_item->fallspeed = 40;
        lara.move_angle = item->pos.y_rot;
    }
}

void extra_as_breath(ITEM_INFO *item, COLL_INFO *coll)
{
    item->anim_number = BREATH_A;
    item->frame_number = BREATH_F;
    item->current_anim_state = item->goal_anim_state = AS_STOP;
    lara.gun_status = LG_ARMLESS;
    camera.type = CHASE_CAMERA;
    AlterFOV(GAME_FOV * ONE_DEGREE);
    lara.extra_anim = 0;
}

void extra_as_sharkkill(ITEM_INFO *item, COLL_INFO *coll)
{
    int wh;

    camera.target_angle = ONE_DEGREE * 160;
    camera.target_distance = 3 * WALL_L;
    lara.hit_direction = -1;
    if (item->frame_number == anims[item->anim_number].frame_end)
    {
        wh = GetWaterHeight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);
        if (wh != NO_HEIGHT && wh < item->pos.y_pos - 100)
            item->pos.y_pos -= 5;
    }
    if (item->frame_number < anims[item->anim_number].frame_end - 30)
        lara.death_count = 1;
}

void extra_as_airlock(ITEM_INFO *item, COLL_INFO *coll)
{
    camera.target_angle = 80 * ONE_DEGREE;
    camera.target_elevation = -25 * ONE_DEGREE;
}

void extra_as_gongbong(ITEM_INFO *item, COLL_INFO *coll)
{
    camera.target_angle = -25 * ONE_DEGREE;
    camera.target_elevation = -20 * ONE_DEGREE;
    camera.target_distance = WALL_L * 3;
}

void extra_as_dinokill(ITEM_INFO *item, COLL_INFO *coll)
{
    camera.flags = FOLLOW_CENTRE;
    camera.target_angle = 170 * ONE_DEGREE;
    camera.target_elevation = -25 * ONE_DEGREE;
    lara.hit_direction = -1;
    if (item->frame_number < anims[item->anim_number].frame_end - 30)
        lara.death_count = 1;
}

void extra_as_startanim(ITEM_INFO *item, COLL_INFO *coll)
{
    sint16 room_number;
    FLOOR_INFO *floor;

    room_number = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    TestTriggers(trigger_index, 0);
}

void extra_as_trainkill(ITEM_INFO *item, COLL_INFO *coll)
{
    FLOOR_INFO *floor;
    sint16 room_number;

    lara.hit_direction = -1;
    room_number = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    item->pos.y_pos = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    if (item->frame_number < anims[item->anim_number].frame_end - 30)
        lara.death_count = 1;
    item->gravity_status = 0;
    item->hit_points = -1;
}

void extra_as_rapidsdrown(ITEM_INFO *item, COLL_INFO *coll)
{
    sint32 h;
    FLOOR_INFO *floor;
    sint16 room_number;

    GetLaraCollisionInfo(item, coll);
    room_number = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    h = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    item->pos.y_pos = h + 384;
    item->pos.y_rot += 1024;
    lara.death_count++;
    if ((wibble & 3) == 0)
        TriggerWaterfallMist(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, GetRandomControl() & 4095);
}

void lara_col_forwardjump(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->speed >= 0)
        lara.move_angle = item->pos.y_rot;
    else
        lara.move_angle = item->pos.y_rot - 0x8000;
    coll->bad_pos = NO_BAD_POS;
    coll->bad_neg = -STEPUP_HEIGHT;
    coll->bad_ceiling = BAD_JUMP_CEILING;
    GetLaraCollisionInfo(item, coll);
    LaraDeflectEdgeJump(item, coll);
    if (item->speed < 0)
        lara.move_angle = item->pos.y_rot;
    if (coll->mid_floor <= 0 && item->fallspeed > 0)
    {
        if (LaraLandedBad(item, coll))
            item->goal_anim_state = AS_DEATH;
        else if (lara.water_status == LARA_WADE)
            item->goal_anim_state = AS_STOP;
        else if ((input & IN_FORWARD) && !(input & IN_SLOW))
            item->goal_anim_state = AS_RUN;
        else
            item->goal_anim_state = AS_STOP;
        item->fallspeed = 0;
        item->pos.y_pos += coll->mid_floor;
        item->gravity_status = 0;
        item->speed = 0;
        AnimateLara(item);
    }
}

void lara_col_fastback(ITEM_INFO *item, COLL_INFO *coll)
{
    item->gravity_status = item->fallspeed = 0;
    lara.move_angle = item->pos.y_rot - 32768;
    coll->bad_pos = NO_BAD_POS;
    coll->bad_neg = -STEPUP_HEIGHT;
    coll->bad_ceiling = 0;
    coll->slopes_are_pits = 1;
    coll->slopes_are_walls = 0;
    GetLaraCollisionInfo(item, coll);
    if (LaraHitCeiling(item, coll))
        return;
    if (coll->mid_floor > 200)
    {
        item->anim_number = FALLBACK_A;
        item->frame_number = FALLBACK_F;
        item->current_anim_state = AS_FALLBACK;
        item->goal_anim_state = AS_FALLBACK;
        item->fallspeed = 0;
        item->gravity_status = 1;
        return;
    }
    if (TestLaraSlide(item, coll))
        return;
    if (LaraDeflectEdge(item, coll))
        LaraCollideStop(item, coll);
    item->pos.y_pos += coll->mid_floor;
}

void lara_col_turn_r(ITEM_INFO *item, COLL_INFO *coll)
{
    item->gravity_status = item->fallspeed = 0;
    lara.move_angle = item->pos.y_rot;
    coll->bad_pos = STEPUP_HEIGHT;
    coll->bad_neg = -STEPUP_HEIGHT;
    coll->bad_ceiling = 0;
    coll->slopes_are_pits = 1;
    coll->slopes_are_walls = 1;
    GetLaraCollisionInfo(item, coll);
    if (coll->mid_floor > 100 && !(room[item->room_number].flags & SWAMP))
    {
        item->anim_number = FALLDOWN_A;
        item->frame_number = FALLDOWN_F;
        item->current_anim_state = AS_FORWARDJUMP;
        item->goal_anim_state = AS_FORWARDJUMP;
        item->fallspeed = 0;
        item->gravity_status = 1;
        return;
    }
    if (TestLaraSlide(item, coll))
        return;
    if (!(room[item->room_number].flags & SWAMP) || coll->mid_floor < 0)
        item->pos.y_pos += coll->mid_floor;
    else if ((room[item->room_number].flags & SWAMP) && coll->mid_floor)
        item->pos.y_pos += SWAMP_GRAVITY;
}

void lara_col_turn_l(ITEM_INFO *item, COLL_INFO *coll)
{
    lara_col_turn_r(item, coll);
}

void lara_col_death(ITEM_INFO *item, COLL_INFO *coll)
{
    StopSoundEffect(30);
    lara.move_angle = item->pos.y_rot;
    coll->bad_pos = STEPUP_HEIGHT;
    coll->bad_neg = -STEPUP_HEIGHT;
    coll->bad_ceiling = 0;
    coll->radius = LARA_RAD * 4;
    GetLaraCollisionInfo(item, coll);
    ShiftItem(item, coll);
    item->pos.y_pos += coll->mid_floor;
    item->hit_points = -1;
    lara.air = -1;
}

void lara_col_fastfall(ITEM_INFO *item, COLL_INFO *coll)
{
    item->gravity_status = 1;
    coll->bad_pos = NO_BAD_POS;
    coll->bad_neg = -STEPUP_HEIGHT;
    coll->bad_ceiling = BAD_JUMP_CEILING;
    GetLaraCollisionInfo(item, coll);
    LaraSlideEdgeJump(item, coll);
    if (coll->mid_floor <= 0)
    {
        if (LaraLandedBad(item, coll))
        {
            item->goal_anim_state = AS_DEATH;
        }
        else
        {
            item->goal_anim_state = AS_STOP;
            item->current_anim_state = AS_STOP;
            item->frame_number = LANDFAR_F;
            item->anim_number = LANDFAR_A;
        }
        StopSoundEffect(30);
        item->pos.y_pos += coll->mid_floor;
        item->gravity_status = 0;
        item->fallspeed = 0;
    }
}

void lara_col_reach(ITEM_INFO *item, COLL_INFO *coll)
{
    item->gravity_status = 1;
    lara.move_angle = item->pos.y_rot;
    coll->bad_pos = NO_BAD_POS;
    coll->bad_neg = 0;
    coll->bad_ceiling = BAD_JUMP_CEILING;
    GetLaraCollisionInfo(item, coll);
    if (LaraTestHangJump(item, coll))
        return;
    LaraSlideEdgeJump(item, coll);
    GetLaraCollisionInfo(item, coll);
    ShiftItem(item, coll);
    if (item->fallspeed > 0 && coll->mid_floor <= 0)
    {
        if (LaraLandedBad(item, coll))
            item->goal_anim_state = AS_DEATH;
        else
            item->goal_anim_state = AS_STOP;
        item->fallspeed = 0;
        item->pos.y_pos += coll->mid_floor;
        item->gravity_status = 0;
    }
}

void lara_col_splat(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.move_angle = item->pos.y_rot;
    coll->bad_pos = STEPUP_HEIGHT;
    coll->bad_neg = -STEPUP_HEIGHT;
    coll->bad_ceiling = 0;
    coll->slopes_are_pits = 1;
    coll->slopes_are_walls = 1;
    GetLaraCollisionInfo(item, coll);
    ShiftItem(item, coll);
    if (coll->mid_floor > -STEP_L && coll->mid_floor < STEP_L)
        item->pos.y_pos += coll->mid_floor;
}

void lara_col_land(ITEM_INFO *item, COLL_INFO *coll)
{
    lara_col_stop(item, coll);
}

void lara_col_compress(ITEM_INFO *item, COLL_INFO *coll)
{
    item->gravity_status = item->fallspeed = 0;
    coll->bad_pos = NO_BAD_POS;
    coll->bad_neg = NO_BAD_NEG;
    coll->bad_ceiling = 0;
    GetLaraCollisionInfo(item, coll);
    if (coll->mid_ceiling > -100)
    {
        item->goal_anim_state = AS_STOP;
        item->current_anim_state = AS_STOP;
        item->anim_number = STOP_A;
        item->frame_number = STOP_F;
        item->speed = 0;
        item->fallspeed = 0;
        item->gravity_status = 0;
        item->pos.x_pos = coll->old.x;
        item->pos.y_pos = coll->old.y;
        item->pos.z_pos = coll->old.z;
    }
    if (coll->mid_floor > -STEP_L && coll->mid_floor < STEP_L)
        item->pos.y_pos += coll->mid_floor;
}

#define BACKSTEPD_LEFT_A 61
#define BACKSTEPD_LEFT_F GF(BACKSTEPD_LEFT_A, 0)
#define BACKSTEPD_RIGHT_A 62
#define BACKSTEPD_RIGHT_F GF(BACKSTEPD_RIGHT_A, 0)

void lara_col_stepright(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->current_anim_state == AS_STEPRIGHT)
        lara.move_angle = item->pos.y_rot + 16384;
    else
        lara.move_angle = item->pos.y_rot - 16384;
    item->gravity_status = item->fallspeed = 0;
    if (lara.water_status == LARA_WADE)
        coll->bad_pos = NO_BAD_POS;
    else
        coll->bad_pos = STEP_L / 2;
    coll->bad_neg = -STEP_L / 2;
    coll->bad_ceiling = 0;
    coll->slopes_are_pits = 1;
    coll->slopes_are_walls = 1;
    GetLaraCollisionInfo(item, coll);
    if (LaraHitCeiling(item, coll))
        return;
    if (LaraDeflectEdge(item, coll))
        LaraCollideStop(item, coll);
    if (LaraFallen(item, coll))
        return;
    if (TestLaraSlide(item, coll))
        return;
    item->pos.y_pos += coll->mid_floor;
}

void lara_col_stepleft(ITEM_INFO *item, COLL_INFO *coll)
{
    lara_col_stepright(item, coll);
}

void lara_col_slide(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.move_angle = item->pos.y_rot;
    lara_slide_slope(item, coll);
}

void lara_col_backjump(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.move_angle = item->pos.y_rot - 32768;
    lara_col_jumper(item, coll);
}

void lara_col_rightjump(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.move_angle = item->pos.y_rot + 16384;
    lara_col_jumper(item, coll);
}

void lara_col_leftjump(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.move_angle = item->pos.y_rot - 16384;
    lara_col_jumper(item, coll);
}

void lara_col_fallback(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.move_angle = item->pos.y_rot - 32768;
    coll->bad_pos = NO_BAD_POS;
    coll->bad_neg = -STEPUP_HEIGHT;
    coll->bad_ceiling = BAD_JUMP_CEILING;
    GetLaraCollisionInfo(item, coll);
    LaraDeflectEdgeJump(item, coll);
    if (coll->mid_floor <= 0 && item->fallspeed > 0)
    {
        if (LaraLandedBad(item, coll))
            item->goal_anim_state = AS_DEATH;
        else
            item->goal_anim_state = AS_STOP;
        item->fallspeed = 0;
        item->pos.y_pos += coll->mid_floor;
        item->gravity_status = 0;
    }
}

void lara_col_hangleft(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.move_angle = item->pos.y_rot - 16384;
    LaraHangTest(item, coll);
    lara.move_angle = item->pos.y_rot - 16384;
}

void lara_col_hangright(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.move_angle = item->pos.y_rot + 16384;
    LaraHangTest(item, coll);
    lara.move_angle = item->pos.y_rot + 16384;
}

void lara_col_slideback(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.move_angle = item->pos.y_rot - 32768;
    lara_slide_slope(item, coll);
}

void lara_col_pushblock(ITEM_INFO *item, COLL_INFO *coll)
{
    lara_default_col(item, coll);
}

void lara_col_roll(ITEM_INFO *item, COLL_INFO *coll)
{
    item->gravity_status = item->fallspeed = 0;
    lara.move_angle = item->pos.y_rot;
    coll->bad_pos = NO_BAD_POS;
    coll->bad_neg = -STEPUP_HEIGHT;
    coll->bad_ceiling = 0;
    coll->slopes_are_walls = 1;
    GetLaraCollisionInfo(item, coll);
    if (LaraHitCeiling(item, coll))
        return;
    if (TestLaraSlide(item, coll))
        return;
    if (LaraFallen(item, coll))
        return;
    ShiftItem(item, coll);
    item->pos.y_pos += coll->mid_floor;
}

void lara_col_roll2(ITEM_INFO *item, COLL_INFO *coll)
{
    item->gravity_status = item->fallspeed = 0;
    lara.move_angle = item->pos.y_rot - 32768;
    coll->bad_pos = NO_BAD_POS;
    coll->bad_neg = -STEPUP_HEIGHT;
    coll->bad_ceiling = 0;
    coll->slopes_are_walls = 1;
    GetLaraCollisionInfo(item, coll);
    if (LaraHitCeiling(item, coll))
        return;
    if (TestLaraSlide(item, coll))
        return;
    if (coll->mid_floor > 200)
    {
        item->anim_number = FALLBACK_A;
        item->frame_number = FALLBACK_F;
        item->current_anim_state = AS_FALLBACK;
        item->goal_anim_state = AS_FALLBACK;
        item->fallspeed = 0;
        item->gravity_status = 1;
        return;
    }
    ShiftItem(item, coll);
    item->pos.y_pos += coll->mid_floor;
}

void lara_col_swandive(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.move_angle = item->pos.y_rot;
    coll->bad_pos = NO_BAD_POS;
    coll->bad_neg = -STEPUP_HEIGHT;
    coll->bad_ceiling = BAD_JUMP_CEILING;
    GetLaraCollisionInfo(item, coll);
    LaraDeflectEdgeJump(item, coll);
    if (coll->mid_floor <= 0 && item->fallspeed > 0)
    {
        item->goal_anim_state = AS_STOP;
        item->fallspeed = 0;
        item->pos.y_pos += coll->mid_floor;
        item->gravity_status = 0;
    }
}

void lara_col_fastdive(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.move_angle = item->pos.y_rot;
    coll->bad_pos = NO_BAD_POS;
    coll->bad_neg = -STEPUP_HEIGHT;
    coll->bad_ceiling = BAD_JUMP_CEILING;
    GetLaraCollisionInfo(item, coll);
    LaraDeflectEdgeJump(item, coll);
    if (coll->mid_floor <= 0 && item->fallspeed > 0)
    {
        if (item->fallspeed > 133)
            item->goal_anim_state = AS_DEATH;
        else
            item->goal_anim_state = AS_STOP;
        item->fallspeed = 0;
        item->pos.y_pos += coll->mid_floor;
        item->gravity_status = 0;
    }
}

void lara_default_col(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.move_angle = item->pos.y_rot;
    coll->bad_pos = STEPUP_HEIGHT;
    coll->bad_neg = -STEPUP_HEIGHT;
    coll->bad_ceiling = 0;
    coll->slopes_are_pits = 1;
    coll->slopes_are_walls = 1;
    GetLaraCollisionInfo(item, coll);
}

void lara_col_jumper(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->bad_pos = NO_BAD_POS;
    coll->bad_neg = -STEPUP_HEIGHT;
    coll->bad_ceiling = BAD_JUMP_CEILING;
    GetLaraCollisionInfo(item, coll);
    LaraDeflectEdgeJump(item, coll);
    if (item->fallspeed > 0)
    {
        if (coll->mid_floor <= 0)
        {
            if (LaraLandedBad(item, coll))
                item->goal_anim_state = AS_DEATH;
            else
                item->goal_anim_state = AS_STOP;
            item->fallspeed = 0;
            item->pos.y_pos += coll->mid_floor;
            item->gravity_status = 0;
        }
    }
}

extern char GotJointPos[];
extern char IsJointUnderwater[];

void AddJointPos(ITEM_INFO *item, int mesh)
{
    sint16 room_num;

    room_num = item->room_number;
    GetFloor((phd_mxptr[3] >> W2V_SHIFT) + item->pos.x_pos,
             (phd_mxptr[7] >> W2V_SHIFT) + item->pos.y_pos,
             (phd_mxptr[11] >> W2V_SHIFT) + item->pos.z_pos, &room_num);
    if (room[room_num].flags & UNDERWATER)
        IsJointUnderwater[mesh] = 1;
    else
        IsJointUnderwater[mesh] = 0;
    GotJointPos[mesh] = 1;
}

void LookUpDown(void)
{
    camera.type = LOOK_CAMERA;
    if (input & IN_FORWARD)
    {
        input -= IN_FORWARD;
        if (lara.head_x_rot > MIN_HEAD_TILT)
            lara.head_x_rot -= HEAD_TURN;
    }
    else if (input & IN_BACK)
    {
        input -= IN_BACK;
        if (lara.head_x_rot < MAX_HEAD_TILT)
            lara.head_x_rot += HEAD_TURN;
    }
    if (lara.gun_status != LG_HANDSBUSY && lara.skidoo == NO_ITEM && lara.left_arm.lock == 0 && lara.right_arm.lock == 0)
        lara.torso_x_rot = lara.head_x_rot;
}

void LookLeftRight(void)
{
    camera.type = LOOK_CAMERA;
    if (input & IN_LEFT)
    {
        input -= IN_LEFT;
        if (lara.head_y_rot > -MAX_HEAD_ROTATION)
            lara.head_y_rot -= HEAD_TURN;
    }
    else if (input & IN_RIGHT)
    {
        input -= IN_RIGHT;
        if (lara.head_y_rot < MAX_HEAD_ROTATION)
            lara.head_y_rot += HEAD_TURN;
    }
    if (lara.gun_status != LG_HANDSBUSY && lara.skidoo == NO_ITEM && lara.left_arm.lock == 0 && lara.right_arm.lock == 0)
        lara.torso_y_rot = lara.head_y_rot;
}

void ResetLook(void)
{
    if (camera.type == LOOK_CAMERA)
        return;
    if (lara.head_x_rot > -HEAD_TURN && lara.head_x_rot < HEAD_TURN)
        lara.head_x_rot = 0;
    else
        lara.head_x_rot -= lara.head_x_rot / 8;
    if (lara.head_y_rot > -HEAD_TURN && lara.head_y_rot < HEAD_TURN)
        lara.head_y_rot = 0;
    else
        lara.head_y_rot -= lara.head_y_rot / 8;
    if (lara.gun_status != LG_HANDSBUSY && lara.skidoo == NO_ITEM)
    {
        lara.torso_x_rot = lara.head_x_rot;
        lara.torso_y_rot = lara.head_y_rot;
    }
}

extern sint16 DeathSlideBounds[];
extern PHD_VECTOR DeathSlidePosition;

int TestLaraPosition(sint16 *bounds, ITEM_INFO *item, ITEM_INFO *laraitem);
void AlignLaraPosition(PHD_VECTOR *vec, ITEM_INFO *item, ITEM_INFO *laraitem);

void DeathSlideCollision(sint16 item_number, ITEM_INFO *laraitem, COLL_INFO *coll)
{
    ITEM_INFO *item;

    if (!(input & IN_ACTION) || laraitem->gravity_status || lara.gun_status != LG_ARMLESS || laraitem->current_anim_state != AS_STOP)
        return;
    item = &items[item_number];
    if (item->status != NOT_ACTIVE)
        return;
    if (TestLaraPosition(DeathSlideBounds, item, laraitem))
    {
        AlignLaraPosition(&DeathSlidePosition, item, laraitem);
        lara.gun_status = LG_HANDSBUSY;
        laraitem->goal_anim_state = AS_DEATHSLIDE;
        do
            AnimateItem(laraitem);
        while (laraitem->current_anim_state != AS_NULL);
        if (!item->active)
            AddActiveItem(item_number);
        item->status = ACTIVE;
        item->flags |= ONESHOT;
    }
}

extern char LaraOnPad;

int LaraLandedBad(ITEM_INFO *item, COLL_INFO *coll)
{
    int landspeed, oy;
    FLOOR_INFO *floor;
    sint16 room_number;

    room_number = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    oy = item->pos.y_pos;
    item->floor = item->pos.y_pos = GetHeight(floor, item->pos.x_pos, item->pos.y_pos - LARA_HITE, item->pos.z_pos);
    if (abs(oy - item->pos.y_pos) > 256)
        item->pos.y_pos = oy;
    LaraOnPad = 0;
    TestTriggers(trigger_index, 0);
    if (!LaraOnPad)
        lara_item->item_flags[1] = 0;
    item->pos.y_pos = oy;
    landspeed = item->fallspeed - DAMAGE_START;
    if (landspeed <= 0)
        return 0;
    else if (landspeed > DAMAGE_LENGTH)
        item->hit_points = -1;
    else
        item->hit_points -= landspeed * landspeed * 1000 / (DAMAGE_LENGTH * DAMAGE_LENGTH);
    if (item->hit_points <= 0)
        return 1;
    return 0;
}

int LaraHitCeiling(ITEM_INFO *item, COLL_INFO *coll)
{
    if (coll->coll_type == COLL_TOP || coll->coll_type == COLL_CLAMP)
    {
        item->pos.x_pos = coll->old.x;
        item->pos.y_pos = coll->old.y;
        item->pos.z_pos = coll->old.z;
        item->goal_anim_state = AS_STOP;
        item->current_anim_state = AS_STOP;
        item->frame_number = STOP_F;
        item->anim_number = STOP_A;
        item->speed = 0;
        item->fallspeed = 0;
        item->gravity_status = 0;
        return 1;
    }
    return 0;
}

sint16 LaraFloorFront(ITEM_INFO *item, PHD_ANGLE ang, sint32 dist)
{
    sint16 room_num;
    FLOOR_INFO *floor;
    sint32 x, y, z, height;

    x = item->pos.x_pos + ((phd_sin(ang) * dist) >> W2V_SHIFT);
    y = item->pos.y_pos - LARA_HITE;
    z = item->pos.z_pos + ((phd_cos(ang) * dist) >> W2V_SHIFT);
    room_num = item->room_number;
    floor = GetFloor(x, y, z, &room_num);
    height = GetHeight(floor, x, y, z);
    if (height != NO_HEIGHT)
        height -= item->pos.y_pos;
    return (sint16)height;
}

sint16 LaraCeilingFront(ITEM_INFO *item, PHD_ANGLE ang, sint32 dist)
{
    sint16 room_num;
    FLOOR_INFO *floor;
    sint32 x, y, z, height;

    x = item->pos.x_pos + ((phd_sin(ang) * dist) >> W2V_SHIFT);
    y = item->pos.y_pos - LARA_HITE;
    z = item->pos.z_pos + ((phd_cos(ang) * dist) >> W2V_SHIFT);
    room_num = item->room_number;
    floor = GetFloor(x, y, z, &room_num);
    height = GetCeiling(floor, x, y, z);
    if (height != NO_HEIGHT)
        height -= item->pos.y_pos - LARA_HITE;
    return (sint16)height;
}

int LaraFallen(ITEM_INFO *item, COLL_INFO *coll)
{
    if (coll->mid_floor > STEPUP_HEIGHT && lara.water_status != LARA_WADE)
    {
        item->anim_number = FALLDOWN_A;
        item->frame_number = FALLDOWN_F;
        item->current_anim_state = AS_FORWARDJUMP;
        item->goal_anim_state = AS_FORWARDJUMP;
        item->fallspeed = 0;
        item->gravity_status = 1;
        return 1;
    }
    return 0;
}

int LaraTestEdgeCatch(ITEM_INFO *item, COLL_INFO *coll, sint32 *edge)
{
    int hdif;
    sint16 *bounds;

    bounds = GetBoundsAccurate(item);
    hdif = coll->front_floor - bounds[2];
    if ((hdif < 0 && hdif + item->fallspeed < 0) || (hdif > 0 && hdif + item->fallspeed > 0)) {
        hdif = item->pos.y_pos + bounds[2];
        if ((hdif >> (WALL_SHIFT - 2)) != ((hdif + item->fallspeed) >> (WALL_SHIFT - 2))) {
            if (item->fallspeed > 0)
                *edge = (hdif + item->fallspeed) & ~(STEP_L - 1);
            else
                *edge = hdif & ~(STEP_L - 1);
            return -1;
        }
        return 0;
    }
    if (ABS(coll->left_floor2 - coll->right_floor2) >= 60)
        return 0;
    return 1;
}

int LaraDeflectEdge(ITEM_INFO *item, COLL_INFO *coll)
{
    if (coll->coll_type == COLL_FRONT || coll->coll_type == COLL_TOPFRONT)
    {
        ShiftItem(item, coll);
        item->goal_anim_state = AS_STOP;
        item->current_anim_state = AS_STOP;
        item->speed = 0;
        item->gravity_status = 0;
        return 1;
    }
    else if (coll->coll_type == COLL_LEFT)
    {
        ShiftItem(item, coll);
        item->pos.y_rot += LARA_DEF_ADD_EDGE;
    }
    else if (coll->coll_type == COLL_RIGHT)
    {
        ShiftItem(item, coll);
        item->pos.y_rot -= LARA_DEF_ADD_EDGE;
    }
    return 0;
}
