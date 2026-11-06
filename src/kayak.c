#include "specific/standard.h"
#include "game.h"

#define KAYAK_DRAW_SHIFT 32
#define NUM_WAKE_SPRITES 32
#define KAYAK_DROWN_A 25
#define EXTRA_RAPIDSDROWN 12

typedef struct {
    sint32 Vel;
    sint32 Rot;
    sint32 FallSpeedF;
    sint32 FallSpeedL;
    sint32 FallSpeedR;
    sint32 Water;
    PHD_3DPOS OldPos;
    char Turn;
    char Forward;
    char TrueWater;
    char Flags;
} KAYAKINFO;

typedef struct {
    sint32 x[2];
    sint32 y;
    sint32 z[2];
    sint16 xvel[2];
    sint16 zvel[2];
    uint8 life;
    uint8 pad[3];
} WAKE_PTS;

extern WAKE_PTS WakePts[NUM_WAKE_SPRITES][2];

void LaraRapidsDrown(void)
{
    ITEM_INFO *l;

    l = lara_item;
    l->anim_number = objects[VEHICLE_ANIM].anim_index + KAYAK_DROWN_A;
    l->frame_number = anims[l->anim_number].frame_base;
    l->current_anim_state = EXTRA_RAPIDSDROWN;
    l->goal_anim_state = EXTRA_RAPIDSDROWN;
    l->hit_points = 0;
    l->fallspeed = 0;
    l->gravity_status = 0;
    l->speed = 0;
    AnimateItem(l);
    lara.extra_anim = 1;
    lara.gun_status = LG_HANDSBUSY;
    lara.gun_type = LG_UNARMED;
    lara.hit_direction = -1;
}

void KayakInitialise(sint16 item_number)
{
    int i;
    ITEM_INFO *v;
    KAYAKINFO *Kayak;

    v = &items[item_number];
    Kayak = v->data = game_malloc(sizeof(KAYAKINFO), 0);
    Kayak->Vel = Kayak->Rot = Kayak->Flags = 0;
    Kayak->FallSpeedF = Kayak->FallSpeedL = Kayak->FallSpeedR = 0;
    Kayak->OldPos = v->pos;
    for (i = 0; i < NUM_WAKE_SPRITES; i++)
    {
        WakePts[i][0].life = 0;
        WakePts[i][1].life = 0;
    }
}

void KayakDraw(ITEM_INFO *v)
{
    v->pos.y_pos += KAYAK_DRAW_SHIFT;
    DrawAnimatingItem(v);
    v->pos.y_pos -= KAYAK_DRAW_SHIFT;
    S_DrawWakeFX(v);
}

sint32 GetWaterHeight(sint32, sint32, sint32, sint16);
RIPPLE_STRUCT *SetupRipple(sint32 x, sint32 y, sint32 z, sint32 size, sint32 moretrans);

typedef struct {
    sint32 x;
    sint32 y;
    sint32 z;
    sint16 InnerXZoff;
    sint16 InnerXZsize;
    sint16 InnerYsize;
    sint16 InnerXZvel;
    sint16 InnerYvel;
    sint16 InnerGravity;
    sint16 InnerFriction;
    sint16 MiddleXZoff;
    sint16 MiddleXZsize;
    sint16 MiddleYsize;
    sint16 MiddleXZvel;
    sint16 MiddleYvel;
    sint16 MiddleGravity;
    sint16 MiddleFriction;
    sint16 OuterXZoff;
    sint16 OuterXZsize;
    sint16 OuterXZvel;
    sint16 OuterFriction;
} SPLASH_SETUP;

extern SPLASH_SETUP splash_setup;
extern sint32 SplashCount;
void SetupSplash(SPLASH_SETUP *spl);

void KayakSplash(ITEM_INFO *item, sint32 fallspeed, sint32 water)
{
    splash_setup.x = item->pos.x_pos;
    splash_setup.y = water;
    splash_setup.z = item->pos.z_pos;
    splash_setup.InnerXZoff = 16 << 3;
    splash_setup.InnerXZsize = 12 << 2;
    splash_setup.InnerYsize = -96 << 2;
    splash_setup.InnerXZvel = 0xa0;
    splash_setup.InnerYvel = -fallspeed << 5;
    splash_setup.InnerGravity = 0x80;
    splash_setup.InnerFriction = 7;
    splash_setup.MiddleXZoff = 24 << 3;
    splash_setup.MiddleXZsize = 24 << 2;
    splash_setup.MiddleYsize = -64 << 2;
    splash_setup.MiddleXZvel = 0xe0;
    splash_setup.MiddleYvel = -fallspeed << 4;
    splash_setup.MiddleGravity = 0x48;
    splash_setup.MiddleFriction = 8;
    splash_setup.OuterXZoff = 32 << 3;
    splash_setup.OuterXZsize = 32 << 2;
    splash_setup.OuterXZvel = 0x110;
    splash_setup.OuterFriction = -9;
    SetupSplash(&splash_setup);
    SplashCount = 16;
}

int GetInKayak(sint16 item_number, COLL_INFO *coll);
void CreateFlare(int thrown);
void ObjectCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);

void KayakCollision(sint16 item_number, ITEM_INFO *l, COLL_INFO *coll)
{
    int geton;

    if (l->hit_points < 0 || lara.skidoo != NO_ITEM)
        return;

    if ((geton = GetInKayak(item_number, coll))) {
        KAYAKINFO *Kayak;
        ITEM_INFO *v = &items[item_number];

        lara.skidoo = item_number;
        if (lara.gun_type == LG_FLARE) {
            CreateFlare(0);
            undraw_flare_meshes();
            lara.flare_control_left = 0;
            lara.request_gun_type = lara.gun_type = LG_ARMLESS;
        }

        if (geton > 0)
            l->anim_number = objects[VEHICLE_ANIM].anim_index + 3;
        else
            l->anim_number = objects[VEHICLE_ANIM].anim_index + 28;
        l->frame_number = anims[l->anim_number].frame_base;
        l->current_anim_state = l->goal_anim_state = 4;
        lara.water_status = LARA_ABOVEWATER;
        l->pos.x_pos = v->pos.x_pos;
        l->pos.y_pos = v->pos.y_pos;
        l->pos.z_pos = v->pos.z_pos;
        l->pos.y_rot = v->pos.y_rot;
        l->pos.x_rot = l->pos.z_rot = 0;
        l->gravity_status = 0;
        l->speed = 0;
        l->fallspeed = 0;
        if (l->room_number != v->room_number)
            ItemNewRoom(lara.item_number, v->room_number);
        AnimateItem(l);
        Kayak = (KAYAKINFO *)v->data;
        Kayak->Water = v->pos.y_pos;
        Kayak->Flags = 0;
    } else {
        coll->enable_baddie_push = 1;
        ObjectCollision(item_number, l, coll);
    }
}

int GetInKayak(sint16 item_number, COLL_INFO *coll)
{
    int dist;
    sint32 x, z;
    ITEM_INFO *v, *l;
    FLOOR_INFO *floor;
    sint16 room_number;

    l = lara_item;
    if (!(input & IN_ACTION) || lara.gun_status != LG_ARMLESS || l->gravity_status)
        return 0;

    v = &items[item_number];
    x = l->pos.x_pos - v->pos.x_pos;
    z = l->pos.z_pos - v->pos.z_pos;
    dist = x * x + z * z;
    if (dist > 130000)
        return 0;

    room_number = v->room_number;
    floor = GetFloor(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, &room_number);
    if (GetHeight(floor, v->pos.x_pos, v->pos.y_pos, v->pos.z_pos) > -32000) {
        sint16 ang;
        uint16 tempang;

        ang = phd_atan(v->pos.z_pos - l->pos.z_pos, v->pos.x_pos - l->pos.x_pos);
        ang -= v->pos.y_rot;
        tempang = l->pos.y_rot - v->pos.y_rot;
        if (ang > -(ONE_DEGREE * 45) && ang < ONE_DEGREE * 135) {
            if (tempang > 45 * ONE_DEGREE && tempang < 135 * ONE_DEGREE)
                return -1;
        } else {
            if (tempang > 225 * ONE_DEGREE && tempang < 315 * ONE_DEGREE)
                return 1;
        }
    }
    return 0;
}

sint32 GetFreeSpark(void);

void TriggerRapidsMist(sint32 x, sint32 y, sint32 z)
{
    SPARKS *sptr;
    sint32 xsize;

    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sR = 128;
    sptr->sG = 128;
    sptr->sB = 128;
    sptr->dR = 192;
    sptr->dG = 192;
    sptr->dB = 192;
    sptr->ColFadeSpeed = 2;
    sptr->FadeToBlack = 4;
    sptr->sLife = sptr->Life = 6 + (GetRandomControl() & 3);
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = x + ((GetRandomControl() & 15) - 8);
    sptr->y = y + ((GetRandomControl() & 15) - 8);
    sptr->z = z + ((GetRandomControl() & 15) - 8);
    sptr->Xvel = (GetRandomControl() & 255) - 128;
    sptr->Zvel = (GetRandomControl() & 255) - 128;
    sptr->Yvel = (GetRandomControl() & 255) - 128;
    sptr->Friction = 3;
    if (GetRandomControl() & 1) {
        sptr->Flags = 2 | 8 | 16 | 512;
        sptr->RotAng = GetRandomControl() & 4095;
        if (GetRandomControl() & 1)
            sptr->RotAdd = -(GetRandomControl() & 15) - 16;
        else
            sptr->RotAdd = (GetRandomControl() & 15) + 16;
    } else {
        sptr->Flags = 2 | 8 | 512;
    }
    sptr->Def = objects[296].mesh_index;
    sptr->Scalar = 4;
    sptr->Gravity = 0;
    sptr->MaxYvel = 0;
    xsize = (GetRandomControl() & 7) + 16;
    sptr->Width = sptr->sWidth = xsize >> 1;
    sptr->dWidth = xsize;
    sptr->Height = sptr->sHeight = xsize >> 1;
    sptr->dHeight = xsize;
}

void DoRipple(ITEM_INFO *v, sint16 xoff, sint16 zoff)
{
    sint16 room_number;
    FLOOR_INFO *floor;
    RIPPLE_STRUCT *r;
    sint32 s, c, x, z;

    c = phd_cos(v->pos.y_rot);
    s = phd_sin(v->pos.y_rot);
    x = v->pos.x_pos + ((s * zoff + c * xoff) >> W2V_SHIFT);
    z = v->pos.z_pos + ((c * zoff - s * xoff) >> W2V_SHIFT);
    room_number = v->room_number;
    floor = GetFloor(x, v->pos.y_pos, z, &room_number);
    if (GetWaterHeight(x, v->pos.y_pos, z, room_number) != NO_HEIGHT) {
        r = SetupRipple(x, v->pos.y_pos, z, -2 - (GetRandomControl() & 1), 0);
        r->init = 0;
    }
}

extern sint16 rcossin_tbl[];

sint32 mGetAngle(sint32 x1, sint32 y1, sint32 x2, sint32 y2);
int abs(int x);
#pragma intrinsic(abs)

void DoCurrent(ITEM_INFO *item)
{
    ROOM_INFO *r;
    PHD_VECTOR target;
    sint32 shifter, absvel;
    sint32 angle, dx, dz, speed, sinkval;

    r = &room[item->room_number];
    if (!lara.current_active) {
        absvel = abs(lara.current_xvel);
        if (absvel > 16)
            shifter = 4;
        else if (absvel > 8)
            shifter = 3;
        else
            shifter = 2;
        lara.current_xvel -= lara.current_xvel >> shifter;
        if (abs(lara.current_xvel) < 4)
            lara.current_xvel = 0;
        absvel = abs(lara.current_zvel);
        if (absvel > 16)
            shifter = 4;
        else if (absvel > 8)
            shifter = 3;
        else
            shifter = 2;
        lara.current_zvel -= lara.current_zvel >> shifter;
        if (abs(lara.current_zvel) < 4)
            lara.current_zvel = 0;
        if (lara.current_xvel == 0 && lara.current_zvel == 0)
            return;
    } else {
        sinkval = lara.current_active - 1;
        target.x = camera.fixed[sinkval].x;
        target.y = camera.fixed[sinkval].y;
        target.z = camera.fixed[sinkval].z;
        angle = ((mGetAngle(target.x, target.z, lara_item->pos.x_pos, lara_item->pos.z_pos) - 0x4000) >> 4) & 4095;
        dx = target.x - lara_item->pos.x_pos;
        dz = target.z - lara_item->pos.z_pos;
        speed = camera.fixed[sinkval].data;
        dx = (rcossin_tbl[angle << 1] * speed) >> 2;
        dz = (rcossin_tbl[(angle << 1) + 1] * speed) >> 2;
        lara.current_xvel += (dx - lara.current_xvel) >> 4;
        lara.current_zvel += (dz - lara.current_zvel) >> 4;
    }
    item->pos.x_pos += lara.current_xvel >> 8;
    item->pos.z_pos += lara.current_zvel >> 8;
    lara.current_active = 0;
}

int TestBoundsCollide(ITEM_INFO *item, ITEM_INFO *laraitem, sint32 radius);
void DoLotsOfBlood(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num, int number);

void KayakToBaddieCollision(ITEM_INFO *v)
{
    int i;
    sint16 roomies[20], numroom, *door;
    sint16 item_num, nex;
    ITEM_INFO *item;
    OBJECT_INFO *object;
    int x, y, z;

    numroom = 1;
    roomies[0] = v->room_number;
    if ((door = room[v->room_number].door)) {
        for (i = (int)*(door++); i > 0; i--) {
            roomies[numroom++] = *door;
            door += 16;
        }
    }
    for (i = 0; i < numroom; i++) {
        item_num = room[roomies[i]].item_number;
        while (item_num != NO_ITEM) {
            item = &items[item_num];
            nex = item->next_item;
            if (item->collidable && item->status != INVISIBLE) {
                object = &objects[item->object_number];
                if (object->collision &&
                    (item->object_number == 87 || item->object_number == 90 || item->object_number == 94 ||
                     (item->object_number == 111 && item->current_anim_state != 1) ||
                     (item->object_number == 113 && item->current_anim_state != 3))) {
                    x = v->pos.x_pos - item->pos.x_pos;
                    y = v->pos.y_pos - item->pos.y_pos;
                    z = v->pos.z_pos - item->pos.z_pos;
                    if (x > -2048 && x < 2048 && z > -2048 && z < 2048 && y > -2048 && y < 2048) {
                        if (TestBoundsCollide(item, v, 256)) {
                            DoLotsOfBlood(lara_item->pos.x_pos, lara_item->pos.y_pos - 256, lara_item->pos.z_pos,
                                          v->speed, v->pos.y_rot, lara_item->room_number, 3);
                            lara_item->hit_points -= 5;
                        }
                    }
                }
            }
            item_num = nex;
        }
    }
}

extern uint8 WakeShade;
extern char kayak_mist_cnt;
extern sint16 kayak_mist_zpos[10];
extern sint16 kayak_mist_xpos[10];
extern sint32 wibble;

void KayakUserInput(ITEM_INFO *v, ITEM_INFO *l, KAYAKINFO *Kayak);
void KayakToBackground(ITEM_INFO *v, KAYAKINFO *Kayak);
void DoWake_kayak(ITEM_INFO *v, sint16 xoff, sint16 zoff, sint16 rotate);
void UpdateWakeFX_kayak(void);
void AnimateItem(ITEM_INFO *item);
void phd_PushUnitMatrix(void);
void phd_RotYXZ(sint16 ry, sint16 rx, sint16 rz);

int KayakControl(void)
{
    sint32 h;
    KAYAKINFO *Kayak;
    ITEM_INFO *v, *l;
    FLOOR_INFO *floor;
    sint32 ofs, water, lp;
    sint16 room_number;
    PHD_VECTOR dest;
    sint32 *mptr;

    l = lara_item;
    v = &items[lara.skidoo];
    Kayak = (KAYAKINFO *)v->data;
    if (input & IN_LOOK)
        LookUpDown();

    ofs = v->fallspeed;
    KayakUserInput(v, l, Kayak);
    KayakToBackground(v, Kayak);

    room_number = v->room_number;
    floor = GetFloor(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, &room_number);
    h = GetHeight(floor, v->pos.x_pos, v->pos.y_pos, v->pos.z_pos);
    TestTriggers(trigger_index, 0);

    if ((Kayak->Water = water = GetWaterHeight(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, room_number)) == NO_HEIGHT) {
        Kayak->Water = water = h;
        Kayak->TrueWater = 0;
    } else {
        Kayak->Water -= 5;
        Kayak->TrueWater = 1;
    }

    if ((ofs - v->fallspeed) > 128 && v->fallspeed == 0 && water != NO_HEIGHT) {
        sint32 damage;

        if ((damage = ofs - v->fallspeed) > 160)
            l->hit_points -= (damage - 160) << 3;
        KayakSplash(v, ofs - v->fallspeed, water);
    }

    if (lara.skidoo != NO_ITEM) {
        if (v->room_number != room_number) {
            ItemNewRoom(lara.skidoo, room_number);
            ItemNewRoom(lara.item_number, room_number);
        }
        l->pos.x_pos = v->pos.x_pos;
        l->pos.y_pos = v->pos.y_pos + 32;
        l->pos.z_pos = v->pos.z_pos;
        l->pos.x_rot = v->pos.x_rot;
        l->pos.y_rot = v->pos.y_rot;
        l->pos.z_rot = v->pos.z_rot >> 1;
        AnimateItem(l);
        v->anim_number = objects[14].anim_index + (l->anim_number - objects[VEHICLE_ANIM].anim_index);
        v->frame_number = anims[v->anim_number].frame_base + (l->frame_number - anims[l->anim_number].frame_base);
        camera.target_elevation = -30 * ONE_DEGREE;
        camera.target_distance = WALL_L * 2;
    }

    if (!(wibble & 15) && Kayak->TrueWater) {
        DoWake_kayak(v, -128, 0, 0);
        DoWake_kayak(v, 128, 0, 1);
    }

    if (wibble & 7) {
        if (!Kayak->TrueWater && v->fallspeed < 20) {
            kayak_mist_cnt ^= 1;
            for (lp = kayak_mist_cnt; lp < 10; lp += 2) {
                if (GetRandomControl() & 1)
                    dest.x = kayak_mist_xpos[lp] >> 1;
                else
                    dest.x = -(kayak_mist_xpos[lp] >> 1);
                dest.y = 50;
                dest.z = kayak_mist_zpos[lp];
                phd_PushUnitMatrix();
                mptr = phd_mxptr;
                phd_RotYXZ(v->pos.y_rot, v->pos.x_rot, v->pos.z_rot);
                dest.x = v->pos.x_pos + ((*(mptr + 0) * dest.x + *(mptr + 1) * dest.y + *(mptr + 2) * dest.z) >> W2V_SHIFT);
                dest.y = v->pos.y_pos + ((*(mptr + 4) * dest.x + *(mptr + 5) * dest.y + *(mptr + 6) * dest.z) >> W2V_SHIFT);
                dest.z = v->pos.z_pos + ((*(mptr + 8) * dest.x + *(mptr + 9) * dest.y + *(mptr + 10) * dest.z) >> W2V_SHIFT);
                TriggerRapidsMist(dest.x, dest.y, dest.z);
                phd_PopMatrix();
            }
        }
    }

    if (v->speed == 0 && !lara.current_xvel && !lara.current_zvel) {
        if (WakeShade)
            WakeShade--;
    } else {
        if (WakeShade < 16)
            WakeShade++;
    }

    UpdateWakeFX_kayak();
    KayakToBaddieCollision(v);
    return (lara.skidoo != NO_ITEM) ? 1 : 0;
}

void UpdateWakeFX_kayak(void)
{
    int i, j;

    for (i = 0; i < 2; i++) {
        for (j = 0; j < NUM_WAKE_SPRITES; j++) {
            if (WakePts[j][i].life) {
                WakePts[j][i].life--;
                WakePts[j][i].x[0] += WakePts[j][i].xvel[0];
                WakePts[j][i].z[0] += WakePts[j][i].zvel[0];
                WakePts[j][i].x[1] += WakePts[j][i].xvel[1];
                WakePts[j][i].z[1] += WakePts[j][i].zvel[1];
            }
        }
    }
}

int GetCollisionAnim_kayak(ITEM_INFO *v, sint32 x, sint32 z)
{
    sint32 s, c, front, side;

    x = v->pos.x_pos - x;
    z = v->pos.z_pos - z;
    if (!x && !z)
        return 0;

    c = phd_cos(v->pos.y_rot);
    s = phd_sin(v->pos.y_rot);
    front = (z * c + x * s) >> W2V_SHIFT;
    side = (x * c - z * s) >> W2V_SHIFT;
    if ((front < 0 ? -front : front) > (side < 0 ? -side : side)) {
        if (front > 0)
            return 1;
        else
            return 2;
    } else {
        if (side > 0)
            return 3;
        else
            return 4;
    }
}

sint32 DoDynamics_kayak(sint32 height, sint32 fallspeed, sint32 *y)
{
    sint32 kick;

    if (height > *y) {
        *y += fallspeed;
        if (*y > height) {
            *y = height;
            fallspeed = 0;
        } else {
            fallspeed += 6;
        }
    } else {
        kick = (height - *y) << 2;
        if (kick < -80)
            kick = -80;
        fallspeed += (kick - fallspeed) >> 3;
        if (*y > height)
            *y = height;
    }
    return fallspeed;
}

sint32 TestHeight_kayak(ITEM_INFO *v, sint32 x, sint32 z, PHD_VECTOR *pos);

int CanGetOut_kayak(ITEM_INFO *v, int direction)
{
    PHD_VECTOR pos;
    sint32 height;

    height = TestHeight_kayak(v, (direction < 0) ? -768 : 768, 0, &pos);
    if (v->pos.y_pos - height > 0)
        return 0;
    return 1;
}

FLOOR_INFO *GetFloor(sint32 x, sint32 y, sint32 z, sint16 *room_number);
sint32 GetHeight(FLOOR_INFO *floor, sint32 x, sint32 y, sint32 z);
sint32 TestHeight_kayak(ITEM_INFO *v, sint32 x, sint32 z, PHD_VECTOR *pos);
sint32 DoDynamics_kayak(sint32 height, sint32 fallspeed, sint32 *y);

sint32 TestHeight_kayak(ITEM_INFO *v, sint32 x, sint32 z, PHD_VECTOR *pos)
{
    sint32 s, c, h;
    sint16 room_number;
    FLOOR_INFO *floor;

    c = phd_cos(v->pos.y_rot);
    s = phd_sin(v->pos.y_rot);
    pos->x = v->pos.x_pos + ((z * s + x * c) >> W2V_SHIFT);
    pos->y = v->pos.y_pos + ((x * phd_sin(v->pos.z_rot)) >> W2V_SHIFT) - ((z * phd_sin(v->pos.x_rot)) >> W2V_SHIFT);
    pos->z = v->pos.z_pos + ((z * c - x * s) >> W2V_SHIFT);
    room_number = v->room_number;
    GetFloor(pos->x, pos->y, pos->z, &room_number);
    h = GetWaterHeight(pos->x, pos->y, pos->z, room_number);
    if (h == NO_HEIGHT) {
        room_number = v->room_number;
        floor = GetFloor(pos->x, pos->y, pos->z, &room_number);
        h = GetHeight(floor, pos->x, pos->y, pos->z);
        if (h == NO_HEIGHT)
            return h;
    }
    return h - 5;
}

int DoShift_kayak(ITEM_INFO *v, PHD_VECTOR *pos, PHD_VECTOR *old)
{
    sint32 x, z, x_old, z_old, shift_x, shift_z;

    x = pos->x >> WALL_SHIFT;
    z = pos->z >> WALL_SHIFT;
    x_old = old->x >> WALL_SHIFT;
    z_old = old->z >> WALL_SHIFT;
    shift_x = pos->x & (WALL_L - 1);
    shift_z = pos->z & (WALL_L - 1);
    if (x == x_old) {
        old->x = 0;
        if (z == z_old) {
            v->pos.z_pos += old->z - pos->z;
            v->pos.x_pos += old->x - pos->x;
        } else if (z > z_old) {
            v->pos.z_pos -= shift_z + 1;
            return pos->x - v->pos.x_pos;
        } else {
            v->pos.z_pos += WALL_L - shift_z;
            return v->pos.x_pos - pos->x;
        }
    } else if (z == z_old) {
        old->z = 0;
        if (x > x_old) {
            v->pos.x_pos -= shift_x + 1;
            return v->pos.z_pos - pos->z;
        } else {
            v->pos.x_pos += WALL_L - shift_x;
            return pos->z - v->pos.z_pos;
        }
    } else {
        sint16 room_number;
        FLOOR_INFO *floor;
        sint32 height;

        x = z = 0;
        room_number = v->room_number;
        floor = GetFloor(old->x, pos->y, pos->z, &room_number);
        height = GetHeight(floor, old->x, pos->y, pos->z);
        if (height < old->y - STEP_L) {
            if (pos->z > old->z)
                z = -shift_z - 1;
            else
                z = WALL_L - shift_z;
        }
        room_number = v->room_number;
        floor = GetFloor(pos->x, pos->y, old->z, &room_number);
        height = GetHeight(floor, pos->x, pos->y, old->z);
        if (height < old->y - STEP_L) {
            if (pos->x > old->x)
                x = -shift_x - 1;
            else
                x = WALL_L - shift_x;
        }
        if (x && z) {
            v->pos.z_pos += z;
            v->pos.x_pos += x;
        } else if (z) {
            v->pos.z_pos += z;
            if (z > 0)
                return v->pos.x_pos - pos->x;
            else
                return pos->x - v->pos.x_pos;
        } else if (x) {
            v->pos.x_pos += x;
            if (x > 0)
                return pos->z - v->pos.z_pos;
            else
                return v->pos.z_pos - pos->z;
        } else {
            v->pos.z_pos += old->z - pos->z;
            v->pos.x_pos += old->x - pos->x;
        }
    }
    return 0;
}

sint32 CameraCollisionBounds(GAME_VECTOR *ideal, sint32 push, sint32 yfirst);
sint32 TestHeight_kayak(ITEM_INFO *v, sint32 x, sint32 z, PHD_VECTOR *pos);
int DoShift_kayak(ITEM_INFO *v, PHD_VECTOR *pos, PHD_VECTOR *old);
sint32 DoDynamics_kayak(sint32 height, sint32 fallspeed, sint32 *y);

void KayakToBackground(ITEM_INFO *v, KAYAKINFO *Kayak)
{
    sint32 h, slip = 0, rot = 0;
    PHD_VECTOR pos;
    int i, oldx, oldz;
    FLOOR_INFO *floor;
    sint16 room_number;
    sint32 height[8];
    PHD_VECTOR oldpos[9];
    sint32 x, z;
    sint32 fh, lh, rh;
    PHD_VECTOR fpos, lpos, rpos;

    Kayak->OldPos = v->pos;
    height[0] = TestHeight_kayak(v, 0, 1024, &oldpos[0]);
    height[1] = TestHeight_kayak(v, -96, 512, &oldpos[1]);
    height[2] = TestHeight_kayak(v, 96, 512, &oldpos[2]);
    height[3] = TestHeight_kayak(v, -128, 128, &oldpos[3]);
    height[4] = TestHeight_kayak(v, 128, 128, &oldpos[4]);
    height[5] = TestHeight_kayak(v, -128, -320, &oldpos[5]);
    height[6] = TestHeight_kayak(v, 128, -320, &oldpos[6]);
    height[7] = TestHeight_kayak(v, 0, -640, &oldpos[7]);
    for (i = 0; i < 8; i++) {
        if (oldpos[i].y > height[i])
            oldpos[i].y = height[i];
    }
    oldpos[8].x = v->pos.x_pos;
    oldpos[8].y = v->pos.y_pos;
    oldpos[8].z = v->pos.z_pos;
    fh = TestHeight_kayak(v, 0, 1024, &fpos);
    lh = TestHeight_kayak(v, -128, 128, &lpos);
    rh = TestHeight_kayak(v, 128, 128, &rpos);
    v->pos.y_rot += Kayak->Rot >> 16;
    v->pos.x_pos += (v->speed * phd_sin(v->pos.y_rot)) >> W2V_SHIFT;
    v->pos.z_pos += (v->speed * phd_cos(v->pos.y_rot)) >> W2V_SHIFT;
    DoCurrent(v);
    Kayak->FallSpeedL = DoDynamics_kayak(lh, Kayak->FallSpeedL, &lpos.y);
    Kayak->FallSpeedR = DoDynamics_kayak(rh, Kayak->FallSpeedR, &rpos.y);
    Kayak->FallSpeedF = DoDynamics_kayak(fh, Kayak->FallSpeedF, &fpos.y);
    v->fallspeed = DoDynamics_kayak(Kayak->Water, v->fallspeed, &v->pos.y_pos);
    h = (lpos.y + rpos.y) >> 1;
    x = phd_atan(1024, v->pos.y_pos - fpos.y);
    z = phd_atan(128, h - lpos.y);
    v->pos.x_rot = x;
    v->pos.z_rot = z;
    oldx = v->pos.x_pos;
    oldz = v->pos.z_pos;
    if ((h = TestHeight_kayak(v, 0, -640, &pos)) < oldpos[7].y - 64)
        rot = DoShift_kayak(v, &pos, &oldpos[7]);
    if ((h = TestHeight_kayak(v, 128, -320, &pos)) < oldpos[6].y - 64)
        rot += DoShift_kayak(v, &pos, &oldpos[6]);
    if ((h = TestHeight_kayak(v, -128, -320, &pos)) < oldpos[5].y - 64)
        rot += DoShift_kayak(v, &pos, &oldpos[5]);
    if ((h = TestHeight_kayak(v, 128, 128, &pos)) < oldpos[4].y - 64)
        rot += DoShift_kayak(v, &pos, &oldpos[4]);
    if ((h = TestHeight_kayak(v, -128, 128, &pos)) < oldpos[3].y - 64)
        rot += DoShift_kayak(v, &pos, &oldpos[3]);
    if ((h = TestHeight_kayak(v, 96, 512, &pos)) < oldpos[2].y - 64)
        rot += DoShift_kayak(v, &pos, &oldpos[2]);
    if ((h = TestHeight_kayak(v, -96, 512, &pos)) < oldpos[1].y - 64)
        rot += DoShift_kayak(v, &pos, &oldpos[1]);
    if ((h = TestHeight_kayak(v, 0, 1024, &pos)) < oldpos[0].y - 64)
        rot += DoShift_kayak(v, &pos, &oldpos[0]);
    v->pos.y_rot += rot;
    room_number = v->room_number;
    floor = GetFloor(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, &room_number);
    h = GetWaterHeight(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, room_number);
    if (h == NO_HEIGHT)
        h = GetHeight(floor, v->pos.x_pos, v->pos.y_pos, v->pos.z_pos);
    if (h < v->pos.y_pos - 64)
        DoShift_kayak(v, (PHD_VECTOR *)&v->pos, &oldpos[8]);
    room_number = v->room_number;
    floor = GetFloor(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, &room_number);
    h = GetWaterHeight(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, room_number);
    if (h == NO_HEIGHT)
        h = GetHeight(floor, v->pos.x_pos, v->pos.y_pos, v->pos.z_pos);
    if (h == NO_HEIGHT) {
        GAME_VECTOR kpos;

        kpos.x = Kayak->OldPos.x_pos;
        kpos.y = Kayak->OldPos.y_pos;
        kpos.z = Kayak->OldPos.z_pos;
        kpos.room_number = v->room_number;
        CameraCollisionBounds(&kpos, 256, 0);
        v->pos.x_pos = kpos.x;
        v->pos.y_pos = kpos.y;
        v->pos.z_pos = kpos.z;
        v->room_number = kpos.room_number;
    }
    i = GetCollisionAnim_kayak(v, oldx, oldz);
    if (slip || i) {
        sint32 newspeed;

        newspeed = ((v->pos.x_pos - oldpos[8].x) * phd_sin(v->pos.y_rot) + (v->pos.z_pos - oldpos[8].z) * phd_cos(v->pos.y_rot)) >> W2V_SHIFT;
        newspeed <<= 8;
        if (slip) {
            if (Kayak->Vel <= 0x380000)
                Kayak->Vel = newspeed;
        } else {
            if (Kayak->Vel > 0 && newspeed < Kayak->Vel)
                Kayak->Vel = newspeed;
            else if (Kayak->Vel < 0 && newspeed > Kayak->Vel)
                Kayak->Vel = newspeed;
        }
        if (Kayak->Vel < -0x380000)
            Kayak->Vel = -0x380000;
    }
}

#define LARA_LEG_BITS ((1 << HIPS) | (1 << THIGH_L) | (1 << CALF_L) | (1 << FOOT_L) | (1 << THIGH_R) | (1 << CALF_R) | (1 << FOOT_R))

int CanGetOut_kayak(ITEM_INFO *v, int direction);
void GetLaraHandAbsPosition(PHD_VECTOR *vec, int joint);

void KayakUserInput(ITEM_INFO *v, ITEM_INFO *l, KAYAKINFO *Kayak)
{
    static char lr;
    sint16 frame;
    sint16 *tmp;
    PHD_VECTOR vec;

    if (l->hit_points <= 0 && l->current_anim_state != 5) {
        l->anim_number = objects[VEHICLE_ANIM].anim_index + 5;
        l->frame_number = anims[l->anim_number].frame_base;
        l->current_anim_state = l->goal_anim_state = 5;
    }
    frame = l->frame_number - anims[l->anim_number].frame_base;
    switch (l->current_anim_state) {
    case 1:
        if ((input & IN_ROLL) && !lara.current_active && !lara.current_xvel && !lara.current_zvel) {
            if ((input & IN_LEFT) && CanGetOut_kayak(v, -1)) {
                l->goal_anim_state = 9;
                l->required_anim_state = 13;
            } else if ((input & IN_RIGHT) && CanGetOut_kayak(v, 1)) {
                l->goal_anim_state = 9;
                l->required_anim_state = 14;
            }
        } else if (input & IN_FORWARD) {
            l->goal_anim_state = 3;
            Kayak->Turn = 0;
            Kayak->Forward = 1;
        } else if (input & IN_BACK) {
            l->goal_anim_state = 0;
        } else if (input & IN_LEFT) {
            l->goal_anim_state = 2;
            if (Kayak->Vel)
                Kayak->Turn = 0;
            else
                Kayak->Turn = 1;
            Kayak->Forward = 0;
        } else if (input & IN_RIGHT) {
            l->goal_anim_state = 3;
            if (Kayak->Vel)
                Kayak->Turn = 0;
            else
                Kayak->Turn = 1;
            Kayak->Forward = 0;
        } else if ((input & IN_STEPL) && (Kayak->Vel || lara.current_xvel || lara.current_zvel)) {
            l->goal_anim_state = 10;
        } else if ((input & IN_STEPR) && (Kayak->Vel || lara.current_xvel || lara.current_zvel)) {
            l->goal_anim_state = 11;
        }
        break;
    case 2:
        if (Kayak->Forward) {
            if (!frame)
                lr = 0;
            if (frame == 2 && !(lr & 0x80))
                lr++;
            else if (frame > 2)
                lr &= ~0x80;
            if (input & IN_FORWARD) {
                if (input & IN_LEFT) {
                    if ((lr & ~0x80) >= 2)
                        l->goal_anim_state = 3;
                } else {
                    l->goal_anim_state = 3;
                }
            } else {
                l->goal_anim_state = 1;
            }
        } else if (!(input & IN_LEFT)) {
            l->goal_anim_state = 1;
        }
        if (frame == 7) {
            if (Kayak->Forward) {
                if ((Kayak->Rot -= 0x800000) < -0x1000000)
                    Kayak->Rot = -0x1000000;
                Kayak->Vel += 0x180000;
            } else if (Kayak->Turn) {
                if ((Kayak->Rot -= 0x1000000) < -0x1000000)
                    Kayak->Rot = -0x1000000;
            } else {
                if ((Kayak->Rot -= 0xc00000) < -0xc00000)
                    Kayak->Rot = -0xc00000;
                Kayak->Vel += 0x100000;
            }
        }
        if (frame > 6 && frame < 24 && (frame & 1))
            DoRipple(v, -384, -64);
        break;
    case 3:
        if (Kayak->Forward) {
            if (!frame)
                lr = 0;
            if (frame == 2 && !(lr & 0x80))
                lr++;
            else if (frame > 2)
                lr &= ~0x80;
            if (input & IN_FORWARD) {
                if (input & IN_RIGHT) {
                    if ((lr & ~0x80) >= 2)
                        l->goal_anim_state = 2;
                } else {
                    l->goal_anim_state = 2;
                }
            } else {
                l->goal_anim_state = 1;
            }
        } else if (!(input & IN_RIGHT)) {
            l->goal_anim_state = 1;
        }
        if (frame == 7) {
            if (Kayak->Forward) {
                if ((Kayak->Rot += 0x800000) > 0x1000000)
                    Kayak->Rot = 0x1000000;
                Kayak->Vel += 0x180000;
            } else if (Kayak->Turn) {
                if ((Kayak->Rot += 0x1000000) > 0x1000000)
                    Kayak->Rot = 0x1000000;
            } else {
                if ((Kayak->Rot += 0xc00000) > 0xc00000)
                    Kayak->Rot = 0xc00000;
                Kayak->Vel += 0x100000;
            }
        }
        if (frame > 6 && frame < 24 && (frame & 1))
            DoRipple(v, 384, -64);
        break;
    case 0:
        if (!(input & IN_BACK))
            l->goal_anim_state = 1;
        if (l->anim_number - objects[VEHICLE_ANIM].anim_index == 2) {
            if (frame == 8) {
                Kayak->Rot += 0x800000;
                Kayak->Vel -= 0x180000;
            }
            if (frame == 31) {
                Kayak->Rot -= 0x800000;
                Kayak->Vel -= 0x180000;
            }
            if (frame < 15 && (frame & 1))
                DoRipple(v, 384, -128);
            else if (frame >= 20 && frame <= 34 && (frame & 1))
                DoRipple(v, -384, -128);
        }
        break;
    case 10:
        if (!(input & IN_STEPL) || (!Kayak->Vel && !lara.current_xvel && !lara.current_zvel)) {
            l->goal_anim_state = 1;
        } else if (l->anim_number - objects[VEHICLE_ANIM].anim_index == 26) {
            if (Kayak->Vel >= 0) {
                if ((Kayak->Rot -= 0x200000) < -0x1000000)
                    Kayak->Rot = -0x1000000;
                if ((Kayak->Vel += -0x8000) < 0)
                    Kayak->Vel = 0;
            }
            if (Kayak->Vel < 0) {
                Kayak->Rot += 0x200000;
                if ((Kayak->Vel += 0x8000) > 0)
                    Kayak->Vel = 0;
            }
            if (!(wibble & 3))
                DoRipple(v, -256, -256);
        }
        break;
    case 11:
        if (!(input & IN_STEPR) || (!Kayak->Vel && !lara.current_xvel && !lara.current_zvel)) {
            l->goal_anim_state = 1;
        } else if (l->anim_number - objects[VEHICLE_ANIM].anim_index == 27) {
            if (Kayak->Vel >= 0) {
                if ((Kayak->Rot += 0x200000) > 0x1000000)
                    Kayak->Rot = 0x1000000;
                if ((Kayak->Vel += -0x8000) < 0)
                    Kayak->Vel = 0;
            }
            if (Kayak->Vel < 0) {
                Kayak->Rot -= 0x200000;
                if ((Kayak->Vel += 0x8000) > 0)
                    Kayak->Vel = 0;
            }
            if (!(wibble & 3))
                DoRipple(v, 256, -256);
        }
        break;
    case 4:
        if (l->anim_number == objects[VEHICLE_ANIM].anim_index + 4 && frame == 24 && !(Kayak->Flags & 0x80)) {
            tmp = lara.mesh_ptrs[HAND_R];
            lara.mesh_ptrs[HAND_R] = meshes[objects[VEHICLE_ANIM].mesh_index + HAND_R];
            meshes[objects[VEHICLE_ANIM].mesh_index + HAND_R] = tmp;
            l->mesh_bits &= ~LARA_LEG_BITS;
            Kayak->Flags |= 0x80;
        }
        break;
    case 9:
        if (l->anim_number == objects[VEHICLE_ANIM].anim_index + 14 && frame == 27 && (Kayak->Flags & 0x80)) {
            tmp = lara.mesh_ptrs[HAND_R];
            lara.mesh_ptrs[HAND_R] = meshes[objects[VEHICLE_ANIM].mesh_index + HAND_R];
            meshes[objects[VEHICLE_ANIM].mesh_index + HAND_R] = tmp;
            l->mesh_bits |= LARA_LEG_BITS;
            Kayak->Flags &= ~0x80;
        }
        l->goal_anim_state = l->required_anim_state;
        break;
    case 13:
        if (l->anim_number == objects[VEHICLE_ANIM].anim_index + 24 && frame == 83) {
            vec.x = 0;
            vec.y = 350;
            vec.z = 500;
            GetLaraHandAbsPosition(&vec, 2);
            l->pos.x_pos = vec.x;
            l->pos.y_pos = vec.y;
            l->pos.z_pos = vec.z;
            l->pos.x_rot = 0;
            l->pos.y_rot = v->pos.y_rot - 0x4000;
            l->pos.z_rot = 0;
            l->anim_number = 23;
            l->frame_number = anims[23].frame_base;
            l->current_anim_state = l->goal_anim_state = AS_FASTFALL;
            l->fallspeed = 0;
            l->gravity_status = 1;
            lara.gun_status = LG_ARMLESS;
            lara.skidoo = NO_ITEM;
        }
        break;
    case 14:
        if (l->anim_number == objects[VEHICLE_ANIM].anim_index + 32 && frame == 83) {
            vec.x = 0;
            vec.y = 350;
            vec.z = 500;
            GetLaraHandAbsPosition(&vec, 2);
            l->pos.x_pos = vec.x;
            l->pos.y_pos = vec.y;
            l->pos.z_pos = vec.z;
            l->pos.x_rot = 0;
            l->pos.y_rot = v->pos.y_rot + 0x4000;
            l->pos.z_rot = 0;
            l->anim_number = 23;
            l->frame_number = anims[23].frame_base;
            l->current_anim_state = l->goal_anim_state = AS_FASTFALL;
            l->fallspeed = 0;
            l->gravity_status = 1;
            lara.gun_status = LG_ARMLESS;
            lara.skidoo = NO_ITEM;
        }
    }
    if (Kayak->Vel > 0) {
        if ((Kayak->Vel -= 0x8000) < 0)
            Kayak->Vel = 0;
    } else if (Kayak->Vel < 0) {
        if ((Kayak->Vel += 0x8000) > 0)
            Kayak->Vel = 0;
    }
    if (Kayak->Vel > 0x380000)
        Kayak->Vel = 0x380000;
    else if (Kayak->Vel < -0x380000)
        Kayak->Vel = -0x380000;
    v->speed = Kayak->Vel >> 16;
    if (Kayak->Rot >= 0) {
        if ((Kayak->Rot -= 0x50000) < 0)
            Kayak->Rot = 0;
    } else if (Kayak->Rot < 0) {
        if ((Kayak->Rot += 0x50000) > 0)
            Kayak->Rot = 0;
    }
}
