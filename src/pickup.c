#include "game.h"

#define SS_OFF 0
#define SS_ON 1

void SwitchControl(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    item->flags |= CODE_BITS;
    if (!TriggerActive(item) && !(item->flags & ONESHOT))
    {
        item->goal_anim_state = SS_ON;
        item->timer = 0;
    }
    AnimateItem(item);
}

int SwitchTrigger(sint16 item_num, sint16 timer)
{
    ITEM_INFO *item;

    item = &items[item_num];
    if (item->status == DEACTIVATED)
    {
        if (item->current_anim_state == SS_OFF && timer > 0)
        {
            item->timer = timer;
            item->status = ACTIVE;
            if (timer != 1)
                item->timer *= 30;
        }
        else
        {
            RemoveActiveItem(item_num);
            item->status = NOT_ACTIVE;
            if (item->item_flags[0])
                item->flags |= ONESHOT;
        }
        return 1;
    }
    if (item->flags & ONESHOT)
        return 1;
    else
        return 0;
}

int KeyTrigger(sint16 item_num)
{
    ITEM_INFO *item;

    item = &items[item_num];
    if (item->status == ACTIVE && lara.gun_status != LG_HANDSBUSY)
    {
        item->status = DEACTIVATED;
        return 1;
    }
    return 0;
}

int PickupTrigger(sint16 item_num)
{
    ITEM_INFO *item;

    item = &items[item_num];
    if (item->status == INVISIBLE)
    {
        item->status = DEACTIVATED;
        return 1;
    }
    return 0;
}

void BossDropIcon(sint16 item_number)
{
    sint16 pickup_number;
    ITEM_INFO *item, *pickup;

    item = &items[item_number];
    pickup_number = item->carried_item;
    while (pickup_number != NO_ITEM) {
        pickup = &items[pickup_number];
        pickup->pos.x_pos = item->pos.x_pos;
        pickup->pos.y_pos = item->pos.y_pos;
        pickup->pos.z_pos = item->pos.z_pos;
        ItemNewRoom(pickup_number, item->room_number);
        AddActiveItem(pickup_number);
        item->status = ACTIVE;
        pickup_number = pickup->carried_item;
    }
}
