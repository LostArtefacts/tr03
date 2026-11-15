#include "specific/standard.h"
#include "game.h"

sint32 GetWaterHeight(sint32 x, sint32 y, sint32 z, sint16 room_number);
void LaraSwimCollision(ITEM_INFO *item, COLL_INFO *coll);
void SwimTurn(ITEM_INFO *item);

#define UW_MAXSPEED 200
#define WATER_FRICTION 6
#define WATERROLL_A 203
#define WATERROLL_F GF(WATERROLL_A, 0)

extern void (*lara_control_routines[])(ITEM_INFO *item, COLL_INFO *coll);
extern void (*extra_control_routines[])(ITEM_INFO *item, COLL_INFO *coll);
extern void (*lara_collision_routines[])(ITEM_INFO *item, COLL_INFO *coll);
extern char LaraOnPad;

void LookLeftRight(void);
void ResetLook(void);
void LaraWaterCurrent(COLL_INFO *coll);
void AnimateLara(ITEM_INFO *item);
void LaraBaddieCollision(ITEM_INFO *laraitem, COLL_INFO *coll);
void UpdateLaraRoom(ITEM_INFO *item, int height);
void LaraGun(void);

void LaraUnderWater(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->bad_pos = -NO_HEIGHT;
    coll->bad_neg = -400;
    coll->bad_ceiling = 400;
    coll->old.x = item->pos.x_pos;
    coll->old.y = item->pos.y_pos;
    coll->old.z = item->pos.z_pos;
    coll->radius = 300;
    coll->trigger = NULL;
    coll->slopes_are_walls = 0;
    coll->slopes_are_pits = 0;
    coll->lava_is_pit = 0;
    coll->enable_hit = 0;
    coll->enable_baddie_push = 1;

    if ((input & IN_LOOK) && lara.look) {
        LookLeftRight();
    } else {
        ResetLook();
    }
    lara.look = 1;

    if (lara.extra_anim) {
        (*extra_control_routines[item->current_anim_state])(item, coll);
    } else {
        (*lara_control_routines[item->current_anim_state])(item, coll);
    }

    if (item->pos.z_rot >= -364 && item->pos.z_rot <= 364) {
        item->pos.z_rot = 0;
    } else if (item->pos.z_rot < 0) {
        item->pos.z_rot += 364;
    } else {
        item->pos.z_rot -= 364;
    }

    if (item->pos.x_rot < -85 * ONE_DEGREE) {
        item->pos.x_rot = -85 * ONE_DEGREE;
    } else if (item->pos.x_rot > 85 * ONE_DEGREE) {
        item->pos.x_rot = 85 * ONE_DEGREE;
    }
    if (item->pos.z_rot < -4004) {
        item->pos.z_rot = -4004;
    } else if (item->pos.z_rot > 4004) {
        item->pos.z_rot = 4004;
    }

    if (lara.turn_rate >= -364 && lara.turn_rate <= 364) {
        lara.turn_rate = 0;
    } else if (lara.turn_rate < -364) {
        lara.turn_rate += 364;
    } else {
        lara.turn_rate -= 364;
    }
    item->pos.y_rot += lara.turn_rate;

    if (lara.current_active && lara.water_status != LARA_CHEAT) {
        LaraWaterCurrent(coll);
    }

    AnimateLara(item);
    item->pos.y_pos -= (phd_sin(item->pos.x_rot) * item->fallspeed) >> (W2V_SHIFT + 2);
    item->pos.x_pos += (((phd_sin(item->pos.y_rot) * item->fallspeed) >> (W2V_SHIFT + 2)) * phd_cos(item->pos.x_rot)) >> W2V_SHIFT;
    item->pos.z_pos += (((phd_cos(item->pos.y_rot) * item->fallspeed) >> (W2V_SHIFT + 2)) * phd_cos(item->pos.x_rot)) >> W2V_SHIFT;

    if (lara.water_status != LARA_CHEAT && !lara.extra_anim) {
        LaraBaddieCollision(item, coll);
    }

    if (!lara.extra_anim && lara.skidoo == NO_ITEM) {
        (*lara_collision_routines[item->current_anim_state])(item, coll);
    }

    UpdateLaraRoom(item, 0);

    LaraGun();

    LaraOnPad = 0;
    TestTriggers(coll->trigger, 0);
    if (!LaraOnPad) {
        lara_item->item_flags[1] = 0;
    }
}

void lara_as_swim(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_UWDEATH;
        return;
    }
    if (input & IN_ROLL)
    {
        item->current_anim_state = AS_WATERROLL;
        item->anim_number = WATERROLL_A;
        item->frame_number = WATERROLL_F;
        return;
    }
    SwimTurn(item);
    item->fallspeed += 2 * 4;
    if (item->fallspeed > UW_MAXSPEED)
        item->fallspeed = UW_MAXSPEED;
    if (!(input & IN_JUMP))
        item->goal_anim_state = AS_GLIDE;
}

void SwimTurn(ITEM_INFO *item)
{
    if (input & IN_FORWARD)
        item->pos.x_rot -= 2 * ONE_DEGREE;
    else if (input & IN_BACK)
        item->pos.x_rot += 2 * ONE_DEGREE;
    if (input & IN_LEFT)
    {
        lara.turn_rate -= LARA_TURN_RATE;
        if (lara.turn_rate < -LARA_MED_TURN)
            lara.turn_rate = -LARA_MED_TURN;
        item->pos.z_rot -= LARA_LEAN_RATE * 2;
    }
    else if (input & IN_RIGHT)
    {
        lara.turn_rate += LARA_TURN_RATE;
        if (lara.turn_rate > LARA_MED_TURN)
            lara.turn_rate = LARA_MED_TURN;
        item->pos.z_rot += LARA_LEAN_RATE * 2;
    }
}

void lara_as_glide(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_UWDEATH;
        return;
    }
    if (input & IN_ROLL)
    {
        item->current_anim_state = AS_WATERROLL;
        item->anim_number = WATERROLL_A;
        item->frame_number = WATERROLL_F;
        return;
    }
    SwimTurn(item);
    if (input & IN_JUMP)
        item->goal_anim_state = AS_SWIM;
    item->fallspeed -= WATER_FRICTION;
    if (item->fallspeed < 0)
        item->fallspeed = 0;
    if (item->fallspeed <= (UW_MAXSPEED * 2) / 3)
        item->goal_anim_state = AS_TREAD;
}

void lara_as_tread(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_UWDEATH;
        return;
    }
    if (input & IN_ROLL)
    {
        item->current_anim_state = AS_WATERROLL;
        item->anim_number = WATERROLL_A;
        item->frame_number = WATERROLL_F;
        return;
    }
    if (input & IN_LOOK)
        LookUpDown();
    SwimTurn(item);
    if (input & IN_JUMP)
        item->goal_anim_state = AS_SWIM;
    item->fallspeed -= WATER_FRICTION;
    if (item->fallspeed < 0)
        item->fallspeed = 0;
    if (lara.gun_status == LG_HANDSBUSY)
        lara.gun_status = LG_ARMLESS;
}

void lara_as_dive(ITEM_INFO *item, COLL_INFO *coll)
{
    if (input & IN_FORWARD)
        item->pos.x_rot -= ONE_DEGREE;
}

void lara_as_uwdeath(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.look = 0;
    item->fallspeed -= 8;
    if (item->fallspeed <= 0)
        item->fallspeed = 0;
    if (item->pos.x_rot >= -2 * ONE_DEGREE && item->pos.x_rot <= 2 * ONE_DEGREE)
        item->pos.x_rot = 0;
    else if (item->pos.x_rot < 0)
        item->pos.x_rot += 2 * ONE_DEGREE;
    else
        item->pos.x_rot -= 2 * ONE_DEGREE;
}

void lara_as_waterroll(ITEM_INFO *item, COLL_INFO *coll)
{
    item->fallspeed = 0;
}

void lara_col_swim(ITEM_INFO *item, COLL_INFO *coll)
{
    LaraSwimCollision(item, coll);
}

void LaraTestWaterDepth(ITEM_INFO *item, COLL_INFO *coll);

void LaraSwimCollision(ITEM_INFO *item, COLL_INFO *coll)
{
    int height;

    if (item->pos.x_rot >= -16384 && item->pos.x_rot <= 16384)
        coll->facing = lara.move_angle = item->pos.y_rot;
    else
        coll->facing = lara.move_angle = item->pos.y_rot - 32768;
    height = phd_sin(item->pos.x_rot) * LARA_HITE >> W2V_SHIFT;
    if (height < 0)
        height = -height;
    if (height < 200)
        height = 200;
    coll->bad_neg = -height;
    GetCollisionInfo(coll, item->pos.x_pos, item->pos.y_pos + height / 2, item->pos.z_pos, item->room_number, height);
    ShiftItem(item, coll);
    if (coll->coll_type == COLL_FRONT) {
        if (item->pos.x_rot > 45 * 182)
            item->pos.x_rot += 2 * 182;
        else if (item->pos.x_rot < -45 * 182)
            item->pos.x_rot -= 2 * 182;
        else
            item->fallspeed = 0;
    } else if (coll->coll_type == COLL_TOP) {
        if (item->pos.x_rot >= -45 * 182)
            item->pos.x_rot -= 2 * 182;
    } else if (coll->coll_type == COLL_TOPFRONT)
        item->fallspeed = 0;
    else if (coll->coll_type == COLL_LEFT)
        item->pos.y_rot += 5 * 182;
    else if (coll->coll_type == COLL_RIGHT)
        item->pos.y_rot -= 5 * 182;
    else if (coll->coll_type == COLL_CLAMP) {
        item->pos.x_pos = coll->old.x;
        item->pos.y_pos = coll->old.y;
        item->pos.z_pos = coll->old.z;
        item->fallspeed = 0;
        return;
    }
    if (coll->mid_floor < 0) {
        item->pos.y_pos += coll->mid_floor;
        item->pos.x_rot += 2 * 182;
    }
    if (lara.water_status != LARA_CHEAT && !lara.extra_anim)
        LaraTestWaterDepth(item, coll);
}

sint32 GetWaterDepth(sint32 x, sint32 y, sint32 z, sint16 room_number);

void LaraTestWaterDepth(ITEM_INFO *item, COLL_INFO *coll)
{
    int wd;
    FLOOR_INFO *floor;
    sint16 room_number;

    room_number = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    wd = GetWaterDepth(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, room_number);
    if (wd == NO_HEIGHT)
    {
        item->pos.x_pos = coll->old.x;
        item->pos.y_pos = coll->old.y;
        item->pos.z_pos = coll->old.z;
        item->fallspeed = 0;
    }
    else if (wd <= STEP_L * 2)
    {
        item->anim_number = 192;
        item->frame_number = GF(192, 0);
        item->current_anim_state = AS_WATEROUT;
        item->goal_anim_state = AS_STOP;
        item->pos.x_rot = item->pos.z_rot = 0;
        item->gravity_status = 0;
        item->speed = 0;
        item->fallspeed = 0;
        lara.water_status = LARA_WADE;
        item->pos.y_pos = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    }
}

void lara_col_glide(ITEM_INFO *item, COLL_INFO *coll)
{
    LaraSwimCollision(item, coll);
}

void lara_col_tread(ITEM_INFO *item, COLL_INFO *coll)
{
    LaraSwimCollision(item, coll);
}

void lara_col_dive(ITEM_INFO *item, COLL_INFO *coll)
{
    LaraSwimCollision(item, coll);
}

void lara_col_uwdeath(ITEM_INFO *item, COLL_INFO *coll)
{
    int wh;

    item->hit_points = -1;
    lara.air = -1;
    lara.gun_status = LG_HANDSBUSY;
    wh = GetWaterHeight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);
    if (wh != NO_HEIGHT && wh < item->pos.y_pos - 100)
        item->pos.y_pos -= 5;
    LaraSwimCollision(item, coll);
}

void lara_col_waterroll(ITEM_INFO *item, COLL_INFO *coll)
{
    LaraSwimCollision(item, coll);
}

sint16 GetDoor(FLOOR_INFO *floor);

sint32 GetWaterDepth(sint32 x, sint32 y, sint32 z, sint16 room_number)
{
    int wh;
    ROOM_INFO *r;
    FLOOR_INFO *floor;
    sint16 data;
    int x_floor, y_floor;

    r = &room[room_number];
    do {
        x_floor = (z - r->z) >> WALL_SHIFT;
        y_floor = (x - r->x) >> WALL_SHIFT;
        if (x_floor <= 0) {
            x_floor = 0;
            if (y_floor < 1)
                y_floor = 1;
            else if (y_floor > r->y_size - 2)
                y_floor = r->y_size - 2;
        } else if (x_floor >= r->x_size - 1) {
            x_floor = r->x_size - 1;
            if (y_floor < 1)
                y_floor = 1;
            else if (y_floor > r->y_size - 2)
                y_floor = r->y_size - 2;
        } else if (y_floor < 0)
            y_floor = 0;
        else if (y_floor >= r->y_size)
            y_floor = r->y_size - 1;
        floor = &r->floor[x_floor + y_floor * r->x_size];
        data = GetDoor(floor);
        if (data != NO_ROOM) {
            room_number = data;
            r = &room[data];
        }
    } while (data != NO_ROOM);
    if (r->flags & (UNDERWATER | 0x80)) {
        while (floor->sky_room != NO_ROOM) {
            r = &room[floor->sky_room];
            if (!(r->flags & (UNDERWATER | 0x80))) {
                wh = floor->ceiling << 8;
                floor = GetFloor(x, y, z, &room_number);
                return GetHeight(floor, x, y, z) - wh;
            }
            floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
        }
        return 0x7fff;
    } else {
        while (floor->pit_room != NO_ROOM) {
            r = &room[floor->pit_room];
            if (r->flags & (UNDERWATER | 0x80)) {
                wh = floor->floor << 8;
                floor = GetFloor(x, y, z, &room_number);
                return GetHeight(floor, x, y, z) - wh;
            }
            floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
        }
        return NO_HEIGHT;
    }
}

extern sint16 rcossin_tbl[];

sint32 mGetAngle(sint32 x1, sint32 y1, sint32 x2, sint32 y2);

int abs(int x);
#pragma intrinsic(abs)

void LaraWaterCurrent(COLL_INFO *coll)
{
    PHD_VECTOR target;
    ITEM_INFO *item;

    item = lara_item;

    if (!lara.current_active) {
        long shifter, absvel;

        absvel = abs(lara.current_xvel);
        if (absvel > 16)
            shifter = 4;
        else if (absvel > 8)
            shifter = 3;
        else
            shifter = 2;

        lara.current_xvel -= lara.current_xvel >> shifter;
        if (abs(lara.current_xvel) < 4)
            lara.current_xvel = 0;

        absvel = abs(lara.current_zvel);
        if (absvel > 16)
            shifter = 4;
        else if (absvel > 8)
            shifter = 3;
        else
            shifter = 2;

        lara.current_zvel -= lara.current_zvel >> shifter;
        if (abs(lara.current_zvel) < 4)
            lara.current_zvel = 0;

        if (lara.current_xvel == 0 && lara.current_zvel == 0)
            return;
    } else {
        long angle, dx, dz, speed, sinkval;

        sinkval = lara.current_active - 1;
        target.x = camera.fixed[sinkval].x;
        target.y = camera.fixed[sinkval].y;
        target.z = camera.fixed[sinkval].z;
        angle = ((mGetAngle(target.x, target.z, lara_item->pos.x_pos, lara_item->pos.z_pos) - 0x4000) >> 4) & 4095;

        speed = camera.fixed[sinkval].data;
        dx = (rcossin_tbl[angle << 1] * speed) >> 2;
        dz = (rcossin_tbl[(angle << 1) + 1] * speed) >> 2;

        lara.current_xvel += (dx - lara.current_xvel) >> 4;
        lara.current_zvel += (dz - lara.current_zvel) >> 4;
        lara_item->pos.y_pos += (target.y - lara_item->pos.y_pos) >> 4;
    }

    item->pos.x_pos += lara.current_xvel >> 8;
    item->pos.z_pos += lara.current_zvel >> 8;

    lara.current_active = 0;

    coll->facing = phd_atan(
        (item->pos.z_pos - coll->old.z), (item->pos.x_pos - coll->old.x));
    GetCollisionInfo(
        coll, item->pos.x_pos, item->pos.y_pos + 200, item->pos.z_pos,
        item->room_number, 400);
    if (coll->coll_type == COLL_FRONT) {
        if (item->pos.x_rot > 35 * ONE_DEGREE)
            item->pos.x_rot += 2 * ONE_DEGREE;
        else if (item->pos.x_rot < -35 * ONE_DEGREE)
            item->pos.x_rot -= 2 * ONE_DEGREE;
        else
            item->fallspeed = 0;
    } else if (coll->coll_type == COLL_TOP)
        item->pos.x_rot -= 2 * ONE_DEGREE;
    else if (coll->coll_type == COLL_TOPFRONT)
        item->fallspeed = 0;
    else if (coll->coll_type == COLL_LEFT)
        item->pos.y_rot += 5 * ONE_DEGREE;
    else if (coll->coll_type == COLL_RIGHT)
        item->pos.y_rot -= 5 * ONE_DEGREE;

    if (coll->mid_floor < 0) {
        item->pos.y_pos += coll->mid_floor;
    }
    ShiftItem(item, coll);

    coll->old.x = item->pos.x_pos;
    coll->old.y = item->pos.y_pos;
    coll->old.z = item->pos.z_pos;
}
