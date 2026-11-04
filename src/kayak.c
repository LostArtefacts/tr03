#include "game.h"

#define KAYAK_DRAW_SHIFT 32
#define NUM_WAKE_SPRITES 32
#define KAYAK_DROWN_A 25
#define EXTRA_RAPIDSDROWN 12

typedef struct {
    sint32 Vel;
    sint32 Rot;
    sint32 FallSpeedF;
    sint32 FallSpeedL;
    sint32 FallSpeedR;
    sint32 Water;
    PHD_3DPOS OldPos;
    char Turn;
    char Forward;
    char TrueWater;
    char Flags;
} KAYAKINFO;

typedef struct {
    sint32 x[2];
    sint32 y;
    sint32 z[2];
    sint16 xvel[2];
    sint16 zvel[2];
    uint8 life;
    uint8 pad[3];
} WAKE_PTS;

extern WAKE_PTS WakePts[NUM_WAKE_SPRITES][2];

void LaraRapidsDrown(void)
{
    ITEM_INFO *l;

    l = lara_item;
    l->anim_number = objects[VEHICLE_ANIM].anim_index + KAYAK_DROWN_A;
    l->frame_number = anims[l->anim_number].frame_base;
    l->current_anim_state = EXTRA_RAPIDSDROWN;
    l->goal_anim_state = EXTRA_RAPIDSDROWN;
    l->hit_points = 0;
    l->fallspeed = 0;
    l->gravity_status = 0;
    l->speed = 0;
    AnimateItem(l);
    lara.extra_anim = 1;
    lara.gun_status = LG_HANDSBUSY;
    lara.gun_type = LG_UNARMED;
    lara.hit_direction = -1;
}

void KayakInitialise(sint16 item_number)
{
    int i;
    ITEM_INFO *v;
    KAYAKINFO *Kayak;

    v = &items[item_number];
    Kayak = v->data = game_malloc(sizeof(KAYAKINFO), 0);
    Kayak->Vel = Kayak->Rot = Kayak->Flags = 0;
    Kayak->FallSpeedF = Kayak->FallSpeedL = Kayak->FallSpeedR = 0;
    Kayak->OldPos = v->pos;
    for (i = 0; i < NUM_WAKE_SPRITES; i++)
    {
        WakePts[i][0].life = 0;
        WakePts[i][1].life = 0;
    }
}

void KayakDraw(ITEM_INFO *v)
{
    v->pos.y_pos += KAYAK_DRAW_SHIFT;
    DrawAnimatingItem(v);
    v->pos.y_pos -= KAYAK_DRAW_SHIFT;
    S_DrawWakeFX(v);
}

sint32 GetWaterHeight(sint32 x, sint32 y, sint32 z, sint16 room_number);
RIPPLE_STRUCT *SetupRipple(sint32 x, sint32 y, sint32 z, sint32 size, sint32 moretrans);

typedef struct {
    sint32 x;
    sint32 y;
    sint32 z;
    sint16 InnerXZoff;
    sint16 InnerXZsize;
    sint16 InnerYsize;
    sint16 InnerXZvel;
    sint16 InnerYvel;
    sint16 InnerGravity;
    sint16 InnerFriction;
    sint16 MiddleXZoff;
    sint16 MiddleXZsize;
    sint16 MiddleYsize;
    sint16 MiddleXZvel;
    sint16 MiddleYvel;
    sint16 MiddleGravity;
    sint16 MiddleFriction;
    sint16 OuterXZoff;
    sint16 OuterXZsize;
    sint16 OuterXZvel;
    sint16 OuterFriction;
} SPLASH_SETUP;

extern SPLASH_SETUP splash_setup;
extern sint32 SplashCount;
void SetupSplash(SPLASH_SETUP *spl);

void KayakSplash(ITEM_INFO *, sint32, sint32);

int GetInKayak(sint16 item_number, COLL_INFO *coll)
{
    int dist;
    sint32 x, z;
    ITEM_INFO *v, *l;
    FLOOR_INFO *floor;
    sint16 room_number;

    l = lara_item;
    if (!(input & IN_ACTION) || lara.gun_status != LG_ARMLESS || l->gravity_status)
        return 0;

    v = &items[item_number];
    x = l->pos.x_pos - v->pos.x_pos;
    z = l->pos.z_pos - v->pos.z_pos;
    dist = x * x + z * z;
    if (dist > 130000)
        return 0;

    room_number = v->room_number;
    floor = GetFloor(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, &room_number);
    if (GetHeight(floor, v->pos.x_pos, v->pos.y_pos, v->pos.z_pos) > -32000) {
        sint16 ang;
        uint16 tempang;

        ang = phd_atan(v->pos.z_pos - l->pos.z_pos, v->pos.x_pos - l->pos.x_pos);
        ang -= v->pos.y_rot;
        tempang = l->pos.y_rot - v->pos.y_rot;
        if (ang > -(ONE_DEGREE * 45) && ang < ONE_DEGREE * 135) {
            if (tempang > 45 * ONE_DEGREE && tempang < 135 * ONE_DEGREE)
                return -1;
        } else {
            if (tempang > 225 * ONE_DEGREE && tempang < 315 * ONE_DEGREE)
                return 1;
        }
    }
    return 0;
}

void DoRipple(ITEM_INFO *v, sint16 xoff, sint16 zoff)
{
    sint16 room_number;
    FLOOR_INFO *floor;
    RIPPLE_STRUCT *r;
    sint32 s, c, x, z;

    c = phd_cos(v->pos.y_rot);
    s = phd_sin(v->pos.y_rot);
    x = v->pos.x_pos + ((s * zoff + c * xoff) >> W2V_SHIFT);
    z = v->pos.z_pos + ((c * zoff - s * xoff) >> W2V_SHIFT);
    room_number = v->room_number;
    floor = GetFloor(x, v->pos.y_pos, z, &room_number);
    if (GetWaterHeight(x, v->pos.y_pos, z, room_number) != NO_HEIGHT) {
        r = SetupRipple(x, v->pos.y_pos, z, -2 - (GetRandomControl() & 1), 0);
        r->init = 0;
    }
}
