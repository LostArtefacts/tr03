#include "game.h"

extern sint32 compy_scared_timer;
extern sint32 compys_attack_lara;
extern sint16 CarcassItem;

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
