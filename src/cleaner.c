#include "game.h"

#define CLEANER_TURN 1024
#define CLEANER_SPEED 64

void InitialiseCleaner(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    item->pos.x_pos &= ~1023;
    item->pos.z_pos &= ~1023;
    item->pos.x_pos |= 512;
    item->pos.z_pos |= 512;
    item->item_flags[0] = CLEANER_TURN;
    item->item_flags[1] = 0;
    item->item_flags[2] = CLEANER_SPEED;
    item->collidable = 1;
    item->data = NULL;
}

#define CLEANER_TOUCH 0xfffc

extern uint8 elecspark[3];
extern char HeavyTriggered;

int CreatureActive(sint16 item_number);
void TestTriggers(sint16 *data, int heavy);
void TriggerElectricSparks_cleaner(PHD_VECTOR *pos, sint16 item_number, sint16 node);
void TriggerDynamic(sint32 x, sint32 y, sint32 z, sint32 falloff, sint32 r, sint32 g, sint32 b);
int SoundEffect(int nSFX, PHD_3DPOS *pPos, int flags);

void CleanerControl(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *cleaner;
    FLOOR_INFO *floor;
    ROOM_INFO *r;
    sint16 angle, room_number, stop;
    sint32 height, left_ok, forward_ok;
    sint32 x, z;

    if (!CreatureActive(item_number) || items[item_number].item_flags[2] == 0)
        return;

    item = &items[item_number];
    cleaner = (CREATURE_INFO *)item->data;
    angle = 0;

    if ((item->pos.y_rot & 0x3fff) == 0) {
        if (((item->pos.z_pos & 0x3ff) == 512 && (item->pos.y_rot == 0x0 || item->pos.y_rot == -0x8000))
            || ((item->pos.x_pos & 0x3ff) == 512 && (item->pos.y_rot == 0x4000 || item->pos.y_rot == -0x4000))) {
            if (item->item_flags[1] == 1) {
                x = item->pos.x_pos + ((1024 * phd_sin(item->pos.y_rot + 0x8000)) >> W2V_SHIFT);
                z = item->pos.z_pos + ((1024 * phd_cos(item->pos.y_rot + 0x8000)) >> W2V_SHIFT);
                room_number = item->room_number;
                GetFloor(x, item->pos.y_pos, z, &room_number);
                r = &room[room_number];
                r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size].stopper = 0;
                item->item_flags[1] = 0;
            }

            switch (item->pos.y_rot) {
            case 0x0:
                room_number = item->room_number;
                floor = GetFloor(item->pos.x_pos - WALL_L, item->pos.y_pos, item->pos.z_pos, &room_number);
                height = GetHeight(floor, item->pos.x_pos - WALL_L, item->pos.y_pos, item->pos.z_pos);
                r = &room[room_number];
                stop = r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - WALL_L - r->x) >> WALL_SHIFT) * r->x_size].stopper;
                left_ok = (height == item->pos.y_pos && stop == 0);
                room_number = item->room_number;
                floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos + WALL_L, &room_number);
                height = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos + WALL_L);
                r = &room[room_number];
                stop = r->floor[((item->pos.z_pos + WALL_L - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size].stopper;
                forward_ok = (height == item->pos.y_pos && stop == 0);
                if (!forward_ok && !left_ok && item->item_flags[0] > 0) {
                    item->pos.y_rot += CLEANER_TURN;
                    item->item_flags[0] = CLEANER_TURN;
                } else if (!forward_ok && !left_ok && item->item_flags[0] < 0) {
                    item->pos.y_rot -= CLEANER_TURN;
                    item->item_flags[0] = -CLEANER_TURN;
                } else if (left_ok && item->item_flags[0] > 0) {
                    item->pos.y_rot -= CLEANER_TURN;
                    item->item_flags[0] = -CLEANER_TURN;
                } else {
                    item->item_flags[0] = CLEANER_TURN;
                    item->pos.z_pos += item->item_flags[2];
                    r->floor[((item->pos.z_pos + WALL_L - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size].stopper = 1;
                    item->item_flags[1] = 1;
                }
                break;
            case 0x4000:
                room_number = item->room_number;
                floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos + WALL_L, &room_number);
                height = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos + WALL_L);
                r = &room[room_number];
                stop = r->floor[((item->pos.z_pos + WALL_L - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size].stopper;
                left_ok = (height == item->pos.y_pos && stop == 0);
                room_number = item->room_number;
                floor = GetFloor(item->pos.x_pos + WALL_L, item->pos.y_pos, item->pos.z_pos, &room_number);
                height = GetHeight(floor, item->pos.x_pos + WALL_L, item->pos.y_pos, item->pos.z_pos);
                r = &room[room_number];
                stop = r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + ((item->pos.x_pos + WALL_L - r->x) >> WALL_SHIFT) * r->x_size].stopper;
                forward_ok = (height == item->pos.y_pos && stop == 0);
                if (!forward_ok && !left_ok && item->item_flags[0] > 0) {
                    item->pos.y_rot += CLEANER_TURN;
                    item->item_flags[0] = CLEANER_TURN;
                } else if (!forward_ok && !left_ok && item->item_flags[0] < 0) {
                    item->pos.y_rot -= CLEANER_TURN;
                    item->item_flags[0] = -CLEANER_TURN;
                } else if (left_ok && item->item_flags[0] > 0) {
                    item->pos.y_rot -= CLEANER_TURN;
                    item->item_flags[0] = -CLEANER_TURN;
                } else {
                    item->item_flags[0] = CLEANER_TURN;
                    item->pos.x_pos += item->item_flags[2];
                    r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + ((item->pos.x_pos + WALL_L - r->x) >> WALL_SHIFT) * r->x_size].stopper = 1;
                    item->item_flags[1] = 1;
                }
                break;
            case -0x8000:
                room_number = item->room_number;
                floor = GetFloor(item->pos.x_pos + WALL_L, item->pos.y_pos, item->pos.z_pos, &room_number);
                height = GetHeight(floor, item->pos.x_pos + WALL_L, item->pos.y_pos, item->pos.z_pos);
                r = &room[room_number];
                stop = r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + ((item->pos.x_pos + WALL_L - r->x) >> WALL_SHIFT) * r->x_size].stopper;
                left_ok = (height == item->pos.y_pos && stop == 0);
                room_number = item->room_number;
                floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos - WALL_L, &room_number);
                height = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos - WALL_L);
                r = &room[room_number];
                stop = r->floor[((item->pos.z_pos - WALL_L - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size].stopper;
                forward_ok = (height == item->pos.y_pos && stop == 0);
                if (!forward_ok && !left_ok && item->item_flags[0] > 0) {
                    item->pos.y_rot += CLEANER_TURN;
                    item->item_flags[0] = CLEANER_TURN;
                } else if (!forward_ok && !left_ok && item->item_flags[0] < 0) {
                    item->pos.y_rot -= CLEANER_TURN;
                    item->item_flags[0] = -CLEANER_TURN;
                } else if (left_ok && item->item_flags[0] > 0) {
                    item->pos.y_rot -= CLEANER_TURN;
                    item->item_flags[0] = -CLEANER_TURN;
                } else {
                    item->item_flags[0] = CLEANER_TURN;
                    item->pos.z_pos -= item->item_flags[2];
                    r->floor[((item->pos.z_pos - WALL_L - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size].stopper = 1;
                    item->item_flags[1] = 1;
                }
                break;
            case -0x4000:
                room_number = item->room_number;
                floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos - WALL_L, &room_number);
                height = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos - WALL_L);
                r = &room[room_number];
                stop = r->floor[((item->pos.z_pos - WALL_L - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size].stopper;
                left_ok = (height == item->pos.y_pos && stop == 0);
                room_number = item->room_number;
                floor = GetFloor(item->pos.x_pos - WALL_L, item->pos.y_pos, item->pos.z_pos, &room_number);
                height = GetHeight(floor, item->pos.x_pos - WALL_L, item->pos.y_pos, item->pos.z_pos);
                r = &room[room_number];
                stop = r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - WALL_L - r->x) >> WALL_SHIFT) * r->x_size].stopper;
                forward_ok = (height == item->pos.y_pos && stop == 0);
                if (!forward_ok && !left_ok && item->item_flags[0] > 0) {
                    item->pos.y_rot += CLEANER_TURN;
                    item->item_flags[0] = CLEANER_TURN;
                } else if (!forward_ok && !left_ok && item->item_flags[0] < 0) {
                    item->pos.y_rot -= CLEANER_TURN;
                    item->item_flags[0] = -CLEANER_TURN;
                } else if (left_ok && item->item_flags[0] > 0) {
                    item->pos.y_rot -= CLEANER_TURN;
                    item->item_flags[0] = -CLEANER_TURN;
                } else {
                    item->item_flags[0] = CLEANER_TURN;
                    item->pos.x_pos -= item->item_flags[2];
                    r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - WALL_L - r->x) >> WALL_SHIFT) * r->x_size].stopper = 1;
                    item->item_flags[1] = 1;
                }
                break;
            default:
                break;
            }

            floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
            height = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
            TestTriggers(trigger_index, 1);
            if (HeavyTriggered) {
                item->item_flags[2] = 0;
                SoundEffect(131, &item->pos, 0);
            }
        } else {
            switch (item->pos.y_rot) {
            case 0x0:
                item->pos.z_pos += item->item_flags[2];
                break;
            case 0x4000:
                item->pos.x_pos += item->item_flags[2];
                break;
            case -0x8000:
                item->pos.z_pos -= item->item_flags[2];
                break;
            case -0x4000:
                item->pos.x_pos -= item->item_flags[2];
                break;
            }
        }
    } else {
        item->pos.y_rot += item->item_flags[0];
    }

    if ((item->touch_bits & CLEANER_TOUCH) && !lara.electric) {
        lara.electric = 1;
        lara_item->hit_points = 0;
        item->item_flags[2] = 0;
        SoundEffect(131, &item->pos, 0);
    }

    AnimateItem(item);
    room_number = item->room_number;
    GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    if (room_number != item->room_number)
        ItemNewRoom(item_number, room_number);

    {
        PHD_VECTOR pos;
        sint32 r, g, b;

        SoundEffect(191, &item->pos, 0);
        if (((GetRandomControl() & 7) == 0 && elecspark[0] == 0) || elecspark[0]) {
            if (elecspark[0] == 0)
                elecspark[0] = (GetRandomControl() & 7) + 4;
            else
                elecspark[0]--;
            pos.x = -160;
            pos.y = -8;
            pos.z = 16;
            GetJointAbsPosition(item, &pos, 5);
            TriggerElectricSparks_cleaner(&pos, item_number, 9);
            pos.x += (GetRandomControl() & 31) - 16;
            pos.y += (GetRandomControl() & 31) - 16;
            pos.z += (GetRandomControl() & 31) - 16;
            b = (GetRandomControl() & 15) + 16;
            r = b >> 2;
            g = b >> 1;
            TriggerDynamic(pos.x, pos.y, pos.z, 10, r, g, b);
        }
        if (((GetRandomControl() & 7) == 0 && elecspark[1] == 0) || elecspark[1]) {
            if (elecspark[1] == 0)
                elecspark[1] = (GetRandomControl() & 7) + 4;
            else
                elecspark[1]--;
            pos.x = -160;
            pos.y = -8;
            pos.z = 16;
            GetJointAbsPosition(item, &pos, 9);
            TriggerElectricSparks_cleaner(&pos, item_number, 10);
            pos.x += (GetRandomControl() & 31) - 16;
            pos.y += (GetRandomControl() & 31) - 16;
            pos.z += (GetRandomControl() & 31) - 16;
            b = (GetRandomControl() & 15) + 16;
            r = b >> 2;
            g = b >> 1;
            TriggerDynamic(pos.x, pos.y, pos.z, 10, r, g, b);
        }
        if (((GetRandomControl() & 7) == 0 && elecspark[2] == 0) || elecspark[2]) {
            if (elecspark[2] == 0)
                elecspark[2] = (GetRandomControl() & 7) + 4;
            else
                elecspark[2]--;
            pos.x = -160;
            pos.y = -8;
            pos.z = 16;
            GetJointAbsPosition(item, &pos, 13);
            TriggerElectricSparks_cleaner(&pos, item_number, 11);
            pos.x += (GetRandomControl() & 31) - 16;
            pos.y += (GetRandomControl() & 31) - 16;
            pos.z += (GetRandomControl() & 31) - 16;
            b = (GetRandomControl() & 15) + 16;
            r = b >> 2;
            g = b >> 1;
            TriggerDynamic(pos.x, pos.y, pos.z, 10, r, g, b);
        }
    }
}

sint32 GetFreeSpark(void);

void TriggerElectricSparks_cleaner(PHD_VECTOR *pos, sint16 item_number, sint16 node)
{
    SPARKS *sptr;
    sint32 dx, dz;

    dx = lara_item->pos.x_pos - pos->x;
    dz = lara_item->pos.z_pos - pos->z;
    if (dx < -0x5000 || dx > 0x5000 || dz < -0x5000 || dz > 0x5000)
        return;
    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sB = (GetRandomControl() & 63) + 192;
    sptr->sR = sptr->sB;
    sptr->sG = sptr->sB;
    sptr->dB = (GetRandomControl() & 63) + 192;
    sptr->dR = sptr->sB >> 2;
    sptr->dG = sptr->sB >> 1;
    sptr->ColFadeSpeed = 8;
    sptr->FadeToBlack = 8;
    sptr->Life = (GetRandomControl() & 7) + 20;
    sptr->TransType = 2;
    sptr->Dynamic = -1;
    sptr->sLife = sptr->Life;
    sptr->x = (GetRandomControl() & 31) - 16;
    sptr->y = (GetRandomControl() & 31) - 16;
    sptr->z = (GetRandomControl() & 31) - 16;
    sptr->Xvel = ((GetRandomControl() & 255) << 2) - 512;
    sptr->Yvel = (GetRandomControl() & 7) - 4;
    sptr->Zvel = ((GetRandomControl() & 255) << 2) - 512;
    sptr->Friction = 4;
    sptr->Flags = 0x1082;
    sptr->FxObj = (uint8)item_number;
    sptr->NodeNumber = (uint8)node;
    sptr->Scalar = 1;
    sptr->Width = sptr->sWidth = (GetRandomControl() & 3) + 4;
    sptr->dWidth = sptr->Width >> 1;
    sptr->Height = sptr->sHeight = sptr->Width;
    sptr->dHeight = sptr->Height >> 1;
    sptr->Gravity = (GetRandomControl() & 3) + 4;
    sptr->MaxYvel = 0;
}
