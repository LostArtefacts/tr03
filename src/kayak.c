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

sint32 GetWaterHeight(sint32 x, sint32 y, sint32 z, sint16 room_number);
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
