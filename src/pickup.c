#include "game.h"

#define SS_OFF 0
#define SS_ON 1

extern sint16 Switch2Bounds[];
extern PHD_VECTOR Switch2Position;

int TestLaraPosition(sint16 *bounds, ITEM_INFO *item, ITEM_INFO *laraitem);
int MoveLaraPosition(PHD_VECTOR *vec, ITEM_INFO *item, ITEM_INFO *laraitem);
void AnimateLara(ITEM_INFO *item);

extern sint16 Switch1Bounds[];
extern PHD_VECTOR SmallSwitchPosition;
extern PHD_VECTOR PushSwitchPosition;
extern PHD_VECTOR AirlockPosition;

void AlignLaraPosition(PHD_VECTOR *vec, ITEM_INFO *item, ITEM_INFO *laraitem);

extern sint16 PickUpBounds[];
extern sint16 PickUpBoundsUW[];
extern PHD_VECTOR PickUpPosition;
extern PHD_VECTOR PickUpPositionUW;
extern uint32 CurrentLevel;

void AddDisplayPickup(sint16 objnum);
int Inv_AddItem(int itemNum);
void RemoveDrawnItem(sint16 item_num);
void InitialiseNewWeapon(void);
void GF_ModifyInventory(int level, int type);
void draw_flare_meshes(void);

void PickUpCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll)
{
    ITEM_INFO *item;
    sint16 x_rot, y_rot, z_rot;

    item = &items[item_num];
    z_rot = item->pos.z_rot;
    y_rot = item->pos.y_rot;
    x_rot = item->pos.x_rot;
    item->pos.y_rot = laraitem->pos.y_rot;
    item->pos.z_rot = 0;

    if (lara.water_status == LARA_ABOVEWATER || lara.water_status == LARA_WADE) {
        item->pos.x_rot = 0;
        if (TestLaraPosition(PickUpBounds, item, laraitem)) {
            if (laraitem->current_anim_state == AS_PICKUP) {
                if ((laraitem->frame_number == anims[135].frame_base + 42 ||
                     laraitem->frame_number == anims[291].frame_base + 22 ||
                     laraitem->frame_number == anims[291].frame_base + 20) &&
                    item->object_number != 179) {
                    AddDisplayPickup(item->object_number);
                    Inv_AddItem(item->object_number);
                    if (item->object_number == 221 || item->object_number == 222 || item->object_number == 223) {
                        if ((savegame.secrets & 1) + ((savegame.secrets & 2) >> 1) + ((savegame.secrets & 4) >> 2) >= 3)
                            GF_ModifyInventory(CurrentLevel, 1);
                    }
                    item->status = INVISIBLE;
                    if (item->object_number == 240 || item->object_number == 241 || item->object_number == 242 ||
                        item->object_number == 243) {
                        if (CurrentLevel != 19)
                            level_complete = 1;
                        else
                            KillItem(item_num);
                    } else
                        RemoveDrawnItem(item_num);
                } else {
                    item->pos.x_rot = x_rot;
                    item->pos.y_rot = y_rot;
                    item->pos.z_rot = z_rot;
                }
                return;
            } else if (laraitem->current_anim_state == AS_FLAREPICKUP) {
                if (((laraitem->anim_number == 312 && laraitem->frame_number == anims[312].frame_base + 22) ||
                     laraitem->frame_number == anims[204].frame_base + 58) &&
                    item->object_number == 179 && lara.gun_type != LG_FLARE) {
                    lara.request_gun_type = LG_FLARE;
                    lara.gun_type = LG_FLARE;
                    InitialiseNewWeapon();
                    lara.gun_status = LG_SPECIAL;
                    lara.flare_age = (sint32)item->data & 0x7fff;
                    KillItem(item_num);
                }
                return;
            } else if ((input & IN_ACTION) && laraitem->current_anim_state == AS_ALL4S)
                laraitem->goal_anim_state = AS_DUCK;
            else if ((input & IN_ACTION) && !laraitem->gravity_status &&
                     ((laraitem->current_anim_state == AS_STOP && laraitem->anim_number == 103) ||
                      (laraitem->current_anim_state == AS_DUCK && laraitem->anim_number == 222)) &&
                     lara.gun_status == LG_ARMLESS && !(lara.gun_type == LG_FLARE && item->object_number == 179)) {
                if (item->object_number == 179) {
                    laraitem->goal_anim_state = AS_FLAREPICKUP;
                    do {
                        AnimateLara(laraitem);
                    } while (laraitem->current_anim_state != AS_FLAREPICKUP);
                } else {
                    AlignLaraPosition(&PickUpPosition, item, laraitem);
                    laraitem->goal_anim_state = AS_PICKUP;
                    AnimateLara(laraitem);
                }
                if (lara.IsDucked)
                    laraitem->goal_anim_state = AS_DUCK;
                else
                    laraitem->goal_anim_state = AS_STOP;
                lara.gun_status = LG_HANDSBUSY;
                item->pos.x_rot = x_rot;
                item->pos.y_rot = y_rot;
                item->pos.z_rot = z_rot;
                return;
            }
        }
    } else if (lara.water_status == LARA_UNDERWATER) {
        item->pos.x_rot = -25 * ONE_DEGREE;
        if (TestLaraPosition(PickUpBoundsUW, item, laraitem)) {
            if (laraitem->current_anim_state == AS_PICKUP) {
                if (laraitem->frame_number == anims[130].frame_base + 18 && item->object_number != 179) {
                    AddDisplayPickup(item->object_number);
                    Inv_AddItem(item->object_number);
                    if (item->object_number == 221 || item->object_number == 222 || item->object_number == 223) {
                        if ((savegame.secrets & 1) + ((savegame.secrets & 2) >> 1) + ((savegame.secrets & 4) >> 2) >= 3)
                            GF_ModifyInventory(CurrentLevel, 1);
                    }
                    item->status = INVISIBLE;
                    RemoveDrawnItem(item_num);
                }
                return;
            } else if (laraitem->current_anim_state == AS_FLAREPICKUP) {
                if (laraitem->frame_number == anims[206].frame_base + 20 && item->object_number == 179 &&
                    lara.gun_type != LG_FLARE) {
                    lara.request_gun_type = LG_FLARE;
                    lara.gun_type = LG_FLARE;
                    InitialiseNewWeapon();
                    lara.gun_status = LG_SPECIAL;
                    lara.flare_age = (sint32)item->data & 0x7fff;
                    draw_flare_meshes();
                    KillItem(item_num);
                }
                return;
            } else if ((input & IN_ACTION) && laraitem->current_anim_state == AS_TREAD && lara.gun_status == LG_ARMLESS &&
                       !(lara.gun_type == LG_FLARE && item->object_number == 179)) {
                if (!MoveLaraPosition(&PickUpPositionUW, item, laraitem))
                    return;
                if (item->object_number == 179) {
                    laraitem->fallspeed = 0;
                    laraitem->current_anim_state = AS_FLAREPICKUP;
                    laraitem->anim_number = 206;
                    laraitem->frame_number = anims[206].frame_base;
                } else {
                    laraitem->goal_anim_state = AS_PICKUP;
                    do {
                        AnimateLara(laraitem);
                    } while (laraitem->current_anim_state != AS_PICKUP);
                }
                laraitem->goal_anim_state = AS_TREAD;
                return;
            }
        }
    }

    item->pos.x_rot = x_rot;
    item->pos.y_rot = y_rot;
    item->pos.z_rot = z_rot;
}

void SwitchCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll)
{
    ITEM_INFO *item;

    item = &items[item_num];

    if (!(input & IN_ACTION) || item->status != NOT_ACTIVE || (item->flags & ONESHOT) || lara.gun_status != LG_ARMLESS ||
        laraitem->gravity_status || laraitem->current_anim_state != AS_STOP || laraitem->anim_number != 103 ||
        !TestLaraPosition(Switch1Bounds, item, laraitem))
        return;

    lara.head_x_rot = lara.head_y_rot = 0;
    lara.torso_x_rot = lara.torso_y_rot = 0;
    laraitem->pos.y_rot = item->pos.y_rot;

    if (item->object_number == 118)
        AlignLaraPosition(&SmallSwitchPosition, item, laraitem);
    else if (item->object_number == 128)
        AlignLaraPosition(&PushSwitchPosition, item, laraitem);
    else if (item->object_number == 117)
        AlignLaraPosition(&AirlockPosition, item, laraitem);

    if (item->current_anim_state == SS_ON) {
        laraitem->current_anim_state = AS_SWITCHON;
        if (item->object_number == 118) {
            laraitem->anim_number = objects[LARA].anim_index + 195;
        } else if (item->object_number == 128) {
            laraitem->anim_number = objects[LARA].anim_index + 197;
        } else if (item->object_number == 117) {
            laraitem->anim_number = objects[13].anim_index;
            laraitem->frame_number = anims[laraitem->anim_number].frame_base;
            laraitem->current_anim_state = 0;
            laraitem->goal_anim_state = 4;
            AnimateItem(laraitem);
            lara.extra_anim = 1;
        } else {
            laraitem->anim_number = objects[LARA].anim_index + 63;
        }
        item->goal_anim_state = SS_OFF;
    } else {
        laraitem->current_anim_state = AS_SWITCHOFF;
        if (item->object_number == 118) {
            laraitem->anim_number = objects[LARA].anim_index + 196;
        } else if (item->object_number == 128) {
            laraitem->anim_number = objects[LARA].anim_index + 197;
        } else if (item->object_number == 117) {
            laraitem->anim_number = objects[13].anim_index;
            laraitem->frame_number = anims[laraitem->anim_number].frame_base;
            laraitem->current_anim_state = 0;
            laraitem->goal_anim_state = 4;
            AnimateItem(laraitem);
            lara.extra_anim = 1;
        } else {
            laraitem->anim_number = objects[LARA].anim_index + 64;
        }
        item->goal_anim_state = SS_ON;
    }

    if (!lara.extra_anim) {
        laraitem->frame_number = anims[laraitem->anim_number].frame_base;
        laraitem->goal_anim_state = AS_STOP;
    }

    lara.gun_status = LG_HANDSBUSY;
    item->status = ACTIVE;
    AddActiveItem(item_num);
    AnimateItem(item);
}

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

extern sint16 PuzzleHoleBounds[];
extern PHD_VECTOR PuzzleHolePosition;

void PuzzleHoleCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll)
{
    ITEM_INFO *item;
    int correct = 0;

    item = &items[item_num];
    if (laraitem->current_anim_state == AS_STOP && laraitem->anim_number == 103) {
        if ((Inventory_Chosen == -1 && !(input & IN_ACTION)) || lara.gun_status != LG_ARMLESS || laraitem->gravity_status)
            return;
        if (!TestLaraPosition(PuzzleHoleBounds, item, laraitem))
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
        case 213:
            if (Inventory_Chosen != 209)
                break;
            Inv_RemoveItem(209);
            correct = 1;
            break;
        case 214:
            if (Inventory_Chosen != 210)
                break;
            Inv_RemoveItem(210);
            correct = 1;
            break;
        case 215:
            if (Inventory_Chosen != 211)
                break;
            Inv_RemoveItem(211);
            correct = 1;
            break;
        case 216:
            if (Inventory_Chosen != 212)
                break;
            Inv_RemoveItem(212);
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
        AlignLaraPosition(&PuzzleHolePosition, item, laraitem);
        laraitem->goal_anim_state = AS_USEPUZZLE;
        do {
            AnimateLara(laraitem);
        } while (laraitem->current_anim_state != AS_USEPUZZLE);
        laraitem->goal_anim_state = AS_STOP;
        lara.gun_status = LG_HANDSBUSY;
        item->status = ACTIVE;
        pup_x = laraitem->pos.x_pos;
        pup_y = laraitem->pos.y_pos;
        pup_z = laraitem->pos.z_pos;
        return;
    } else if (laraitem->current_anim_state == AS_USEPUZZLE) {
        if (!TestLaraPosition(PuzzleHoleBounds, item, laraitem))
            return;
        if (laraitem->frame_number == GF(134, 80)) {
            switch (item->object_number) {
            case 213:
                item->object_number = 217;
                break;
            case 214:
                item->object_number = 218;
                break;
            case 215:
                item->object_number = 219;
                break;
            case 216:
                item->object_number = 220;
                break;
            }
            item->anim_number = objects[item->object_number].anim_index;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = item->goal_anim_state = anims[item->anim_number].current_anim_state;
            item->required_anim_state = 0;
            AddActiveItem(item_num);
            item->status = ACTIVE;
            item->flags = 0x3e00;
            AnimateItem(item);
        }
    }
}

extern sint16 PickUpBounds[];
extern PHD_VECTOR DetonatorPosition;
extern sint16 Inventory_Chosen;
sint32 Display_Inventory(sint32 inventory_mode);
int Inv_RemoveItem(int item_num);
void AlignLaraPosition(PHD_VECTOR *vec, ITEM_INFO *item, ITEM_INFO *laraitem);
void ObjectCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);

void DetonatorCollision(sint16 item_number, ITEM_INFO *laraitem, COLL_INFO *coll)
{
    ITEM_INFO *item;
    sint16 x_rot, y_rot, z_rot;

    if (lara.extra_anim)
        return;
    item = &items[item_number];
    x_rot = item->pos.x_rot;
    y_rot = item->pos.y_rot;
    z_rot = item->pos.z_rot;
    item->pos.y_rot = laraitem->pos.y_rot;
    item->pos.z_rot = 0;
    item->pos.x_rot = 0;
    if (item->status != DEACTIVATED && (input & IN_ACTION) && lara.gun_status == LG_ARMLESS &&
        !laraitem->gravity_status && laraitem->current_anim_state == AS_STOP) {
        if (item->object_number == 295) {
            if (!TestLaraPosition(PickUpBounds, item, laraitem))
                goto normal_collision;
        }
        if (Inventory_Chosen == -1)
            Display_Inventory(2);
        if (Inventory_Chosen != 229)
            goto normal_collision;
        Inv_RemoveItem(229);
        AlignLaraPosition(&DetonatorPosition, item, laraitem);
        laraitem->anim_number = objects[13].anim_index;
        laraitem->frame_number = anims[laraitem->anim_number].frame_base;
        laraitem->current_anim_state = 0;
        if (item->object_number == 295)
            laraitem->goal_anim_state = 1;
        AnimateItem(laraitem);
        lara.extra_anim = 1;
        lara.gun_status = LG_HANDSBUSY;
        if (item->object_number == 295) {
            item->status = ACTIVE;
            AddActiveItem(item_number);
        }
        return;
    }
normal_collision:
    item->pos.x_rot = x_rot;
    item->pos.y_rot = y_rot;
    item->pos.z_rot = z_rot;
    ObjectCollision(item_number, laraitem, coll);
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
