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

extern uint32 CurrentLevel;
extern int assault_timer_display;
extern sint16 phd_winxmax;

int sprintf(char *buf, const char *fmt, ...);
void S_DrawScreenSprite2d(sint32 x, sint32 y, sint32 z, sint32 scaleH, sint32 scaleV, sint16 sprnum, sint16 shade, uint16 flags);

void DrawAssaultTimer(void)
{
    int x, timer, hund, ttimer;
    char txt[8], *c;

    if (CurrentLevel != 0 || !assault_timer_display || QuadbikeLapTimeDisplayTimer)
        return;
    if (savegame.timer > 0x1a5dd)
        savegame.timer = 0x1a5dd;
    ttimer = savegame.timer;
    if (ttimer < 2)
        ttimer = 0;
    timer = ttimer / 30;
    hund = ((ttimer % 30) * 334) / 100;
    sprintf(txt, "%d:%02d.%02d", timer / 60, timer % 60, hund);
    x = (phd_winxmax >> 1) - 20 * 5 / 2;
    for (c = txt; *c; c++) {
        if (*c == ':') {
            x -= 6;
            S_DrawScreenSprite2d(x, 36, 0, 0x10000, 0x10000, (sint16)(objects[362].mesh_index + 10), 2, 0);
            x += 14;
        } else if (*c == '.') {
            x -= 6;
            S_DrawScreenSprite2d(x, 36, 0, 0x10000, 0x10000, (sint16)(objects[362].mesh_index + 11), 2, 0);
            x += 14;
        } else {
            S_DrawScreenSprite2d(x, 36, 0, 0x10000, 0x10000, (sint16)(objects[362].mesh_index + (*c - '0')), 2, 0);
            x += 20;
        }
    }
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

void S_DrawAirBar(int percent);

void DrawAirBar(int flash_state)
{
    int air;

    if ((lara.skidoo == NO_ITEM || items[lara.skidoo].object_number != 19) && lara.water_status != LARA_UNDERWATER && lara.water_status != LARA_SURFACE && !((room[lara_item->room_number].flags & 128) && lara.water_surface_dist < -775))
        return;
    air = lara.air;
    if (air < 0)
        air = 0;
    else if (air > 1800)
        air = 1800;
    if (air <= 450) {
        if (flash_state)
            S_DrawAirBar(air * 100 / 1800);
        else
            S_DrawAirBar(0);
    } else {
        S_DrawAirBar(air * 100 / 1800);
    }
}

void RemoveAmmoText(void)
{
    if (ammotext)
    {
        T_RemovePrint(ammotext);
        ammotext = NULL;
    }
}

TEXTSTRING *T_Print(sint32 xpos, sint32 ypos, sint32 zpos, char *string);
__declspec(dllimport) int __cdecl wsprintfA(char *buf, const char *fmt, ...);

void DrawAmmoInfo(void)
{
    char ammostring[80];

    ammostring[0] = 0;
    if (items[lara.skidoo].object_number == 19) {
        wsprintfA(ammostring, "%5d", lara.harpoon.ammo);
    } else {
        if (lara.gun_status != LG_READY || overlay_flag <= 0 || savegame.bonus_flag) {
            RemoveAmmoText();
            return;
        }
        switch (lara.gun_type) {
        case LG_PISTOLS:
            return;
        case LG_MAGNUMS:
            wsprintfA(ammostring, "%5d", lara.magnums.ammo);
            break;
        case LG_UZIS:
            wsprintfA(ammostring, "%5d", lara.uzis.ammo);
            break;
        case LG_SHOTGUN:
            wsprintfA(ammostring, "%5d", lara.shotgun.ammo / 6);
            break;
        case LG_HARPOON:
            wsprintfA(ammostring, "%5d", lara.harpoon.ammo);
            break;
        case LG_M16:
            wsprintfA(ammostring, "%5d", lara.m16.ammo);
            break;
        case LG_ROCKET:
            wsprintfA(ammostring, "%5d", lara.rocket.ammo);
            break;
        case LG_GRENADE:
            wsprintfA(ammostring, "%5d", lara.grenade.ammo);
            break;
        default:
            return;
        }
    }
    RemoveAmmoText();
    ammotext = T_Print(-10, 50, 0, ammostring);
    T_RightAlign(ammotext, 1);
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
