#include "game.h"

#define NUM_PU 1

typedef struct {
    sint16 duration;
    sint16 sprnum;
} DISPLAYPU;

extern DISPLAYPU pickups[NUM_PU];
extern TEXTSTRING *ammotext;

void RemoveAmmoText(void)
{
    if (ammotext)
    {
        T_RemovePrint(ammotext);
        ammotext = NULL;
    }
}
