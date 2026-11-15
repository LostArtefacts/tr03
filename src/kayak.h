#pragma once

#include "game.h"

#define KAYAK_DRAW_SHIFT 32
#define NUM_WAKE_SPRITES 32
#define WAKE_SPEED 4
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
