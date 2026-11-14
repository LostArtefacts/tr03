#include <process.h>
#include <float.h>
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

typedef struct {
    sint16 zone_number;
    sint16 enemy_zone;
    sint32 distance;
    sint32 ahead;
    sint32 bite;
    sint16 angle;
    sint16 x_angle;
    sint16 enemy_facing;
} AI_INFO;

sint16 *GetBestFrame(ITEM_INFO *item);
int CalculateTarget(PHD_VECTOR *target, ITEM_INFO *item, LOT_INFO *LOT);

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



void GetCreatureMood(ITEM_INFO *item, AI_INFO *info, int violent)
{
    CREATURE_INFO *creature;
    MOOD_TYPE mood;
    LOT_INFO *LOT;
    ITEM_INFO *enemy;

    if (item->data == NULL)
        return;

    creature = (CREATURE_INFO *)item->data;
    enemy = creature->enemy;
    LOT = &creature->LOT;

    if (LOT->node[item->box_number].search_number == (LOT->search_number | 0x8000))
        LOT->required_box = NO_BOX;

    if (creature->mood != ATTACK_MOOD && LOT->required_box != NO_BOX) {
        if (!ValidBox(item, info->zone_number, LOT->target_box)) {
            if (info->zone_number == info->enemy_zone)
                creature->mood = BORED_MOOD;
            LOT->required_box = NO_BOX;
        }
    }

    mood = creature->mood;

    if (!enemy) {
        creature->mood = BORED_MOOD;
        enemy = lara_item;
    } else if (enemy->hit_points <= 0 && enemy == lara_item) {
        creature->mood = BORED_MOOD;
    } else if (violent) {
        switch (creature->mood) {
        case ATTACK_MOOD:
            if (info->zone_number != info->enemy_zone)
                creature->mood = BORED_MOOD;
            break;

        case BORED_MOOD:
        case STALK_MOOD:
            if (info->zone_number == info->enemy_zone)
                creature->mood = ATTACK_MOOD;
            else if (item->hit_status)
                creature->mood = ESCAPE_MOOD;
            break;

        case ESCAPE_MOOD:
            if (info->zone_number == info->enemy_zone)
                creature->mood = ATTACK_MOOD;
            break;
        }
    } else {
        switch (creature->mood) {
        case ATTACK_MOOD:
            if (item->hit_status && (GetRandomControl() < 0x800 || info->zone_number != info->enemy_zone))
                creature->mood = STALK_MOOD;
            else if (info->zone_number != info->enemy_zone && info->distance > WALL_L * 6)
                creature->mood = BORED_MOOD;
            break;

        case BORED_MOOD:
        case STALK_MOOD:
            if (creature->alerted && info->zone_number != info->enemy_zone) {
                if (info->distance > WALL_L * 3)
                    creature->mood = STALK_MOOD;
                else
                    creature->mood = BORED_MOOD;
            } else if (info->zone_number == info->enemy_zone) {
                if (info->distance < (WALL_L * 3) * (WALL_L * 3) || (creature->mood == STALK_MOOD && LOT->required_box == NO_BOX))
                    creature->mood = ATTACK_MOOD;
                else
                    creature->mood = STALK_MOOD;
            }
            break;

        case ESCAPE_MOOD:
            if (info->zone_number == info->enemy_zone && GetRandomControl() < 0x100)
                creature->mood = STALK_MOOD;
            break;
        }
    }

    if (mood != creature->mood) {
        if (mood == ATTACK_MOOD)
            TargetBox(LOT, LOT->target_box);
        LOT->required_box = NO_BOX;
    }
}

void CreatureMood(ITEM_INFO *item, AI_INFO *info, int violent)
{
    sint16 box_number;
    CREATURE_INFO *creature;
    LOT_INFO *LOT;
    OBJECT_INFO *object;
    ITEM_INFO *enemy;
    sint16 *bounds;

    if (item->data == NULL)
        return;

    creature = (CREATURE_INFO *)item->data;
    enemy = creature->enemy;
    LOT = &creature->LOT;

    switch (creature->mood) {
    case ATTACK_MOOD:
        LOT->target.x = enemy->pos.x_pos;
        LOT->target.y = enemy->pos.y_pos;
        LOT->target.z = enemy->pos.z_pos;
        LOT->required_box = enemy->box_number;
        if (LOT->fly != NO_FLYING) {
            object = &objects[enemy->object_number];
            if (lara.water_status == LARA_ABOVEWATER) {
                bounds = GetBestFrame(enemy);
                LOT->target.y += bounds[2];
            }
        }
        break;

    case BORED_MOOD:
        box_number = LOT->node[GetRandomControl() * LOT->zone_count >> 15].box_number;
        if (ValidBox(item, info->zone_number, box_number)) {
            if (StalkBox(item, enemy, box_number) && enemy->hit_points > 0 && creature->enemy) {
                TargetBox(LOT, box_number);
                creature->mood = BORED_MOOD;
            } else if (LOT->required_box == NO_BOX) {
                TargetBox(LOT, box_number);
            }
        }
        break;

    case STALK_MOOD:
        if (LOT->required_box == NO_BOX || !StalkBox(item, enemy, LOT->required_box)) {
            box_number = LOT->node[GetRandomControl() * LOT->zone_count >> 15].box_number;
            if (ValidBox(item, info->zone_number, box_number)) {
                if (StalkBox(item, enemy, box_number)) {
                    TargetBox(LOT, box_number);
                } else if (LOT->required_box == NO_BOX) {
                    TargetBox(LOT, box_number);
                    if (info->zone_number != info->enemy_zone)
                        creature->mood = BORED_MOOD;
                }
            }
        }
        break;

    case ESCAPE_MOOD:
        box_number = LOT->node[GetRandomControl() * LOT->zone_count >> 15].box_number;
        if (ValidBox(item, info->zone_number, box_number) && LOT->required_box == NO_BOX) {
            if (EscapeBox(item, enemy, box_number)) {
                TargetBox(LOT, box_number);
            } else if (info->zone_number == info->enemy_zone && StalkBox(item, enemy, box_number) && !violent) {
                TargetBox(LOT, box_number);
                creature->mood = STALK_MOOD;
            }
        }
        break;
    }

    if (LOT->target_box == NO_BOX)
        TargetBox(LOT, item->box_number);

    CalculateTarget(&creature->target, item, &creature->LOT);
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

extern uint16 *overlap;

int SearchLOT(LOT_INFO *LOT, int expansion)
{
    int i, index, done, change, box_number;
    BOX_NODE *expand, *node;
    BOX_INFO *box;
    sint16 *zone, search_zone;

    if (LOT->fly == NO_FLYING) {
        zone = ground_zone[ZONE(LOT->step)][flip_status];
        search_zone = zone[LOT->head];
    } else {
        zone = ground_zone[ZONE(1)][0];
        search_zone = 0x2000;
    }
    for (i = 0; i < expansion; i++) {
        if (LOT->head == NO_BOX) {
            LOT->tail = NO_BOX;
            return 0;
        }
        node = &LOT->node[LOT->head];
        box = &boxes[LOT->head];
        index = box->overlap_index & 0x3fff;
        done = 0;
        do {
            box_number = overlap[index++];
            if (box_number & 0x8000) {
                done = 1;
                box_number &= BOX_NUMBER;
            }
            if (LOT->fly == NO_FLYING && search_zone != zone[box_number])
                continue;
            change = boxes[box_number].height - box->height;
            if (change > LOT->step || change < LOT->drop)
                continue;
            expand = &LOT->node[box_number];
            if ((node->search_number & 0x7fff) < (expand->search_number & 0x7fff))
                continue;
            if (node->search_number & 0x8000) {
                if ((node->search_number & 0x7fff) == (expand->search_number & 0x7fff))
                    continue;
                expand->search_number = node->search_number;
            } else {
                if ((node->search_number & 0x7fff) == (expand->search_number & 0x7fff) && !(expand->search_number & 0x8000))
                    continue;
                if (boxes[box_number].overlap_index & LOT->block_mask)
                    expand->search_number = node->search_number | 0x8000;
                else {
                    expand->search_number = node->search_number;
                    expand->exit_box = LOT->head;
                }
            }
            if (expand->next_expansion == NO_BOX && box_number != LOT->tail) {
                LOT->node[LOT->tail].next_expansion = box_number;
                LOT->tail = box_number;
            }
        } while (!done);
        LOT->head = node->next_expansion;
        node->next_expansion = NO_BOX;
    }
    return 1;
}

int CalculateTarget(PHD_VECTOR *target, ITEM_INFO *item, LOT_INFO *LOT)
{
    int box_number, prime_free;
    BOX_INFO *box;
    sint32 left, right, top, bottom;
    sint32 box_left, box_right, box_top, box_bottom;

    UpdateLOT(LOT, 5);
    target->x = item->pos.x_pos;
    target->y = item->pos.y_pos;
    target->z = item->pos.z_pos;
    box_number = item->box_number;
    if (box_number == NO_BOX)
        return 0;

    box = &boxes[box_number];
    box_left = (sint32)box->left << WALL_SHIFT;
    box_right = ((sint32)box->right << WALL_SHIFT) - 1;
    box_top = (sint32)box->top << WALL_SHIFT;
    box_bottom = ((sint32)box->bottom << WALL_SHIFT) - 1;
    left = box_left;
    right = box_right;
    top = box_top;
    bottom = box_bottom;
    prime_free = 15;
    do {
        box = &boxes[box_number];
        if (LOT->fly == NO_FLYING) {
            if (target->y > box->height)
                target->y = box->height;
        } else {
            if (target->y > box->height - WALL_L)
                target->y = box->height - WALL_L;
        }
        box_left = (sint32)box->left << WALL_SHIFT;
        box_right = ((sint32)box->right << WALL_SHIFT) - 1;
        box_top = (sint32)box->top << WALL_SHIFT;
        box_bottom = ((sint32)box->bottom << WALL_SHIFT) - 1;
        if (item->pos.z_pos >= box_left && item->pos.z_pos <= box_right && item->pos.x_pos >= box_top && item->pos.x_pos <= box_bottom) {
            left = box_left;
            right = box_right;
            top = box_top;
            bottom = box_bottom;
        } else {
            if (item->pos.z_pos < box_left) {
                if ((prime_free & 1) && item->pos.x_pos >= box_top && item->pos.x_pos <= box_bottom) {
                    if (target->z < box_left + 512)
                        target->z = box_left + 512;
                    if (prime_free & 16)
                        return 2;
                    if (box_top > top)
                        top = box_top;
                    if (box_bottom < bottom)
                        bottom = box_bottom;
                    prime_free = 1;
                } else if (prime_free != 1) {
                    target->z = right - 512;
                    if (prime_free != 15)
                        return 2;
                    prime_free |= 16;
                }
            } else if (item->pos.z_pos > box_right) {
                if ((prime_free & 2) && item->pos.x_pos >= box_top && item->pos.x_pos <= box_bottom) {
                    if (target->z > box_right - 512)
                        target->z = box_right - 512;
                    if (prime_free & 16)
                        return 2;
                    if (box_top > top)
                        top = box_top;
                    if (box_bottom < bottom)
                        bottom = box_bottom;
                    prime_free = 2;
                } else if (prime_free != 2) {
                    target->z = left + 512;
                    if (prime_free != 15)
                        return 2;
                    prime_free |= 16;
                }
            }

            if (item->pos.x_pos < box_top) {
                if ((prime_free & 4) && item->pos.z_pos >= box_left && item->pos.z_pos <= box_right) {
                    if (target->x < box_top + 512)
                        target->x = box_top + 512;
                    if (prime_free & 16)
                        return 2;
                    if (box_left > left)
                        left = box_left;
                    if (box_right < right)
                        right = box_right;
                    prime_free = 4;
                } else if (prime_free != 4) {
                    target->x = bottom - 512;
                    if (prime_free != 15)
                        return 2;
                    prime_free |= 16;
                }
            } else if (item->pos.x_pos > box_bottom) {
                if ((prime_free & 8) && item->pos.z_pos >= box_left && item->pos.z_pos <= box_right) {
                    if (target->x > box_bottom - 512)
                        target->x = box_bottom - 512;
                    if (prime_free & 16)
                        return 2;
                    if (box_left > left)
                        left = box_left;
                    if (box_right < right)
                        right = box_right;
                    prime_free = 8;
                } else if (prime_free != 8) {
                    target->x = top + 512;
                    if (prime_free != 15)
                        return 2;
                    prime_free |= 16;
                }
            }
        }

        if (box_number == LOT->target_box) {
            if (prime_free & (1 | 2))
                target->z = LOT->target.z;
            else if (!(prime_free & 16)) {
                if (target->z < box_left + 512)
                    target->z = box_left + 512;
                else if (target->z > box_right - 512)
                    target->z = box_right - 512;
            }
            if (prime_free & (4 | 8))
                target->x = LOT->target.x;
            else if (!(prime_free & 16)) {
                if (target->x < box_top + 512)
                    target->x = box_top + 512;
                else if (target->x > box_bottom - 512)
                    target->x = box_bottom - 512;
            }
            target->y = LOT->target.y;
            return 1;
        }

        box_number = LOT->node[box_number].exit_box;
        if (box_number != NO_BOX && (boxes[box_number].overlap_index & LOT->block_mask))
            break;
    } while (box_number != NO_BOX);

    if (prime_free & (1 | 2))
        target->z = box_left + WALL_L / 2 + (GetRandomControl() * (box_right - box_left - WALL_L) >> 15);
    else if (!(prime_free & 16)) {
        if (target->z < box_left + 512)
            target->z = box_left + 512;
        else if (target->z > box_right - 512)
            target->z = box_right - 512;
    }
    if (prime_free & (4 | 8))
        target->x = box_top + WALL_L / 2 + (GetRandomControl() * (box_bottom - box_top - WALL_L) >> 15);
    else if (!(prime_free & 16)) {
        if (target->x < box_top + 512)
            target->x = box_top + 512;
        else if (target->x > box_bottom - 512)
            target->x = box_bottom - 512;
    }

    if (LOT->fly == NO_FLYING)
        target->y = box->height;
    else
        target->y = box->height - STEP_L * 3 / 2;
    return 0;
}

int abs(int x);
#pragma intrinsic(abs)

void CreatureAIInfo(ITEM_INFO *item, AI_INFO *info)
{
    ROOM_INFO *r;
    sint32 x, y, z;
    sint16 angle, lara_anim, *zone;
    CREATURE_INFO *creature;
    OBJECT_INFO *object;
    ITEM_INFO *enemy;

    if (item->data == NULL)
        return;
    creature = (CREATURE_INFO *)item->data;
    enemy = creature->enemy;
    if (!enemy) {
        enemy = lara_item;
        creature->enemy = lara_item;
    }
    if (creature->LOT.fly == NO_FLYING)
        zone = ground_zone[ZONE(creature->LOT.step)][flip_status];
    else
        zone = ground_zone[ZONE(1)][0];
    r = &room[item->room_number];
    item->box_number = r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size].box;
    if (creature->LOT.fly == NO_FLYING)
        info->zone_number = zone[item->box_number];
    else
        info->zone_number = 0x2000;
    r = &room[enemy->room_number];
    enemy->box_number = r->floor[((enemy->pos.z_pos - r->z) >> WALL_SHIFT) + ((enemy->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size].box;
    if (creature->LOT.fly == NO_FLYING)
        info->enemy_zone = zone[enemy->box_number];
    else
        info->enemy_zone = 0x2000;
    if (!objects[item->object_number].non_lot) {
        if (boxes[enemy->box_number].overlap_index & creature->LOT.block_mask)
            info->enemy_zone |= BLOCKED;
        else if (creature->LOT.node[item->box_number].search_number == (creature->LOT.search_number | 0x8000))
            info->enemy_zone |= BLOCKED;
    }
    object = &objects[item->object_number];
    if (enemy == lara_item) {
        z = (enemy->pos.z_pos + (enemy->speed * 14 * phd_cos(lara.move_angle) >> W2V_SHIFT)) - (item->pos.z_pos + (object->pivot_length * phd_cos(item->pos.y_rot) >> W2V_SHIFT));
        x = (enemy->pos.x_pos + (enemy->speed * 14 * phd_sin(lara.move_angle) >> W2V_SHIFT)) - (item->pos.x_pos + (object->pivot_length * phd_sin(item->pos.y_rot) >> W2V_SHIFT));
    } else {
        z = (enemy->pos.z_pos + (enemy->speed * 14 * phd_cos(enemy->pos.y_rot) >> W2V_SHIFT)) - (item->pos.z_pos + (object->pivot_length * phd_cos(item->pos.y_rot) >> W2V_SHIFT));
        x = (enemy->pos.x_pos + (enemy->speed * 14 * phd_sin(enemy->pos.y_rot) >> W2V_SHIFT)) - (item->pos.x_pos + (object->pivot_length * phd_sin(item->pos.y_rot) >> W2V_SHIFT));
    }
    y = item->pos.y_pos - enemy->pos.y_pos;
    angle = phd_atan(z, x);
    if (z > 32000 || z < -32000 || x > 32000 || x < -32000)
        info->distance = 0x7fffffff;
    else if (creature->enemy)
        info->distance = z * z + x * x;
    else
        info->distance = 0x7fffffff;
    info->angle = angle - item->pos.y_rot;
    info->enemy_facing = 0x8000 + angle - enemy->pos.y_rot;
    z = abs(z);
    x = abs(x);
    if (enemy == lara_item) {
        lara_anim = lara_item->current_anim_state;
        if (lara_anim == AS_DUCK || lara_anim == AS_DUCKROLL || lara_anim == AS_ALL4S || lara_anim == AS_CRAWL || lara_anim == AS_ALL4TURNL || lara_anim == AS_ALL4TURNR)
            y -= STEP_L * 3 / 2;
    }
    if (x > z)
        info->x_angle = phd_atan(x + (z >> 1), y);
    else
        info->x_angle = phd_atan(z + (x >> 1), y);
    info->ahead = (info->angle > -0x4000 && info->angle < 0x4000);
    info->bite = (info->ahead && enemy->hit_points > 0 && (enemy->pos.y_pos - item->pos.y_pos < 0 ? -(enemy->pos.y_pos - item->pos.y_pos) : enemy->pos.y_pos - item->pos.y_pos) <= STEP_L * 2);
}

#define KILLED_ITEM (-0x8000)
#define ABS(x) (((x) < 0) ? (-(x)) : (x))

extern sint32 level_items;

void GetAITarget(CREATURE_INFO *creature)
{
    ITEM_INFO *item, *target_item, *enemy;
    FLOOR_INFO *floor;
    char ai_bits;
    int i;
    sint16 enemy_object;

    enemy = creature->enemy;
    if (enemy == NULL)
        enemy_object = -1;
    else
        enemy_object = enemy->object_number;
    item = &items[creature->item_num];
    ai_bits = item->ai_bits;
    if (ai_bits & 1) {
        creature->enemy = lara_item;
        if (creature->alerted) {
            item->ai_bits &= ~1;
            if (ai_bits & 2)
                item->ai_bits |= MODIFY;
        }
    } else if (ai_bits & 4) {
        if (creature->alerted || creature->hurt_by_lara) {
            item->ai_bits &= ~4;
            if (ai_bits & 2)
                item->ai_bits |= MODIFY;
        } else if (!creature->patrol2 && enemy_object != 76) {
            for (i = 0, target_item = &items[0]; i < level_items; i++, target_item++) {
                if (target_item->object_number == 76 && target_item->room_number != NO_ROOM) {
                    if (SameZone(creature, target_item) && target_item->pos.y_rot == item->item_flags[3]) {
                        creature->enemy = target_item;
                        break;
                    }
                }
            }
        } else if (creature->patrol2 && enemy_object != 79) {
            for (i = 0, target_item = &items[0]; i < level_items; i++, target_item++) {
                if (target_item->object_number == 79 && target_item->room_number != NO_ROOM) {
                    if (SameZone(creature, target_item) && target_item->pos.y_rot == item->item_flags[3]) {
                        creature->enemy = target_item;
                        break;
                    }
                }
            }
        } else if (ABS(enemy->pos.x_pos - item->pos.x_pos) < STEP_L * 3 && ABS(enemy->pos.z_pos - item->pos.z_pos) < STEP_L * 3 && ABS(enemy->pos.y_pos - item->pos.y_pos) < STEP_L * 3) {
            floor = GetFloor(enemy->pos.x_pos, enemy->pos.y_pos, enemy->pos.z_pos, &enemy->room_number);
            GetHeight(floor, enemy->pos.x_pos, enemy->pos.y_pos, enemy->pos.z_pos);
            TestTriggers(trigger_index, 1);
            creature->patrol2 = ~creature->patrol2;
        }
    } else if (ai_bits & 2) {
        if (!(ai_bits & MODIFY) && !creature->hurt_by_lara) {
            creature->enemy = lara_item;
        } else if (enemy_object != 75) {
            for (i = 0, target_item = &items[0]; i < level_items; i++, target_item++) {
                if (target_item->object_number == 75 && target_item->room_number != NO_ROOM) {
                    if (SameZone(creature, target_item) && (target_item->pos.y_rot == item->item_flags[3] || item->object_number == MONKEY)) {
                        creature->enemy = target_item;
                        break;
                    }
                }
            }
        } else if (item->object_number == MONKEY) {
            return;
        } else if (ABS(enemy->pos.x_pos - item->pos.x_pos) < STEP_L * 3 && ABS(enemy->pos.z_pos - item->pos.z_pos) < STEP_L * 3 && ABS(enemy->pos.y_pos - item->pos.y_pos) < STEP_L * 3) {
            floor = GetFloor(enemy->pos.x_pos, enemy->pos.y_pos, enemy->pos.z_pos, &enemy->room_number);
            GetHeight(floor, enemy->pos.x_pos, enemy->pos.y_pos, enemy->pos.z_pos);
            TestTriggers(trigger_index, 1);
            creature->reached_goal = 1;
            creature->enemy = lara_item;
            item->ai_bits &= ~2;
            item->ai_bits &= ~MODIFY;
            item->ai_bits |= 1;
            creature->alerted = 0;
        }
    } else if (ai_bits & 16) {
        if (creature->hurt_by_lara) {
            creature->enemy = lara_item;
            creature->alerted = 1;
            item->ai_bits &= ~16;
        } else if (item->hit_status) {
            item->ai_bits &= ~16;
        } else if (enemy_object != 78) {
            for (i = 0, target_item = &items[0]; i < level_items; i++, target_item++) {
                if (target_item->object_number == 78 && target_item->room_number != NO_ROOM) {
                    if (SameZone(creature, target_item) && target_item->pos.y_rot == item->item_flags[3]) {
                        creature->enemy = target_item;
                        break;
                    }
                }
            }
        } else if (ABS(enemy->pos.x_pos - item->pos.x_pos) < STEP_L * 3 && ABS(enemy->pos.z_pos - item->pos.z_pos) < STEP_L * 3 && ABS(enemy->pos.y_pos - item->pos.y_pos) < STEP_L * 3) {
            creature->reached_goal = 1;
            item->ai_bits &= ~16;
        }
    } else if (item->object_number == MONKEY && item->carried_item == NO_ITEM) {
        if (item->ai_bits != MODIFY) {
            if (enemy_object != 176) {
                for (i = 0, target_item = &items[0]; i < level_items; i++, target_item++) {
                    if (target_item->object_number == 176 && target_item->room_number != NO_ROOM && !target_item->ai_bits && target_item->status != INVISIBLE && !(target_item->flags & KILLED_ITEM)) {
                        if (SameZone(creature, target_item)) {
                            creature->enemy = target_item;
                            break;
                        }
                    }
                }
            }
        } else if (enemy_object != 227) {
            for (i = 0, target_item = &items[0]; i < level_items; i++, target_item++) {
                if (target_item->object_number == 227 && target_item->room_number != NO_ROOM && !target_item->ai_bits && target_item->status != INVISIBLE && !(target_item->flags & KILLED_ITEM)) {
                    if (SameZone(creature, target_item)) {
                        creature->enemy = target_item;
                        break;
                    }
                }
            }
        }
    }
}

int BadFloor(sint32 x, sint32 y, sint32 z, sint32 box_height, sint32 next_height, sint16 room_number, LOT_INFO *LOT);

int CreatureAnimation(sint16 item_number, sint16 angle, sint16 tilt)
{
    sint32 radius, shift_x, shift_z, top;
    sint32 height, box_height, next_height, ceiling;
    sint32 x, y, z, pos_x, pos_z, dy;
    PHD_VECTOR old;
    ITEM_INFO *item;
    CREATURE_INFO *creature;
    LOT_INFO *LOT;
    FLOOR_INFO *floor;
    sint16 *zone, *bounds;
    sint16 room_number;
    sint16 next_box;
    sint16 biff_angle;

    item = &items[item_number];
    if (item->data == NULL)
        return 0;
    creature = (CREATURE_INFO *)item->data;
    LOT = &creature->LOT;
    old.x = item->pos.x_pos;
    old.y = item->pos.y_pos;
    old.z = item->pos.z_pos;
    box_height = boxes[item->box_number].height;
    if (LOT->fly == NO_FLYING)
        zone = ground_zone[ZONE(LOT->step)][flip_status];
    else
        zone = ground_zone[ZONE(1)][0];
    if (!objects[item->object_number].water_creature) {
        room_number = item->room_number;
        GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
        if (room_number != item->room_number)
            ItemNewRoom(item_number, room_number);
    }
    AnimateItem(item);
    if (item->status == DEACTIVATED) {
        CreatureDie(item_number, 0);
        return 0;
    }
    bounds = GetBoundsAccurate(item);
    y = item->pos.y_pos + bounds[2];
    room_number = item->room_number;
    GetFloor(old.x, y, old.z, &room_number);
    floor = GetFloor(item->pos.x_pos, y, item->pos.z_pos, &room_number);
    height = boxes[floor->box].height;
    if (!objects[item->object_number].non_lot) {
        next_box = LOT->node[floor->box].exit_box;
    } else {
        floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
        height = boxes[floor->box].height;
        next_box = floor->box;
    }
    if (next_box != NO_BOX)
        next_height = boxes[next_box].height;
    else
        next_height = height;
    if (floor->box == NO_BOX || (LOT->fly == NO_FLYING && zone[item->box_number] != zone[floor->box]) || box_height - height > LOT->step || box_height - height < LOT->drop) {
        pos_x = item->pos.x_pos >> WALL_SHIFT;
        pos_z = item->pos.z_pos >> WALL_SHIFT;
        shift_x = old.x >> WALL_SHIFT;
        shift_z = old.z >> WALL_SHIFT;
        if (pos_x < shift_x)
            item->pos.x_pos = old.x & ~(WALL_L - 1);
        else if (pos_x > shift_x)
            item->pos.x_pos = old.x | (WALL_L - 1);
        if (pos_x < shift_z)
            item->pos.z_pos = old.z & ~(WALL_L - 1);
        else if (pos_x > shift_z)
            item->pos.z_pos = old.z | (WALL_L - 1);
        floor = GetFloor(item->pos.x_pos, y, item->pos.z_pos, &room_number);
        height = boxes[floor->box].height;
        if (!objects[item->object_number].non_lot) {
            next_box = LOT->node[floor->box].exit_box;
        } else {
            floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
            height = boxes[floor->box].height;
            next_box = floor->box;
        }
        if (next_box != NO_BOX)
            next_height = boxes[next_box].height;
        else
            next_height = height;
    }
    x = item->pos.x_pos;
    z = item->pos.z_pos;
    pos_x = x & (WALL_L - 1);
    pos_z = z & (WALL_L - 1);
    radius = objects[item->object_number].radius;
    shift_x = shift_z = 0;
    if (pos_z < radius) {
        if (BadFloor(x, y, z - radius, height, next_height, room_number, LOT))
            shift_z = radius - pos_z;
        if (pos_x < radius) {
            if (BadFloor(x - radius, y, z, height, next_height, room_number, LOT))
                shift_x = radius - pos_x;
            else if (!shift_z && BadFloor(x - radius, y, z - radius, height, next_height, room_number, LOT)) {
                if (item->pos.y_rot > -0x6000 && item->pos.y_rot < 0x2000)
                    shift_z = radius - pos_z;
                else
                    shift_x = radius - pos_x;
            }
        } else if (pos_x > WALL_L - radius) {
            if (BadFloor(x + radius, y, z, height, next_height, room_number, LOT))
                shift_x = WALL_L - radius - pos_x;
            else if (!shift_z && BadFloor(x + radius, y, z - radius, height, next_height, room_number, LOT)) {
                if (item->pos.y_rot > -0x2000 && item->pos.y_rot < 0x6000)
                    shift_z = radius - pos_z;
                else
                    shift_x = WALL_L - radius - pos_x;
            }
        }
    } else if (pos_z > WALL_L - radius) {
        if (BadFloor(x, y, z + radius, height, next_height, room_number, LOT))
            shift_z = WALL_L - radius - pos_z;
        if (pos_x < radius) {
            if (BadFloor(x - radius, y, z, height, next_height, room_number, LOT))
                shift_x = radius - pos_x;
            else if (!shift_z && BadFloor(x - radius, y, z + radius, height, next_height, room_number, LOT)) {
                if (item->pos.y_rot > -0x2000 && item->pos.y_rot < 0x6000)
                    shift_x = radius - pos_x;
                else
                    shift_z = WALL_L - radius - pos_z;
            }
        } else if (pos_x > WALL_L - radius) {
            if (BadFloor(x + radius, y, z, height, next_height, room_number, LOT))
                shift_x = WALL_L - radius - pos_x;
            else if (!shift_z && BadFloor(x + radius, y, z + radius, height, next_height, room_number, LOT)) {
                if (item->pos.y_rot > -0x6000 && item->pos.y_rot < 0x2000)
                    shift_x = WALL_L - radius - pos_x;
                else
                    shift_z = WALL_L - radius - pos_z;
            }
        }
    } else if (pos_x < radius) {
        if (BadFloor(x - radius, y, z, height, next_height, room_number, LOT))
            shift_x = radius - pos_x;
    } else if (pos_x > WALL_L - radius) {
        if (BadFloor(x + radius, y, z, height, next_height, room_number, LOT))
            shift_x = WALL_L - radius - pos_x;
    }
    item->pos.x_pos += shift_x;
    item->pos.z_pos += shift_z;
    if (shift_x || shift_z) {
        floor = GetFloor(item->pos.x_pos, y, item->pos.z_pos, &room_number);
        item->pos.y_rot += angle;
        if (tilt)
            CreatureTilt(item, (sint16)(tilt * 2));
    }
    if (item->object_number != 287 && item->speed && item->hit_points > 0)
        biff_angle = CreatureCreature(item_number);
    else
        biff_angle = 0;
    if (biff_angle) {
        if (abs(biff_angle) < 0x800)
            item->pos.y_rot -= biff_angle;
        else if (biff_angle > 0)
            item->pos.y_rot -= 0x800;
        else
            item->pos.y_rot += 0x800;
        return 1;
    }
    if (LOT->fly != NO_FLYING) {
        dy = creature->target.y - item->pos.y_pos;
        if (dy > LOT->fly)
            dy = LOT->fly;
        else if (dy < -LOT->fly)
            dy = -LOT->fly;
        height = GetHeight(floor, item->pos.x_pos, y, item->pos.z_pos);
        if (item->pos.y_pos + dy > height) {
            if (item->pos.y_pos > height) {
                item->pos.x_pos = old.x;
                item->pos.z_pos = old.z;
                dy = -LOT->fly;
            } else {
                dy = 0;
                item->pos.y_pos = height;
            }
        } else if (!objects[item->object_number].water_creature) {
            floor = GetFloor(item->pos.x_pos, y + STEP_L, item->pos.z_pos, &room_number);
            if (room[room_number].flags & (UNDERWATER | 0x80))
                dy = -LOT->fly;
        } else {
            ceiling = GetCeiling(floor, item->pos.x_pos, y, item->pos.z_pos);
            if (item->object_number == 25)
                top = STEP_L / 2;
            else
                top = bounds[2];
            if (item->pos.y_pos + top + dy < ceiling) {
                if (item->pos.y_pos + top < ceiling) {
                    item->pos.x_pos = old.x;
                    item->pos.z_pos = old.z;
                    dy = LOT->fly;
                } else {
                    dy = 0;
                }
            }
        }
        item->pos.y_pos += dy;
        floor = GetFloor(item->pos.x_pos, y, item->pos.z_pos, &room_number);
        item->floor = GetHeight(floor, item->pos.x_pos, y, item->pos.z_pos);
        angle = item->speed ? phd_atan(item->speed, -dy) : 0;
        if (angle < -ONE_DEGREE * 20)
            angle = -ONE_DEGREE * 20;
        else if (angle > ONE_DEGREE * 20)
            angle = ONE_DEGREE * 20;
        if (angle < item->pos.x_rot - ONE_DEGREE)
            item->pos.x_rot -= ONE_DEGREE;
        else if (angle > item->pos.x_rot + ONE_DEGREE)
            item->pos.x_rot += ONE_DEGREE;
        else
            item->pos.x_rot = angle;
    } else {
        floor = GetFloor(item->pos.x_pos, y, item->pos.z_pos, &room_number);
        ceiling = GetCeiling(floor, item->pos.x_pos, y, item->pos.z_pos);
        if (item->object_number == 287 || item->object_number == SHIVA || item->object_number == 45)
            top = STEP_L * 3;
        else
            top = bounds[2];
        if (item->pos.y_pos + top < ceiling) {
            item->pos.x_pos = old.x;
            item->pos.z_pos = old.z;
            item->pos.y_pos = old.y;
        }
        floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
        item->floor = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
        if (item->pos.y_pos > item->floor)
            item->pos.y_pos = item->floor;
        else if (item->floor - item->pos.y_pos > STEP_L / 4)
            item->pos.y_pos += STEP_L / 4;
        else if (item->pos.y_pos < item->floor)
            item->pos.y_pos = item->floor;
        item->pos.x_rot = 0;
    }
    if (!objects[item->object_number].water_creature) {
        GetFloor(item->pos.x_pos, item->pos.y_pos - STEP_L * 2, item->pos.z_pos, &room_number);
        if (room[room_number].flags & UNDERWATER)
            item->hit_points = 0;
    }
    if (item->room_number != room_number)
        ItemNewRoom(item_number, room_number);
    return 1;
}
