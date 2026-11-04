#include "game.h"

#define MAX_TILT (ONE_DEGREE * 3)
#define MAX_JOINT_CHANGE (ONE_DEGREE * 5)
#define JOINT_ARC 0x3000
#define GUNFIRE_ALERT_RADIUS 8000
#define MODIFY 8
#define ESCAPE_DIST (WALL_L * 5)

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

void TargetBox(LOT_INFO *LOT, sint16 box_number)
{
    BOX_INFO *box;

    box_number &= BOX_NUMBER;
    box = &boxes[box_number];
    LOT->target.z = ((uint32)box->left << WALL_SHIFT) + GetRandomControl() * (((uint32)box->right - (uint32)box->left - 1) >> (15 - WALL_SHIFT)) + WALL_L / 2;
    LOT->target.x = ((uint32)box->top << WALL_SHIFT) + GetRandomControl() * (((uint32)box->bottom - (uint32)box->top - 1) >> (15 - WALL_SHIFT)) + WALL_L / 2;
    LOT->required_box = box_number;
    if (LOT->fly == NO_FLYING)
        LOT->target.y = box->height;
    else
        LOT->target.y = box->height - STEP_L * 3 / 2;
}

int EscapeBox(ITEM_INFO *item, ITEM_INFO *enemy, sint16 box_number)
{
    BOX_INFO *box;
    sint32 x, z;

    box = &boxes[box_number];
    z = (((uint32)box->left + (uint32)box->right) << (WALL_SHIFT - 1)) - enemy->pos.z_pos;
    x = (((uint32)box->top + (uint32)box->bottom) << (WALL_SHIFT - 1)) - enemy->pos.x_pos;
    if (x > -ESCAPE_DIST && x < ESCAPE_DIST && z > -ESCAPE_DIST && z < ESCAPE_DIST)
        return 0;
    if (((z > 0) ^ (item->pos.z_pos > enemy->pos.z_pos)) && ((x > 0) ^ (item->pos.x_pos > enemy->pos.x_pos)))
        return 0;
    return 1;
}

int BadFloor(sint32 x, sint32 y, sint32 z, sint32 box_height, sint32 next_height, sint16 room_number, LOT_INFO *LOT)
{
    sint32 height;
    FLOOR_INFO *floor;

    floor = GetFloor(x, y, z, &room_number);
    if (floor->box == NO_BOX)
        return 1;
    if (boxes[floor->box].overlap_index & LOT->block_mask)
        return 1;
    height = boxes[floor->box].height;
    if (box_height - height > LOT->step || box_height - height < LOT->drop)
        return 1;
    if (box_height - height < -LOT->step && height > next_height)
        return 1;
    if (LOT->fly != NO_FLYING && y > height + LOT->fly)
        return 1;
    return 0;
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

void AlertNearbyGuards(ITEM_INFO *item)
{
    int slot;
    CREATURE_INFO *cinfo;
    ITEM_INFO *target;
    sint32 x, y, z, distance;

    cinfo = baddie_slots;
    for (slot = 0; slot < NUM_SLOTS; slot++, cinfo++)
    {
        if (cinfo->item_num == NO_ITEM)
            continue;
        target = &items[cinfo->item_num];
        if (target->room_number == item->room_number)
        {
            cinfo->alerted = 1;
            continue;
        }
        x = (target->pos.x_pos - item->pos.x_pos) >> 6;
        y = (target->pos.y_pos - item->pos.y_pos) >> 6;
        z = (target->pos.z_pos - item->pos.z_pos) >> 6;
        distance = x * x + y * y + z * z;
        if (distance < GUNFIRE_ALERT_RADIUS)
            cinfo->alerted = 1;
    }
}

sint16 AIGuard(CREATURE_INFO *creature)
{
    int random;

    if (items[creature->item_num].ai_bits & MODIFY)
        return 0;
    random = GetRandomControl();
    if (random < 0x100)
    {
        creature->head_left = 1;
        creature->head_right = 1;
    }
    else if (random < 0x180)
    {
        creature->head_left = 1;
        creature->head_right = 0;
    }
    else if (random < 0x200)
    {
        creature->head_left = 0;
        creature->head_right = 1;
    }
    if (creature->head_left && creature->head_right)
        return 0;
    else if (creature->head_left)
        return -0x4000;
    else if (creature->head_right)
        return 0x4000;
    return 0;
}
