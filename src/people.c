#include "game.h"

void Richochet(GAME_VECTOR *pos);

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
int LOS(GAME_VECTOR *start, GAME_VECTOR *target);

int Targetable(ITEM_INFO *item, AI_INFO *info)
{
    GAME_VECTOR start, target;
    CREATURE_INFO *creature;
    ITEM_INFO *enemy;
    sint16 *bounds;

    creature = (CREATURE_INFO *)item->data;
    if ((enemy = creature->enemy) == NULL)
        return 0;
    if (enemy->hit_points <= 0 || enemy->data == NULL)
        return 0;
    if (info->ahead && info->distance < 0x4000000) {
        start.x = item->pos.x_pos;
        bounds = GetBestFrame(item);
        start.y = item->pos.y_pos + (sint32)(((bounds[2] << 1) + bounds[2] + bounds[3]) >> 2);
        start.z = item->pos.z_pos;
        start.room_number = item->room_number;
        target.x = enemy->pos.x_pos;
        bounds = GetBestFrame(enemy);
        target.y = enemy->pos.y_pos + (sint32)(((bounds[2] << 1) + bounds[2] + bounds[3]) >> 2);
        target.z = enemy->pos.z_pos;
        return LOS(&start, &target);
    }
    return 0;
}

sint16 GunShot(sint32 x, sint32 y, sint32 z, sint16 speed, PHD_ANGLE yrot, sint16 room_number)
{
    return NO_ITEM;
}

sint16 DoBloodSplat(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num);

sint16 GunHit(sint32 x, sint32 y, sint32 z, sint16 speed, PHD_ANGLE yrot, sint16 room_number)
{
    PHD_VECTOR pos;

    pos.x = pos.y = pos.z = 0;
    GetJointAbsPosition(lara_item, &pos, GetRandomControl() * 25 / 0x7fff);
    DoBloodSplat(pos.x, pos.y, pos.z, lara_item->speed, lara_item->pos.y_rot, lara_item->room_number);
    SoundEffect(50, &lara_item->pos, 0);
    return GunShot(x, y, z, speed, yrot, room_number);
}

sint16 GunMiss(sint32 x, sint32 y, sint32 z, sint16 speed, PHD_ANGLE yrot, sint16 room_number)
{
    GAME_VECTOR pos;

    pos.x = lara_item->pos.x_pos + (GetRandomControl() - 0x4000) * (WALL_L / 2) / 0x7fff;
    pos.y = lara_item->floor;
    pos.z = lara_item->pos.z_pos + (GetRandomControl() - 0x4000) * (WALL_L / 2) / 0x7fff;
    pos.room_number = lara_item->room_number;
    Richochet(&pos);
    return GunShot(x, y, z, speed, yrot, room_number);
}

extern uint32 CurrentLevel;

sint16 CreateEffect(sint16 room_num);
void TriggerGunSmoke(sint32 x, sint32 y, sint32 z, sint32 xv, sint32 yv, sint32 zv, sint32 initial, sint32 weapon, sint32 count);
sint16 CreatureEffect(ITEM_INFO *item, BITE_INFO *bite, sint16 (*generate)(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 yrot, sint16 room_number));
int ObjectOnLOS(GAME_VECTOR *start, GAME_VECTOR *target);
void SmashItem(sint16 item_number, int weapon_type);

int ShotLara(ITEM_INFO *item, AI_INFO *info, BITE_INFO *gun, sint16 extra_rotation, int damage)
{
    int hit, targetable, random, distance;
    sint16 fx_number;
    sint16 smash_item;
    GAME_VECTOR start, target;
    CREATURE_INFO *creature;
    ITEM_INFO *enemy;
    PHD_VECTOR pos;

    creature = (CREATURE_INFO *)item->data;
    enemy = creature->enemy;
    if (info->distance > 0x4000000 || !Targetable(item, info))
        hit = targetable = 0;
    else {
        distance = (phd_sin(info->enemy_facing) * enemy->speed >> W2V_SHIFT) * 0x4000000 / 300;
        distance = info->distance + distance * distance;
        if (distance > 0x4000000)
            hit = 0;
        else {
            random = (0x4000000 - info->distance) / (0x4000000 / 0x5000) + 0x2000;
            hit = (GetRandomControl() < random);
        }
        targetable = 1;
    }

    item->fired_weapon = 3;
    fx_number = CreateEffect(item->room_number);
    if (fx_number != NO_ITEM) {
        FX_INFO *fx;
        PHD_VECTOR pos;

        pos.x = gun->x >> 2;
        pos.y = gun->y >> 2;
        pos.z = gun->z >> 2;
        GetJointAbsPosition(item, &pos, gun->mesh_num);
        fx = &effects[fx_number];
        fx->pos.x_pos = pos.x;
        fx->pos.y_pos = pos.y;
        fx->pos.z_pos = pos.z;
        fx->room_number = item->room_number;
        fx->pos.y_rot = 0;
        fx->pos.x_rot = 0;
        fx->pos.z_rot = GetRandomControl();
        fx->speed = 16 + (GetRandomControl() & 31);
        fx->fallspeed = -48 - (GetRandomControl() & 7);
        fx->object_number = 366;
        fx->frame_number = objects[366].mesh_index;
        fx->shade = 0x4210;
        fx->counter = 1;
        fx->flag1 = item->pos.y_rot - 0x4800 + (GetRandomControl() & 0xfff);
        pos.x = gun->x - (gun->x >> 2);
        pos.y = gun->y - (gun->y >> 2);
        pos.z = gun->z - (gun->z >> 2);
        GetJointAbsPosition(item, &pos, gun->mesh_num);
        TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 0, LG_PISTOLS, 32);
    }

    if (damage) {
        if (enemy == lara_item) {
            if (hit) {
                CreatureEffect(item, gun, GunHit);
                lara_item->hit_points -= damage;
                lara_item->hit_status = 1;
            } else if (targetable)
                CreatureEffect(item, gun, GunMiss);
        } else {
            CreatureEffect(item, gun, GunShot);
            if (hit) {
                enemy->hit_points -= damage / 10;
                enemy->hit_status = 1;
                random = GetRandomControl() & 0xf;
                if (random > 14)
                    random = 0;
                pos.x = pos.y = pos.z = 0;
                GetJointAbsPosition(enemy, &pos, random);
                DoBloodSplat(pos.x, pos.y, pos.z, enemy->speed, enemy->pos.y_rot, enemy->room_number);
            }
        }
    }

    start.x = item->pos.x_pos;
    start.y = item->pos.y_pos - 768;
    start.z = item->pos.z_pos;
    start.room_number = item->room_number;
    target.x = enemy->pos.x_pos;
    target.y = enemy->pos.y_pos - 768;
    target.z = enemy->pos.z_pos;
    smash_item = ObjectOnLOS(&start, &target);
    if (smash_item != NO_ITEM) {
        if (!(items[smash_item].object_number == 102 && CurrentLevel == 6))
            SmashItem(smash_item, 0);
    }
    return targetable;
}
