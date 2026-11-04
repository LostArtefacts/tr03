#include "game.h"

#define MAX_TILT (ONE_DEGREE * 3)
#define MAX_JOINT_CHANGE (ONE_DEGREE * 5)
#define JOINT_ARC 0x3000

int SearchLOT(LOT_INFO *LOT, int expansion);
sint32 GetWaterHeight(sint32 x, sint32 y, sint32 z, sint16 room_number);

void InitialiseCreature(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    if (item->object_number != ELECTRIC_CLEANER && item->object_number != SHIVA && item->object_number != TARGETS)
        item->pos.y_rot += (GetRandomControl() - 0x4000) >> 1;
    item->collidable = 1;
    item->data = NULL;
}

int CreatureActive(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    if (item->status == INVISIBLE)
    {
        if (!EnableBaddieAI(item_number, 0))
            return 0;
        item->status = ACTIVE;
    }
    return item->data != NULL;
}

int UpdateLOT(LOT_INFO *LOT, int expansion)
{
    BOX_NODE *expand;

    if (LOT->required_box != NO_BOX && LOT->required_box != LOT->target_box)
    {
        LOT->target_box = LOT->required_box;
        expand = &LOT->node[LOT->target_box];
        if (expand->next_expansion == NO_BOX && LOT->tail != LOT->target_box)
        {
            expand->next_expansion = LOT->head;
            if (LOT->head == NO_BOX)
                LOT->tail = LOT->target_box;
            LOT->head = LOT->target_box;
        }
        expand->search_number = ++LOT->search_number;
        expand->exit_box = NO_BOX;
    }
    return SearchLOT(LOT, expansion);
}

void CreatureTilt(ITEM_INFO *item, sint16 angle)
{
    angle = (angle << 2) - item->pos.z_rot;
    if (angle < -MAX_TILT)
        angle = -MAX_TILT;
    else if (angle > MAX_TILT)
        angle = MAX_TILT;
    item->pos.z_rot += angle;
}

void CreatureJoint(ITEM_INFO *item, sint16 joint, sint16 required)
{
    sint16 change;
    CREATURE_INFO *creature;

    if (item->data == NULL)
        return;

    creature = (CREATURE_INFO *)item->data;
    change = required - creature->joint_rotation[joint];
    if (change > MAX_JOINT_CHANGE)
        change = MAX_JOINT_CHANGE;
    else if (change < -MAX_JOINT_CHANGE)
        change = -MAX_JOINT_CHANGE;
    creature->joint_rotation[joint] += change;
    if (creature->joint_rotation[joint] > JOINT_ARC)
        creature->joint_rotation[joint] = JOINT_ARC;
    else if (creature->joint_rotation[joint] < -JOINT_ARC)
        creature->joint_rotation[joint] = -JOINT_ARC;
}

void CreatureUnderwater(ITEM_INFO *item, sint32 depth)
{
    sint32 water_level;

    water_level = GetWaterHeight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);
    if (item->pos.y_pos < water_level + depth)
    {
        item->pos.y_pos = water_level + depth;
        if (item->pos.x_rot > 2 * ONE_DEGREE)
            item->pos.x_rot -= 2 * ONE_DEGREE;
        else if (item->pos.x_rot > 0)
            item->pos.x_rot = 0;
    }
}

sint16 CreatureEffect(ITEM_INFO *item, BITE_INFO *bite, sint16 (*generate)(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 yrot, sint16 room_number))
{
    PHD_VECTOR pos;

    pos.x = bite->x;
    pos.y = bite->y;
    pos.z = bite->z;
    GetJointAbsPosition(item, &pos, bite->mesh_num);
    return (*generate)(pos.x, pos.y, pos.z, item->speed, item->pos.y_rot, item->room_number);
}

void AlertAllGuards(sint16 item_number)
{
    int slot;
    CREATURE_INFO *cinfo;
    ITEM_INFO *target;
    sint16 obj_number;

    cinfo = baddie_slots;
    obj_number = items[item_number].object_number;
    for (slot = 0; slot < NUM_SLOTS; slot++, cinfo++)
    {
        if (cinfo->item_num == NO_ITEM)
            continue;
        target = &items[cinfo->item_num];
        if (obj_number == target->object_number && target->status == ACTIVE)
            cinfo->alerted = 1;
    }
}
