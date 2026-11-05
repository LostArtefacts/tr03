#include "game.h"

typedef struct {
    sint32 Velocity;
    sint16 FrontRot;
    sint16 RearRot;
    sint32 Revs;
    sint32 EngineRevs;
    sint16 track_mesh;
    sint32 skidoo_turn;
    sint32 left_fallspeed;
    sint32 right_fallspeed;
    sint16 momentum_angle;
    sint16 extra_rotation;
    sint32 pitch;
    uint8 Flags;
} QUADINFO;

void InitialiseQuadBike(sint16 item_number)
{
    ITEM_INFO *v;
    QUADINFO *bike;

    v = &items[item_number];
    bike = v->data = game_malloc(sizeof(QUADINFO), 0);
    bike->Velocity = 0;
    bike->skidoo_turn = 0;
    bike->left_fallspeed = bike->right_fallspeed = 0;
    bike->momentum_angle = v->pos.y_rot;
    bike->extra_rotation = 0;
    bike->track_mesh = 0;
    bike->pitch = 0;
    bike->Flags = 0;
}

int GetOnQuadBike(sint16 item_number, COLL_INFO *coll)
{
    int dist;
    sint32 x, z;
    ITEM_INFO *v, *l;
    FLOOR_INFO *floor;
    sint16 room_number, ang;
    uint16 tempang;

    l = lara_item;
    v = &items[item_number];
    if (!(input & IN_ACTION) || (v->flags & ONESHOT) || lara.gun_status != LG_ARMLESS || l->gravity_status
        || ABS(v->pos.y_pos - l->pos.y_pos) > 256)
        return 0;

    x = l->pos.x_pos - v->pos.x_pos;
    z = l->pos.z_pos - v->pos.z_pos;
    dist = x * x + z * z;
    if (dist > 170000)
        return 0;

    room_number = v->room_number;
    floor = GetFloor(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, &room_number);
    if (GetHeight(floor, v->pos.x_pos, v->pos.y_pos, v->pos.z_pos) < -32000)
        return 0;

    ang = phd_atan(v->pos.z_pos - l->pos.z_pos, v->pos.x_pos - l->pos.x_pos);
    ang -= v->pos.y_rot;
    if (ang > -(ONE_DEGREE * 45) && ang < ONE_DEGREE * 135) {
        tempang = l->pos.y_rot - v->pos.y_rot;
        if (tempang > 45 * ONE_DEGREE && tempang < 135 * ONE_DEGREE)
            return 1;
        else
            return 0;
    } else {
        tempang = l->pos.y_rot - v->pos.y_rot;
        if (tempang > 225 * ONE_DEGREE && tempang < 315 * ONE_DEGREE)
            return 1;
        else
            return 0;
    }
    return 0;
}

void CreateFlare(int thrown);
void S_CDPlay(sint32 track, int mode);
void ObjectCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);

extern char tunes[];
extern sint16 cdtrack;
extern char IsAtmospherePlaying;
extern uint32 CurrentLevel;

void QuadBikeCollision(sint16 item_number, ITEM_INFO *l, COLL_INFO *coll)
{
    int geton;

    if (l->hit_points < 0 || lara.skidoo != NO_ITEM)
        return;

    if ((geton = GetOnQuadBike(item_number, coll))) {
        sint16 ang;
        QUADINFO *bike;
        ITEM_INFO *v;

        lara.skidoo = item_number;
        if (lara.gun_type == LG_FLARE) {
            CreateFlare(0);
            undraw_flare_meshes();
            lara.flare_control_left = 0;
            lara.request_gun_type = lara.gun_type = LG_ARMLESS;
        }
        lara.gun_status = LG_HANDSBUSY;

        v = &items[item_number];
        ang = phd_atan(v->pos.z_pos - l->pos.z_pos, v->pos.x_pos - l->pos.x_pos);
        ang -= v->pos.y_rot;
        if (ang > -(ONE_DEGREE * 45) && ang < ONE_DEGREE * 135) {
            l->anim_number = objects[VEHICLE_ANIM].anim_index + 23;
            l->current_anim_state = l->goal_anim_state = 23;
        } else {
            l->anim_number = objects[VEHICLE_ANIM].anim_index + 9;
            l->current_anim_state = l->goal_anim_state = 9;
        }
        l->frame_number = anims[l->anim_number].frame_base;

        v->hit_points = 1;
        l->pos.x_pos = v->pos.x_pos;
        l->pos.y_pos = v->pos.y_pos;
        l->pos.z_pos = v->pos.z_pos;
        l->pos.y_rot = v->pos.y_rot;
        lara.head_x_rot = lara.head_y_rot = 0;
        lara.torso_x_rot = lara.torso_y_rot = 0;
        lara.hit_direction = -1;
        AnimateItem(l);

        if (CurrentLevel == 3) {
            int track;

            track = tunes[GetRandomControl() & 3];
            if (track != cdtrack && IsAtmospherePlaying) {
                cdtrack = track;
                S_CDPlay(track, 0);
            }
        }

        bike = (QUADINFO *)v->data;
        bike->Revs = 0;
    } else {
        ObjectCollision(item_number, l, coll);
    }
}

void TriggerUnderwaterExplosion(ITEM_INFO *item);
void TriggerExplosionSparks(sint32 x, sint32 y, sint32 z, sint32 extra_trigs, sint32 dyn, sint32 uw, sint16 roomnum);
int ExplodingDeath(sint16 item_number, sint32 mesh_bits, sint16 damage);

void QuadbikeExplode(ITEM_INFO *v)
{
    sint32 lp;

    if (room[v->room_number].flags & UNDERWATER) {
        TriggerUnderwaterExplosion(v);
    } else {
        TriggerExplosionSparks(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, 3, -2, 0, v->room_number);
        for (lp = 0; lp < 3; lp++)
            TriggerExplosionSparks(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, 3, -1, 0, v->room_number);
    }
    ExplodingDeath(lara.skidoo, 0xfffffffe, 1);
    KillItem(lara.skidoo);
    v->status = DEACTIVATED;
    SoundEffect(105, NULL, 0);
    SoundEffect(106, NULL, 0);
    lara.skidoo = NO_ITEM;
}

sint32 GetFreeSpark(void);

void TriggerExhaustSmoke(sint32 x, sint32 y, sint32 z, sint16 angle, sint32 speed, sint32 moving)
{
    sint32 size, xv, zv;
    SPARKS *sptr;

    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sR = 0;
    sptr->sG = 0;
    sptr->sB = 0;
    sptr->dR = 96;
    sptr->dG = 96;
    sptr->dB = 128;
    if (moving) {
        sptr->dR = (sptr->dR * speed) >> 5;
        sptr->dG = (sptr->dG * speed) >> 5;
        sptr->dB = (sptr->dB * speed) >> 5;
    }
    sptr->ColFadeSpeed = 4;
    sptr->FadeToBlack = 4;
    sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 20 - (speed >> 12);
    if (sptr->sLife < 9)
        sptr->sLife = sptr->Life = 9;
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = x + ((GetRandomControl() & 15) - 8);
    sptr->y = y + ((GetRandomControl() & 15) - 8);
    sptr->z = z + ((GetRandomControl() & 15) - 8);
    zv = (speed * phd_cos(angle)) >> (W2V_SHIFT + 2);
    xv = (speed * phd_sin(angle)) >> (W2V_SHIFT + 2);
    sptr->Xvel = xv + ((GetRandomControl() & 255) - 128);
    sptr->Yvel = -(GetRandomControl() & 7) - 8;
    sptr->Zvel = zv + ((GetRandomControl() & 255) - 128);
    sptr->Friction = 4;
    if (GetRandomControl() & 1) {
        sptr->Flags = 2 | 8 | 16 | 512;
        sptr->RotAng = GetRandomControl() & 4095;
        if (GetRandomControl() & 1)
            sptr->RotAdd = -(GetRandomControl() & 7) - 24;
        else
            sptr->RotAdd = (GetRandomControl() & 7) + 24;
    } else
        sptr->Flags = 2 | 8 | 512;
    sptr->Def = objects[296].mesh_index;
    sptr->Scalar = 2;
    sptr->Gravity = -(GetRandomControl() & 3) - 4;
    sptr->MaxYvel = -(GetRandomControl() & 7) - 8;
    size = (GetRandomControl() & 7) + 32 + (speed >> 7);
    sptr->Width = sptr->sWidth = size >> 1;
    sptr->dWidth = size;
    sptr->Height = sptr->sHeight = size >> 1;
    sptr->dHeight = size;
}
