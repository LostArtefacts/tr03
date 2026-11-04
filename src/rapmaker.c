#include "game.h"

void InitialiseRaptorEmitter(sint16 item_number)
{
    items[item_number].item_flags[0] = (item_number & 3) * 96;
}
