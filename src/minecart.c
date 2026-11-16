#include "specific/standard.h"
#include <io.h>
#include "game.h"

typedef struct {
    sint32 Speed;
    sint32 MidPos;
    sint32 FrontPos;
    sint32 TurnX;
    sint32 TurnZ;
    sint16 TurnLen;
    sint16 TurnRot;
    sint16 YVel;
    sint16 Gradient;
    char Flags;
    char StopDelay;
} CARTINFO;

sint16 GetCollision(ITEM_INFO *v, sint16 ang, sint32 dist, sint16 *ceiling)
{
    sint16 room_number;
    FLOOR_INFO *floor;
    sint32 x, y, z, height, cheight;

    x = v->pos.x_pos + ((phd_sin(ang) * dist) >> W2V_SHIFT);
    y = v->pos.y_pos - LARA_HITE;
    z = v->pos.z_pos + ((phd_cos(ang) * dist) >> W2V_SHIFT);
    room_number = v->room_number;
    floor = GetFloor(x, y, z, &room_number);
    height = GetHeight(floor, x, y, z);
    cheight = GetCeiling(floor, x, y, z);
    *ceiling = (sint16)cheight;
    if (height != NO_HEIGHT)
        height -= v->pos.y_pos;
    return (sint16)height;
}

void MineCartInitialise(sint16 item_number)
{
    ITEM_INFO *v;

    v = &items[item_number];
    v->data = game_malloc(sizeof(CARTINFO), 0);
}

int TestBoundsCollide(ITEM_INFO *item, ITEM_INFO *laraitem, sint32 radius);
int TestCollision(ITEM_INFO *item, ITEM_INFO *laraitem);

int GetInMineCart(ITEM_INFO *v, ITEM_INFO *l, COLL_INFO *coll);
void CreateFlare(int thrown);
sint32 mGetAngle(sint32 x1, sint32 y1, sint32 x2, sint32 y2);
void S_CDPlay(sint32 track, int mode);
void ObjectCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);

void MineCartCollision(sint16 item_number, ITEM_INFO *l, COLL_INFO *coll)
{
    int geton;
    ITEM_INFO *v;

    if (l->hit_points < 0 || lara.skidoo != NO_ITEM)
        return;

    v = &items[item_number];
    if ((geton = GetInMineCart(v, l, coll))) {
        sint16 ang;
        CARTINFO *cart;

        lara.skidoo = item_number;
        if (lara.gun_type == LG_FLARE) {
            CreateFlare(0);
            undraw_flare_meshes();
            lara.flare_control_left = 0;
            lara.request_gun_type = lara.gun_type = LG_ARMLESS;
        }
        lara.gun_status = LG_HANDSBUSY;

        ang = (sint16)mGetAngle(v->pos.x_pos, v->pos.z_pos, l->pos.x_pos, l->pos.z_pos) - v->pos.y_rot;
        if (ang > -(ONE_DEGREE * 45) && ang < ONE_DEGREE * 135)
            l->anim_number = objects[VEHICLE_ANIM].anim_index + 46;
        else
            l->anim_number = objects[VEHICLE_ANIM].anim_index;
        l->frame_number = anims[l->anim_number].frame_base;
        l->current_anim_state = l->goal_anim_state = 0;
        l->pos.x_pos = v->pos.x_pos;
        l->pos.y_pos = v->pos.y_pos;
        l->pos.z_pos = v->pos.z_pos;
        l->pos.x_rot = v->pos.x_rot;
        l->pos.y_rot = v->pos.y_rot;
        l->pos.z_rot = v->pos.z_rot;

        cart = (CARTINFO *)v->data;
        cart->Flags = 0;
        cart->Speed = 0;
        cart->YVel = 0;
        cart->Gradient = 0;
        S_CDPlay(12, 0);
    } else {
        ObjectCollision(item_number, l, coll);
    }
}

int GetInMineCart(ITEM_INFO *v, ITEM_INFO *l, COLL_INFO *coll)
{
    int dist;
    sint32 x, z;
    FLOOR_INFO *floor;
    sint16 room_number;

    if (!(input & IN_ACTION) || lara.gun_status != LG_ARMLESS || l->gravity_status)
        return 0;
    if (!TestBoundsCollide(v, l, coll->radius))
        return 0;
    if (!TestCollision(v, l))
        return 0;
    x = l->pos.x_pos - v->pos.x_pos;
    z = l->pos.z_pos - v->pos.z_pos;
    dist = x * x + z * z;
    if (dist > 200000)
        return 0;
    room_number = v->room_number;
    floor = GetFloor(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, &room_number);
    if (GetHeight(floor, v->pos.x_pos, v->pos.y_pos, v->pos.z_pos) < -32000)
        return 0;
    return 1;
}

void DoUserInput(ITEM_INFO *v, ITEM_INFO *l, CARTINFO *cart);
void MoveCart(ITEM_INFO *v, ITEM_INFO *l, CARTINFO *cart);

int MineCartControl(void)
{
    CARTINFO *cart;
    ITEM_INFO *v, *l;
    FLOOR_INFO *floor;
    sint16 room_number;

    l = lara_item;
    v = &items[lara.skidoo];
    cart = (CARTINFO *)v->data;

    DoUserInput(v, l, cart);
    if (cart->Flags & 16)
        MoveCart(v, l, cart);

    if (lara.skidoo != NO_ITEM) {
        l->pos.x_pos = v->pos.x_pos;
        l->pos.y_pos = v->pos.y_pos;
        l->pos.z_pos = v->pos.z_pos;
        l->pos.x_rot = v->pos.x_rot;
        l->pos.y_rot = v->pos.y_rot;
        l->pos.z_rot = v->pos.z_rot;
    }

    room_number = v->room_number;
    floor = GetFloor(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, &room_number);
    GetHeight(floor, v->pos.x_pos, v->pos.y_pos, v->pos.z_pos);
    if (room_number != v->room_number) {
        ItemNewRoom(lara.skidoo, room_number);
        ItemNewRoom(lara.item_number, room_number);
    }

    TestTriggers(trigger_index, 0);

    if (!(cart->Flags & 128)) {
        camera.target_elevation = -45 * ONE_DEGREE;
        camera.target_distance = WALL_L * 2;
    }

    return (lara.skidoo == NO_ITEM) ? 0 : 1;
}

#define CF_TURNINGL 2
#define CF_TURNINGR 4
#define CF_CONTROL 16
#define CF_STOPPED 32

sint32 TestHeight_minecart(ITEM_INFO *v, sint32 x, sint32 z);

void MoveCart(ITEM_INFO *v, ITEM_INFO *l, CARTINFO *cart)
{
    sint16 val;

    if (cart->StopDelay)
        cart->StopDelay--;

    if (lara.MineL && lara.MineR && !cart->StopDelay &&
        ((v->pos.x_pos & 0x380) == 512 || (v->pos.z_pos & 0x380) == 512)) {
        if (cart->Speed < 0xf000) {
            cart->Flags |= CF_STOPPED | CF_CONTROL;
            cart->Speed = v->speed = 0;
            return;
        } else
            cart->StopDelay = 16;
    }

    if ((lara.MineL || lara.MineR) && !(lara.MineL && lara.MineR) && !cart->StopDelay &&
        !(cart->Flags & (CF_TURNINGL | CF_TURNINGR))) {
        sint16 ang;
        uint16 rot = (((uint16)v->pos.y_rot) >> 14) | (lara.MineL << 2);

        switch (rot) {
        case 0:
            cart->TurnX = (v->pos.x_pos + 4096) & ~1023;
            cart->TurnZ = v->pos.z_pos & ~1023;
            break;
        case 1:
            cart->TurnX = v->pos.x_pos & ~1023;
            cart->TurnZ = (v->pos.z_pos - 4096) | 1023;
            break;
        case 2:
            cart->TurnX = (v->pos.x_pos - 4096) | 1023;
            cart->TurnZ = v->pos.z_pos | 1023;
            break;
        case 3:
            cart->TurnX = v->pos.x_pos | 1023;
            cart->TurnZ = (v->pos.z_pos + 4096) & ~1023;
            break;
        case 4:
            cart->TurnX = (v->pos.x_pos - 4096) | 1023;
            cart->TurnZ = v->pos.z_pos & ~1023;
            break;
        case 5:
            cart->TurnX = v->pos.x_pos & ~1023;
            cart->TurnZ = (v->pos.z_pos + 4096) & ~1023;
            break;
        case 6:
            cart->TurnX = (v->pos.x_pos + 4096) & ~1023;
            cart->TurnZ = v->pos.z_pos | 1023;
            break;
        case 7:
            cart->TurnX = v->pos.x_pos | 1023;
            cart->TurnZ = (v->pos.z_pos - 4096) | 1023;
            break;
        }

        ang = mGetAngle(v->pos.x_pos, v->pos.z_pos, cart->TurnX, cart->TurnZ) & 0x3fff;
        if (rot < 4) {
            cart->TurnRot = v->pos.y_rot;
            cart->TurnLen = ang;
        } else {
            cart->TurnRot = v->pos.y_rot;
            if (ang)
                ang = 16384 - ang;
            cart->TurnLen = ang;
        }
        cart->Flags |= lara.MineL ? CF_TURNINGL : CF_TURNINGR;
    }

    if (cart->Speed < 0xa00)
        cart->Speed = 0xa00;
    cart->Speed += -cart->Gradient << 2;

    if ((v->speed = cart->Speed >> 8) < 32) {
        v->speed = 32;
        StopSoundEffect(209);
        if (cart->YVel)
            StopSoundEffect(210);
        else
            SoundEffect(210, &v->pos, 2);
    } else {
        StopSoundEffect(210);
        if (cart->YVel)
            StopSoundEffect(209);
        else
            SoundEffect(209, &v->pos, (2 | 4) + 0x1000000 + (v->speed << 15));
    }

    if (cart->Flags & (CF_TURNINGL | CF_TURNINGR)) {
        sint32 x, z;
        uint16 quad, deg;

        if ((cart->TurnLen += v->speed * 3) > ONE_DEGREE * 90) {
            if (cart->Flags & CF_TURNINGL)
                v->pos.y_rot = cart->TurnRot - 16384;
            else
                v->pos.y_rot = cart->TurnRot + 16384;
            cart->Flags &= ~(CF_TURNINGL | CF_TURNINGR);
        } else {
            if (cart->Flags & CF_TURNINGL)
                v->pos.y_rot = cart->TurnRot - cart->TurnLen;
            else
                v->pos.y_rot = cart->TurnRot + cart->TurnLen;
        }

        if (cart->Flags & (CF_TURNINGL | CF_TURNINGR)) {
            quad = ((uint16)v->pos.y_rot) >> 14;
            deg = v->pos.y_rot & 16383;
            switch (quad) {
            case 0:
                x = -phd_cos(deg);
                z = phd_sin(deg);
                break;
            case 1:
                x = phd_sin(deg);
                z = phd_cos(deg);
                break;
            case 2:
                x = phd_cos(deg);
                z = -phd_sin(deg);
                break;
            default:
                x = -phd_sin(deg);
                z = -phd_cos(deg);
                break;
            }
            if (cart->Flags & CF_TURNINGL) {
                x = -x;
                z = -z;
            }
            v->pos.x_pos = cart->TurnX + ((x * 3584) >> W2V_SHIFT);
            v->pos.z_pos = cart->TurnZ + ((z * 3584) >> W2V_SHIFT);
        }
    } else {
        v->pos.x_pos += (v->speed * phd_sin(v->pos.y_rot)) >> W2V_SHIFT;
        v->pos.z_pos += (v->speed * phd_cos(v->pos.y_rot)) >> W2V_SHIFT;
    }

    cart->MidPos = TestHeight_minecart(v, 0, 0);
    if (!cart->YVel) {
        cart->FrontPos = TestHeight_minecart(v, 0, 256);
        cart->Gradient = cart->MidPos - cart->FrontPos;
        v->pos.y_pos = cart->MidPos;
    } else {
        if (v->pos.y_pos > cart->MidPos) {
            if (cart->YVel > 0)
                SoundEffect(202, &v->pos, 2);
            v->pos.y_pos = cart->MidPos;
            cart->YVel = 0;
        } else {
            cart->YVel += 0x401;
            if (cart->YVel > 0x3f00)
                cart->YVel = 0x3f00;
            v->pos.y_pos += cart->YVel >> 8;
        }
    }

    v->pos.x_rot = cart->Gradient << 5;
    val = v->pos.y_rot & 16383;
    if (cart->Flags & (CF_TURNINGL | CF_TURNINGR)) {
        if (cart->Flags & CF_TURNINGR)
            v->pos.z_rot = -(val * v->speed) >> 9;
        else
            v->pos.z_rot = ((16384 - val) * v->speed) >> 9;
    } else
        v->pos.z_rot -= v->pos.z_rot >> 3;
}

void AnimateItem(ITEM_INFO *item);

sint32 TestHeight_minecart(ITEM_INFO *v, sint32 x, sint32 z)
{
    sint32 c;
    sint32 s;
    PHD_VECTOR pos;
    FLOOR_INFO *floor;
    sint16 room_number;

    c = phd_cos(v->pos.y_rot);
    s = phd_sin(v->pos.y_rot);

    pos.x = v->pos.x_pos + ((x * c + z * s) >> W2V_SHIFT);
    pos.z = c;
    c = s;
    s = pos.z;
    pos.y = v->pos.y_pos - (z * phd_sin(v->pos.x_rot) >> W2V_SHIFT) + (x * phd_sin(v->pos.z_rot) >> W2V_SHIFT);
    pos.z = v->pos.z_pos + ((z * s - x * c) >> W2V_SHIFT);

    room_number = v->room_number;
    floor = GetFloor(pos.x, pos.y, pos.z, &room_number);
    return GetHeight(floor, pos.x, pos.y, pos.z);
}

#define CART_GETIN 0
#define CART_GETOUT 1
#define CART_GETOUTL 2
#define CART_GETOUTR 3
#define CART_STILL 4
#define CART_DUCK 5
#define CART_MOVE 6
#define CART_RIGHT 7
#define CART_LEFT 9
#define CART_BRAKE 11
#define CART_FWD 12
#define CART_BACK 13
#define CART_TURNDEATH 14
#define CART_WALLDEATH 16
#define CART_HIT 17
#define CART_USE 18
#define CART_BRAKING 19
#define CF_MESH 1
#define CF_RDIR 8
#define CF_NOANIM 64
#define CF_DEAD 128

void GetLaraHandAbsPosition(PHD_VECTOR *vec, int hand);
int CollideStaticObjects(COLL_INFO *coll, sint32 x, sint32 y, sint32 z, sint16 room_number, sint32 hite);
void DoLotsOfBlood(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num, int number);
void CartToBaddieCollision(ITEM_INFO *v);
int CanGetOut_minecart(int direction);
extern sint32 wibble;

void DoUserInput(ITEM_INFO *v, ITEM_INFO *l, CARTINFO *cart)
{
    sint16 fh, ch;

    switch (l->current_anim_state) {
    case CART_MOVE:
        if (input & IN_ACTION)
            l->goal_anim_state = CART_USE;
        else if (input & IN_DUCK)
            l->goal_anim_state = CART_DUCK;
        else if (input & IN_JUMP)
            l->goal_anim_state = CART_BRAKE;
        else if (cart->Speed == 32 || (cart->Flags & CF_STOPPED))
            l->goal_anim_state = CART_STILL;
        else if (cart->Gradient < -128)
            l->goal_anim_state = CART_FWD;
        else if (cart->Gradient > 128)
            l->goal_anim_state = CART_BACK;
        else if (input & IN_LEFT)
            l->goal_anim_state = CART_LEFT;
        else if (input & IN_RIGHT)
            l->goal_anim_state = CART_RIGHT;
        break;

    case CART_FWD:
        if (input & IN_ACTION)
            l->goal_anim_state = CART_USE;
        else if (input & IN_DUCK)
            l->goal_anim_state = CART_DUCK;
        else if (input & IN_JUMP)
            l->goal_anim_state = CART_BRAKE;
        else if (cart->Gradient > -128)
            l->goal_anim_state = CART_MOVE;
        break;

    case CART_BACK:
        if (input & IN_ACTION)
            l->goal_anim_state = CART_USE;
        else if (input & IN_DUCK)
            l->goal_anim_state = CART_DUCK;
        else if (input & IN_JUMP)
            l->goal_anim_state = CART_BRAKE;
        else if (cart->Gradient < 128)
            l->goal_anim_state = CART_MOVE;
        break;

    case CART_LEFT:
        if (input & IN_ACTION)
            l->goal_anim_state = CART_USE;
        else if (input & IN_DUCK)
            l->goal_anim_state = CART_DUCK;
        else if (input & IN_JUMP)
            l->goal_anim_state = CART_BRAKE;
        if (!(input & IN_LEFT))
            l->goal_anim_state = CART_MOVE;
        break;

    case CART_RIGHT:
        if (input & IN_ACTION)
            l->goal_anim_state = CART_USE;
        else if (input & IN_DUCK)
            l->goal_anim_state = CART_DUCK;
        else if (input & IN_JUMP)
            l->goal_anim_state = CART_BRAKE;
        if (!(input & IN_RIGHT))
            l->goal_anim_state = CART_MOVE;
        break;

    case CART_STILL:
        if (!(cart->Flags & CF_CONTROL)) {
            SoundEffect(211, &v->pos, 2);
            cart->Flags |= CF_CONTROL;
            cart->StopDelay = 64;
        }
        if ((input & IN_ROLL) && (cart->Flags & CF_STOPPED)) {
            if ((input & IN_LEFT) && CanGetOut_minecart(-1)) {
                l->goal_anim_state = CART_GETOUT;
                cart->Flags &= ~CF_RDIR;
            } else if ((input & IN_RIGHT) && CanGetOut_minecart(1)) {
                l->goal_anim_state = CART_GETOUT;
                cart->Flags |= CF_RDIR;
            }
        }
        if (input & IN_DUCK)
            l->goal_anim_state = CART_DUCK;
        else if (cart->Speed > 32)
            l->goal_anim_state = CART_MOVE;
        break;

    case CART_DUCK:
        if (input & IN_ACTION)
            l->goal_anim_state = CART_USE;
        else if (input & IN_JUMP)
            l->goal_anim_state = CART_BRAKE;
        else if (!(input & IN_DUCK))
            l->goal_anim_state = CART_STILL;
        break;

    case CART_USE:
        l->goal_anim_state = CART_MOVE;
        break;

    case CART_BRAKING:
        if (input & IN_DUCK) {
            l->goal_anim_state = CART_DUCK;
            StopSoundEffect(219);
        } else if (!(input & IN_JUMP) || (cart->Flags & CF_STOPPED)) {
            l->goal_anim_state = CART_MOVE;
            StopSoundEffect(219);
        } else {
            cart->Speed += -0x600;
            SoundEffect(219, &l->pos, 2);
        }
        break;

    case CART_BRAKE:
        l->goal_anim_state = CART_BRAKING;
        break;

    case CART_GETOUT:
        if (l->anim_number == objects[11].anim_index + 7) {
            if (l->frame_number == anims[objects[11].anim_index + 7].frame_base + 20 && (cart->Flags & CF_MESH)) {
                sint16 *tmp;

                tmp = lara.mesh_ptrs[HAND_R];
                lara.mesh_ptrs[HAND_R] = meshes[objects[11].mesh_index + HAND_R];
                meshes[objects[11].mesh_index + HAND_R] = tmp;
                cart->Flags &= ~CF_MESH;
            }
            if (cart->Flags & CF_RDIR)
                l->goal_anim_state = CART_GETOUTR;
            else
                l->goal_anim_state = CART_GETOUTL;
        }
        break;

    case CART_GETOUTL:
        if (l->anim_number == objects[11].anim_index + 1 && l->frame_number == anims[l->anim_number].frame_end) {
            PHD_VECTOR vec = {0, 640, 0};

            GetLaraHandAbsPosition(&vec, 2);
            l->pos.x_pos = vec.x;
            l->pos.y_pos = vec.y;
            l->pos.z_pos = vec.z;
            l->pos.x_rot = 0;
            l->pos.y_rot = v->pos.y_rot + 0x4000;
            l->pos.z_rot = 0;
            l->anim_number = 11;
            l->frame_number = anims[11].frame_base;
            l->current_anim_state = l->goal_anim_state = 2;
            lara.skidoo = NO_ITEM;
            lara.gun_status = LG_ARMLESS;
        }
        break;

    case CART_GETOUTR:
        if (l->anim_number == objects[11].anim_index + 47 && l->frame_number == anims[l->anim_number].frame_end) {
            PHD_VECTOR vec = {0, 640, 0};

            GetLaraHandAbsPosition(&vec, 2);
            l->pos.x_pos = vec.x;
            l->pos.y_pos = vec.y;
            l->pos.z_pos = vec.z;
            l->pos.x_rot = 0;
            l->pos.y_rot = v->pos.y_rot - 0x4000;
            l->pos.z_rot = 0;
            l->anim_number = 11;
            l->frame_number = anims[11].frame_base;
            l->current_anim_state = l->goal_anim_state = 2;
            lara.skidoo = NO_ITEM;
            lara.gun_status = LG_ARMLESS;
        }
        break;

    case CART_GETIN:
        if (l->anim_number == objects[11].anim_index + 5 &&
            l->frame_number == anims[objects[11].anim_index + 5].frame_base + 20 && (!cart->Flags & CF_MESH)) {
            sint16 *tmp;

            tmp = lara.mesh_ptrs[HAND_R];
            lara.mesh_ptrs[HAND_R] = meshes[objects[11].mesh_index + HAND_R];
            meshes[objects[11].mesh_index + HAND_R] = tmp;
            cart->Flags |= CF_MESH;
        }
        break;

    case CART_WALLDEATH:
        camera.target_elevation = -25 * ONE_DEGREE;
        camera.target_distance = WALL_L * 4;
        break;

    case CART_TURNDEATH:
        camera.target_elevation = -45 * ONE_DEGREE;
        camera.target_distance = WALL_L * 2;
        fh = GetCollision(v, v->pos.y_rot, 512, &ch);
        if (fh > -STEP_L && fh < STEP_L) {
            if ((wibble & 7) == 0)
                SoundEffect(202, &v->pos, 2);
            v->pos.x_pos += (128 * phd_sin(v->pos.y_rot)) >> W2V_SHIFT;
            v->pos.z_pos += (128 * phd_cos(v->pos.y_rot)) >> W2V_SHIFT;
        } else {
            if (l->anim_number == objects[11].anim_index + 30) {
                cart->Flags |= CF_NOANIM;
                l->hit_points = -1;
            }
        }
        break;

    case CART_HIT:
        if (l->hit_points <= 0 && l->frame_number == anims[objects[11].anim_index + 34].frame_base + 28) {
            l->frame_number = anims[objects[11].anim_index + 34].frame_base + 28;
            cart->Flags = (cart->Flags & ~CF_CONTROL) | CF_NOANIM;
            cart->Speed = v->speed = 0;
        }
        break;
    }

    if (lara.skidoo != NO_ITEM && !(cart->Flags & CF_NOANIM)) {
        AnimateItem(l);
        v->anim_number = objects[17].anim_index + (l->anim_number - objects[11].anim_index);
        v->frame_number = l->frame_number + anims[v->anim_number].frame_base - anims[l->anim_number].frame_base;
    }

    if (l->current_anim_state != CART_TURNDEATH && l->current_anim_state != CART_WALLDEATH && l->hit_points > 0) {
        if (v->pos.z_rot > 4096 || v->pos.z_rot < -4096) {
            l->anim_number = objects[11].anim_index + 31;
            l->frame_number = anims[l->anim_number].frame_base;
            l->current_anim_state = l->goal_anim_state = CART_TURNDEATH;
            cart->Flags = (cart->Flags & ~CF_CONTROL) | CF_STOPPED | CF_DEAD;
            cart->Speed = v->speed = 0;
            return;
        }

        fh = GetCollision(v, v->pos.y_rot, 512, &ch);
        if (fh < -(STEP_L * 2)) {
            l->anim_number = objects[11].anim_index + 23;
            l->frame_number = anims[l->anim_number].frame_base;
            l->current_anim_state = l->goal_anim_state = CART_WALLDEATH;
            cart->Flags = (cart->Flags & ~CF_CONTROL) | (CF_STOPPED | CF_DEAD);
            cart->Speed = v->speed = 0;
            l->hit_points = -1;
            return;
        }

        if (l->current_anim_state != CART_DUCK && l->current_anim_state != CART_HIT) {
            COLL_INFO coll;

            coll.quadrant = (uint16)(v->pos.y_rot + 0x2000) / 0x4000;
            coll.radius = 100;
            if (CollideStaticObjects(&coll, v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, v->room_number, 768)) {
                int hits;

                l->anim_number = objects[11].anim_index + 34;
                l->frame_number = anims[l->anim_number].frame_base;
                l->current_anim_state = l->goal_anim_state = CART_HIT;
                DoLotsOfBlood(l->pos.x_pos, l->pos.y_pos - 768, l->pos.z_pos, v->speed, v->pos.y_rot,
                              l->room_number, 3);
                if ((hits = 25 * (((uint16)cart->Speed) >> 11)) < 20)
                    hits = 20;
                l->hit_points -= hits;
                return;
            }
        }

        if (fh > 512 + 64 && !cart->YVel)
            cart->YVel = 0xfc00;
        CartToBaddieCollision(v);
    }
}

extern int height_type;

int CanGetOut_minecart(int direction)
{
    ITEM_INFO *v;
    FLOOR_INFO *floor;
    sint16 room_number, angle;
    int x, y, z, height, ceiling;

    v = &items[lara.skidoo];

    if (direction < 0)
        angle = v->pos.y_rot + 0x4000;
    else
        angle = v->pos.y_rot - 0x4000;

    x = v->pos.x_pos - (330 * phd_sin(angle) >> W2V_SHIFT);
    y = v->pos.y_pos;
    z = v->pos.z_pos - (330 * phd_cos(angle) >> W2V_SHIFT);

    room_number = v->room_number;
    floor = GetFloor(x, y, z, &room_number);
    height = GetHeight(floor, x, y, z);

    if (height_type == 2 || height_type == 3 || height == NO_HEIGHT)
        return 0;

    if (ABS(height - v->pos.y_pos) > WALL_L / 2)
        return 0;

    ceiling = GetCeiling(floor, x, y, z);

    if (ceiling - v->pos.y_pos > -762 || height - ceiling < 762)
        return 0;

    return 1;
}

int TestBoundsCollide(ITEM_INFO *item, ITEM_INFO *laraitem, sint32 radius);
void DoLotsOfBlood(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num, int number);

void CartToBaddieCollision(ITEM_INFO *v)
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
                if (object->collision && (object->intelligent || item->object_number == 350)) {
                    x = v->pos.x_pos - item->pos.x_pos;
                    y = v->pos.y_pos - item->pos.y_pos;
                    z = v->pos.z_pos - item->pos.z_pos;
                    if (x > -2048 && x < 2048 && z > -2048 && z < 2048 && y > -2048 && y < 2048) {
                        if (TestBoundsCollide(item, v, 256)) {
                            if (item->object_number == 350) {
                                if (item->frame_number == anims[item->anim_number].frame_base &&
                                    lara_item->current_anim_state == 18 &&
                                    lara_item->anim_number == objects[11].anim_index + 6) {
                                    sint16 frame;
                                    FLOOR_INFO *floor;
                                    sint16 room_number;

                                    frame = lara_item->frame_number - anims[lara_item->anim_number].frame_base;
                                    if (frame >= 12 && frame <= 22) {
                                        SoundEffect(220, &item->pos, 2);
                                        room_number = item->room_number;
                                        floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
                                        GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
                                        TestTriggers(trigger_index, 1);
                                        item->frame_number++;
                                    }
                                }
                            } else {
                                DoLotsOfBlood(item->pos.x_pos, v->pos.y_pos - 256, item->pos.z_pos, v->speed,
                                              v->pos.y_rot, item->room_number, 3);
                                item->hit_points = 0;
                            }
                        }
                    }
                }
            }
            item_num = nex;
        }
    }
}
