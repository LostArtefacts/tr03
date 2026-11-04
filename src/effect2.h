#ifndef EFFECT2_H
#define EFFECT2_H

#include "types.h"

#define MAX_DYNAMICS 32

typedef struct {
    sint32 x;
    sint32 y;
    sint32 z;
    uint16 falloff;
    uint8 used;
    uint8 pad1;
    uint8 on;
    uint8 r;
    uint8 g;
    uint8 b;
} DYNAMIC;

#define MAX_SPARKS 192
#define MAX_SPLASHES 4
#define MAX_RIPPLES 16
#define MAX_BATS 32

typedef struct {
    sint32 x;
    sint32 y;
    sint32 z;
    sint16 Xvel;
    sint16 Yvel;
    sint16 Zvel;
    sint16 Gravity;
    sint16 RotAng;
    sint16 Flags;
    uint8 sWidth;
    uint8 dWidth;
    uint8 sHeight;
    uint8 dHeight;
    uint8 Friction;
    uint8 Width;
    uint8 Height;
    uint8 Scalar;
    uint8 Def;
    sint8 RotAdd;
    sint8 MaxYvel;
    uint8 On;
    uint8 sR;
    uint8 sG;
    uint8 sB;
    uint8 dR;
    uint8 dG;
    uint8 dB;
    uint8 R;
    uint8 G;
    uint8 B;
    uint8 ColFadeSpeed;
    uint8 FadeToBlack;
    uint8 sLife;
    uint8 Life;
    uint8 TransType;
    uint8 extras;
    sint8 Dynamic;
    uint8 FxObj;
    uint8 RoomNumber;
    uint8 NodeNumber;
    uint8 pad;
} SPARKS;

typedef struct {
    sint32 x;
    sint32 y;
    sint32 z;
    sint8 flags;
    uint8 life;
    uint8 size;
    uint8 init;
} RIPPLE_STRUCT;

typedef struct {
    sint16 wx;
    sint16 wy;
    sint16 wz;
    sint16 xv;
    sint32 yv;
    sint16 zv;
    sint16 oxv;
    sint16 ozv;
    uint8 friction;
    uint8 gravity;
} SPLASH_VERTS;

typedef struct {
    sint32 x;
    sint32 y;
    sint32 z;
    sint8 flags;
    uint8 life;
    sint8 pad[2];
    SPLASH_VERTS sv[48];
} SPLASH_STRUCT;

typedef struct {
    sint32 x;
    sint32 y;
    sint32 z;
    sint16 angle;
    sint16 speed;
    uint8 WingYoff;
    uint8 flags;
    uint8 life;
    uint8 pad;
} BAT_STRUCT;

extern SPARKS sparks[MAX_SPARKS];
extern RIPPLE_STRUCT ripples[MAX_RIPPLES];
extern SPLASH_STRUCT splashes[MAX_SPLASHES];
extern BAT_STRUCT bats[MAX_BATS];

extern sint32 KillEverythingFlag;
extern DYNAMIC dynamics[];
extern int number_dynamics;

void KillAllCurrentItems(sint16 item_number);
void TriggerDynamic(sint32 x, sint32 y, sint32 z, sint32 falloff, sint32 r, sint32 g, sint32 b);
void ClearDynamics(void);

#endif
