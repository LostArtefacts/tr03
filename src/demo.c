#include "game.h"

#define DEMOCOUNT_MAX 3608

extern uint32 *demoptr;
extern int democount;
extern int demo_level;

sint32 DoDemoSequence(int level_number)
{
    if (level_number < 0 && !gameflow.num_demos)
        return EXIT_TO_TITLE;
    if (level_number < 0)
    {
        if (demo_level >= gameflow.num_demos)
            demo_level = 0;
        level_number = GF_valid_demos[demo_level];
        demo_level++;
    }
    else
        demo_level = level_number;
    return GF_DoLevelSequence(level_number, IL_DEMO);
}

void GetDemoInput(void)
{
    if (democount < DEMOCOUNT_MAX)
        input = demoptr[democount];
    else
        input = -1;
    if (input != -1)
        democount++;
}
