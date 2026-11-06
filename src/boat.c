#include <math.h>
#include <direct.h>
#include "game.h"

#define GRAVITY 6
#define NUM_WAKE_SPRITES 32

typedef struct {
    sint32 boat_turn;
    sint32 left_fallspeed;
    sint32 right_fallspeed;
    sint16 tilt_angle;
    sint16 extra_rotation;
    sint32 water;
    sint32 pitch;
    sint16 prop_rot;
} BOAT_INFO;

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

void InitialiseBoat(sint16 item_number)
{
    ITEM_INFO *boat;
    BOAT_INFO *binfo;
    int i;

    boat = &items[item_number];
    binfo = boat->data = game_malloc(sizeof(BOAT_INFO), 0);
    binfo->boat_turn = 0;
    binfo->left_fallspeed = binfo->right_fallspeed = 0;
    binfo->tilt_angle = 0;
    binfo->extra_rotation = 0;
    binfo->water = 0;
    binfo->pitch = 0;
    for (i = 0; i < NUM_WAKE_SPRITES; i++)
    {
        WakePts[i][0].life = 0;
        WakePts[i][1].life = 0;
    }
}

int BoatCheckGeton(sint16 item_number, COLL_INFO *coll);
void ObjectCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void S_CDPlay(sint32 track, int mode);

void BoatCollision(sint16 item_number, ITEM_INFO *litem, COLL_INFO *coll)
{
    int geton;
    ITEM_INFO *item;

    if (litem->hit_points < 0 || lara.skidoo != NO_ITEM)
        return;
    geton = BoatCheckGeton(item_number, coll);
    if (!geton) {
        coll->enable_baddie_push = 1;
        ObjectCollision(item_number, litem, coll);
        return;
    }
    lara.skidoo = item_number;
    if (geton == 2)
        litem->anim_number = objects[VEHICLE_ANIM].anim_index;
    else if (geton == 1)
        litem->anim_number = objects[VEHICLE_ANIM].anim_index + 8;
    else if (geton == 3)
        litem->anim_number = objects[VEHICLE_ANIM].anim_index + 6;
    else
        litem->anim_number = objects[VEHICLE_ANIM].anim_index + 1;
    lara.water_status = LARA_ABOVEWATER;
    item = &items[item_number];
    litem->pos.x_pos = item->pos.x_pos;
    litem->pos.y_pos = item->pos.y_pos - 5;
    litem->pos.z_pos = item->pos.z_pos;
    litem->pos.y_rot = item->pos.y_rot;
    litem->pos.x_rot = litem->pos.z_rot = 0;
    litem->gravity_status = 0;
    litem->speed = 0;
    litem->fallspeed = 0;
    litem->frame_number = anims[litem->anim_number].frame_base;
    litem->current_anim_state = litem->goal_anim_state = 0;
    if (litem->room_number != item->room_number)
        ItemNewRoom(lara.item_number, item->room_number);
    AnimateItem(litem);
    if (items[item_number].status != ACTIVE) {
        AddActiveItem(item_number);
        items[item_number].status = ACTIVE;
    }
    S_CDPlay(12, 0);
}

void LookUpDown(void);

int BoatUserControl(ITEM_INFO *boat)
{
    int no_turn = 1, max_speed;
    BOAT_INFO *binfo;

    binfo = (BOAT_INFO *)boat->data;

    if (boat->pos.y_pos >= binfo->water - 128 && binfo->water != NO_HEIGHT) {
        if ((!(input & IN_ROLL) && !(input & IN_LOOK)) || boat->speed) {
            if (((input & (IN_LEFT | IN_STEPL)) && !(input & IN_JUMP))
                || ((input & (IN_RIGHT | IN_STEPR)) && (input & IN_JUMP))) {
                if (binfo->boat_turn > 0) {
                    binfo->boat_turn -= 45;
                } else {
                    binfo->boat_turn -= 22;
                    if (binfo->boat_turn < -728) {
                        binfo->boat_turn = -728;
                    }
                }
                no_turn = 0;
            } else if (((input & (IN_RIGHT | IN_STEPR)) && !(input & IN_JUMP))
                || ((input & (IN_LEFT | IN_STEPL)) && (input & IN_JUMP))) {
                if (binfo->boat_turn < 0) {
                    binfo->boat_turn += 45;
                } else {
                    binfo->boat_turn += 22;
                    if (binfo->boat_turn > 728) {
                        binfo->boat_turn = 728;
                    }
                }
                no_turn = 0;
            }

            if (input & IN_JUMP) {
                if (boat->speed > 0) {
                    boat->speed -= 5;
                } else if (boat->speed > -20) {
                    boat->speed += -2;
                }
            } else if (input & IN_ACTION) {
                if (input & (IN_DASH | IN_DUCK)) {
                    max_speed = 185;
                } else {
                    max_speed = (input & IN_SLOW) ? 36 : 110;
                }

                if (boat->speed < max_speed) {
                    boat->speed += 5 / 2 + 5 * boat->speed / (2 * max_speed);
                } else if (boat->speed > max_speed + 1) {
                    boat->speed -= 1;
                }
            } else if (boat->speed >= 0 && boat->speed < 20
                && (input & (IN_LEFT | IN_STEPL | IN_RIGHT | IN_STEPR))) {
                if (boat->speed == 0 && !(input & IN_ROLL)) {
                    boat->speed = 20;
                }
            } else if (boat->speed > 1) {
                boat->speed -= 1;
            } else {
                boat->speed = 0;
            }
        } else {
            if (boat->speed >= 0 && boat->speed < 20
                && (input & (IN_LEFT | IN_STEPL | IN_RIGHT | IN_STEPR))) {
                if (boat->speed == 0 && !(input & IN_ROLL)) {
                    boat->speed = 20;
                }
            } else if (boat->speed > 1) {
                boat->speed -= 1;
            } else {
                boat->speed = 0;
            }

            if ((input & IN_LOOK) && boat->speed == 0) {
                LookUpDown();
            }
        }
    }

    return no_turn;
}

int CanGetOff_boat(int direction);

void BoatAnimation(ITEM_INFO *boat, int collide)
{
    BOAT_INFO *binfo;

    binfo = (BOAT_INFO *)boat->data;

    if (lara_item->hit_points <= 0) {
        if (lara_item->current_anim_state != 8) {
            lara_item->anim_number = objects[11].anim_index + 18;
            lara_item->frame_number = anims[lara_item->anim_number].frame_base;
            lara_item->current_anim_state = lara_item->goal_anim_state = 8;
        }
    } else if (boat->pos.y_pos < binfo->water - 128 && boat->fallspeed > 0) {
        if (lara_item->current_anim_state != 6) {
            lara_item->anim_number = objects[11].anim_index + 15;
            lara_item->frame_number = anims[lara_item->anim_number].frame_base;
            lara_item->current_anim_state = lara_item->goal_anim_state = 6;
        }
    } else if (collide) {
        if (lara_item->current_anim_state != 5) {
            lara_item->anim_number = (sint16)(objects[11].anim_index + collide);
            lara_item->frame_number = anims[lara_item->anim_number].frame_base;
            lara_item->current_anim_state = lara_item->goal_anim_state = 5;
        }
    } else {
        switch (lara_item->current_anim_state) {
        case 1:
            if (input & IN_ROLL) {
                if (boat->speed == 0) {
                    if ((input & (IN_RIGHT | IN_STEPR))
                        && CanGetOff_boat(boat->pos.y_rot + 0x4000)) {
                        lara_item->goal_anim_state = 3;
                    } else if ((input & (IN_LEFT | IN_STEPL))
                        && CanGetOff_boat(boat->pos.y_rot - 0x4000)) {
                        lara_item->goal_anim_state = 4;
                    }
                }
            }
            if (boat->speed > 0) {
                lara_item->goal_anim_state = 2;
            }
            break;

        case 2:
            if (boat->speed <= 0) {
                lara_item->goal_anim_state = 1;
            } else if (input & (IN_RIGHT | IN_STEPR)) {
                lara_item->goal_anim_state = 7;
            } else if (input & (IN_LEFT | IN_STEPL)) {
                lara_item->goal_anim_state = 9;
            }
            break;

        case 6:
            lara_item->goal_anim_state = 2;
            break;

        case 7:
            if (boat->speed <= 0) {
                lara_item->goal_anim_state = 1;
            } else if (!(input & (IN_RIGHT | IN_STEPR))) {
                lara_item->goal_anim_state = 2;
            }
            break;

        case 9:
            if (boat->speed <= 0) {
                lara_item->goal_anim_state = 1;
            } else if (!(input & (IN_LEFT | IN_STEPL))) {
                lara_item->goal_anim_state = 2;
            }
            break;
        }
    }
}

extern int height_type;

int CanGetOff_boat(int direction)
{
    ITEM_INFO *v;
    FLOOR_INFO *floor;
    sint16 room_number, angle;
    int x, y, z, height, ceiling;

    v = &items[lara.skidoo];

    if (direction < 0) {
        angle = v->pos.y_rot - 0x4000;
    } else {
        angle = v->pos.y_rot + 0x4000;
    }

    x = v->pos.x_pos + (1024 * phd_sin(angle) >> W2V_SHIFT);
    y = v->pos.y_pos;
    z = v->pos.z_pos + (1024 * phd_cos(angle) >> W2V_SHIFT);

    room_number = v->room_number;
    floor = GetFloor(x, y, z, &room_number);

    height = GetHeight(floor, x, y, z);

    if ((height - v->pos.y_pos) < -512) {
        return 0;
    }

    if (height_type == 2 || height_type == 3) {
        return 0;
    }

    ceiling = GetCeiling(floor, x, y, z);

    if (ceiling - v->pos.y_pos > -762 || height - ceiling < 762) {
        return 0;
    }

    return 1;
}

int DoBoatDynamics(int height, int fallspeed, int *y)
{
    if (height > *y)
    {
        *y += fallspeed;
        if (*y > height)
        {
            *y = height;
            fallspeed = 0;
        }
        else
            fallspeed += GRAVITY;
    }
    else
    {
        fallspeed += (height - *y - fallspeed) >> 3;
        if (fallspeed < -20)
            fallspeed = -20;
        if (*y > height)
            *y = height;
    }
    return fallspeed;
}

void DrawBoat(ITEM_INFO *item);
void TriggerBoatMist(sint32 x, sint32 y, sint32 z, sint32 speed, sint16 angle, sint32 snow);
void BoatSplash(ITEM_INFO *item, sint32 fallspeed, sint32 water);
sint32 TestWaterHeight(ITEM_INFO *item, sint32 z_off, sint32 x_off, PHD_VECTOR *pos);
int DoShift_boat(ITEM_INFO *boat, PHD_VECTOR *newer, PHD_VECTOR *old);
int GetCollisionAnim_boat(ITEM_INFO *boat, PHD_VECTOR *moved);
void DoBoatShift(int boat_number);
sint32 GetWaterHeight(sint32 x, sint32 y, sint32 z, sint16 room_number);

int BoatDynamics(sint16 boat_number)
{
    ITEM_INFO *boat;
    BOAT_INFO *binfo;
    PHD_VECTOR moved, fl, fr, br, bl, f;
    PHD_VECTOR old, fl_old, fr_old, bl_old, br_old, f_old;
    sint32 hfl, hfr, hbr, hbl, hf;
    sint32 hfr_old, hfl_old, hbr_old, hbl_old, hf_old;
    FLOOR_INFO *floor;
    sint32 height, slip, collide;
    sint16 room_number, rot;
    int newspeed;

    boat = &items[boat_number];
    binfo = (BOAT_INFO *)boat->data;
    boat->pos.z_rot -= binfo->tilt_angle;

    hfl_old = TestWaterHeight(boat, 750, -300, &fl_old);
    hfr_old = TestWaterHeight(boat, 750, 300, &fr_old);
    hbl_old = TestWaterHeight(boat, -750, -300, &bl_old);
    hbr_old = TestWaterHeight(boat, -750, 300, &br_old);
    hf_old = TestWaterHeight(boat, 750 + 250, 0, &f_old);
    old.x = boat->pos.x_pos;
    old.y = boat->pos.y_pos;
    old.z = boat->pos.z_pos;

    if (bl_old.y > hbl_old)
        bl_old.y = hbl_old;
    if (br_old.y > hbr_old)
        br_old.y = hbr_old;
    if (fl_old.y > hfl_old)
        fl_old.y = hfl_old;
    if (fr_old.y > hfr_old)
        fr_old.y = hfr_old;
    if (f_old.y > hf_old)
        f_old.y = hf_old;

    boat->pos.y_rot += binfo->boat_turn + binfo->extra_rotation;
    binfo->tilt_angle = binfo->boat_turn * 6;
    boat->pos.z_pos += boat->speed * phd_cos(boat->pos.y_rot) >> W2V_SHIFT;
    boat->pos.x_pos += boat->speed * phd_sin(boat->pos.y_rot) >> W2V_SHIFT;
    if (boat->speed >= 0)
        binfo->prop_rot += (boat->speed * (ONE_DEGREE * 3)) + (ONE_DEGREE << 1);
    else
        binfo->prop_rot += ONE_DEGREE * 33;

    slip = 30 * phd_sin(boat->pos.z_rot) >> W2V_SHIFT;
    if (!slip && boat->pos.z_rot)
        slip = (boat->pos.z_rot > 0) ? 1 : -1;
    boat->pos.z_pos -= slip * phd_sin(boat->pos.y_rot) >> W2V_SHIFT;
    boat->pos.x_pos += slip * phd_cos(boat->pos.y_rot) >> W2V_SHIFT;

    slip = 10 * phd_sin(boat->pos.x_rot) >> W2V_SHIFT;
    if (!slip && boat->pos.x_rot)
        slip = (boat->pos.x_rot > 0) ? 1 : -1;
    boat->pos.z_pos -= slip * phd_cos(boat->pos.y_rot) >> W2V_SHIFT;
    boat->pos.x_pos -= slip * phd_sin(boat->pos.y_rot) >> W2V_SHIFT;

    moved.x = boat->pos.x_pos;
    moved.z = boat->pos.z_pos;
    DoBoatShift(boat_number);

    rot = 0;
    hbl = TestWaterHeight(boat, -750, -300, &bl);
    if (hbl < bl_old.y - STEP_L / 2)
        rot = DoShift_boat(boat, &bl, &bl_old);
    hbr = TestWaterHeight(boat, -750, 300, &br);
    if (hbr < br_old.y - STEP_L / 2)
        rot += DoShift_boat(boat, &br, &br_old);
    hfl = TestWaterHeight(boat, 750, -300, &fl);
    if (hfl < fl_old.y - STEP_L / 2)
        rot += DoShift_boat(boat, &fl, &fl_old);
    hfr = TestWaterHeight(boat, 750, 300, &fr);
    if (hfr < fr_old.y - STEP_L / 2)
        rot += DoShift_boat(boat, &fr, &fr_old);
    if (!slip) {
        hf = TestWaterHeight(boat, 750 + 250, 0, &f);
        if (hf < f_old.y - STEP_L / 2)
            DoShift_boat(boat, &f, &f_old);
    }

    room_number = boat->room_number;
    floor = GetFloor(boat->pos.x_pos, boat->pos.y_pos, boat->pos.z_pos, &room_number);
    height = GetWaterHeight(boat->pos.x_pos, boat->pos.y_pos, boat->pos.z_pos, room_number);
    if (height == NO_HEIGHT)
        height = GetHeight(floor, boat->pos.x_pos, boat->pos.y_pos, boat->pos.z_pos);
    if (height < boat->pos.y_pos - STEP_L / 2)
        DoShift_boat(boat, (PHD_VECTOR *)&boat->pos, &old);

    binfo->extra_rotation = rot;
    collide = GetCollisionAnim_boat(boat, &moved);
    if (slip || collide) {
        newspeed = ((boat->pos.x_pos - old.x) * phd_sin(boat->pos.y_rot) + (boat->pos.z_pos - old.z) * phd_cos(boat->pos.y_rot)) >> W2V_SHIFT;
        if (lara.skidoo == boat_number && boat->speed > 110 + 5 && newspeed < boat->speed - 10) {
            lara_item->hit_points -= boat->speed;
            lara_item->hit_status = 1;
            SoundEffect(31, &lara_item->pos, 0);
            newspeed >>= 1;
            boat->speed >>= 1;
        }
        if (slip) {
            if (boat->speed <= 110 + 10)
                boat->speed = newspeed;
        } else {
            if (boat->speed > 0 && newspeed < boat->speed)
                boat->speed = newspeed;
            else if (boat->speed < 0 && newspeed > boat->speed)
                boat->speed = newspeed;
        }
        if (boat->speed < -20)
            boat->speed = -20;
    }
    return collide;
}

void DoBoatShift(int boat_number)
{
    ITEM_INFO *item, *boat;
    int item_number, distance, x, z;

    boat = &items[boat_number];
    item_number = room[boat->room_number].item_number;
    while (item_number != NO_ITEM) {
        item = &items[item_number];
        if (item->object_number == 15 && item_number != boat_number && lara.skidoo != item_number) {
            x = item->pos.x_pos - boat->pos.x_pos;
            z = item->pos.z_pos - boat->pos.z_pos;
            distance = x * x + z * z;
            if (distance < 1000 * 1000) {
                boat->pos.x_pos = item->pos.x_pos - x * (1000 * 1000) / distance;
                boat->pos.z_pos = item->pos.z_pos - z * (1000 * 1000) / distance;
            }
            return;
        }
        item_number = item->next_item;
    }
}

void DrawBoat(ITEM_INFO *item)
{
    BOAT_INFO *b;

    b = (BOAT_INFO *)item->data;
    item->data = &b->prop_rot;
    DrawAnimatingItem(item);
    item->data = b;
    S_DrawWakeFX(item);
}


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

sint32 GetFreeSpark(void);

void TriggerBoatMist(sint32 x, sint32 y, sint32 z, sint32 speed, sint16 angle, sint32 snow)
{
    sint32 size, xv, zv;
    SPARKS *sptr;

    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sR = 0;
    sptr->sG = 0;
    sptr->sB = 0;
    if (snow) {
        sptr->dR = 255;
        sptr->dG = 255;
        sptr->dB = 255;
    } else {
        sptr->dR = 64;
        sptr->dG = 64;
        sptr->dB = 64;
    }
    sptr->ColFadeSpeed = 4 + (GetRandomControl() & 3);
    sptr->FadeToBlack = 12 - (snow << 3);
    sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 20;
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = x + ((GetRandomControl() & 15) - 8);
    sptr->y = y + ((GetRandomControl() & 15) - 8);
    sptr->z = z + ((GetRandomControl() & 15) - 8);
    zv = (speed * phd_cos(angle)) >> (W2V_SHIFT + 2);
    xv = (speed * phd_sin(angle)) >> (W2V_SHIFT + 2);
    sptr->Xvel = xv + ((GetRandomControl() & 127) - 64);
    sptr->Yvel = (speed << 3) + (speed << 2);
    sptr->Zvel = zv + ((GetRandomControl() & 127) - 64);
    sptr->Friction = 3;
    if (GetRandomControl() & 1) {
        sptr->Flags = 2 | 8 | 16 | 512;
        sptr->RotAng = GetRandomControl() & 4095;
        if (GetRandomControl() & 1)
            sptr->RotAdd = -(GetRandomControl() & 15) - 16;
        else
            sptr->RotAdd = (GetRandomControl() & 15) + 16;
    } else
        sptr->Flags = 2 | 8 | 512;
    sptr->Def = objects[296].mesh_index;
    if (!snow) {
        sptr->Scalar = 4;
        sptr->Gravity = sptr->MaxYvel = 0;
        size = (GetRandomControl() & 7) + (speed >> 1) + 16;
        sptr->Width = sptr->sWidth = size >> 2;
        sptr->Height = sptr->sHeight = size >> 2;
    } else {
        sptr->Friction = 0;
        sptr->Scalar = 3;
        sptr->Yvel = -sptr->Yvel >> 5;
        sptr->Gravity = (GetRandomControl() & 31) + 32;
        sptr->MaxYvel = 0;
        size = (GetRandomControl() & 7) + 16;
        sptr->Width = sptr->sWidth = size;
        sptr->Height = sptr->sHeight = size;
    }
    sptr->dWidth = size;
    sptr->dHeight = size;
}

void BoatSplash(ITEM_INFO *item, sint32 fallspeed, sint32 water)
{
    splash_setup.x = item->pos.x_pos;
    splash_setup.y = water;
    splash_setup.z = item->pos.z_pos;
    splash_setup.InnerXZoff = 16 << 2;
    splash_setup.InnerXZsize = 12 << 2;
    splash_setup.InnerYsize = -96 << 2;
    splash_setup.InnerXZvel = 0xa0;
    splash_setup.InnerYvel = -fallspeed << 7;
    splash_setup.InnerGravity = 0x80;
    splash_setup.InnerFriction = 7;
    splash_setup.MiddleXZoff = 24 << 2;
    splash_setup.MiddleXZsize = 24 << 2;
    splash_setup.MiddleYsize = -64 << 2;
    splash_setup.MiddleXZvel = 0xe0;
    splash_setup.MiddleYvel = -fallspeed << 6;
    splash_setup.MiddleGravity = 0x48;
    splash_setup.MiddleFriction = 8;
    splash_setup.OuterXZoff = 32 << 2;
    splash_setup.OuterXZsize = 32 << 2;
    splash_setup.OuterXZvel = 0x110;
    splash_setup.OuterFriction = 9;
    SetupSplash(&splash_setup);
    SplashCount = 16;
}

FLOOR_INFO *GetFloor(sint32 x, sint32 y, sint32 z, sint16 *room_number);

sint32 TestWaterHeight(ITEM_INFO *item, sint32 z_off, sint32 x_off, PHD_VECTOR *pos)
{
    FLOOR_INFO *floor;
    sint32 s, c, height;
    sint16 room_number;

    pos->y = item->pos.y_pos + ((x_off * phd_sin(item->pos.z_rot) >> W2V_SHIFT) - (z_off * phd_sin(item->pos.x_rot) >> W2V_SHIFT));
    c = phd_cos(item->pos.y_rot);
    s = phd_sin(item->pos.y_rot);
    pos->z = item->pos.z_pos + ((z_off * c - x_off * s) >> W2V_SHIFT);
    pos->x = item->pos.x_pos + ((z_off * s + x_off * c) >> W2V_SHIFT);

    room_number = item->room_number;
    GetFloor(pos->x, pos->y, pos->z, &room_number);
    height = GetWaterHeight(pos->x, pos->y, pos->z, room_number);
    if (height == NO_HEIGHT) {
        floor = GetFloor(pos->x, pos->y, pos->z, &room_number);
        height = GetHeight(floor, pos->x, pos->y, pos->z);
        if (height == NO_HEIGHT)
            return height;
    }
    return height - 5;
}

int GetCollisionAnim_boat(ITEM_INFO *boat, PHD_VECTOR *moved);

int GetCollisionAnim_boat(ITEM_INFO *boat, PHD_VECTOR *moved)
{
    sint32 c, s, front, side;

    moved->x = boat->pos.x_pos - moved->x;
    moved->z = boat->pos.z_pos - moved->z;
    if (moved->x || moved->z) {
        c = phd_cos(boat->pos.y_rot);
        s = phd_sin(boat->pos.y_rot);
        front = (moved->z * c + moved->x * s) >> W2V_SHIFT;
        side = (moved->x * c - moved->z * s) >> W2V_SHIFT;
        if ((front < 0 ? -front : front) > (side < 0 ? -side : side)) {
            if (front > 0)
                return 14;
            else
                return 13;
        } else {
            if (side > 0)
                return 11;
            else
                return 12;
        }
    }
    return 0;
}

int DoShift_boat(ITEM_INFO *boat, PHD_VECTOR *newer, PHD_VECTOR *old)
{
    sint32 x, z, x_old, z_old, shift_x, shift_z;

    x = newer->x >> WALL_SHIFT;
    z = newer->z >> WALL_SHIFT;
    x_old = old->x >> WALL_SHIFT;
    z_old = old->z >> WALL_SHIFT;
    shift_x = newer->x & (WALL_L - 1);
    shift_z = newer->z & (WALL_L - 1);
    if (x == x_old) {
        if (z == z_old) {
            boat->pos.z_pos += old->z - newer->z;
            boat->pos.x_pos += old->x - newer->x;
        } else if (z > z_old) {
            boat->pos.z_pos -= shift_z + 1;
            return newer->x - boat->pos.x_pos;
        } else {
            boat->pos.z_pos += WALL_L - shift_z;
            return boat->pos.x_pos - newer->x;
        }
    } else if (z == z_old) {
        if (x > x_old) {
            boat->pos.x_pos -= shift_x + 1;
            return boat->pos.z_pos - newer->z;
        } else {
            boat->pos.x_pos += WALL_L - shift_x;
            return newer->z - boat->pos.z_pos;
        }
    } else {
        sint16 room_number;
        FLOOR_INFO *floor;
        sint32 height;

        x = z = 0;
        room_number = boat->room_number;
        floor = GetFloor(old->x, newer->y, newer->z, &room_number);
        height = GetHeight(floor, old->x, newer->y, newer->z);
        if (height < old->y - STEP_L) {
            if (newer->z > old->z)
                z = -shift_z - 1;
            else
                z = WALL_L - shift_z;
        }
        room_number = boat->room_number;
        floor = GetFloor(newer->x, newer->y, old->z, &room_number);
        height = GetHeight(floor, newer->x, newer->y, old->z);
        if (height < old->y - STEP_L) {
            if (newer->x > old->x)
                x = -shift_x - 1;
            else
                x = WALL_L - shift_x;
        }
        if (x && z) {
            boat->pos.z_pos += z;
            boat->pos.x_pos += x;
        } else if (z) {
            boat->pos.z_pos += z;
            if (z > 0)
                return boat->pos.x_pos - newer->x;
            else
                return newer->x - boat->pos.x_pos;
        } else if (x) {
            boat->pos.x_pos += x;
            if (x > 0)
                return newer->z - boat->pos.z_pos;
            else
                return boat->pos.z_pos - newer->z;
        } else {
            boat->pos.z_pos += old->z - newer->z;
            boat->pos.x_pos += old->x - newer->x;
        }
    }
    return 0;
}

#define BOAT_GETON 0
#define BOAT_JUMPR 3
#define BOAT_JUMPL 4
#define BOAT_FRONT 750
#define BOAT_SIDE 300
#define BOAT_SLOWDOWN 1
#define BOAT_MAX_SPEED 110
#define BOAT_UNDO_TURN (ONE_DEGREE / 4)
#define PITCH_SHIFT 4
#define UNDERWATER 1

extern sint32 wibble;
extern uint8 WakeShade;

void TestTriggers(sint16 *data, int heavy);
int BoatUserControl(ITEM_INFO *boat);
void BoatAnimation(ITEM_INFO *boat, int collide);
int DoBoatDynamics(int height, int fallspeed, int *y);
int BoatDynamics(sint16 boat_number);
void DoWake_boat(ITEM_INFO *item, sint32 xoff, sint32 zoff, sint16 rotate);
void UpdateWakeFX_boat(void);
void CreateBubble(PHD_3DPOS *pos, sint16 room_number, sint32 size, sint32 sizerange);
int abs(int x);
#pragma intrinsic(abs)

void BoatControl(sint16 item_number)
{
    ITEM_INFO *boat;
    BOAT_INFO *binfo;
    PHD_VECTOR fl, fr, prop;
    sint32 hfl, hfr, no_turn = 1, drive = 0;
    FLOOR_INFO *floor;
    sint32 height, collide, water, ceiling, pitch, h, ofs, nowake;
    sint16 room_number, x_rot, z_rot;

    boat = &items[item_number];
    binfo = (BOAT_INFO *)boat->data;
    collide = BoatDynamics(item_number);
    hfl = TestWaterHeight(boat, BOAT_FRONT, -BOAT_SIDE, &fl);
    hfr = TestWaterHeight(boat, BOAT_FRONT, BOAT_SIDE, &fr);

    room_number = boat->room_number;
    floor = GetFloor(boat->pos.x_pos, boat->pos.y_pos, boat->pos.z_pos, &room_number);
    height = GetHeight(floor, boat->pos.x_pos, boat->pos.y_pos, boat->pos.z_pos);
    ceiling = GetCeiling(floor, boat->pos.x_pos, boat->pos.y_pos, boat->pos.z_pos);
    if (lara.skidoo == item_number) {
        TestTriggers(trigger_index, 0);
        TestTriggers(trigger_index, 1);
    }

    binfo->water = water = GetWaterHeight(boat->pos.x_pos, boat->pos.y_pos, boat->pos.z_pos, room_number);

    if (lara.skidoo == item_number && lara_item->hit_points > 0) {
        switch (lara_item->current_anim_state) {
        case BOAT_GETON:
        case BOAT_JUMPR:
        case BOAT_JUMPL:
            break;
        default:
            drive = 1;
            no_turn = BoatUserControl(boat);
        }
    } else {
        if (boat->speed > BOAT_SLOWDOWN)
            boat->speed -= BOAT_SLOWDOWN;
        else
            boat->speed = 0;
    }

    if (no_turn) {
        if (binfo->boat_turn < -BOAT_UNDO_TURN)
            binfo->boat_turn += BOAT_UNDO_TURN;
        else if (binfo->boat_turn > BOAT_UNDO_TURN)
            binfo->boat_turn -= BOAT_UNDO_TURN;
        else
            binfo->boat_turn = 0;
    }

    boat->floor = height - 5;
    if (binfo->water == NO_HEIGHT)
        binfo->water = height;
    else
        binfo->water -= 5;

    binfo->left_fallspeed = DoBoatDynamics(hfl, binfo->left_fallspeed, (int *)&fl.y);
    binfo->right_fallspeed = DoBoatDynamics(hfr, binfo->right_fallspeed, (int *)&fr.y);
    ofs = boat->fallspeed;
    boat->fallspeed = DoBoatDynamics(binfo->water, boat->fallspeed, (int *)&boat->pos.y_pos);
    if (ofs - boat->fallspeed > 32 && boat->fallspeed == 0 && water != NO_HEIGHT)
        BoatSplash(boat, ofs - boat->fallspeed, water);

    height = fl.y + fr.y;
    if (height < 0)
        height = -(abs(height) >> 1);
    else
        height = height >> 1;

    x_rot = phd_atan(BOAT_FRONT, boat->pos.y_pos - height);
    z_rot = phd_atan(BOAT_SIDE, height - fl.y);
    boat->pos.x_rot += (x_rot - boat->pos.x_rot) >> 1;
    boat->pos.z_rot += (z_rot - boat->pos.z_rot) >> 1;
    if (!x_rot && ABS(boat->pos.x_rot) < 4)
        boat->pos.x_rot = 0;
    if (!z_rot && ABS(boat->pos.z_rot) < 4)
        boat->pos.z_rot = 0;

    if (lara.skidoo == item_number) {
        BoatAnimation(boat, collide);
        if (room_number != boat->room_number) {
            ItemNewRoom(item_number, room_number);
            ItemNewRoom(lara.item_number, room_number);
        }
        boat->pos.z_rot += binfo->tilt_angle;
        lara_item->pos.x_pos = boat->pos.x_pos;
        lara_item->pos.y_pos = boat->pos.y_pos;
        lara_item->pos.z_pos = boat->pos.z_pos;
        lara_item->pos.x_rot = boat->pos.x_rot;
        lara_item->pos.y_rot = boat->pos.y_rot;
        lara_item->pos.z_rot = boat->pos.z_rot;
        AnimateItem(lara_item);
        if (lara_item->hit_points > 0) {
            boat->anim_number = objects[15].anim_index + (lara_item->anim_number - objects[11].anim_index);
            boat->frame_number = anims[boat->anim_number].frame_base
                + (lara_item->frame_number - anims[lara_item->anim_number].frame_base);
        }
        camera.target_elevation = -20 * ONE_DEGREE;
        camera.target_distance = WALL_L * 2;
    } else {
        if (room_number != boat->room_number)
            ItemNewRoom(item_number, room_number);
        boat->pos.z_rot += binfo->tilt_angle;
    }

    pitch = boat->speed;
    binfo->pitch += (pitch - binfo->pitch) >> 2;
    if (boat->speed > 8)
        SoundEffect(197, &boat->pos, PITCH_SHIFT + ((0x10000 - (BOAT_MAX_SPEED - binfo->pitch) * 100) << 8));
    else if (drive)
        SoundEffect(195, &boat->pos, PITCH_SHIFT + ((0x10000 - (BOAT_MAX_SPEED - binfo->pitch) * 100) << 8));

    if (lara.skidoo != item_number)
        return;

    if ((lara_item->current_anim_state == BOAT_JUMPR || lara_item->current_anim_state == BOAT_JUMPL)
        && lara_item->frame_number == anims[lara_item->anim_number].frame_end) {
        sint16 room_number;
        int x, y, z;
        FLOOR_INFO *floor;

        if (lara_item->current_anim_state == BOAT_JUMPL)
            lara_item->pos.y_rot -= 0x4000;
        else
            lara_item->pos.y_rot += 0x4000;
        lara_item->anim_number = 77;
        lara_item->frame_number = anims[lara_item->anim_number].frame_base;
        lara_item->current_anim_state = lara_item->goal_anim_state = AS_FORWARDJUMP;
        lara_item->gravity_status = 1;
        lara_item->fallspeed = -40;
        lara_item->speed = 20;
        lara_item->pos.x_rot = lara_item->pos.z_rot = 0;
        lara.skidoo = NO_ITEM;

        room_number = lara_item->room_number;
        x = lara_item->pos.x_pos + (360 * phd_sin(lara_item->pos.y_rot) >> W2V_SHIFT);
        y = lara_item->pos.y_pos - 90;
        z = lara_item->pos.z_pos + (360 * phd_cos(lara_item->pos.y_rot) >> W2V_SHIFT);
        floor = GetFloor(x, y, z, &room_number);
        if (GetHeight(floor, x, y, z) >= y - STEP_L) {
            lara_item->pos.x_pos = x;
            lara_item->pos.z_pos = z;
            if (room_number != lara_item->room_number)
                ItemNewRoom(lara.item_number, room_number);
        }
        lara_item->pos.y_pos = y;
        boat->anim_number = objects[15].anim_index + 0;
        boat->frame_number = anims[boat->anim_number].frame_base;
    }

    room_number = boat->room_number;
    floor = GetFloor(boat->pos.x_pos, boat->pos.y_pos + 128, boat->pos.z_pos, &room_number);
    h = GetWaterHeight(boat->pos.x_pos, boat->pos.y_pos + 128, boat->pos.z_pos, room_number);
    if (h > boat->pos.y_pos + 32 || h == NO_HEIGHT)
        h = 0;
    else
        h = 1;

    if (lara_item->current_anim_state == BOAT_JUMPR || lara_item->current_anim_state == BOAT_JUMPL)
        nowake = 1;
    else
        nowake = 0;

    if (!(wibble & 15) && h && !nowake) {
        DoWake_boat(boat, -256 - 128, 0, 0);
        DoWake_boat(boat, 256 + 128, 0, 1);
    }

    if (boat->speed == 0 || !h || nowake) {
        if (WakeShade)
            WakeShade--;
    } else {
        if (WakeShade < 16)
            WakeShade++;
    }

    prop.x = prop.y = 0;
    prop.z = -80;
    GetJointAbsPosition(boat, &prop, 2);
    room_number = boat->room_number;
    floor = GetFloor(prop.x, prop.y, prop.z, &room_number);
    h = GetWaterHeight(prop.x, prop.y, prop.z, room_number);
    if (boat->speed && h < prop.y && h != NO_HEIGHT) {
        TriggerBoatMist(prop.x, prop.y, prop.z, abs(boat->speed), boat->pos.y_rot + 0x8000, 0);
        if ((GetRandomControl() & 1) == 0) {
            PHD_3DPOS pos;
            sint16 room_number;

            pos.x_pos = prop.x + (GetRandomControl() & 63) - 32;
            pos.y_pos = prop.y + (GetRandomControl() & 15);
            pos.z_pos = prop.z + (GetRandomControl() & 63) - 32;
            room_number = boat->room_number;
            GetFloor(pos.x_pos, pos.y_pos, pos.z_pos, &room_number);
            CreateBubble((PHD_3DPOS *)&pos, room_number, 16, 8);
        }
    } else {
        GAME_VECTOR pos;
        sint32 cnt;

        h = GetHeight(floor, prop.x, prop.y, prop.z);
        if (prop.y > h && !(room[room_number].flags & UNDERWATER)) {
            pos.x = prop.x;
            pos.y = prop.y;
            pos.z = prop.z;
            cnt = (GetRandomControl() & 3) + 3;
            for (; cnt > 0; cnt--)
                TriggerBoatMist(prop.x, prop.y, prop.z, ((GetRandomControl() & 15) + 96) << 4,
                    boat->pos.y_rot + 0x4000 + GetRandomControl(), 1);
        }
    }

    UpdateWakeFX_boat();
}
