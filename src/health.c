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

void S_CDPlay(sint32 track, int mode);
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

extern int overlay_flag;
extern uint16 DashTimer;
extern sint16 ExposureMeter;

void DrawAmmoInfo(void);
void DrawModeInfo(void);
void DrawAssaultTimer(void);
void DrawAssaultPenalties(int type);
void DrawQuadbikeLapTime(void);
void DrawHealthBar(int flash_state);
void DrawAirBar(int flash_state);
void S_DrawDashBar(int percent);
void S_DrawColdBar(int percent);

void DrawGameInfo(int timed)
{
    int flash_state;

    DrawAmmoInfo();
    DrawModeInfo();
    if (overlay_flag > 0) {
        flash_state = FlashIt();
        DrawAssaultTimer();
        DrawAssaultPenalties(0);
        DrawAssaultPenalties(1);
        DrawQuadbikeLapTime();
        DrawHealthBar(flash_state);
        DrawAirBar(flash_state);
        if (DashTimer < 120)
            S_DrawDashBar(DashTimer * 100 / 120);
        if (ExposureMeter < 600) {
            if (ExposureMeter < 0)
                S_DrawColdBar(0);
            else if (!(ExposureMeter < 150 && flash_state))
                S_DrawColdBar(ExposureMeter * 100 / 600);
        }
    }
    T_DrawText();
}

extern int old_hitpoints;
extern int health_bar_timer;
extern char PoisonFlag;

void S_DrawHealthBar(int percent);

void DrawHealthBar(int flash_state)
{
    int hitpoints;

    hitpoints = lara_item->hit_points;
    if (hitpoints < 0)
        hitpoints = 0;
    else if (hitpoints > 1000)
        hitpoints = 1000;
    if (old_hitpoints != hitpoints) {
        old_hitpoints = hitpoints;
        health_bar_timer = 40;
    }
    if (health_bar_timer < 0)
        health_bar_timer = 0;
    if (hitpoints <= 250) {
        if (flash_state)
            S_DrawHealthBar(hitpoints / 10);
        else
            S_DrawHealthBar(0);
    } else if (health_bar_timer > 0 || hitpoints <= 0 || lara.gun_status == LG_READY || lara.poisoned) {
        if (lara.poisoned) {
            if (flash_state)
                S_DrawHealthBar(hitpoints / 10);
            else
                S_DrawHealthBar(0);
        } else {
            S_DrawHealthBar(hitpoints / 10);
        }
    }
    if (PoisonFlag)
        PoisonFlag--;
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
