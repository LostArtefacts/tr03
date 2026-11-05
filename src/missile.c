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
