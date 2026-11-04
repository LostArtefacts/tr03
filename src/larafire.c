#include "game.h"

extern sint16 HoldStates[14];

int CheckForHoldingState(int state)
{
    sint16 *holds = HoldStates;

    if (lara.extra_anim)
        return 0;

    while (*holds >= 0)
    {
        if (state == *holds)
            return 1;
        holds++;
    }
    return 0;
}
