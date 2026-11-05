#include "game.h"

#define DIVER_HARPOON_SPEED 150
#define DIVER_HARPOON 306

void ShootAtLara(FX_INFO *fx);

sint16 Harpoon(sint32 x, sint32 y, sint32 z, sint16 speed, PHD_ANGLE yrot, sint16 room_number)
{
    sint16 fx_number;
    FX_INFO *fx;

    fx_number = CreateEffect(room_number);
    if (fx_number != NO_ITEM)
    {
        fx = &effects[fx_number];
        fx->pos.x_pos = x;
        fx->pos.y_pos = y;
        fx->pos.z_pos = z;
        fx->room_number = room_number;
        fx->pos.x_rot = fx->pos.z_rot = 0;
        fx->pos.y_rot = yrot;
        fx->speed = DIVER_HARPOON_SPEED;
        fx->fallspeed = 0;
        fx->frame_number = 0;
        fx->object_number = DIVER_HARPOON;
        fx->shade = 14 * 256;
        ShootAtLara(fx);
    }
    return fx_number;
}

sint32 GetWaterSurface(sint32 x, sint32 y, sint32 z, sint16 room_number)
{
    ROOM_INFO *r;
    FLOOR_INFO *floor;

    r = &room[room_number];
    floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
    if (r->flags & UNDERWATER) {
        while (floor->sky_room != NO_ROOM) {
            r = &room[floor->sky_room];
            if (!(r->flags & UNDERWATER))
                return floor->ceiling << 8;
            floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
        }
        return NO_HEIGHT;
    } else {
        while (floor->pit_room != NO_ROOM) {
            r = &room[floor->pit_room];
            if (r->flags & UNDERWATER)
                return floor->floor << 8;
            floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
        }
    }
    return NO_HEIGHT;
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

int CreatureActive(sint16 item_number);
void CreatureAIInfo(ITEM_INFO *item, AI_INFO *info);
void GetCreatureMood(ITEM_INFO *item, AI_INFO *info, int violent);
void CreatureMood(ITEM_INFO *item, AI_INFO *info, int violent);
sint16 CreatureTurn(ITEM_INFO *item, sint16 maximum_turn);
int CreatureAnimation(sint16 item_number, sint16 angle, sint16 tilt);
void CreatureFloat(sint16 item_number);
void CreatureJoint(ITEM_INFO *item, sint16 joint, sint16 required);
void CreatureUnderwater(ITEM_INFO *item, sint32 depth);
sint16 CreatureEffect(ITEM_INFO *item, BITE_INFO *bite, sint16 (*generate)(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 yrot, sint16 room_number));
sint16 DoBloodSplat(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num);

extern BITE_INFO croc_bite;
int LOS(GAME_VECTOR *start, GAME_VECTOR *target);

extern BITE_INFO diver_poon;

void DiverControl(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *diver;
    sint16 angle, head, neck;
    AI_INFO info;
    sint32 water_level, shoot;
    GAME_VECTOR start, target;

    if (!CreatureActive(item_number))
        return;
    item = &items[item_number];
    diver = (CREATURE_INFO *)item->data;
    angle = head = neck = 0;
    if (item->hit_points <= 0) {
        if (item->current_anim_state != 9) {
            item->anim_number = objects[26].anim_index + 16;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = 9;
        }
        CreatureFloat(item_number);
        return;
    }
    CreatureAIInfo(item, &info);
    GetCreatureMood(item, &info, 0);
    CreatureMood(item, &info, 0);
    if (lara.water_status == LARA_ABOVEWATER) {
        start.x = item->pos.x_pos;
        start.y = item->pos.y_pos - 256;
        start.z = item->pos.z_pos;
        start.room_number = item->room_number;
        target.x = lara_item->pos.x_pos;
        target.y = lara_item->pos.y_pos - 612;
        target.z = lara_item->pos.z_pos;
        shoot = LOS(&start, &target);
        if (shoot) {
            diver->target.x = lara_item->pos.x_pos;
            diver->target.y = lara_item->pos.y_pos;
            diver->target.z = lara_item->pos.z_pos;
        }
        if (info.angle < -0x2000 || info.angle > 0x2000)
            shoot = 0;
    } else if (info.angle > -0x2000 && info.angle < 0x2000) {
        start.x = item->pos.x_pos;
        start.y = item->pos.y_pos;
        start.z = item->pos.z_pos;
        start.room_number = item->room_number;
        target.x = lara_item->pos.x_pos;
        target.y = lara_item->pos.y_pos;
        target.z = lara_item->pos.z_pos;
        shoot = LOS(&start, &target);
    } else
        shoot = 0;
    angle = CreatureTurn(item, diver->maximum_turn);
    water_level = GetWaterSurface(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number) + 512;
    switch (item->current_anim_state) {
    case 1:
        diver->maximum_turn = 546;
        if (shoot)
            neck = -info.angle;
        if (diver->target.y < water_level && item->pos.y_pos < water_level + diver->LOT.fly)
            item->goal_anim_state = 2;
        else if (diver->mood == ESCAPE_MOOD)
            break;
        else if (shoot)
            item->goal_anim_state = 4;
        break;
    case 4:
        diver->flags = 0;
        if (shoot)
            neck = -info.angle;
        if (!shoot || diver->mood == ESCAPE_MOOD ||
            (diver->target.y < water_level && item->pos.y_pos < water_level + diver->LOT.fly))
            item->goal_anim_state = 1;
        else
            item->goal_anim_state = 3;
        break;
    case 3:
        if (shoot)
            neck = -info.angle;
        if (!diver->flags) {
            CreatureEffect(item, &diver_poon, Harpoon);
            diver->flags = 1;
        }
        break;
    case 2:
        diver->maximum_turn = 546;
        if (shoot)
            head = info.angle;
        if (diver->target.y > water_level)
            item->goal_anim_state = 1;
        else if (diver->mood == ESCAPE_MOOD)
            break;
        else if (shoot)
            item->goal_anim_state = 6;
        break;
    case 6:
        diver->flags = 0;
        if (shoot)
            head = info.angle;
        if (!shoot || diver->mood == ESCAPE_MOOD || diver->target.y > water_level)
            item->goal_anim_state = 2;
        else
            item->goal_anim_state = 7;
        break;
    case 7:
        if (shoot)
            head = info.angle;
        if (!diver->flags) {
            CreatureEffect(item, &diver_poon, Harpoon);
            diver->flags = 1;
        }
        break;
    }
    CreatureAnimation(item_number, angle, 0);
    switch (item->current_anim_state) {
    case 1:
    case 4:
    case 3:
        CreatureUnderwater(item, 512);
        break;
    default:
        item->pos.y_pos = water_level - 512;
    }
}

sint32 GetFreeSpark(void);
extern sint32 wibble;

void ControlGhostGasEmitter(sint16 item_number)
{
    ITEM_INFO *item;
    SPARKS *sptr;
    sint32 size;
    sint32 dx, dz;

    item = &items[item_number];
    if (!TriggerActive(item) || (wibble & 15))
        return;
    dx = lara_item->pos.x_pos - item->pos.x_pos;
    dz = lara_item->pos.z_pos - item->pos.z_pos;
    if (dx < -0x4000 || dx > 0x4000 || dz < -0x4000 || dz > 0x4000)
        return;
    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sR = 0;
    sptr->sG = 0;
    sptr->sB = 0;
    sptr->dR = 12;
    sptr->dG = 32;
    sptr->dB = 0;
    sptr->ColFadeSpeed = 24 + (GetRandomControl() & 7);
    sptr->FadeToBlack = 32;
    sptr->sLife = sptr->Life = (GetRandomControl() & 7) + 64;
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = item->pos.x_pos + ((GetRandomControl() & 511) - 256);
    sptr->y = item->pos.y_pos - 256 - ((GetRandomControl() & 15) + 8);
    sptr->z = item->pos.z_pos + ((GetRandomControl() & 1023) - 512);
    sptr->Xvel = (GetRandomControl() & 255) - 128;
    sptr->Yvel = -(GetRandomControl() & 1) - 1;
    sptr->Zvel = (GetRandomControl() & 255) - 128;
    sptr->Friction = 4;
    if (GetRandomControl() & 1) {
        sptr->Flags = 2 | 8 | 16 | 512;
        sptr->RotAng = GetRandomControl() & 4095;
        if (GetRandomControl() & 1)
            sptr->RotAdd = -(GetRandomControl() & 7) - 4;
        else
            sptr->RotAdd = (GetRandomControl() & 7) + 4;
    } else
        sptr->Flags = 2 | 8 | 512;
    sptr->Def = objects[296].mesh_index;
    sptr->Scalar = 3;
    sptr->Gravity = sptr->MaxYvel = 0;
    size = (GetRandomControl() & 31) + 96;
    sptr->Width = sptr->sWidth = size >> 1;
    sptr->dWidth = size >> 1;
    size += (GetRandomControl() & 31) + 32;
    sptr->Height = sptr->sHeight = size >> 1;
    sptr->dHeight = size >> 1;
}
