#ifndef GAME_H
#define GAME_H

#include <stddef.h>

#include "types.h"
#include "items.h"
#include "lara.h"
#include "anim.h"
#include "camera.h"
#include "effects.h"
#include "sound.h"
#include "savegame.h"
#include "specific/specific.h"

extern int level_complete;
extern int flipeffect, fliptimer;
extern int assault_timer_display, assault_timer_active;
extern int assault_penalties, assault_penalty_display_timer;
extern int assault_target_penalties, assault_targets;
extern int QuadbikeLapTime, QuadbikeLapTimeDisplayTimer;
extern char ExerciseNumber;

void FlipMap(void);
void InitialiseHair(void);
void ResetTargets(void);

#endif
