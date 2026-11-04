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

extern sint32 KillEverythingFlag;
extern DYNAMIC dynamics[];
extern int number_dynamics;

void KillAllCurrentItems(sint16 item_number);
void ClearDynamics(void);

#endif
