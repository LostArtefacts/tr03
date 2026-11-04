#include <stdio.h>
#include "game.h"

extern int SGcount;
extern char *SGpoint;

void ModifyStartInfo(int level_number);

void InitialiseStartInfo(void)
{
    int i;
    FILE *file;

    if (savegame.bonus_flag)
        return;

    for (i = 0; i < 24; i++) {
        ModifyStartInfo(i);
        savegame.start[i].available = 0;
        savegame.start[i].timer = 0;
        savegame.start[i].ammo_used = 0;
        savegame.start[i].ammo_hit = 0;
        savegame.start[i].distance_travelled = 0;
        savegame.start[i].kills = 0;
        savegame.start[i].secrets_found = 0;
        savegame.start[i].health_used = 0;
    }

    for (i = 0; i < 10; i++) {
        savegame.best_assault_times[i] = 0;
        savegame.best_quadbike_times[i] = 0;
    }

    savegame.AfterAdventureSave = 0;
    savegame.WorldRequired = 0;
    savegame.IndiaComplete = 0;
    savegame.SPacificComplete = 0;
    savegame.NevadaComplete = 0;
    savegame.LondonComplete = 0;
    savegame.AntarcticaComplete = 0;
    savegame.PeruComplete = 0;
    savegame.AfterIndia = 0;
    savegame.AfterSPacific = 0;
    savegame.AfterLondon = 0;
    savegame.AfterNevada = 0;
    savegame.QuadbikeKeyFlag = 0;
    savegame.start[0].available = 1;
    savegame.start[1].available = 1;
    savegame.bonus_flag = 0;
    savegame.GameComplete = 0;

    file = fopen("data.bin", "rb");
    if (file) {
        fread(savegame.best_assault_times, 4, 10, file);
        fread(savegame.best_quadbike_times, 4, 10, file);
        fread(&savegame.QuadbikeKeyFlag, 4, 1, file);
    }
}

void ModifyStartInfo(int level_number)
{
    START_INFO *start;

    start = &savegame.start[level_number];
    start->got_pistols = 1;
    start->gun_type = LG_PISTOLS;
    start->pistol_ammo = 1000;

    switch (level_number) {
    case 0:
        start->available = 1;
        start->pistol_ammo = start->magnum_ammo = start->uzi_ammo = start->shotgun_ammo = start->harpoon_ammo
            = start->rocket_ammo = start->grenade_ammo = start->m16_ammo = start->harpoon_ammo = start->rocket_ammo = 0;
        start->got_pistols = start->got_magnums = start->got_uzis = start->got_shotgun = start->got_harpoon
            = start->got_rocket = start->got_grenade = start->got_m16 = start->got_harpoon = 0;
        start->num_medis = start->num_big_medis = start->num_flares = 0;
        start->num_sgcrystals = 1;
        start->gun_type = LG_UNARMED;
        start->gun_status = LG_ARMLESS;
        break;

    case 1:
        start->available = 1;
        start->num_flares = 2;
        start->num_medis = 1;
        start->num_big_medis = 1;
        start->num_sgcrystals = 1;
        start->got_magnums = start->got_uzis = start->got_m16 = start->got_harpoon = start->got_rocket
            = start->got_grenade = 0;
        start->magnum_ammo = start->uzi_ammo = start->m16_ammo = start->harpoon_ammo = start->rocket_ammo
            = start->grenade_ammo = 0;
        start->gun_status = LG_ARMLESS;
        break;
    }

    if (savegame.bonus_flag && level_number != 0) {
        start->available = 1;
        start->got_pistols = start->got_magnums = start->got_uzis = start->got_shotgun = start->got_rocket
            = start->got_grenade = start->got_m16 = start->got_harpoon = 1;
        start->magnum_ammo = start->uzi_ammo = start->shotgun_ammo = start->rocket_ammo = start->grenade_ammo
            = start->m16_ammo = start->harpoon_ammo = 10000;
        start->gun_type = LG_ROCKET;
        start->num_flares = 255;
        start->num_sgcrystals = 50;
    }
}

void ResetSG(void)
{
    SGcount = 0;
    SGpoint = savegame.buffer;
}

void WriteSG(void *pointer, int size)
{
    char *data;

    SGcount += size;
    if (SGcount >= MAX_SAVEGAME_BUFFER)
        S_ExitSystem("FATAL: Savegame is too big to fit in buffer");

    data = pointer;
    for (; size > 0; size--)
        *(SGpoint++) = *(data++);
}

void ReadSG(void *pointer, int size)
{
    char *data;

    SGcount += size;
    data = pointer;
    for (; size > 0; size--)
        *(data++) = *(SGpoint++);
}
