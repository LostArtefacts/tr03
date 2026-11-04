#include "game.h"

#define MAX_TILT (ONE_DEGREE * 3)
#define MAX_JOINT_CHANGE (ONE_DEGREE * 5)
#define JOINT_ARC 0x3000
#define GUNFIRE_ALERT_RADIUS 8000
#define MODIFY 8
#define ESCAPE_DIST (WALL_L * 5)
#define ZONE(A) (((A) >> (WALL_SHIFT - 2)) - 1)

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

extern sint16 *ground_zone[4][2];

int StalkBox(ITEM_INFO *item, ITEM_INFO *enemy, sint16 box_number);
sint16 SameZone(CREATURE_INFO *creature, ITEM_INFO *target_item);
void GetAITarget(CREATURE_INFO *creature);
sint16 CreatureTurn(ITEM_INFO *item, sint16 maximum_turn);

int ValidBox(ITEM_INFO *item, sint16 zone_number, sint16 box_number)
{
    BOX_INFO *box;
    CREATURE_INFO *creature;
    sint16 *zone;

    creature = (CREATURE_INFO *)item->data;
    if (creature->LOT.fly == NO_FLYING)
        zone = ground_zone[ZONE(creature->LOT.step)][flip_status];
    else
        zone = ground_zone[ZONE(1)][0];
    if (creature->LOT.fly == NO_FLYING && zone[box_number] != zone_number)
        return 0;
    box = &boxes[box_number];
    if (box->overlap_index & creature->LOT.block_mask)
        return 0;
    if (item->pos.z_pos > ((sint32)box->left << WALL_SHIFT) && item->pos.z_pos < ((sint32)box->right << WALL_SHIFT) && item->pos.x_pos > ((sint32)box->top << WALL_SHIFT) && item->pos.x_pos < ((sint32)box->bottom << WALL_SHIFT))
        return 0;
    return 1;
}

int StalkBox(ITEM_INFO *item, ITEM_INFO *enemy, sint16 box_number)
{
    BOX_INFO *box;
    int baddie_quad, box_quad, enemy_quad;
    sint32 x, z, xrange, zrange;

    if (enemy == NULL)
        return 0;

    box = &boxes[box_number];
    z = (((uint32)box->left + (uint32)box->right) << (WALL_SHIFT - 1)) - enemy->pos.z_pos;
    x = (((uint32)box->top + (uint32)box->bottom) << (WALL_SHIFT - 1)) - enemy->pos.x_pos;
    zrange = 3072 + (((uint32)box->right - (uint32)box->left) << WALL_SHIFT);
    xrange = 3072 + (((uint32)box->bottom - (uint32)box->top) << WALL_SHIFT);
    if (x > xrange || x < -xrange || z > zrange || z < -zrange)
        return 0;

    enemy_quad = (enemy->pos.y_rot >> 14) + 2;
    if (z > 0)
        box_quad = (x > 0) ? 2 : 1;
    else
        box_quad = (x > 0) ? 3 : 0;

    if (enemy_quad == box_quad)
        return 0;

    if (item->pos.z_pos > enemy->pos.z_pos)
        baddie_quad = (item->pos.x_pos > enemy->pos.x_pos) ? 2 : 1;
    else
        baddie_quad = (item->pos.x_pos > enemy->pos.x_pos) ? 3 : 0;

    if (enemy_quad == baddie_quad && abs(enemy_quad - box_quad) == 2)
        return 0;

    return 1;
}

int CreatureCreature(sint16 item_number)
{
    sint16 link, angle;
    sint32 x, y, z, y_rot, xdistance, zdistance, distance, radius;
    ROOM_INFO *r;
    ITEM_INFO *item;

    item = &items[item_number];
    x = item->pos.x_pos;
    y = item->pos.y_pos;
    z = item->pos.z_pos;
    y_rot = item->pos.y_rot;
    radius = objects[item->object_number].radius;

    r = &room[item->room_number];
    link = r->item_number;
    do {
        item = &items[link];
        if (link == item_number)
            return 0;

        if (item != lara_item && item->status == ACTIVE && item->hit_points > 0) {
            xdistance = abs(item->pos.x_pos - x);
            zdistance = abs(item->pos.z_pos - z);
            if (xdistance > zdistance)
                distance = xdistance + (zdistance >> 1);
            else
                distance = zdistance + (xdistance >> 1);

            if (distance < radius + objects[item->object_number].radius) {
                angle = phd_atan(item->pos.z_pos - z, item->pos.x_pos - x) - y_rot;
                return angle;
            }
        }
        link = item->next_item;
    } while (link != NO_ITEM);

    return 0;
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

extern sint16 body_bag;
int ExplodingDeath(sint16 item_number, sint32 mesh_bits, sint16 damage);

void CreatureDie(sint16 item_number, int explode)
{
    sint16 pickup_number, room_number;
    ITEM_INFO *item, *pickup;
    FLOOR_INFO *floor;

    item = &items[item_number];
    item->collidable = 0;
    item->hit_points = DONT_TARGET;
    if (explode) {
        ExplodingDeath(item_number, 0xffffffff, 0);
        KillItem(item_number);
    } else {
        RemoveActiveItem(item_number);
    }
    DisableBaddieAI(item_number);
    item->flags |= ONESHOT;

    if (item->clear_body) {
        item->next_active = body_bag;
        body_bag = item_number;
    }

    pickup_number = item->carried_item;
    while (pickup_number != NO_ITEM) {
        pickup = &items[pickup_number];
        pickup->pos.x_pos = (item->pos.x_pos & ~1023) | 512;
        pickup->pos.z_pos = (item->pos.z_pos & ~1023) | 512;
        room_number = item->room_number;
        floor = GetFloor(pickup->pos.x_pos, item->pos.y_pos, pickup->pos.z_pos, &room_number);
        pickup->pos.y_pos = GetHeight(floor, pickup->pos.x_pos, item->pos.y_pos, pickup->pos.z_pos);
        ItemNewRoom(pickup_number, item->room_number);
        pickup_number = pickup->carried_item;
    }
}

sint16 CreatureTurn(ITEM_INFO *item, sint16 maximum_turn)
{
    sint32 x, z, range;
    sint16 angle;
    CREATURE_INFO *creature;

    if (item->data == NULL)
        return 0;
    if (maximum_turn == 0)
        return 0;

    creature = (CREATURE_INFO *)item->data;
    z = creature->target.z - item->pos.z_pos;
    x = creature->target.x - item->pos.x_pos;
    angle = phd_atan(z, x) - item->pos.y_rot;
    range = (item->speed << 14) / maximum_turn;
    if ((angle > 0x4000 || angle < -0x4000) && x * x + z * z < range * range)
        maximum_turn >>= 1;

    if (angle > maximum_turn)
        angle = maximum_turn;
    else if (angle < -maximum_turn)
        angle = -maximum_turn;

    item->pos.y_rot += angle;
    return angle;
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

void CreatureFloat(sint16 item_number)
{
    sint32 water_level;
    ITEM_INFO *item;
    FLOOR_INFO *floor;
    sint16 room_number;

    item = &items[item_number];
    item->hit_points = DONT_TARGET;
    item->pos.x_rot = 0;
    water_level = GetWaterHeight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);
    if (item->pos.y_pos > water_level)
        item->pos.y_pos -= WALL_L / 32;
    if (item->pos.y_pos < water_level)
        item->pos.y_pos = water_level;
    AnimateItem(item);
    room_number = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    item->floor = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    if (room_number != item->room_number)
        ItemNewRoom(item_number, room_number);
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

int CreatureAnimation(sint16 item_number, sint16 angle, sint16 tilt);

int CreatureVault(sint16 item_number, sint16 angle, int vault, int shift)
{
    ITEM_INFO *item;
    sint32 y, xx, yy, x_floor, y_floor;
    sint16 room_number;

    item = &items[item_number];
    xx = item->pos.z_pos >> WALL_SHIFT;
    yy = item->pos.x_pos >> WALL_SHIFT;
    y = item->pos.y_pos;
    room_number = item->room_number;

    CreatureAnimation(item_number, angle, 0);

    if (item->floor > y + STEP_L * 7 / 2)
        vault = -4;
    else if (item->floor > y + STEP_L * 5 / 2 && item->object_number == MONKEY)
        vault = -3;
    else if (item->floor > y + STEP_L * 3 / 2 && item->object_number == MONKEY)
        vault = -2;
    else if (item->pos.y_pos > y - STEP_L * 3 / 2)
        return 0;
    else if (item->pos.y_pos > y - STEP_L * 5 / 2)
        vault = 2;
    else if (item->pos.y_pos > y - STEP_L * 7 / 2)
        vault = 3;
    else
        vault = 4;

    x_floor = item->pos.z_pos >> WALL_SHIFT;
    y_floor = item->pos.x_pos >> WALL_SHIFT;
    if (xx == x_floor) {
        if (yy == y_floor)
            return 0;

        if (yy < y_floor) {
            item->pos.x_pos = (y_floor << WALL_SHIFT) - shift;
            item->pos.y_rot = 0x4000;
        } else {
            item->pos.x_pos = (yy << WALL_SHIFT) + shift;
            item->pos.y_rot = -0x4000;
        }
    } else if (yy == y_floor) {
        if (xx < x_floor) {
            item->pos.z_pos = (x_floor << WALL_SHIFT) - shift;
            item->pos.y_rot = 0;
        } else {
            item->pos.z_pos = (xx << WALL_SHIFT) + shift;
            item->pos.y_rot = -0x8000;
        }
    }

    item->pos.y_pos = item->floor = y;
    if (room_number != item->room_number)
        ItemNewRoom(item_number, room_number);

    return vault;
}

void CreatureKill(ITEM_INFO *item, int kill_anim, int kill_state, int lara_kill_state)
{
    item->anim_number = objects[item->object_number].anim_index + kill_anim;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = kill_state;

    lara_item->anim_number = objects[13].anim_index;
    lara_item->frame_number = anims[lara_item->anim_number].frame_base;
    lara_item->current_anim_state = 0;
    lara_item->goal_anim_state = lara_kill_state;
    lara_item->pos.x_pos = item->pos.x_pos;
    lara_item->pos.y_pos = item->pos.y_pos;
    lara_item->pos.z_pos = item->pos.z_pos;
    lara_item->pos.y_rot = item->pos.y_rot;
    lara_item->pos.x_rot = item->pos.x_rot;
    lara_item->pos.z_rot = item->pos.z_rot;
    lara_item->fallspeed = 0;
    lara_item->gravity_status = 0;
    lara_item->speed = 0;

    if (item->room_number != lara_item->room_number)
        ItemNewRoom(lara.item_number, item->room_number);

    AnimateItem(lara_item);

    lara.extra_anim = 1;
    lara.gun_status = LG_HANDSBUSY;
    lara.gun_type = LG_UNARMED;
    lara.hit_direction = -1;
    lara.air = -1;

    camera.pos.room_number = lara_item->room_number;
    camera.type = CHASE_CAMERA;
    camera.flags = FOLLOW_CENTRE;
    camera.target_angle = 170 * ONE_DEGREE;
    camera.target_elevation = -25 * ONE_DEGREE;
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

sint16 SameZone(CREATURE_INFO *creature, ITEM_INFO *target_item)
{
    ROOM_INFO *r;
    sint16 *zone, zone_number, target_zone;
    ITEM_INFO *item;

    if (creature->LOT.fly == NO_FLYING)
        zone = ground_zone[ZONE(creature->LOT.step)][flip_status];
    else
        return 1;

    item = &items[creature->item_num];
    r = &room[item->room_number];
    item->box_number = r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size].box;
    zone_number = zone[item->box_number];

    r = &room[target_item->room_number];
    target_item->box_number = r->floor[((target_item->pos.z_pos - r->z) >> WALL_SHIFT) + ((target_item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size].box;
    target_zone = zone[target_item->box_number];

    if (zone_number == target_zone)
        return 1;
    else
        return 0;
}

void AdjustStopperFlag(ITEM_INFO *item, sint32 dir, sint32 set)
{
    ROOM_INFO *r;
    sint16 room_number;
    sint32 x, z;

    x = item->pos.x_pos;
    z = item->pos.z_pos;
    r = &room[item->room_number];
    r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size].stopper = set;

    x = item->pos.x_pos + ((1024 * phd_sin(dir)) >> W2V_SHIFT);
    z = item->pos.z_pos + ((1024 * phd_cos(dir)) >> W2V_SHIFT);
    room_number = item->room_number;
    GetFloor(x, item->pos.y_pos, z, &room_number);
    r = &room[room_number];
    r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size].stopper = set;
}
