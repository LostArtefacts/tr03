#ifndef GAME_H
#define GAME_H

#include "types.h"
#include "items.h"
#include "lara.h"
#include "anim.h"
#include "camera.h"
#include "effects.h"
#include "sound.h"

extern int level_complete;
extern int flipeffect, fliptimer;

void FlipMap(void);
void InitialiseHair(void);

#endif
