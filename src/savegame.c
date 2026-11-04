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
