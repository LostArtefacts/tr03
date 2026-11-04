#include "game.h"

#define DEMOCOUNT_MAX 3608

extern uint32 *demoptr;
extern int democount;

void GetDemoInput(void)
{
    if (democount < DEMOCOUNT_MAX)
        input = demoptr[democount];
    else
        input = -1;
    if (input != -1)
        democount++;
}
