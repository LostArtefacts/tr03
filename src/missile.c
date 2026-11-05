#include "game.h"

#define SQUARE(A) ((A) * (A))

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
