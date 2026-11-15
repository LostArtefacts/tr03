// cflags: /Gi
#include "game.h"

#define SMALL_FIRE 1
#define LV_RAPIDS 7

extern uint32 CurrentLevel;

void LaraBurn(void);

void KillEffect(sint16 fx_num);

void SideFlameDetection(FX_INFO *fx, sint32 length);

extern sint32 wibble;
extern sint16 rcossin_tbl[];
void TriggerFireFlame(sint32 x, sint32 y, sint32 z, sint32 body_part, sint32 type);
void TriggerFireSmoke(sint32 x, sint32 y, sint32 z, sint32 body_part, sint32 type);
void TriggerStaticFlame(sint32 x, sint32 y, sint32 z, sint32 size);
void TriggerSideFlame(sint32 x, sint32 y, sint32 z, sint32 angle, sint32 speed, sint32 pilot);
void EffectNewRoom(sint16 fx_num, sint16 room_number);
sint32 GetWaterHeight(sint32 x, sint32 y, sint32 z, sint16 room_number);
int ItemNearLara(PHD_3DPOS *pos, sint32 radius);

void FlameControl(sint16 fx_number)
{
    FX_INFO *fx;
    sint32 x, y, z, r, g, ox, oz, od = 0, distance;
    unsigned char xzoffs[16][2] = {
        { 9, 9 },   { 24, 9 },  { 40, 9 },  { 55, 9 },  { 9, 24 },  { 24, 24 },
        { 40, 24 }, { 55, 24 }, { 9, 40 },  { 24, 40 }, { 40, 40 }, { 55, 40 },
        { 9, 55 },  { 24, 55 }, { 40, 55 }, { 55, 55 }
    };

    fx = &effects[fx_number];

    distance = GetRandomControl();
    if (fx->frame_number == 0) {
        if ((wibble & 12) == 0) {
            TriggerFireFlame(
                fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, -1, 0);
            TriggerFireSmoke(
                fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, -1, 0);
        }
        g = (GetRandomControl() & 15) + 96;
        TriggerStaticFlame(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, g);
    } else if (fx->frame_number == SMALL_FIRE) {

        if (fx->counter < 0) {
            for (x = 14; x >= 0; x--) {
                if ((wibble & 12) == 0) {
                    fx->pos.x_pos = 0;
                    fx->pos.y_pos = 0;
                    fx->pos.z_pos = 0;

                    GetJointAbsPosition(lara_item, (PHD_VECTOR *)&fx->pos, x);
                    TriggerFireFlame(
                        fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, -1,
                        255 - lara.BurnGreen);
                }
            }

            if (lara.BurnGreen) {
                r = 24 + (distance & 7);
                g = 12 + ((distance >> 4) & 3);
                TriggerDynamic(
                    lara_item->pos.x_pos, lara_item->pos.y_pos,
                    lara_item->pos.z_pos, 13, (distance >> 2) & 7, r, g);
            } else {
                r = 24 + (distance & 7);
                g = 12 + ((distance >> 4) & 3);
                TriggerDynamic(
                    lara_item->pos.x_pos, lara_item->pos.y_pos,
                    lara_item->pos.z_pos, 13, r, g, 0);
            }

            if (lara_item->room_number != fx->room_number)
                EffectNewRoom(fx_number, lara_item->room_number);

            y = GetWaterHeight(
                fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, fx->room_number);
            if (y != NO_HEIGHT && fx->pos.y_pos > y) {
                if ((room[fx->room_number].flags & 0x80)
                    && (CurrentLevel == 18 || CurrentLevel == 4
                        || CurrentLevel == 19)) {
                    SoundEffect(150, &fx->pos, 0);
                    lara_item->hit_points -= 7;
                    lara_item->hit_status = 1;
                } else {
                    fx->counter = 0;
                    KillEffect(fx_number);
                    lara.burn = 0;
                }
            } else {
                SoundEffect(150, &fx->pos, 0);
                lara_item->hit_points -= 7;
                lara_item->hit_status = 1;
            }
            return;
        }

        r = ((fx->pos.y_rot >> 4) & 4095) << 1;
        z = (rcossin_tbl[r + 1] * 288) >> 12;
        x = (rcossin_tbl[r] * 288) >> 12;

        g = (GetRandomControl() & 15) + 32;
        TriggerStaticFlame(
            fx->pos.x_pos + x, fx->pos.y_pos - 192, fx->pos.z_pos + z, g);

        if ((wibble & 24) == 0)
            TriggerFireFlame(
                fx->pos.x_pos + x, fx->pos.y_pos - 224, fx->pos.z_pos + z, -1,
                1);
        if ((wibble & 24) == 0)
            TriggerFireSmoke(
                fx->pos.x_pos + x, fx->pos.y_pos, fx->pos.z_pos + z, -1, 1);
    } else if (fx->frame_number == 2) {
        if (fx->flag1 == 0) {
            fx->flag1 = (GetRandomControl() & 3) + 8;

            x = GetRandomControl() & 63;
            if (fx->flag2 == x)
                x = (x + 13) & 63;
            fx->flag2 = x;
        } else
            fx->flag1--;

        x = (xzoffs[fx->flag2 & 7][0] << 4) - 512;
        z = (xzoffs[fx->flag2 & 7][1] << 4) - 512;
        if ((wibble & 4) == 0)
            TriggerFireFlame(
                fx->pos.x_pos + x, fx->pos.y_pos, fx->pos.z_pos + z, -1, 2);
        x = (xzoffs[(fx->flag2 >> 3) + 8][0] << 4) - 512;
        z = (xzoffs[(fx->flag2 >> 3) + 8][1] << 4) - 512;
        if (wibble & 4)
            TriggerFireFlame(
                fx->pos.x_pos + x, fx->pos.y_pos, fx->pos.z_pos + z, -1, 2);
    } else {
        r = ((fx->pos.y_rot >> 4) & 4095) << 1;
        oz = (rcossin_tbl[r + 1] * (512 + (distance & 255))) >> 12;
        ox = (rcossin_tbl[r] * (512 + (distance & 255))) >> 12;

        if (fx->flag2) {
            if (wibble & 4)
                TriggerSideFlame(
                    fx->pos.x_pos + ox, fx->pos.y_pos, fx->pos.z_pos + oz,
                    (r + 4096) & 8191, ((GetRandomControl() & 7) == 0), 1);
            fx->flag2--;
        } else {
            if (fx->flag1) {
                if (wibble & 4) {
                    if (fx->flag1 > 112)
                        TriggerSideFlame(
                            fx->pos.x_pos + ox, fx->pos.y_pos,
                            fx->pos.z_pos + oz, (r + 4096) & 8190,
                            (129 - fx->flag1) >> 1, 0);
                    else if (fx->flag1 < 18)
                        TriggerSideFlame(
                            fx->pos.x_pos + ox, fx->pos.y_pos,
                            fx->pos.z_pos + oz, (r + 4096) & 8190,
                            (fx->flag1 >> 1) + 1, 0);
                    else
                        TriggerSideFlame(
                            fx->pos.x_pos + ox, fx->pos.y_pos,
                            fx->pos.z_pos + oz, (r + 4096) & 8190, 9, 0);
                }
                fx->flag1 -= 2;
            } else {
                fx->flag1 = 128;
                if (CurrentLevel == LV_RAPIDS)
                    fx->flag2 = 30 * 4;
                else
                    fx->flag2 = 30 * 2;
            }
        }
    }

    x = fx->pos.x_pos + ((distance & 0xf) << 5);
    y = fx->pos.y_pos + ((distance & 0xf0) << 1);
    z = fx->pos.z_pos + ((distance & 0xf00) >> 3);

    if (fx->frame_number == 3) {
        if (fx->flag2)
            od = 0;
        else if (fx->flag1 < 18)
            od = 2048;
        else if (fx->flag1 < 64)
            od = 2048;
        else
            od = (128 - fx->flag1) << 5;

        r = (((fx->pos.y_rot >> 4) + 2048) & 4095) << 1;
        oz = (rcossin_tbl[r + 1] * od) >> 12;
        ox = (rcossin_tbl[r] * od) >> 12;
        r = 24 + (distance & 7);
        g = 12 + ((distance >> 4) & 3);
        TriggerDynamic(x + ox, y, z + oz, (fx->flag2 == 0) ? 13 : 6, r, g, 0);
    } else {
        r = 24 + (distance & 7);
        g = 12 + ((distance >> 4) & 3);
        TriggerDynamic(x, y, z, 16 - (fx->frame_number << 2), r, g, 0);
    }

    SoundEffect(150, &fx->pos, 0);
    if (fx->counter)
        fx->counter--;
    else if (fx->frame_number == 3) {
        if (!lara.burn && od)
            SideFlameDetection(fx, od);
    } else if (fx->frame_number != SMALL_FIRE) {
        PHD_3DPOS pos;

        pos.x_pos = fx->pos.x_pos;
        pos.y_pos = fx->pos.y_pos;
        pos.z_pos = fx->pos.z_pos;

        if (ItemNearLara(&pos, 600)) {
            x = lara_item->pos.x_pos - pos.x_pos;
            z = lara_item->pos.z_pos - pos.z_pos;
            distance = x * x + z * z;

            lara_item->hit_points -= 5;
            lara_item->hit_status = 1;

            if (distance < (450 * 450)) {

                fx->counter = 100;

                LaraBurn();
            }
        }
    }
}
