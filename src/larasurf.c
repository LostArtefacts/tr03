#include "game.h"

#define SURF_MAXSPEED 60
#define DIVE_COUNT 10

extern void (*lara_control_routines[])(ITEM_INFO *item, COLL_INFO *coll);
extern void (*lara_collision_routines[])(ITEM_INFO *item, COLL_INFO *coll);
extern char LaraOnPad;

void LookLeftRight(void);
void ResetLook(void);
void LaraWaterCurrent(COLL_INFO *coll);
void AnimateLara(ITEM_INFO *item);
void LaraBaddieCollision(ITEM_INFO *laraitem, COLL_INFO *coll);
void UpdateLaraRoom(ITEM_INFO *item, int height);
void LaraGun(void);

void LaraSurface(ITEM_INFO *item, COLL_INFO *coll)
{
    camera.target_elevation = -22 * ONE_DEGREE;
    coll->bad_pos = -NO_HEIGHT;
    coll->bad_neg = -STEP_L / 2;
    coll->bad_ceiling = 100;
    coll->old.x = item->pos.x_pos;
    coll->old.y = item->pos.y_pos;
    coll->old.z = item->pos.z_pos;
    coll->radius = 100;
    coll->trigger = NULL;
    coll->slopes_are_walls = 0;
    coll->slopes_are_pits = 0;
    coll->lava_is_pit = 0;
    coll->enable_hit = 0;
    coll->enable_baddie_push = 0;
    if ((input & IN_LOOK) && lara.look)
        LookLeftRight();
    else
        ResetLook();
    lara.look = 1;
    (*lara_control_routines[item->current_anim_state])(item, coll);
    if (item->pos.z_rot >= -(2 * ONE_DEGREE) && item->pos.z_rot <= 2 * ONE_DEGREE)
        item->pos.z_rot = 0;
    else if (item->pos.z_rot < 0)
        item->pos.z_rot += 2 * ONE_DEGREE;
    else
        item->pos.z_rot -= 2 * ONE_DEGREE;
    if (lara.current_active && lara.water_status != LARA_CHEAT)
        LaraWaterCurrent(coll);
    AnimateLara(item);
    item->pos.x_pos += (phd_sin(lara.move_angle) * item->fallspeed) >> (W2V_SHIFT + 2);
    item->pos.z_pos += (phd_cos(lara.move_angle) * item->fallspeed) >> (W2V_SHIFT + 2);
    LaraBaddieCollision(item, coll);
    if (lara.skidoo == NO_ITEM)
        (*lara_collision_routines[item->current_anim_state])(item, coll);
    UpdateLaraRoom(item, 100);
    LaraGun();
    LaraOnPad = 0;
    TestTriggers(coll->trigger, 0);
    if (!LaraOnPad)
        lara_item->item_flags[1] = 0;
}

void lara_as_surfswim(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_UWDEATH;
        return;
    }
    lara.dive_count = 0;
    if (input & IN_LEFT)
        item->pos.y_rot -= LARA_SLOW_TURN;
    else if (input & IN_RIGHT)
        item->pos.y_rot += LARA_SLOW_TURN;
    if (!(input & IN_FORWARD))
        item->goal_anim_state = AS_SURFTREAD;
    if (input & IN_JUMP)
        item->goal_anim_state = AS_SURFTREAD;
    item->fallspeed += 2 * 4;
    if (item->fallspeed > SURF_MAXSPEED)
        item->fallspeed = SURF_MAXSPEED;
}

void lara_as_surfback(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_UWDEATH;
        return;
    }
    lara.dive_count = 0;
    if (input & IN_LEFT)
        item->pos.y_rot -= LARA_SLOW_TURN / 2;
    else if (input & IN_RIGHT)
        item->pos.y_rot += LARA_SLOW_TURN / 2;
    if (!(input & IN_BACK))
        item->goal_anim_state = AS_SURFTREAD;
    item->fallspeed += 2 * 4;
    if (item->fallspeed > SURF_MAXSPEED)
        item->fallspeed = SURF_MAXSPEED;
}

void lara_as_surfleft(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_UWDEATH;
        return;
    }
    lara.dive_count = 0;
    if (input & IN_LEFT)
        item->pos.y_rot -= LARA_SLOW_TURN / 2;
    else if (input & IN_RIGHT)
        item->pos.y_rot += LARA_SLOW_TURN / 2;
    if (!(input & IN_STEPL))
        item->goal_anim_state = AS_SURFTREAD;
    item->fallspeed += 2 * 4;
    if (item->fallspeed > SURF_MAXSPEED)
        item->fallspeed = SURF_MAXSPEED;
}

void lara_as_surfright(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_UWDEATH;
        return;
    }
    lara.dive_count = 0;
    if (input & IN_LEFT)
        item->pos.y_rot -= LARA_SLOW_TURN / 2;
    else if (input & IN_RIGHT)
        item->pos.y_rot += LARA_SLOW_TURN / 2;
    if (!(input & IN_STEPR))
        item->goal_anim_state = AS_SURFTREAD;
    item->fallspeed += 2 * 4;
    if (item->fallspeed > SURF_MAXSPEED)
        item->fallspeed = SURF_MAXSPEED;
}

void lara_as_surftread(ITEM_INFO *item, COLL_INFO *coll)
{
    item->fallspeed -= 4;
    if (item->fallspeed < 0)
        item->fallspeed = 0;
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_UWDEATH;
        return;
    }
    if (input & IN_LOOK)
    {
        LookUpDown();
        return;
    }
    if (input & IN_LEFT)
        item->pos.y_rot -= LARA_SLOW_TURN;
    else if (input & IN_RIGHT)
        item->pos.y_rot += LARA_SLOW_TURN;
    if (input & IN_FORWARD)
        item->goal_anim_state = AS_SURFSWIM;
    else if (input & IN_BACK)
        item->goal_anim_state = AS_SURFBACK;
    if (input & IN_STEPL)
        item->goal_anim_state = AS_SURFLEFT;
    else if (input & IN_STEPR)
        item->goal_anim_state = AS_SURFRIGHT;
    if (input & IN_JUMP)
    {
        lara.dive_count++;
        if (lara.dive_count == DIVE_COUNT)
            item->goal_anim_state = AS_SWIM;
    }
    else
        lara.dive_count = 0;
}

void lara_col_surfswim(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->bad_neg = -STEPUP_HEIGHT;
    lara.move_angle = item->pos.y_rot;
    LaraSurfaceCollision(item, coll);
    LaraTestWaterClimbOut(item, coll);
}

sint32 GetWaterHeight(sint32 x, sint32 y, sint32 z, sint16 room_number);
int LaraTestWaterStepOut(ITEM_INFO *item, COLL_INFO *coll);

void LaraSurfaceCollision(ITEM_INFO *item, COLL_INFO *coll)
{
    int wh;

    coll->facing = lara.move_angle;
    GetCollisionInfo(coll, item->pos.x_pos, item->pos.y_pos + 700, item->pos.z_pos, item->room_number, 800);
    ShiftItem(item, coll);
    if ((coll->coll_type & (COLL_FRONT | COLL_TOP | COLL_CLAMP | COLL_TOPFRONT))
        || (coll->mid_floor < 0 && (coll->mid_type == 2 || coll->mid_type == 3))) {
        item->fallspeed = 0;
        item->pos.x_pos = coll->old.x;
        item->pos.y_pos = coll->old.y;
        item->pos.z_pos = coll->old.z;
    } else if (coll->coll_type == COLL_LEFT) {
        item->pos.y_rot += 5 * ONE_DEGREE;
    } else if (coll->coll_type == COLL_RIGHT) {
        item->pos.y_rot -= 5 * ONE_DEGREE;
    }

    wh = GetWaterHeight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);
    if (wh - item->pos.y_pos <= -100) {
        item->goal_anim_state = AS_SWIM;
        item->current_anim_state = AS_DIVE;
        item->anim_number = SURFDIVE_A;
        item->frame_number = SURFDIVE_F;
        item->pos.x_rot = -45 * ONE_DEGREE;
        item->fallspeed = 20 * 4;
        lara.water_status = LARA_UNDERWATER;
        return;
    }
    LaraTestWaterStepOut(item, coll);
}

int LaraTestWaterStepOut(ITEM_INFO *item, COLL_INFO *coll)
{
    if (coll->coll_type == COLL_FRONT || coll->mid_type == 2 || coll->mid_type == 3 || coll->mid_floor >= 0)
        return 0;
    if (coll->mid_floor < -(STEP_L / 2))
    {
        item->anim_number = 190;
        item->frame_number = GF(190, 0);
        item->current_anim_state = AS_WATEROUT;
        item->goal_anim_state = AS_STOP;
    }
    else if (item->goal_anim_state == AS_SURFLEFT)
        item->goal_anim_state = AS_STEPLEFT;
    else if (item->goal_anim_state == AS_SURFRIGHT)
        item->goal_anim_state = AS_STEPRIGHT;
    else
    {
        item->goal_anim_state = AS_WADE;
        item->current_anim_state = AS_WADE;
        item->anim_number = 177;
        item->frame_number = GF(177, 0);
    }
    item->pos.y_pos += coll->front_floor + 700 - 5;
    UpdateLaraRoom(item, -LARA_HITE / 2);
    item->pos.x_rot = item->pos.z_rot = 0;
    item->gravity_status = 0;
    item->speed = 0;
    item->fallspeed = 0;
    lara.water_status = LARA_WADE;
    return 1;
}

void lara_col_surfback(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.move_angle = item->pos.y_rot - 32768;
    LaraSurfaceCollision(item, coll);
}

void lara_col_surfleft(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.move_angle = item->pos.y_rot - 16384;
    LaraSurfaceCollision(item, coll);
}

void lara_col_surfright(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.move_angle = item->pos.y_rot + 16384;
    LaraSurfaceCollision(item, coll);
}

void lara_col_surftread(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->goal_anim_state == AS_SWIM)
    {
        item->current_anim_state = AS_DIVE;
        item->anim_number = SURFDIVE_A;
        item->frame_number = SURFDIVE_F;
        item->pos.x_rot = -45 * ONE_DEGREE;
        item->fallspeed = 20 * 4;
        lara.water_status = LARA_UNDERWATER;
    }
    lara.move_angle = item->pos.y_rot;
    LaraSurfaceCollision(item, coll);
}
