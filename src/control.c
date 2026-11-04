#include "game.h"

#define MAX_CD_TRACKS 128

void TriggerNormalCDTrack(sint16 value, sint16 flags, sint16 type);

void TriggerCDTrack(sint16 value, sint16 flags, sint16 type)
{
    if (value <= 1 || value >= MAX_CD_TRACKS)
        return;
    TriggerNormalCDTrack(value, flags, type);
}
