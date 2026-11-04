#include "game.h"

extern sint32 assault_targets_reset;
extern sint32 level_items;

void InitialiseTarget(sint16 item_number);

void ResetTargets(void)
{
    sint32 lp;

    if (!assault_targets_reset)
    {
        assault_targets = 0;
        for (lp = 0; lp < level_items; lp++)
        {
            if (items[lp].object_number == TARGETS)
            {
                InitialiseTarget(lp);
                assault_targets++;
            }
        }
        assault_targets_reset = 1;
    }
}
