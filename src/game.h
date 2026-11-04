#ifndef GAME_H
#define GAME_H

#include <stddef.h>

#include "types.h"
#include "items.h"
#include "lara.h"
#include "room.h"
#include "input.h"
#include "collide.h"
#include "laraanim.h"
#include "larafunc.h"
#include "laraflar.h"
#include "objnums.h"
#include "anim.h"
#include "camera.h"
#include "effects.h"
#include "effect2.h"
#include "text.h"
#include "invdata.h"
#include "sound.h"
#include "savegame.h"
#include "box.h"
#include "title.h"
#include "gameflow.h"
#include "specific/specific.h"

extern ITEM_INFO *items;
extern ROOM_INFO *room;
extern int level_complete;
extern int flipeffect, fliptimer;
extern int assault_timer_display, assault_timer_active;
extern int assault_penalties, assault_penalty_display_timer;
extern int assault_target_penalties, assault_targets;
extern int QuadbikeLapTime, QuadbikeLapTimeDisplayTimer;
extern char ExerciseNumber;

extern int flip_status;

typedef struct {
    sint16 attack_count;
    sint16 death_count;
    uint8 attack_flag;
    uint8 attack_type;
    uint8 attack_head_count;
    uint8 ring_count;
    sint16 explode_count;
    sint16 LizmanItem;
    sint16 LizmanRoom;
    sint16 hp_counter;
    sint16 dropped_icon;
    uint8 charged;
    uint8 dead;
    PHD_VECTOR BeamTarget;
} BOSS_STRUCT;

extern BOSS_STRUCT bossdata;

void FlipMap(void);
sint16 *GetBoundsAccurate(ITEM_INFO *item);
void DrawAnimatingItem(ITEM_INFO *item);
void DrawUnclippedItem(ITEM_INFO *item);
void S_DrawWakeFX(ITEM_INFO *item);
void DrawExplosionRings(void);
void GetJointAbsPosition(ITEM_INFO *item, PHD_VECTOR *pos, int joint);
void InitialiseHair(void);
void ResetTargets(void);

#endif
