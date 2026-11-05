#include "specific/standard.h"
#include "game.h"

extern sint16 body_bag;
extern sint16 next_item_free, next_item_active;
extern sint32 level_items;

void InitialiseItemArray(int numitems)
{
    int i;
    ITEM_INFO *item;

    body_bag = NO_ITEM;
    next_item_active = NO_ITEM;
    next_item_free = level_items;
    item = &items[level_items];
    for (i = level_items + 1; i < numitems; i++, item++)
    {
        item->next_item = i;
        item->active = 0;
        item->il.init = 0;
    }
    item->next_item = NO_ITEM;
}

void DetatchSpark(sint32 num, sint32 type);

void KillItem(sint16 item_num)
{
    sint16 linknum;
    ITEM_INFO *item;

    DetatchSpark(item_num, 128);
    item = &items[item_num];
    item->really_active = 0;
    item->active = 0;
    linknum = next_item_active;
    if (linknum == item_num) {
        next_item_active = item->next_active;
    } else {
        for (; linknum != NO_ITEM; linknum = items[linknum].next_active) {
            if (items[linknum].next_active == item_num) {
                items[linknum].next_active = item->next_active;
                break;
            }
        }
    }

    if (item->room_number != 255) {
        linknum = room[item->room_number].item_number;
        if (linknum == item_num) {
            room[item->room_number].item_number = item->next_item;
        } else {
            for (; linknum != NO_ITEM; linknum = items[linknum].next_item) {
                if (items[linknum].next_item == item_num) {
                    items[linknum].next_item = item->next_item;
                    break;
                }
            }
        }
    }

    if (item == lara.target)
        lara.target = NULL;
    if (item_num < level_items) {
        item->flags |= -0x8000;
    } else {
        item->next_item = next_item_free;
        next_item_free = item_num;
    }
}

sint16 CreateItem(void)
{
    sint16 item_num;

    item_num = next_item_free;
    if (item_num == NO_ITEM)
        return item_num;
    items[item_num].flags = 0;
    items[item_num].il.init = 0;
    next_item_free = items[item_num].next_item;
    return item_num;
}

extern int DemoPlay;

void InitialiseItem(sint16 item_num)
{
    ITEM_INFO *item;
    ROOM_INFO *r;
    FLOOR_INFO *floor;

    item = &items[item_num];
    item->anim_number = objects[item->object_number].anim_index;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = item->goal_anim_state = anims[item->anim_number].current_anim_state;
    item->required_anim_state = 0;
    item->pos.x_rot = item->pos.z_rot = 0;
    item->speed = item->fallspeed = 0;
    item->status = NOT_ACTIVE;
    item->active = 0;
    item->gravity_status = item->hit_status = item->looked_at = item->really_active = item->ai_bits = item->dynamic_light = 0;
    item->item_flags[0] = item->item_flags[1] = item->item_flags[2] = item->item_flags[3] = 0;
    item->hit_points = objects[item->object_number].hit_points;
    item->collidable = 1;
    item->clear_body = 0;
    item->timer = 0;
    item->mesh_bits = 0xffffffff;
    item->touch_bits = 0;
    item->after_death = 0;
    item->il.init = 0;
    item->fired_weapon = 0;
    item->data = NULL;

    if (item->flags & ONESHOT) {
        item->status = INVISIBLE;
        item->flags -= ONESHOT;
    } else if (objects[item->object_number].intelligent) {
        item->status = INVISIBLE;
    }

    if (item->flags & -0x8000) {
        item->clear_body = 1;
        item->flags -= -0x8000;
    }

    if ((item->flags & CODE_BITS) == CODE_BITS) {
        item->flags -= CODE_BITS;
        item->flags |= REVERSE;
        AddActiveItem(item_num);
        item->status = ACTIVE;
    }

    r = &room[item->room_number];
    item->next_item = r->item_number;
    r->item_number = item_num;
    floor = &r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size];
    item->floor = floor->floor << 8;
    item->box_number = floor->box;

    if (savegame.bonus_flag && !DemoPlay)
        item->hit_points *= 2;

    if (objects[item->object_number].initialise)
        (*objects[item->object_number].initialise)(item_num);
}

void RemoveActiveItem(sint16 item_num)
{
    sint16 linknum;

    if (items[item_num].active == 0)
        return;
    items[item_num].active = 0;
    linknum = next_item_active;
    if (linknum == item_num)
    {
        next_item_active = items[item_num].next_active;
        return;
    }
    for (; linknum != NO_ITEM; linknum = items[linknum].next_active)
    {
        if (items[linknum].next_active == item_num)
        {
            items[linknum].next_active = items[item_num].next_active;
            return;
        }
    }
}

void RemoveDrawnItem(sint16 item_num)
{
    sint16 linknum;
    ITEM_INFO *item;

    item = &items[item_num];
    linknum = room[item->room_number].item_number;
    if (linknum == item_num)
    {
        room[item->room_number].item_number = item->next_item;
    }
    else
    {
        for (; linknum != NO_ITEM; linknum = items[linknum].next_item)
        {
            if (items[linknum].next_item == item_num)
            {
                items[linknum].next_item = item->next_item;
                return;
            }
        }
    }
}

void AddActiveItem(sint16 item_num)
{
    ITEM_INFO *item;

    item = &items[item_num];
    if (objects[item->object_number].control == NULL)
    {
        item->status = NOT_ACTIVE;
        return;
    }
    if (item->active == 1)
        return;
    item->active = 1;
    item->next_active = next_item_active;
    next_item_active = item_num;
}

void ItemNewRoom(sint16 item_num, sint16 room_number)
{
    sint16 linknum;
    ROOM_INFO *r;
    ITEM_INFO *item;

    item = &items[item_num];
    if (item->room_number != NO_ROOM)
    {
        r = &room[item->room_number];
        linknum = r->item_number;
        if (linknum == item_num)
        {
            r->item_number = item->next_item;
        }
        else
        {
            for (; linknum != NO_ITEM; linknum = items[linknum].next_item)
            {
                if (items[linknum].next_item == item_num)
                {
                    items[linknum].next_item = item->next_item;
                    break;
                }
            }
        }
    }
    item->room_number = room_number;
    r = &room[room_number];
    item->next_item = r->item_number;
    r->item_number = item_num;
}

int GlobalItemReplace(int in_objnum, int out_objnum)
{
    ROOM_INFO *r;
    int i, changed;
    sint16 item_num;

    changed = 0;
    r = room;
    for (i = 0; i < number_rooms; i++, r++)
    {
        for (item_num = r->item_number; item_num != NO_ITEM; item_num = items[item_num].next_item)
        {
            if (items[item_num].object_number == in_objnum)
            {
                items[item_num].object_number = out_objnum;
                changed++;
            }
        }
    }
    return changed;
}

void InitialiseFXArray(void)
{
    int i;
    FX_INFO *fx;

    next_fx_active = NO_ITEM;
    next_fx_free = 0;
    fx = effects;
    for (i = 1; i < NUM_EFFECTS; i++, fx++)
        fx->next_fx = i;
    fx->next_fx = NO_ITEM;
}

sint16 CreateEffect(sint16 room_num)
{
    ROOM_INFO *r;
    FX_INFO *fx;
    sint16 fx_num;

    fx_num = next_fx_free;
    if (fx_num == NO_ITEM)
        return fx_num;
    fx = &effects[fx_num];
    next_fx_free = fx->next_fx;
    r = &room[room_num];
    fx->room_number = room_num;
    fx->next_fx = r->fx_number;
    r->fx_number = fx_num;
    fx->next_active = next_fx_active;
    next_fx_active = fx_num;
    fx->shade = 0x4210;
    return fx_num;
}


void KillEffect(sint16 fx_num)
{
    sint16 linknum;
    FX_INFO *fx;

    DetatchSpark(fx_num, 64);
    fx = &effects[fx_num];
    linknum = next_fx_active;
    if (linknum == fx_num) {
        next_fx_active = fx->next_active;
    } else {
        for (; linknum != NO_ITEM; linknum = effects[linknum].next_active) {
            if (effects[linknum].next_active == fx_num) {
                effects[linknum].next_active = fx->next_active;
                break;
            }
        }
    }
    linknum = room[fx->room_number].fx_number;
    if (linknum == fx_num) {
        room[fx->room_number].fx_number = fx->next_fx;
    } else {
        for (; linknum != NO_ITEM; linknum = effects[linknum].next_fx) {
            if (effects[linknum].next_fx == fx_num) {
                effects[linknum].next_fx = fx->next_fx;
                break;
            }
        }
    }
    fx->next_fx = next_fx_free;
    next_fx_free = fx_num;
}

void EffectNewRoom(sint16 fx_num, sint16 room_number)
{
    sint16 linknum;
    ROOM_INFO *r;
    FX_INFO *fx;

    fx = &effects[fx_num];
    r = &room[fx->room_number];
    linknum = r->fx_number;
    if (linknum == fx_num)
    {
        r->fx_number = fx->next_fx;
    }
    else
    {
        for (; linknum != NO_ITEM; linknum = effects[linknum].next_fx)
        {
            if (effects[linknum].next_fx == fx_num)
            {
                effects[linknum].next_fx = fx->next_fx;
                break;
            }
        }
    }
    fx->room_number = room_number;
    r = &room[room_number];
    fx->next_fx = r->fx_number;
    r->fx_number = fx_num;
}

void ClearBodyBag(void)
{
    sint16 item_number;
    ITEM_INFO *item;

    if (body_bag == NO_ITEM)
        return;

    item_number = body_bag;
    while (item_number != NO_ITEM)
    {
        item = &items[item_number];
        KillItem(item_number);
        item_number = item->next_active;
        item->next_active = NO_ITEM;
    }
    body_bag = NO_ITEM;
}
