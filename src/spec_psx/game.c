#include "../game.h"

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
