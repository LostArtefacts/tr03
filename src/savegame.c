#include "game.h"

extern int SGcount;
extern char *SGpoint;

void ResetSG(void)
{
    SGcount = 0;
    SGpoint = savegame.buffer;
}
