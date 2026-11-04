#include <string.h>
#include "game.h"

extern sint32 flipmap[10];
extern char cd_flags[128];
extern sint32 SunsetTimer;
extern TEXTSTRING *ammotext;
extern sint32 monks_attack_lara;

extern sint32 compy_scared_timer;
extern sint32 compys_attack_lara;
extern sint16 CarcassItem;

void InitialiseGameFlags(void)
{
    int i;

    flip_status = 0;
    memset(flipmap, 0, sizeof(flipmap));
    memset(cd_flags, 0, sizeof(cd_flags));
    for (i = 0; i < NUMBER_OBJECTS; i++)
        objects[i].loaded = 0;
    SunsetTimer = 0;
    ammotext = NULL;
    level_complete = 0;
    flipeffect = -1;
    monks_attack_lara = 0;
    CarcassItem = NO_ITEM;
    compys_attack_lara = 0;
}

void InitialiseLevelFlags(void)
{
    savegame.secrets = 0;
    savegame.timer = 0;
    savegame.kills = 0;
    savegame.distance_travelled = 0;
    savegame.ammo_used = 0;
    savegame.ammo_hit = 0;
    savegame.health_used = 0;
    assault_timer_active = 0;
    QuadbikeLapTime = QuadbikeLapTimeDisplayTimer = 0;
    compy_scared_timer = 0;
    compys_attack_lara = 0;
    CarcassItem = -1;
}

void BaddyObjects(void);
void TrapObjects(void);
void ObjectObjects(void);
void InitialiseSparks(void);

void InitialiseObjects(void)
{
    int i;
    OBJECT_INFO *obj;

    for (i = 0, obj = objects; i < NUMBER_OBJECTS; i++, obj++) {
        obj->initialise = NULL;
        obj->collision = NULL;
        obj->control = NULL;
        obj->draw_routine = DrawAnimatingItem;
        obj->floor = obj->ceiling = NULL;
        obj->pivot_length = 0;
        obj->radius = 10;
        obj->shadow_size = 0;
        obj->hit_points = DONT_TARGET;
        obj->intelligent = obj->water_creature = 0;
        obj->save_position = obj->save_hitpoints = obj->save_flags = obj->save_anim = 0;
    }

    BaddyObjects();
    TrapObjects();
    ObjectObjects();
    InitialiseHair();
    InitialiseSparks();
}
