#include "game.h"

#define KAYAK_DRAW_SHIFT 32
#define NUM_WAKE_SPRITES 32

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
