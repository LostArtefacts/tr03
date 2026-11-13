#include <windows.h>
#include "game.h"

#define MAX_FISH 8

typedef struct {
    sint16 x;
    sint16 y;
    sint16 z;
    uint16 angle;
    sint16 desty;
    sint8 angadd;
    uint8 speed;
    uint8 acc;
    uint8 swim;
} FISH_INFO;

typedef struct {
    sint16 angle;
    uint8 speed;
    uint8 on;
    sint16 angle_time;
    sint16 speed_time;
    sint16 Xrange;
    sint16 Zrange;
    sint16 Yrange;
} LEADER_INFO;

extern FISH_INFO fish[];
extern LEADER_INFO lead_info[];

extern uint32 CurrentLevel;
extern uint8 jungle_fish_ranges[][3];
extern uint8 temple_fish_ranges[][3];
extern uint8 quadchase_fish_ranges[][3];
extern uint8 house_fish_ranges[][3];
extern uint8 shore_fish_ranges[][3];
extern uint8 crash_fish_ranges[][3];
extern uint8 rapids_fish_ranges[][3];

void SetupShoal(sint32 shoal_number)
{
    if (CurrentLevel == 1) {
        lead_info[shoal_number].Xrange = (jungle_fish_ranges[shoal_number][0] + 2) << 8;
        lead_info[shoal_number].Zrange = (jungle_fish_ranges[shoal_number][1] + 2) << 8;
        lead_info[shoal_number].Yrange = jungle_fish_ranges[shoal_number][2] << 8;
    } else if (CurrentLevel == 2) {
        lead_info[shoal_number].Xrange = (temple_fish_ranges[shoal_number][0] + 2) << 8;
        lead_info[shoal_number].Zrange = (temple_fish_ranges[shoal_number][1] + 2) << 8;
        lead_info[shoal_number].Yrange = temple_fish_ranges[shoal_number][2] << 8;
    } else if (CurrentLevel == 3) {
        lead_info[shoal_number].Xrange = (quadchase_fish_ranges[shoal_number][0] + 2) << 8;
        lead_info[shoal_number].Zrange = (quadchase_fish_ranges[shoal_number][1] + 2) << 8;
        lead_info[shoal_number].Yrange = quadchase_fish_ranges[shoal_number][2] << 8;
    } else if (CurrentLevel == 0) {
        lead_info[shoal_number].Xrange = (house_fish_ranges[shoal_number][0] + 2) << 8;
        lead_info[shoal_number].Zrange = (house_fish_ranges[shoal_number][1] + 2) << 8;
        lead_info[shoal_number].Yrange = house_fish_ranges[shoal_number][2] << 8;
    } else if (CurrentLevel == 5) {
        lead_info[shoal_number].Xrange = (shore_fish_ranges[shoal_number][0] + 2) << 8;
        lead_info[shoal_number].Zrange = (shore_fish_ranges[shoal_number][1] + 2) << 8;
        lead_info[shoal_number].Yrange = shore_fish_ranges[shoal_number][2] << 8;
    } else if (CurrentLevel == 6) {
        lead_info[shoal_number].Xrange = (crash_fish_ranges[shoal_number][0] + 2) << 8;
        lead_info[shoal_number].Zrange = (crash_fish_ranges[shoal_number][1] + 2) << 8;
        lead_info[shoal_number].Yrange = crash_fish_ranges[shoal_number][2] << 8;
    } else if (CurrentLevel == 7) {
        lead_info[shoal_number].Xrange = (rapids_fish_ranges[shoal_number][0] + 2) << 8;
        lead_info[shoal_number].Zrange = (rapids_fish_ranges[shoal_number][1] + 2) << 8;
        lead_info[shoal_number].Yrange = rapids_fish_ranges[shoal_number][2] << 8;
    } else {
        lead_info[shoal_number].Xrange = 1 << 8;
        lead_info[shoal_number].Zrange = 1 << 8;
        lead_info[shoal_number].Yrange = 1 << 8;
    }
}

void SetupFish(sint32 leader, ITEM_INFO *item)
{
    sint32 lp, fish_Xrange, fish_Zrange, fish_Yrange;

    fish_Xrange = lead_info[leader].Xrange;
    fish_Yrange = lead_info[leader].Yrange;
    fish_Zrange = lead_info[leader].Zrange;
    fish[leader].x = 0;
    fish[leader].y = 0;
    fish[leader].z = 0;
    fish[leader].angle = 0;
    fish[leader].speed = (GetRandomControl() & 63) + 8;
    fish[leader].swim = GetRandomControl() & 63;
    for (lp = 0; lp < 24; lp++) {
        fish[MAX_FISH + leader * 24 + lp].x = (GetRandomControl() % (fish_Xrange << 1)) - fish_Xrange;
        fish[MAX_FISH + leader * 24 + lp].y = GetRandomControl() % fish_Yrange;
        fish[MAX_FISH + leader * 24 + lp].desty = GetRandomControl() % fish_Yrange;
        fish[MAX_FISH + leader * 24 + lp].z = (GetRandomControl() % (fish_Zrange << 1)) - fish_Zrange;
        fish[MAX_FISH + leader * 24 + lp].angle = GetRandomControl() & 4095;
        fish[MAX_FISH + leader * 24 + lp].speed = (GetRandomControl() & 31) + 32;
        fish[MAX_FISH + leader * 24 + lp].swim = GetRandomControl() & 63;
    }
    lead_info[leader].on = 1;
    lead_info[leader].angle = 0;
    lead_info[leader].speed = (GetRandomControl() & 127) + 32;
    lead_info[leader].angle_time = 0;
    lead_info[leader].speed_time = 0;
}

#define PIRAHNA_DAMAGE 4

extern sint16 rcossin_tbl[];
extern sint32 PirahnaHitWait;
extern sint16 CarcassItem;

int TriggerActive(ITEM_INFO *item);
sint32 mGetAngle(sint32 x1, sint32 y1, sint32 x2, sint32 y2);
sint32 FishNearLara(PHD_3DPOS *pos, sint32 distance, ITEM_INFO *item);
sint16 DoBloodSplat(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num);
int abs(int x);
#pragma intrinsic(abs)

void ControlFish(sint16 item_number)
{
    ITEM_INFO *item, *enemy;
    sint32 lp, leader;
    sint32 angle, angadd, diff;
    sint32 dx, dz;
    sint32 x, y, z;
    sint32 ftx, fty, ftz;
    sint32 pirahna_attack, fish_Xrange, fish_Zrange;
    PHD_3DPOS pos;
    FISH_INFO *fptr;

    enemy = item = &items[item_number];
    if (!TriggerActive(item))
        return;

    leader = item->hit_points;
    if (!lead_info[leader].on)
        SetupFish(leader, item);

    if (item->object_number == 338) {
        if (CurrentLevel != 6)
            pirahna_attack = (lara_item->room_number == item->room_number);
        else {
            if (CarcassItem != -1)
                pirahna_attack = 2;
            else
                pirahna_attack = (lara_item->room_number == item->room_number);
        }
    } else
        pirahna_attack = 0;

    if (PirahnaHitWait)
        PirahnaHitWait--;

    fptr = (FISH_INFO *)&fish[leader];
    if (pirahna_attack) {
        if (pirahna_attack == 1)
            enemy = lara_item;
        else
            enemy = &items[CarcassItem];
        lead_info[leader].angle = fptr->angle = (-(mGetAngle(fptr->x + item->pos.x_pos, fptr->z + item->pos.z_pos,
                                                       enemy->pos.x_pos, enemy->pos.z_pos) + 0x4000) >> 4) & 4095;
        lead_info[leader].speed = (GetRandomControl() & 63) + 192;
    }

    diff = fptr->angle - lead_info[leader].angle;
    if (diff > 2048)
        diff -= 4096;
    else if (diff < -2048)
        diff += 4096;

    if (diff > 128) {
        fptr->angadd -= 4;
        if (fptr->angadd < -120)
            fptr->angadd = -120;
    } else if (diff < -128) {
        fptr->angadd += 4;
        if (fptr->angadd > 120)
            fptr->angadd = 120;
    } else {
        fptr->angadd -= fptr->angadd >> 2;
        if (abs(fptr->angadd) < 4)
            fptr->angadd = 0;
    }
    fptr->angle += fptr->angadd;
    if (diff > 1024)
        fptr->angle += fptr->angadd >> 2;
    fptr->angle &= 4095;

    diff = fptr->speed - lead_info[leader].speed;
    if (diff < -4) {
        diff = fptr->speed + (GetRandomControl() & 3) + 1;
        if (diff < 0)
            diff = 0;
        fptr->speed = diff;
    } else if (diff > 4) {
        diff = fptr->speed - (GetRandomControl() & 3) - 1;
        if (diff > 255)
            diff = 255;
        fptr->speed = diff;
    }

    fptr->swim += fptr->speed >> 4;
    fptr->swim &= 63;

    z = fptr->z;
    x = fptr->x;
    z += ((rcossin_tbl[(fptr->angle << 1) + 1] * fptr->speed) >> 13);
    x += -((rcossin_tbl[fptr->angle << 1] * fptr->speed) >> 13);

    if (pirahna_attack == 0) {
        fish_Xrange = lead_info[leader].Xrange;
        fish_Zrange = lead_info[leader].Zrange;
        if (z < -fish_Zrange) {
            z = -fish_Zrange;
            if (fptr->angle < 2048)
                lead_info[leader].angle = fptr->angle - ((GetRandomControl() & 127) + 128);
            else
                lead_info[leader].angle = fptr->angle + ((GetRandomControl() & 127) + 128);
            lead_info[leader].angle_time = (GetRandomControl() & 15) + 8;
            lead_info[leader].speed_time = 0;
        } else if (z > fish_Zrange) {
            z = fish_Zrange;
            if (fptr->angle > 3072)
                lead_info[leader].angle = fptr->angle - ((GetRandomControl() & 127) + 128);
            else
                lead_info[leader].angle = fptr->angle + ((GetRandomControl() & 127) + 128);
            lead_info[leader].angle_time = (GetRandomControl() & 15) + 8;
            lead_info[leader].speed_time = 0;
        }
        if (x < -fish_Xrange) {
            x = -fish_Xrange;
            if (fptr->angle < 1024)
                lead_info[leader].angle = fptr->angle - ((GetRandomControl() & 127) + 128);
            else
                lead_info[leader].angle = fptr->angle + ((GetRandomControl() & 127) + 128);
            lead_info[leader].angle_time = (GetRandomControl() & 15) + 8;
            lead_info[leader].speed_time = 0;
        } else if (x > fish_Xrange) {
            x = fish_Xrange;
            if (fptr->angle < 3072)
                lead_info[leader].angle = fptr->angle - ((GetRandomControl() & 127) + 128);
            else
                lead_info[leader].angle = fptr->angle + ((GetRandomControl() & 127) + 128);
            lead_info[leader].angle_time = (GetRandomControl() & 15) + 8;
            lead_info[leader].speed_time = 0;
        }

        if ((GetRandomControl() & 15) == 0)
            lead_info[leader].angle_time = 0;

        if (lead_info[leader].angle_time)
            lead_info[leader].angle_time--;
        else {
            lead_info[leader].angle_time = (GetRandomControl() & 15) + 8;
            angadd = ((GetRandomControl() & 63) + 16) - 8 - 32;
            if ((GetRandomControl() & 3) == 0)
                lead_info[leader].angle += angadd << 5;
            else
                lead_info[leader].angle += angadd;
            lead_info[leader].angle &= 4095;
        }

        if (lead_info[leader].speed_time)
            lead_info[leader].speed_time--;
        else {
            lead_info[leader].speed_time = (GetRandomControl() & 31) + 32;
            if ((GetRandomControl() & 7) == 0)
                lead_info[leader].speed = (GetRandomControl() & 127) + 128;
            else if ((GetRandomControl() & 3) == 0)
                lead_info[leader].speed += (GetRandomControl() & 127) + 32;
            else if (lead_info[leader].speed > 140)
                lead_info[leader].speed -= (GetRandomControl() & 31) + 48;
            else {
                lead_info[leader].speed_time = (GetRandomControl() & 3) + 4;
                lead_info[leader].speed += (GetRandomControl() & 31) - 15;
            }
        }
    }

    fptr->x = ftx = x;
    fty = fptr->y;
    fptr->z = ftz = z;

    fptr = (FISH_INFO *)&fish[MAX_FISH + (leader * 24)];
    for (lp = 0; lp < 24; lp++) {
        if (item->object_number == 338) {
            pos.x_pos = item->pos.x_pos + fptr->x;
            pos.y_pos = item->pos.y_pos + fptr->y;
            pos.z_pos = item->pos.z_pos + fptr->z;
            if (FishNearLara(&pos, 256, (pirahna_attack < 2) ? lara_item : enemy)) {
                if (PirahnaHitWait == 0) {
                    DoBloodSplat(item->pos.x_pos + fptr->x, item->pos.y_pos + fptr->y, item->pos.z_pos + fptr->z, 0, 0,
                        (pirahna_attack < 2) ? lara_item->room_number : enemy->room_number);
                    PirahnaHitWait = 8;
                }
                if (pirahna_attack != 2)
                    lara_item->hit_points -= PIRAHNA_DAMAGE;
            }
        }

        angle = (-(mGetAngle(fptr->x, fptr->z, ftx, ftz) + 0x4000) >> 4) & 4095;
        dx = fptr->x - ftx + ((24 - lp) << 7);
        dz = fptr->z - ftz - ((24 - lp) << 7);
        dx *= dx;
        dz *= dz;

        diff = fptr->angle - angle;
        if (diff > 2048)
            diff -= 4096;
        else if (diff < -2048)
            diff += 4096;

        if (diff > 128) {
            fptr->angadd -= 4;
            if (fptr->angadd < -92 - (lp >> 1))
                fptr->angadd = -92 - (lp >> 1);
        } else if (diff < -128) {
            fptr->angadd += 4;
            if (fptr->angadd > 92 + (lp >> 1))
                fptr->angadd = 92 + (lp >> 1);
        } else {
            fptr->angadd -= fptr->angadd >> 2;
            if (abs(fptr->angadd) < 4)
                fptr->angadd = 0;
        }
        fptr->angle += fptr->angadd;
        if (diff > 1024)
            fptr->angle += fptr->angadd >> 2;
        fptr->angle &= 4095;

        if ((dx + dz) < (0x100000 + ((lp << 7) * (lp << 7)))) {
            if (fptr->speed > 32 + (lp << 1))
                fptr->speed -= fptr->speed >> 5;
        } else {
            if (fptr->speed < 160 + (lp >> 1))
                fptr->speed += (GetRandomControl() & 3) + 1 + (lp >> 1);
            if (fptr->speed > 160 + (lp >> 1) - (lp << 2))
                fptr->speed = 160 + (lp >> 1) - (lp << 2);
        }

        if (GetRandomControl() & 1)
            fptr->speed -= GetRandomControl() & 1;
        else
            fptr->speed += GetRandomControl() & 1;

        if (fptr->speed < 32)
            fptr->speed = 32;
        else if (fptr->speed > 200)
            fptr->speed = 200;

        fptr->swim += (fptr->speed >> 4) + (fptr->speed >> 5);
        fptr->swim &= 63;

        z = fptr->z + ((rcossin_tbl[(fptr->angle << 1) + 1] * fptr->speed) >> 13);
        x = fptr->x - ((rcossin_tbl[fptr->angle << 1] * fptr->speed) >> 13);
        if (z < -32000)
            z = -32000;
        else if (z > 32000)
            z = 32000;
        if (x < -32000)
            x = -32000;
        else if (x > 32000)
            x = 32000;
        fptr->x = x;
        fptr->z = z;

        if (pirahna_attack == 0) {
            if (abs(fptr->y - fptr->desty) < 16)
                fptr->desty = GetRandomControl() % lead_info[leader].Yrange;
        } else {
            y = enemy->pos.y_pos - item->pos.y_pos;
            if (abs(fptr->y - fptr->desty) < 16)
                fptr->desty = y + (GetRandomControl() & 255);
        }
        fptr->y += (fptr->desty - fptr->y) >> 4;
        fptr++;
    }
}

#ifdef TR3_EXE
uint8 jungle_fish_ranges[1][3] = {
    { 8, 20, 3 },
};
uint8 temple_fish_ranges[3][3] = {
    { 4, 4, 2 },
    { 4, 16, 2 },
    { 4, 28, 4 },
};
uint8 quadchase_fish_ranges[8][3] = {
    { 4, 12, 1 },
    { 0, 12, 2 },
    { 8, 4, 2 },
    { 4, 8, 1 },
    { 4, 16, 2 },
    { 4, 24, 1 },
    { 12, 4, 1 },
    { 16, 4, 1 },
};
uint8 house_fish_ranges[7][3] = {
    { 4, 4, 1 },
    { 16, 8, 2 },
    { 24, 8, 2 },
    { 8, 16, 2 },
    { 8, 12, 1 },
    { 20, 8, 2 },
    { 16, 8, 1 },
};
uint8 shore_fish_ranges[3][3] = {
    { 12, 12, 6 },
    { 12, 20, 6 },
    { 20, 4, 8 },
};
uint8 crash_fish_ranges[1][3] = {
    { 20, 4, 6 },
};
uint8 rapids_fish_ranges[2][3] = {
    { 16, 16, 8 },
    { 4, 8, 5 },
};
#endif
