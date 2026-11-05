#include "../game.h"

void SortOutAdventureSave(int world)
{
    if (savegame.WorldRequired == 1)
        savegame.AfterIndia = world;
    if (savegame.WorldRequired == 2)
        savegame.AfterSPacific = world;
    if (savegame.WorldRequired == 3)
        savegame.AfterLondon = world;
    if (savegame.WorldRequired == 4)
        savegame.AfterNevada = world;
    savegame.AfterAdventureSave = 0;
    savegame.WorldRequired = 0;
}

int Level2World(int level)
{
    if (level <= 4)
        return 1;
    else if (level <= 8)
        return 2;
    else if (level <= 11)
        return 3;
    else if (level <= 15)
        return 4;
    else
        return 5;
}

int World2Level(int world)
{
    if (world == 1)
        return 0;
    if (world == 2)
        return 4;
    if (world == 3)
        return 8;
    if (world == 4)
        return 12;
    else
        return 15;
}
