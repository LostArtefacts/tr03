#include "game.h"

#define SQUARE(A) ((A) * (A))

extern sint32 wibble;

void TriggerFireFlame(sint32 x, sint32 y, sint32 z, sint32 body_part, sint32 type);
void TriggerDynamic(sint32 x, sint32 y, sint32 z, sint32 falloff, sint32 r, sint32 g, sint32 b);
void TriggerRocketSmoke(sint32 x, sint32 y, sint32 z, sint32 body_part);
void CreateBubble(PHD_3DPOS *pos, sint16 room_number, sint32 size, sint32 sizerange);
void KillEffect(sint16 fx_num);
void EffectNewRoom(sint16 fx_num, sint16 room_number);
int ItemNearLara(PHD_3DPOS *pos, sint32 radius);
void LaraBurn(void);

void ControlMissile(sint16 fx_number)
{
    FX_INFO *fx;
    FLOOR_INFO *floor;
    sint16 room_number;
    sint32 speed;

    fx = &effects[fx_number];
    if (fx->object_number == 306 && !(room[fx->room_number].flags & UNDERWATER) && fx->pos.x_rot > -0x3000)
        fx->pos.x_rot -= ONE_DEGREE;
    fx->pos.y_pos += fx->speed * phd_sin(-fx->pos.x_rot) >> W2V_SHIFT;
    speed = fx->speed * phd_cos(fx->pos.x_rot) >> W2V_SHIFT;
    fx->pos.z_pos += speed * phd_cos(fx->pos.y_rot) >> W2V_SHIFT;
    fx->pos.x_pos += speed * phd_sin(fx->pos.y_rot) >> W2V_SHIFT;
    room_number = fx->room_number;
    floor = GetFloor(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, &room_number);
    if (fx->pos.y_pos >= GetHeight(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos)
        || fx->pos.y_pos <= GetCeiling(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos)) {
        if (fx->object_number == 308 || fx->object_number == 306) {
            SoundEffect(fx->object_number == 306 ? 10 : 258, &fx->pos, 0);
        } else if (fx->object_number == 307) {
            TriggerFireFlame(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, -1, 0);
            TriggerDynamic(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, 24, 31, 24, GetRandomControl() & 7);
            KillEffect(fx_number);
        }
        return;
    }

    if (room_number != fx->room_number)
        EffectNewRoom(fx_number, room_number);

    if (fx->object_number == 307) {
        if (ItemNearLara(&fx->pos, 350)) {
            lara_item->hit_points -= 3;
            lara_item->hit_status = 1;
            LaraBurn();
            return;
        }
    } else if (ItemNearLara(&fx->pos, 200)) {
        if (fx->object_number == 308 || fx->object_number == 306) {
            lara_item->hit_points -= 50;
            SoundEffect(317, &fx->pos, 0);
            KillEffect(fx_number);
        }
        lara_item->hit_status = 1;
        fx->pos.y_rot = lara_item->pos.y_rot;
        fx->speed = lara_item->speed;
        fx->frame_number = fx->counter = 0;
    }

    if (fx->object_number == 306 && (room[fx->room_number].flags & UNDERWATER)) {
        if ((wibble & 15) == 0)
            CreateBubble(&fx->pos, fx->room_number, 8, 8);
        TriggerRocketSmoke(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, 64);
    } else if (fx->object_number == 307 && !fx->counter--) {
        TriggerDynamic(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, 24, 31, 24, GetRandomControl() & 7);
        SoundEffect(305, &fx->pos, 0);
        KillEffect(fx_number);
    } else if (fx->object_number == 308)
        fx->pos.z_rot += 30 * ONE_DEGREE;
}

void ShootAtLara(FX_INFO *fx)
{
    sint32 x, y, z, distance;
    sint16 *bounds;

    x = lara_item->pos.x_pos - fx->pos.x_pos;
    y = lara_item->pos.y_pos - fx->pos.y_pos;
    z = lara_item->pos.z_pos - fx->pos.z_pos;
    bounds = GetBoundsAccurate(lara_item);
    y += bounds[3] + (bounds[2] - bounds[3]) * 3 / 4;
    distance = phd_sqrt(SQUARE(x) + SQUARE(z));
    fx->pos.x_rot = -phd_atan(distance, y);
    fx->pos.y_rot = phd_atan(z, x);
    fx->pos.x_rot += (GetRandomControl() - 0x4000) / 0x40;
    fx->pos.y_rot += (GetRandomControl() - 0x4000) / 0x40;
}

extern sint32 wibble;
void TriggerFireFlame(sint32 x, sint32 y, sint32 z, sint32 body_part, sint32 type);
void TriggerFireSmoke(sint32 x, sint32 y, sint32 z, sint32 body_part, sint32 type);
void KillEffect(sint16 fx_num);
void EffectNewRoom(sint16 fx_num, sint16 room_number);
int ItemNearLara(PHD_3DPOS *pos, sint32 radius);

sint16 *GetBestFrame(ITEM_INFO *item);
void phd_PushUnitMatrix(void);
void phd_PushMatrix(void);
void phd_RotYXZ(sint16 ry, sint16 rx, sint16 rz);
void phd_TranslateRel(sint32 x, sint32 y, sint32 z);
void phd_RotX(sint16 rx);
void phd_RotY(sint16 ry);
void phd_RotZ(sint16 rz);
void gar_RotYXZsuperpack(sint16 **pprot, int skip);
sint16 CreateEffect(sint16 room_num);
extern sint32 *bones;

int ExplodingDeath(sint16 item_number, sint32 mesh_bits, sint16 damage)
{
    int i, poppush;
    ITEM_INFO *item;
    OBJECT_INFO *object;
    sint32 *bone, bit;
    sint16 *rotation;
    sint16 *frame, *extra_rotation;
    sint16 fx_number;
    FX_INFO *fx;

    item = &items[item_number];
    object = &objects[item->object_number];
    frame = GetBestFrame(item);
    phd_PushUnitMatrix();
    *(phd_mxptr + 3) = 0;
    *(phd_mxptr + 7) = 0;
    *(phd_mxptr + 11) = 0;
    phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);
    phd_TranslateRel((sint32)*(frame + 6), (sint32)*(frame + 7), (sint32)*(frame + 8));
    rotation = frame + 9;
    gar_RotYXZsuperpack(&rotation, 0);
    bone = bones + object->bone_index;
    extra_rotation = (sint16 *)item->data;
    bit = 1;
    if ((bit & mesh_bits) && (bit & item->mesh_bits)) {
        if ((GetRandomControl() & 3) == 0 || item->object_number == 101 || item->object_number == 102
            || item->object_number == 103 || item->object_number == 104) {
            fx_number = CreateEffect(item->room_number);
            if (fx_number != NO_ITEM) {
                fx = &effects[fx_number];
                fx->pos.x_pos = (*(phd_mxptr + 3) >> W2V_SHIFT) + item->pos.x_pos;
                fx->pos.y_pos = (*(phd_mxptr + 7) >> W2V_SHIFT) + item->pos.y_pos;
                fx->pos.z_pos = (*(phd_mxptr + 11) >> W2V_SHIFT) + item->pos.z_pos;
                fx->room_number = item->room_number;
                fx->pos.y_rot = (GetRandomControl() - 0x4000) << 1;
                fx->speed = (sint16)GetRandomControl() >> 8;
                fx->fallspeed = (sint16)-GetRandomControl() >> 8;
                if (item->object_number == 101 || item->object_number == 102 || item->object_number == 103
                    || item->object_number == 104 || item->object_number == 45 || item->object_number == 16)
                    fx->counter = 0;
                else
                    fx->counter = (damage << 2) | (GetRandomControl() & 3);
                fx->frame_number = object->mesh_index;
                fx->object_number = 303;
            }
            item->mesh_bits -= bit;
        }
    }

    for (i = 1; i < object->nmeshes; i++, bone += 3) {
        poppush = *(bone++);
        if (poppush & 1)
            phd_PopMatrix();
        if (poppush & 2)
            phd_PushMatrix();
        phd_TranslateRel(*bone, *(bone + 1), *(bone + 2));
        gar_RotYXZsuperpack(&rotation, 0);
        if (item->data && (poppush & 0x1c)) {
            if (poppush & 8)
                phd_RotY(*(extra_rotation++));
            if (poppush & 4)
                phd_RotX(*(extra_rotation++));
            if (poppush & 16)
                phd_RotZ(*(extra_rotation++));
        }
        bit <<= 1;
        if ((bit & mesh_bits) && (bit & item->mesh_bits)) {
            if ((GetRandomControl() & 3) == 0 || item->object_number == 101 || item->object_number == 102
                || item->object_number == 103 || item->object_number == 104) {
                fx_number = CreateEffect(item->room_number);
                if (fx_number != NO_ITEM) {
                    fx = &effects[fx_number];
                    fx->pos.x_pos = (*(phd_mxptr + 3) >> W2V_SHIFT) + item->pos.x_pos;
                    fx->pos.y_pos = (*(phd_mxptr + 7) >> W2V_SHIFT) + item->pos.y_pos;
                    fx->pos.z_pos = (*(phd_mxptr + 11) >> W2V_SHIFT) + item->pos.z_pos;
                    fx->room_number = item->room_number;
                    fx->pos.y_rot = (GetRandomControl() - 0x4000) << 1;
                    fx->speed = (sint16)GetRandomControl() >> 8;
                    fx->fallspeed = (sint16)-GetRandomControl() >> 8;
                    if (item->object_number == 101 || item->object_number == 102 || item->object_number == 103
                        || item->object_number == 104 || item->object_number == 45 || item->object_number == 16)
                        fx->counter = 0;
                    else
                        fx->counter = (damage << 2) | (GetRandomControl() & 3);
                    fx->frame_number = object->mesh_index + i;
                    fx->object_number = 303;
                    fx->shade = 0x4210;
                }
                item->mesh_bits -= bit;
            }
        }
    }
    phd_PopMatrix();
    if (!((0x7fffffff >> (31 - object->nmeshes)) & item->mesh_bits))
        return 1;
    else
        return 0;
}

void ControlBodyPart(sint16 fx_number)
{
    FX_INFO *fx;
    FLOOR_INFO *floor;
    sint32 height, ceiling, lp;
    sint16 room_number;

    fx = &effects[fx_number];
    fx->pos.x_rot += ONE_DEGREE * 5;
    fx->pos.z_rot += ONE_DEGREE * 10;
    fx->pos.z_pos += (fx->speed * phd_cos(fx->pos.y_rot) >> W2V_SHIFT) >> 2;
    fx->pos.x_pos += (fx->speed * phd_sin(fx->pos.y_rot) >> W2V_SHIFT) >> 2;
    fx->fallspeed += 6 >> 1;
    fx->pos.y_pos += fx->fallspeed;
    if (!(wibble & 12)) {
        if (fx->counter & 1)
            TriggerFireFlame(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, fx_number, 0);
        if (fx->counter & 2)
            TriggerFireSmoke(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, -1, 0);
    }
    room_number = fx->room_number;
    floor = GetFloor(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, &room_number);
    ceiling = GetCeiling(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos);
    if (fx->pos.y_pos < ceiling) {
        fx->fallspeed = -fx->fallspeed;
        fx->pos.y_pos = ceiling;
    }
    height = GetHeight(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos);
    if (fx->pos.y_pos >= height) {
        if (fx->counter & 3) {
            for (lp = 0; lp < 3; lp++) {
                if (fx->counter & 1)
                    TriggerFireFlame(fx->pos.x_pos, height, fx->pos.z_pos, -1, 0);
                if (fx->counter & 2)
                    TriggerFireSmoke(fx->pos.x_pos, height, fx->pos.z_pos, -1, 0);
            }
            SoundEffect(105, &fx->pos, 0);
        }
        KillEffect(fx_number);
        return;
    }
    if (ItemNearLara(&fx->pos, fx->counter & ~3)) {
        lara_item->hit_points -= fx->counter >> 2;
        lara_item->hit_status = 1;
        if (fx->counter & 3) {
            for (lp = 0; lp < 3; lp++) {
                if (fx->counter & 1)
                    TriggerFireFlame(fx->pos.x_pos, height, fx->pos.z_pos, -1, 0);
                if (fx->counter & 2)
                    TriggerFireSmoke(fx->pos.x_pos, height, fx->pos.z_pos, -1, 0);
            }
            SoundEffect(105, &fx->pos, 0);
            KillEffect(fx_number);
        } else
            KillEffect(fx_number);
    }
    if (room_number != fx->room_number)
        EffectNewRoom(fx_number, room_number);
}
