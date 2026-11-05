#include "game.h"
#include <stdlib.h>

#define SS_OFF 0
#define SS_ON 1

extern sint16 Switch2Bounds[];
extern PHD_VECTOR Switch2Position;

int TestLaraPosition(sint16 *bounds, ITEM_INFO *item, ITEM_INFO *laraitem);
int MoveLaraPosition(PHD_VECTOR *vec, ITEM_INFO *item, ITEM_INFO *laraitem);
void AnimateLara(ITEM_INFO *item);

void SwitchCollision2(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll)
{
    ITEM_INFO *item;

    item = &items[item_num];
    if (!(input & IN_ACTION) || item->status != NOT_ACTIVE || lara.water_status != LARA_UNDERWATER ||
        lara.gun_status != LG_ARMLESS)
        return;
    if (laraitem->current_anim_state != AS_TREAD)
        return;
    if (!TestLaraPosition(Switch2Bounds, item, laraitem))
        return;
    if (item->current_anim_state == SS_ON || item->current_anim_state == SS_OFF) {
        if (MoveLaraPosition(&Switch2Position, item, laraitem)) {
            laraitem->fallspeed = 0;
            laraitem->goal_anim_state = AS_SWITCHON;
            do {
                AnimateLara(laraitem);
            } while (laraitem->current_anim_state != AS_SWITCHON);
            laraitem->goal_anim_state = AS_TREAD;
            lara.gun_status = LG_HANDSBUSY;
            item->status = ACTIVE;
            if (item->current_anim_state == SS_ON)
                item->goal_anim_state = SS_OFF;
            else
                item->goal_anim_state = SS_ON;
            AddActiveItem(item_num);
            AnimateItem(item);
        }
    }
}

extern sint16 KeyHoleBounds[];
extern PHD_VECTOR KeyHolePosition;
extern sint32 pup_x, pup_y, pup_z;
extern sint16 Inventory_Chosen;
sint32 Display_Inventory(sint32 inventory_mode);
int Inv_RemoveItem(int itemNum);
void AlignLaraPosition(PHD_VECTOR *vec, ITEM_INFO *item, ITEM_INFO *laraitem);

void KeyHoleCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll)
{
    int correct = 0;
    ITEM_INFO *item;

    if (laraitem->current_anim_state == AS_STOP && laraitem->anim_number == 103) {
        item = &items[item_num];
        if ((Inventory_Chosen == -1 && !(input & IN_ACTION)) || lara.gun_status != LG_ARMLESS || laraitem->gravity_status)
            return;
        if (!TestLaraPosition(KeyHoleBounds, item, laraitem))
            return;
        if (item->status != NOT_ACTIVE) {
            if (laraitem->pos.x_pos != pup_x || laraitem->pos.y_pos != pup_y || laraitem->pos.z_pos != pup_z) {
                pup_x = laraitem->pos.x_pos;
                pup_y = laraitem->pos.y_pos;
                pup_z = laraitem->pos.z_pos;
                SoundEffect(2, &laraitem->pos, 0);
            }
            return;
        }
        if (Inventory_Chosen == -1) {
            Display_Inventory(2);
            if (Inventory_Chosen == -1 && inv_keys_objects)
                return;
            if (Inventory_Chosen != -1)
                pup_y = laraitem->pos.y_pos - 1;
        } else
            pup_y = laraitem->pos.y_pos - 1;
        switch (item->object_number) {
        case 232:
            if (Inventory_Chosen != 228)
                break;
            Inv_RemoveItem(228);
            correct = 1;
            break;
        case 233:
            if (Inventory_Chosen != 229)
                break;
            Inv_RemoveItem(229);
            correct = 1;
            break;
        case 234:
            if (Inventory_Chosen != 230)
                break;
            Inv_RemoveItem(230);
            correct = 1;
            break;
        case 235:
            if (Inventory_Chosen != 231)
                break;
            Inv_RemoveItem(231);
            correct = 1;
            break;
        }
        Inventory_Chosen = -1;
        if (!correct) {
            if (laraitem->pos.x_pos != pup_x || laraitem->pos.y_pos != pup_y || laraitem->pos.z_pos != pup_z) {
                SoundEffect(2, &laraitem->pos, 0);
                pup_x = laraitem->pos.x_pos;
                pup_y = laraitem->pos.y_pos;
                pup_z = laraitem->pos.z_pos;
            }
            return;
        }
        AlignLaraPosition(&KeyHolePosition, item, laraitem);
        laraitem->goal_anim_state = AS_USEKEY;
        do
            AnimateLara(laraitem);
        while (laraitem->current_anim_state != AS_USEKEY);
        laraitem->goal_anim_state = AS_STOP;
        lara.gun_status = LG_HANDSBUSY;
        item->status = ACTIVE;
        pup_x = laraitem->pos.x_pos;
        pup_y = laraitem->pos.y_pos;
        pup_z = laraitem->pos.z_pos;
    }
}

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

#define KILLED_ITEM (-0x8000)

extern sint16 rcossin_tbl[];

void AnimatingPickUp(sint16 item_number)
{
    ITEM_INFO *item;
    sint32 dx, dy, dz;
    sint32 col;

    item = &items[item_number];
    if (item->status == INVISIBLE || (item->flags & KILLED_ITEM))
        return;
    if (item->object_number == 180 && item->item_flags[1] == 0) {
        item->item_flags[1] = 1;
        item->item_flags[2] = item->pos.y_pos;
    }
    item->pos.y_rot += 0x400;
    item->item_flags[0]++;
    item->item_flags[0] &= 63;
    col = abs(rcossin_tbl[item->item_flags[0] << 7] >> 7);
    if (col > 31)
        col = 31;
    if (item->object_number == 180) {
        dx = abs(rcossin_tbl[item->item_flags[0] << 7] >> 4);
        item->pos.y_pos = item->item_flags[2] - dx - 64;
        TriggerDynamic(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 8, 0, col, 0);
        dx = abs(item->pos.x_pos - lara_item->pos.x_pos);
        dy = abs(item->pos.y_pos - lara_item->pos.y_pos);
        dz = abs(item->pos.z_pos - lara_item->pos.z_pos);
        if (dx < 0x100 && dy < 0x400 && dz < 0x100) {
            lara.poisoned = 0;
            lara_item->hit_points += 500;
            if (lara_item->hit_points > 1000)
                lara_item->hit_points = 1000;
            SoundEffect(116, &lara_item->pos, 0);
            KillItem(item_number);
        }
    } else
        TriggerDynamic(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 8, 0, col, col >> 1);
}
