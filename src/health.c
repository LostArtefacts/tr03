#include "game.h"

#define NUM_PU 1

typedef struct {
    sint16 duration;
    sint16 sprnum;
} DISPLAYPU;

extern DISPLAYPU pickups[NUM_PU];
extern TEXTSTRING *ammotext;
extern TEXTSTRING *LpModeTS;
extern int LnModeTSLife;
extern int flash_state, flash_count;

int FlashIt(void)
{
    if (flash_count)
        flash_count--;
    else
    {
        flash_state ^= 1;
        flash_count = 5;
    }
    return flash_state;
}

void RemoveAmmoText(void)
{
    if (ammotext)
    {
        T_RemovePrint(ammotext);
        ammotext = NULL;
    }
}

void InitialisePickUpDisplay(void)
{
    int i;

    for (i = 0; i < NUM_PU; i++)
        pickups[i].duration = 0;
}

void AddDisplayPickup(sint16 objnum)
{
    if (objnum == SECRET_ITEM1 || objnum == SECRET_ITEM2 || objnum == SECRET_ITEM3)
        S_CDPlay(gameflow.secret_track, 0);
    pickups[0].duration = 75;
    pickups[0].sprnum = objnum;
}

TEXTSTRING *T_Print(sint32 xpos, sint32 ypos, sint32 zpos, char *string);

void DisplayModeInfo(char *string)
{
    if (!string) {
        T_RemovePrint(LpModeTS);
        LpModeTS = NULL;
        return;
    }
    if (LpModeTS) {
        T_ChangeText(LpModeTS, string);
    } else {
        LpModeTS = T_Print(-16, -16, 0, string);
        T_RightAlign(LpModeTS, 1);
        T_BottomAlign(LpModeTS, 1);
    }
    LnModeTSLife = 75;
}

void DrawModeInfo(void)
{
    if (LpModeTS && --LnModeTSLife == 0)
    {
        T_RemovePrint(LpModeTS);
        LpModeTS = NULL;
    }
}
