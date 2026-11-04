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
