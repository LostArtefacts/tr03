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
