#include "game.h"

extern sint16 HoldStates[14];

void SmashWindow(sint16 item_number);

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

void SmashItem(sint16 item_number, int weapon_type)
{
    ITEM_INFO *item;

    item = &items[item_number];
    switch (item->object_number)
    {
    case SMASH_WINDOW:
    case SMASH_OBJECT1:
    case SMASH_OBJECT2:
    case SMASH_OBJECT3:
        SmashWindow(item_number);
        break;
    case CARCASS:
    case EXTRAFX6:
        if (item->status != ACTIVE)
        {
            item->status = ACTIVE;
            AddActiveItem(item_number);
        }
        break;
    }
}

int WeaponObject(int weapon_type)
{
    switch (weapon_type)
    {
    case LG_MAGNUMS:
        return MAGNUM;
    case LG_UZIS:
        return UZI;
    case LG_SHOTGUN:
        return SHOTGUN;
    case LG_ROCKET:
        return ROCKET_GUN;
    case LG_GRENADE:
        return GRENADE_GUN;
    case LG_HARPOON:
        return HARPOON_GUN;
    case LG_M16:
        return M16;
    default:
        return PISTOLS;
    }
}
