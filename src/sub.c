#include "game.h"

#define NUM_WAKE_SPRITES 32
#define SF_SURFACE 2

typedef struct {
    sint32 Vel;
    sint32 Rot;
    sint32 RotX;
    sint16 FanRot;
    char Flags;
    char WeaponTimer;
} SUBINFO;

typedef struct {
    sint32 x[2];
    sint32 y[2];
    sint32 z[2];
    uint8 life;
    uint8 pad[3];
} SUB_WAKE_PTS;

extern SUB_WAKE_PTS SubWakePts[NUM_WAKE_SPRITES][2];

void SubInitialise(sint16 item_number)
{
    ITEM_INFO *v;
    SUBINFO *sub;
    sint32 i;

    v = &items[item_number];
    sub = v->data = game_malloc(sizeof(SUBINFO), 0);
    sub->Vel = sub->Rot = 0;
    sub->Flags = SF_SURFACE;
    sub->WeaponTimer = 0;
    for (i = 0; i < NUM_WAKE_SPRITES; i++)
    {
        SubWakePts[i][0].life = 0;
        SubWakePts[i][1].life = 0;
    }
}
