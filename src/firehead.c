#include "game.h"

#define FIREHEAD_REAR 1

void InitialiseFireHead(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    item->anim_number = objects[item->object_number].anim_index + 1;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = FIREHEAD_REAR;
    item->goal_anim_state = FIREHEAD_REAR;
}

extern sint32 wibble;

void TriggerDynamic(sint32 x, sint32 y, sint32 z, sint32 falloff, sint32 r, sint32 g, sint32 b);
void TriggerFireHeadFlame(sint32 x, sint32 y, sint32 z, sint32 angle, sint32 speed);
void LaraBurn(void);

void ControlFireHead(sint16 item_number)
{
    ITEM_INFO *item;
    PHD_VECTOR pos;
    sint32 angle, r, g;

    item = &items[item_number];

    if (!TriggerActive(item))
        return;

    if (item->current_anim_state != 0) {
        angle = (((item->pos.y_rot >> 4) + 1024) & 4095) << 1;
        r = 24 + (GetRandomControl() & 7);
        g = 12 + (GetRandomControl() & 3);

        if (item->current_anim_state == FIREHEAD_REAR) {
            pos.x = pos.z = 0;
            pos.y = 128;
            GetJointAbsPosition(item, &pos, 7);

            item->item_flags[1] = 0;
            item->item_flags[2] = 0;
            item->item_flags[3] = 0;
            item->item_flags[0] = item->hit_points;
            TriggerDynamic(pos.x, pos.y, pos.z, 8, r, g, 0);
        } else {
            pos.x = pos.z = pos.y = 0;
            GetJointAbsPosition(item, &pos, 7);

            if (item->item_flags[2] == 0) {
                SoundEffect(204, &item->pos, 0);
                if (item->item_flags[1] < 2048)
                    item->item_flags[1] += 64;
                if (item->item_flags[3] < 2048)
                    item->item_flags[3] += 32;
            } else {
                if (item->item_flags[1])
                    item->item_flags[1] -= 64;
                if (item->item_flags[3]) {
                    item->item_flags[3] -= 32;
                    if (item->item_flags[3] < 0)
                        item->item_flags[3] = 0;
                }
            }

            if (item->item_flags[0])
                item->item_flags[0]--;
            if (wibble & 4)
                TriggerFireHeadFlame(pos.x, pos.y, pos.z, angle, item->item_flags[1]);

            if (item->item_flags[0] == 0 && item->item_flags[2] == 0 && item->frame_number == anims[item->anim_number].frame_base) {
                item->item_flags[2] = 1;
                item->goal_anim_state = FIREHEAD_REAR;
            }

            TriggerDynamic(pos.x, pos.y, pos.z, 8 + (item->item_flags[1] >> 7), r, g, 0);

            if (!lara.burn) {
                sint32 xdmin, zdmin, ydmin, xdmax, zdmax, ydmax, lx, ly, lz;

                ydmin = item->pos.y_pos - 1024;
                ydmax = item->pos.y_pos + 256;

                switch (item->pos.y_rot) {
                case 0:
                    xdmin = pos.x;
                    xdmax = pos.x + item->item_flags[3];
                    zdmin = pos.z - 512;
                    zdmax = pos.z + 512;
                    break;
                case 16384:
                    zdmax = pos.z;
                    zdmin = pos.z - item->item_flags[3];
                    xdmin = pos.x - 512;
                    xdmax = pos.x + 512;
                    break;
                case -32768:
                    xdmax = pos.x;
                    xdmin = pos.x - item->item_flags[3];
                    zdmin = pos.z - 512;
                    zdmax = pos.z + 512;
                    break;
                default:
                    zdmin = pos.z;
                    zdmax = pos.z + item->item_flags[3];
                    xdmin = pos.x - 512;
                    xdmax = pos.x + 512;
                    break;
                }

                lx = lara_item->pos.x_pos;
                ly = lara_item->pos.y_pos;
                lz = lara_item->pos.z_pos;
                if (lx > xdmin && lx < xdmax && ly > ydmin && ly < ydmax && lz > zdmin && lz < zdmax)
                    LaraBurn();
            }
        }
    } else {
        item->goal_anim_state = FIREHEAD_REAR;
    }

    AnimateItem(item);
}

extern sint16 rcossin_tbl[];
sint32 GetFreeSpark(void);

void TriggerFireHeadFlame(sint32 x, sint32 y, sint32 z, sint32 angle, sint32 speed)
{
    sint32 size, zv, xv;
    SPARKS *sptr;
    sint32 dx, dz;

    dx = lara_item->pos.x_pos - x;
    dz = lara_item->pos.z_pos - z;
    if (dx < -0x4000 || dx > 0x4000 || dz < -0x4000 || dz > 0x4000)
        return;
    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sR = 48 + (GetRandomControl() & 31);
    sptr->sG = sptr->sR;
    sptr->sB = 192 + (GetRandomControl() & 63);
    sptr->dR = 192 + (GetRandomControl() & 63);
    sptr->dG = 128 + (GetRandomControl() & 63);
    sptr->dB = 32;
    sptr->ColFadeSpeed = 12 + (GetRandomControl() & 3);
    sptr->FadeToBlack = 8;
    sptr->sLife = sptr->Life = (GetRandomControl() & 7) + 28;
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = x + ((GetRandomControl() & 31) - 16);
    sptr->y = y + ((GetRandomControl() & 31) - 16);
    sptr->z = z + ((GetRandomControl() & 31) - 16);
    speed -= GetRandomControl() % ((speed >> 3) + 1);
    zv = (rcossin_tbl[angle + 1] * speed) >> 11;
    xv = (rcossin_tbl[angle] * speed) >> 11;
    sptr->Xvel = xv + ((GetRandomControl() & 127) - 64);
    sptr->Yvel = (GetRandomControl() & 7) + 6;
    sptr->Zvel = zv + ((GetRandomControl() & 127) - 64);
    sptr->Friction = 4;
    sptr->Flags = 2 | 8 | 512;
    if (GetRandomControl() & 1) {
        sptr->Flags |= 16;
        sptr->RotAng = GetRandomControl() & 4095;
        sptr->RotAdd = (GetRandomControl() & 63) - 32;
    }
    sptr->Gravity = -(GetRandomControl() & 15) - 8;
    sptr->MaxYvel = -(GetRandomControl() & 7) - 8;
    sptr->Def = objects[296].mesh_index;
    sptr->Scalar = 3;
    size = (GetRandomControl() & 15) + (speed >> 4);
    sptr->Width = sptr->sWidth = size >> 2;
    sptr->Height = sptr->sHeight = size >> 2;
    sptr->dWidth = size;
    sptr->dHeight = size;
}

void ControlRotateyThing(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    if (TriggerActive(item))
        AnimateItem(item);
    else
        KillItem(item_number);
}
