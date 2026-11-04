#include "game.h"

extern sint32 compys_attack_lara;

void InitialiseCompy(sint16 item_number)
{
    InitialiseCreature(item_number);
    compys_attack_lara = 0;
}
