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

sint16 *GetBestFrame(ITEM_INFO *item);

void find_target_point(ITEM_INFO *item, GAME_VECTOR *target)
{
    sint16 *bounds;
    sint32 x, y, z, c, s;

    bounds = GetBestFrame(item);
    x = (sint32)((bounds[0] + bounds[1]) / 2);
    y = (sint32)(bounds[2] + (bounds[3] - bounds[2]) / 3);
    z = (sint32)((bounds[4] + bounds[5]) / 2);
    c = phd_cos(item->pos.y_rot);
    s = phd_sin(item->pos.y_rot);
    target->x = item->pos.x_pos + ((c * x + s * z) >> W2V_SHIFT);
    target->y = item->pos.y_pos + y;
    target->z = item->pos.z_pos + ((c * z - s * x) >> W2V_SHIFT);
    target->room_number = item->room_number;
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
