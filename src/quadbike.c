#include <windows.h>
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

extern sint32 *bones;

int GetFrames(ITEM_INFO *item, sint16 *frmptr[], int *rate);
void phd_TranslateAbs(sint32 x, sint32 y, sint32 z);
void phd_TranslateRel(sint32 x, sint32 y, sint32 z);
void phd_PushMatrix(void);
void phd_RotX(sint16 rx);
void phd_RotYXZ(sint16 ry, sint16 rx, sint16 rz);
int S_GetObjectBounds(sint16 *bptr);
void CalculateObjectLighting(ITEM_INFO *item, sint16 *frame);
void gar_RotYXZsuperpack(sint16 **pprot, int skip);
void phd_PutPolygons(sint16 *objptr, int clip);

void QuadBikeDraw(ITEM_INFO *item)
{
    sint16 *frmptr[2];
    int clip, rate, frac;

    frac = GetFrames(item, frmptr, &rate);

    phd_PushMatrix();
    phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);

    if ((clip = S_GetObjectBounds(frmptr[0]))) {
        sint32 *bone;
        QUADINFO *bike;
        sint16 **meshpp;
        sint16 *rotation;
        OBJECT_INFO *object;

        CalculateObjectLighting(item, frmptr[0]);

        object = &objects[item->object_number];
        meshpp = &meshes[object->mesh_index];
        bone = bones + object->bone_index;

        bike = (QUADINFO *)item->data;

        phd_TranslateRel((sint32)*(frmptr[0] + 6), (sint32)*(frmptr[0] + 7), (sint32)*(frmptr[0] + 8));
        rotation = frmptr[0] + 9;
        gar_RotYXZsuperpack(&rotation, 0);

        phd_PutPolygons(*meshpp++, clip);

        phd_PushMatrix();
        phd_TranslateRel(*(bone + 1), *(bone + 2), *(bone + 3));
        gar_RotYXZsuperpack(&rotation, 0);
        phd_PutPolygons(*meshpp++, clip);
        phd_PopMatrix();

        phd_PushMatrix();
        phd_TranslateRel(*(bone + 1 + 4), *(bone + 2 + 4), *(bone + 3 + 4));
        gar_RotYXZsuperpack(&rotation, 0);
        phd_PutPolygons(*meshpp++, clip);

        phd_PushMatrix();
        phd_TranslateRel(*(bone + 1 + 8), *(bone + 2 + 8), *(bone + 3 + 8));
        gar_RotYXZsuperpack(&rotation, 0);
        phd_RotX(bike->RearRot);
        phd_PutPolygons(*meshpp++, clip);
        phd_PopMatrix();

        phd_PushMatrix();
        phd_TranslateRel(*(bone + 1 + 12), *(bone + 2 + 12), *(bone + 3 + 12));
        gar_RotYXZsuperpack(&rotation, 0);
        phd_RotX(bike->RearRot);
        phd_PutPolygons(*meshpp++, clip);
        phd_PopMatrix();

        phd_PopMatrix();

        phd_PushMatrix();
        phd_TranslateRel(*(bone + 1 + 16), *(bone + 2 + 16), *(bone + 3 + 16));
        gar_RotYXZsuperpack(&rotation, 0);
        phd_PutPolygons(*meshpp++, clip);

        phd_PushMatrix();
        phd_TranslateRel(*(bone + 1 + 20), *(bone + 2 + 20), *(bone + 3 + 20));
        gar_RotYXZsuperpack(&rotation, 0);
        phd_RotX(bike->FrontRot);
        phd_PutPolygons(*meshpp++, clip);
        phd_PopMatrix();

        phd_PushMatrix();
        phd_TranslateRel(*(bone + 1 + 24), *(bone + 2 + 24), *(bone + 3 + 24));
        gar_RotYXZsuperpack(&rotation, 0);
        phd_RotX(bike->FrontRot);
        phd_PutPolygons(*meshpp++, clip);
        phd_PopMatrix();
        phd_PopMatrix();
    }

    phd_PopMatrix();
}

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

int TestBoundsCollide(ITEM_INFO *item, ITEM_INFO *laraitem, sint32 radius);
void DoLotsOfBlood(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num, int number);

void SkidooBaddieCollision(ITEM_INFO *skidoo)
{
    int i, x, y, z;
    ITEM_INFO *item;
    OBJECT_INFO *object;
    sint16 roomies[16], numroom, *door;
    sint16 item_num;

    numroom = 1;
    roomies[0] = skidoo->room_number;
    door = room[skidoo->room_number].door;
    if (door) {
        for (i = (int)*(door++); i > 0; i--) {
            roomies[numroom++] = *door;
            door += 16;
        }
    }
    for (i = 0; i < numroom; i++) {
        item_num = room[roomies[i]].item_number;
        while (item_num != NO_ITEM) {
            item = &items[item_num];
            if (item->collidable && item->status != INVISIBLE && item != lara_item && item != skidoo) {
                object = &objects[item->object_number];
                if (object->collision && (object->intelligent || item->object_number == 112)) {
                    x = skidoo->pos.x_pos - item->pos.x_pos;
                    y = skidoo->pos.y_pos - item->pos.y_pos;
                    z = skidoo->pos.z_pos - item->pos.z_pos;
                    if (x > -2048 && x < 2048 && z > -2048 && z < 2048 && y > -2048 && y < 2048) {
                        if (TestBoundsCollide(item, skidoo, 500)) {
                            if (item->object_number == 112) {
                                if (item->current_anim_state == 1) {
                                    lara_item->hit_status = 1;
                                    lara_item->hit_points -= 100;
                                }
                            } else {
                                DoLotsOfBlood(item->pos.x_pos, skidoo->pos.y_pos - 256, item->pos.z_pos, skidoo->speed,
                                              skidoo->pos.y_rot, item->room_number, 3);
                                item->hit_points = 0;
                            }
                        }
                    }
                }
            }
            item_num = item->next_item;
        }
    }
}

void GetLaraHandAbsPosition(PHD_VECTOR *vec, int hand);

int SkidooCheckGetOff(void)
{
    ITEM_INFO *v, *l;
    QUADINFO *bike;

    v = &items[lara.skidoo];
    l = lara_item;
    if ((l->current_anim_state == 10 || l->current_anim_state == 24) &&
        l->frame_number == anims[l->anim_number].frame_end) {
        if (l->current_anim_state == 24)
            l->pos.y_rot += 0x4000;
        else
            l->pos.y_rot -= 0x4000;
        l->anim_number = 11;
        l->frame_number = anims[11].frame_base;
        l->current_anim_state = l->goal_anim_state = AS_STOP;
        l->pos.x_pos -= 512 * phd_sin(l->pos.y_rot) >> W2V_SHIFT;
        l->pos.z_pos -= 512 * phd_cos(l->pos.y_rot) >> W2V_SHIFT;
        l->pos.x_rot = l->pos.z_rot = 0;
        lara.skidoo = NO_ITEM;
        lara.gun_status = LG_ARMLESS;
    } else if (l->frame_number == anims[l->anim_number].frame_end) {
        bike = (QUADINFO *)v->data;
        if (l->current_anim_state == 20) {
            PHD_VECTOR vec = {0, 0, 0};

            l->anim_number = 23;
            l->frame_number = anims[23].frame_base;
            l->current_anim_state = 9;
            GetLaraHandAbsPosition(&vec, 2);
            l->pos.x_pos = vec.x;
            l->pos.y_pos = vec.y;
            l->pos.z_pos = vec.z;
            l->fallspeed = v->fallspeed;
            l->gravity_status = 1;
            l->pos.x_rot = l->pos.z_rot = 0;
            l->hit_points = 0;
            lara.gun_status = LG_ARMLESS;
            v->flags |= ONESHOT;
            return 0;
        } else if (l->current_anim_state == 19) {
            l->goal_anim_state = 8;
            l->fallspeed = 154;
            l->speed = 0;
            bike->Flags |= 0x80;
            return 0;
        }
    }
    return 1;
}

extern char quad_slowdamp;
extern char dont_exit_quad;
extern char HandbrakeStarting;

int CanGetOff_quadbike(int direction);

void AnimateQuadBike(ITEM_INFO *v, int collide, int dead)
{
    ITEM_INFO *l;
    QUADINFO *bike;

    l = lara_item;
    bike = (QUADINFO *)v->data;
    if (v->pos.y_pos != v->floor && l->current_anim_state != 8 && l->current_anim_state != 17
        && l->current_anim_state != 20 && !dead) {
        if (bike->Velocity < 0)
            l->anim_number = objects[VEHICLE_ANIM].anim_index + 6;
        else
            l->anim_number = objects[VEHICLE_ANIM].anim_index + 25;
        l->frame_number = anims[l->anim_number].frame_base;
        l->current_anim_state = l->goal_anim_state = 8;
    } else if (collide && l->current_anim_state != 12 && l->current_anim_state != 11 && l->current_anim_state != 13
               && l->current_anim_state != 14 && l->current_anim_state != 20 && bike->Velocity > 0xa000 / 3 && !dead) {
        if (collide == 13) {
            l->anim_number = objects[VEHICLE_ANIM].anim_index + 12;
            l->current_anim_state = l->goal_anim_state = 12;
        } else if (collide == 14) {
            l->anim_number = objects[VEHICLE_ANIM].anim_index + 11;
            l->current_anim_state = l->goal_anim_state = 11;
        } else if (collide == 11) {
            l->anim_number = objects[VEHICLE_ANIM].anim_index + 14;
            l->current_anim_state = l->goal_anim_state = 13;
        } else {
            l->anim_number = objects[VEHICLE_ANIM].anim_index + 13;
            l->current_anim_state = l->goal_anim_state = 14;
        }
        l->frame_number = anims[l->anim_number].frame_base;
        SoundEffect(202, &v->pos, 0);
    } else
        switch (l->current_anim_state) {
        case 15:
            if (dead)
                l->goal_anim_state = 7;
            else if ((input & 0x1000) && bike->Velocity == 0 && !dont_exit_quad) {
                if ((input & IN_RIGHT) && CanGetOff_quadbike(1))
                    l->goal_anim_state = 10;
                else if ((input & IN_LEFT) && CanGetOff_quadbike(-1))
                    l->goal_anim_state = 24;
            } else if (input & (IN_ACTION | IN_JUMP)) {
                l->goal_anim_state = 1;
                quad_slowdamp = 0;
            }
            break;
        case 1:
            if (dead) {
                if (bike->Velocity > 0xa000 / 2)
                    l->goal_anim_state = 19;
                else
                    l->goal_anim_state = 7;
            } else if ((bike->Velocity >> 8) == 0 && !(input & (IN_ACTION | IN_JUMP)))
                l->goal_anim_state = 15;
            else if ((input & IN_LEFT) && !HandbrakeStarting)
                l->goal_anim_state = 2;
            else if ((input & IN_RIGHT) && !HandbrakeStarting)
                l->goal_anim_state = 22;
            else if (input & IN_JUMP) {
                if (bike->Velocity > (0xa000 / 3) * 2)
                    l->goal_anim_state = 6;
                else
                    l->goal_anim_state = 5;
            }
            break;
        case 6:
        case 5:
        case 18:
            if ((bike->Velocity >> 8) == 0)
                l->goal_anim_state = 15;
            else if (input & IN_LEFT)
                l->goal_anim_state = 2;
            else if (input & IN_RIGHT)
                l->goal_anim_state = 22;
            break;
        case 2:
            if ((bike->Velocity >> 8) == 0)
                l->goal_anim_state = 15;
            else if (input & IN_RIGHT) {
                l->anim_number = objects[VEHICLE_ANIM].anim_index + 20;
                l->frame_number = anims[l->anim_number].frame_base;
                l->current_anim_state = l->goal_anim_state = 22;
            } else if (!(input & IN_LEFT)) {
                l->goal_anim_state = 1;
                quad_slowdamp = 0;
            }
            break;
        case 22:
            if ((bike->Velocity >> 8) == 0)
                l->goal_anim_state = 15;
            else if (input & IN_LEFT) {
                l->anim_number = objects[VEHICLE_ANIM].anim_index + 3;
                l->frame_number = anims[l->anim_number].frame_base;
                l->current_anim_state = l->goal_anim_state = 2;
            } else if (!(input & IN_RIGHT)) {
                l->goal_anim_state = 1;
                quad_slowdamp = 0;
            }
            break;
        case 8:
            if (v->pos.y_pos == v->floor)
                l->goal_anim_state = 17;
            else if (v->fallspeed > 240 && CurrentLevel != 0)
                bike->Flags |= 0x40;
            break;
        case 20:
            break;
        case 12:
        case 11:
        case 13:
        case 14:
            if (input & (IN_ACTION | IN_JUMP))
                l->goal_anim_state = 1;
            break;
        }

    if (room[v->room_number].flags & (UNDERWATER | 128)) {
        l->goal_anim_state = 20;
        l->hit_points = 0;
        QuadbikeExplode(v);
    }
}

extern char CanHandbrakeStart;

int abs(int x);
#pragma intrinsic(abs)

int UserControl(ITEM_INFO *v, sint32 height, int *pitch)
{
    int drive, revs;
    QUADINFO *bike;

    drive = 0;
    bike = (QUADINFO *)v->data;
    if (!bike->Velocity && !(input & IN_DASH) && !CanHandbrakeStart)
        CanHandbrakeStart = 1;
    else if (bike->Velocity)
        CanHandbrakeStart = 0;

    if (!(input & IN_DASH))
        HandbrakeStarting = 0;

    if (!HandbrakeStarting) {
        if (bike->Revs > 0x10) {
            bike->Velocity += bike->Revs >> 4;
            bike->Revs -= bike->Revs >> 3;
        } else
            bike->Revs = 0;
    }

    if (v->pos.y_pos >= height - STEP_L) {
        if (!bike->Velocity && (input & IN_LOOK))
            LookUpDown();

        if (bike->Velocity > 0) {
            if ((input & IN_DASH) && !HandbrakeStarting && bike->Velocity > 0x3000) {
                if (input & IN_LEFT) {
                    bike->skidoo_turn -= 500;
                    if (bike->skidoo_turn < -0x5b0)
                        bike->skidoo_turn = -0x5b0;
                } else if (input & IN_RIGHT) {
                    bike->skidoo_turn += 500;
                    if (bike->skidoo_turn > 0x5b0)
                        bike->skidoo_turn = 0x5b0;
                }
            } else {
                if (input & IN_LEFT) {
                    bike->skidoo_turn -= 0x1c7;
                    if (bike->skidoo_turn < -0x38e)
                        bike->skidoo_turn = -0x38e;
                } else if (input & IN_RIGHT) {
                    bike->skidoo_turn += 0x1c7;
                    if (bike->skidoo_turn > 0x38e)
                        bike->skidoo_turn = 0x38e;
                }
            }
        } else if (bike->Velocity < 0) {
            if ((input & IN_DASH) && !HandbrakeStarting && bike->Velocity < -0x3000 + 0x800) {
                if (input & IN_RIGHT) {
                    bike->skidoo_turn -= 500;
                    if (bike->skidoo_turn < -0x5b0)
                        bike->skidoo_turn = -0x5b0;
                } else if (input & IN_LEFT) {
                    bike->skidoo_turn += 500;
                    if (bike->skidoo_turn > 0x5b0)
                        bike->skidoo_turn = 0x5b0;
                }
            } else {
                if (input & IN_RIGHT) {
                    bike->skidoo_turn -= 0x1c7;
                    if (bike->skidoo_turn < -0x38e)
                        bike->skidoo_turn = -0x38e;
                } else if (input & IN_LEFT) {
                    bike->skidoo_turn += 0x1c7;
                    if (bike->skidoo_turn > 0x38e)
                        bike->skidoo_turn = 0x38e;
                }
            }
        }

        if (input & IN_JUMP) {
            if ((input & IN_DASH) && (CanHandbrakeStart || HandbrakeStarting)) {
                HandbrakeStarting = 1;
                bike->Revs -= 0x200;
                if (bike->Revs < -0x3000)
                    bike->Revs = -0x3000;
            } else if (bike->Velocity > 0)
                bike->Velocity -= 0x280;
            else {
                if (bike->Velocity > -0x3000)
                    bike->Velocity += -0x300;
            }
        } else if (input & IN_ACTION) {
            if ((input & IN_DASH) && (CanHandbrakeStart || HandbrakeStarting)) {
                HandbrakeStarting = 1;
                bike->Revs += 0x200;
                if (bike->Revs >= 0xa000)
                    bike->Revs = 0xa000;
            } else if (bike->Velocity < 0xa000) {
                if (bike->Velocity < 0x4000)
                    bike->Velocity += 8 + ((0x4000 + 0x800 - bike->Velocity) >> 3);
                else if (bike->Velocity < 0x7000)
                    bike->Velocity += 4 + ((0x7000 + 0x800 - bike->Velocity) >> 4);
                else if (bike->Velocity < 0xa000)
                    bike->Velocity += 2 + ((0xa000 - bike->Velocity) >> 3);
            } else
                bike->Velocity = 0xa000;
            bike->Velocity -= ABS(v->pos.y_rot - bike->momentum_angle) >> 6;
        } else if (bike->Velocity > 0x100) {
            bike->Velocity -= 0x100;
        } else if (bike->Velocity < -0x100) {
            bike->Velocity += 0x100;
        } else {
            bike->Velocity = 0;
        }

        if (HandbrakeStarting && bike->Revs && !(input & (IN_ACTION | IN_JUMP))) {
            if (bike->Revs > 8)
                bike->Revs -= bike->Revs >> 3;
            else
                bike->Revs = 0;
        }

        v->speed = bike->Velocity >> 8;
        if (bike->EngineRevs > 0x7000)
            bike->EngineRevs = -0x2000;

        if (bike->Velocity < 0)
            revs = abs(bike->Velocity >> 1);
        else if (bike->Velocity < 0x7000)
            revs = -0x2000 + ((bike->Velocity * (0x6800 - -0x2000)) / 0x7000);
        else if (bike->Velocity <= 0xa000)
            revs = -0x2800 + (((bike->Velocity - 0x7000) * (0x7000 - -0x2800)) / (0xa000 - 0x7000));
        revs += abs(bike->Revs);
        bike->EngineRevs += (revs - bike->EngineRevs) >> 3;
    } else {
        if (bike->EngineRevs < 0xa000)
            bike->EngineRevs += (0xa000 - bike->EngineRevs) >> 3;
    }

    *pitch = bike->EngineRevs;
    return drive;
}

extern BITE_INFO quad_bites[6];
static uint8 ExhaustStart;

int SkidooDynamics(ITEM_INFO *v);
sint32 TestHeight_quadbike(ITEM_INFO *v, sint32 z_off, sint32 x_off, PHD_VECTOR *pos);
sint32 DoDynamics_quadbike(sint32 height, sint32 fallspeed, sint32 *y);
int UserControl(ITEM_INFO *v, sint32 height, int *pitch);
void AnimateQuadBike(ITEM_INFO *v, int collide, int dead);
void AnimateItem(ITEM_INFO *item);
void QuadbikeExplode(ITEM_INFO *v);
void TriggerExhaustSmoke(sint32 x, sint32 y, sint32 z, sint16 angle, sint32 speed, sint32 moving);
int SkidooCheckGetOff(void);

int abs(int x);
#pragma intrinsic(abs)

int QuadBikeControl(void)
{
    QUADINFO *bike;
    ITEM_INFO *v, *l;
    PHD_VECTOR fl, fr;
    sint32 hfl, hfr;
    FLOOR_INFO *floor;
    sint32 height, collide, drive, ceiling;
    sint16 room_number, x_rot, z_rot, rotadd;
    int pitch, dead;

    dead = 0;
    l = lara_item;
    v = &items[lara.skidoo];
    bike = (QUADINFO *)v->data;
    collide = SkidooDynamics(v);
    room_number = v->room_number;
    floor = GetFloor(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, &room_number);
    height = GetHeight(floor, v->pos.x_pos, v->pos.y_pos, v->pos.z_pos);
    ceiling = GetCeiling(floor, v->pos.x_pos, v->pos.y_pos, v->pos.z_pos);
    hfl = TestHeight_quadbike(v, 550, -260, &fl);
    hfr = TestHeight_quadbike(v, 550, 260, &fr);
    room_number = v->room_number;
    floor = GetFloor(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, &room_number);
    height = GetHeight(floor, v->pos.x_pos, v->pos.y_pos, v->pos.z_pos);
    TestTriggers(trigger_index, 0);

    if (lara_item->hit_points <= 0) {
        input &= ~(IN_LEFT | IN_RIGHT | IN_BACK | IN_FORWARD);
        dead = 1;
    }

    if (bike->Flags) {
        collide = 0;
    } else {
        switch (lara_item->current_anim_state) {
        case 9:
        case 10:
        case 23:
        case 24:
            drive = -1;
            collide = 0;
            break;
        default:
            drive = UserControl(v, height, &pitch);
            break;
        }
    }

    if (bike->Velocity || bike->Revs) {
        bike->pitch = pitch;
        if (bike->pitch < -0x8000)
            bike->pitch = -0x8000;
        else if (bike->pitch > 0xa000)
            bike->pitch = 0xa000;
        SoundEffect(155, &v->pos, 4 + ((0x10000 + bike->pitch) << 8));
    } else {
        if (drive != -1)
            SoundEffect(153, &v->pos, 0);
        bike->pitch = 0;
    }

    v->floor = height;
    rotadd = bike->Velocity >> 2;
    bike->RearRot -= rotadd;
    bike->RearRot -= bike->Revs >> 3;
    bike->FrontRot -= rotadd;
    bike->left_fallspeed = DoDynamics_quadbike(hfl, bike->left_fallspeed, &fl.y);
    bike->right_fallspeed = DoDynamics_quadbike(hfr, bike->right_fallspeed, &fr.y);
    v->fallspeed = DoDynamics_quadbike(height, v->fallspeed, &v->pos.y_pos);
    height = (fl.y + fr.y) >> 1;
    x_rot = phd_atan(550, v->pos.y_pos - height);
    z_rot = phd_atan(260, height - fl.y);
    v->pos.x_rot += (x_rot - v->pos.x_rot) >> 1;
    v->pos.z_rot += (z_rot - v->pos.z_rot) >> 1;

    if (!(bike->Flags & 0x80)) {
        if (room_number != v->room_number) {
            ItemNewRoom(lara.skidoo, room_number);
            ItemNewRoom(lara.item_number, room_number);
        }
        l->pos.x_pos = v->pos.x_pos;
        l->pos.y_pos = v->pos.y_pos;
        l->pos.z_pos = v->pos.z_pos;
        l->pos.y_rot = v->pos.y_rot;
        l->pos.x_rot = v->pos.x_rot;
        l->pos.z_rot = v->pos.z_rot;
        AnimateQuadBike(v, collide, dead);
        AnimateItem(l);
        v->anim_number = objects[16].anim_index + (l->anim_number - objects[VEHICLE_ANIM].anim_index);
        v->frame_number = anims[v->anim_number].frame_base + (l->frame_number - anims[l->anim_number].frame_base);
        camera.target_elevation = -30 * ONE_DEGREE;
        if (bike->Flags & 0x40) {
            if (v->pos.y_pos == v->floor) {
                ExplodingDeath(lara.item_number, 0xffffffff, 1);
                lara_item->hit_points = 0;
                lara_item->flags |= ONESHOT;
                QuadbikeExplode(v);
                return 0;
            }
        }
    }

    if (l->current_anim_state != 9 && l->current_anim_state != 23 && l->current_anim_state != 10 && l->current_anim_state != 24) {
        PHD_VECTOR pos;
        sint32 lp, speed;
        sint16 angle;

        for (lp = 0; lp < 2; lp++) {
            pos.x = quad_bites[lp].x;
            pos.y = quad_bites[lp].y;
            pos.z = quad_bites[lp].z;
            GetJointAbsPosition(v, &pos, quad_bites[lp].mesh_num);
            angle = v->pos.y_rot + ((lp == 0) ? 0x9000 : 0x7000);
            if (v->speed > 32) {
                if (v->speed < 64) {
                    speed = 64 - v->speed;
                    TriggerExhaustSmoke(pos.x, pos.y, pos.z, angle, speed, 1);
                }
            } else {
                if (ExhaustStart < 16) {
                    speed = ((ExhaustStart << 1) + (GetRandomControl() & 7) + (GetRandomControl() & 16)) << 7;
                    ExhaustStart++;
                } else if (HandbrakeStarting)
                    speed = (abs(bike->Revs) >> 2) + ((GetRandomControl() & 7) << 7);
                else if ((GetRandomControl() & 3) == 0)
                    speed = ((GetRandomControl() & 15) + (GetRandomControl() & 16)) << 7;
                else
                    speed = 0;
                TriggerExhaustSmoke(pos.x, pos.y, pos.z, angle, speed, 0);
            }
        }
    } else {
        if (CurrentLevel == 0) {
            savegame.timer = 0;
            assault_timer_active = 0;
            assault_timer_display = 0;
        }
        ExhaustStart = 0;
    }

    return SkidooCheckGetOff();
}

sint32 DoDynamics_quadbike(sint32 height, sint32 fallspeed, sint32 *y)
{
    sint32 kick;

    if (height > *y) {
        *y += fallspeed;
        if (*y > height - 80) {
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

int GetCollisionAnim_quadbike(ITEM_INFO *v, PHD_VECTOR *moved)
{
    sint32 c, s, front, side;

    moved->x = v->pos.x_pos - moved->x;
    moved->z = v->pos.z_pos - moved->z;
    if (moved->x || moved->z) {
        c = phd_cos(v->pos.y_rot);
        s = phd_sin(v->pos.y_rot);
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

FLOOR_INFO *GetFloor(sint32 x, sint32 y, sint32 z, sint16 *room_number);
sint32 GetHeight(FLOOR_INFO *floor, sint32 x, sint32 y, sint32 z);
sint32 GetCeiling(FLOOR_INFO *floor, sint32 x, sint32 y, sint32 z);
sint32 TestHeight_quadbike(ITEM_INFO *v, sint32 z_off, sint32 x_off, PHD_VECTOR *pos);
sint32 DoDynamics_quadbike(sint32 height, sint32 fallspeed, sint32 *y);
sint32 __fastcall phd_sin(sint32 angle);

sint32 TestHeight_quadbike(ITEM_INFO *v, sint32 x, sint32 z, PHD_VECTOR *pos)
{
    sint32 s, c;
    sint16 room_number;
    FLOOR_INFO *floor;

    pos->y = v->pos.y_pos + ((z * phd_sin(v->pos.z_rot)) >> W2V_SHIFT) - ((x * phd_sin(v->pos.x_rot)) >> W2V_SHIFT);
    c = phd_cos(v->pos.y_rot);
    s = phd_sin(v->pos.y_rot);
    pos->z = v->pos.z_pos + ((x * c - z * s) >> W2V_SHIFT);
    pos->x = v->pos.x_pos + ((x * s + z * c) >> W2V_SHIFT);
    room_number = v->room_number;
    floor = GetFloor(pos->x, pos->y, pos->z, &room_number);
    c = GetCeiling(floor, pos->x, pos->y, pos->z);
    if (pos->y < c || c == NO_HEIGHT)
        return NO_HEIGHT;
    return GetHeight(floor, pos->x, pos->y, pos->z);
}

extern int height_type;

int CanGetOff_quadbike(int direction)
{
    ITEM_INFO *v;
    FLOOR_INFO *floor;
    sint16 room_number, angle;
    int x, y, z, height, ceiling;

    v = &items[lara.skidoo];

    if (direction < 0)
        angle = v->pos.y_rot - 0x4000;
    else
        angle = v->pos.y_rot + 0x4000;

    x = v->pos.x_pos + (512 * phd_sin(angle) >> W2V_SHIFT);
    y = v->pos.y_pos;
    z = v->pos.z_pos + (512 * phd_cos(angle) >> W2V_SHIFT);

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

FLOOR_INFO *GetFloor(sint32 x, sint32 y, sint32 z, sint16 *room_number);
sint32 GetHeight(FLOOR_INFO *floor, sint32 x, sint32 y, sint32 z);
int DoShift_boat(ITEM_INFO *boat, PHD_VECTOR *newer, PHD_VECTOR *old);
int GetCollisionAnim_quadbike(ITEM_INFO *v, PHD_VECTOR *moved);
void SkidooBaddieCollision(ITEM_INFO *skidoo);
sint32 DoDynamics_quadbike(sint32 height, sint32 fallspeed, sint32 *y);

int SkidooDynamics(ITEM_INFO *v)
{
    QUADINFO *bike;
    PHD_VECTOR moved, fl, fr, br, bl, mtl, mbl, mtr, mbr, mml, mmr;
    PHD_VECTOR old, fl_old, fr_old, bl_old, br_old, mtl_old, mbl_old, mtr_old, mbr_old, mml_old, mmr_old;
    sint32 hfl, hfr, hbr, hbl, hmtl, hmbl, hmtr, hmbr, hmml, hmmr;
    sint32 hfr_old, hfl_old, hbr_old, hbl_old, hmtl_old, hmbl_old, hmtr_old, hmbr_old, hmml_old, hmmr_old;
    FLOOR_INFO *floor;
    sint32 height, slip, collide, speed;
    sint16 room_number, rot, rotadd;
    int newspeed;

    dont_exit_quad = 0;
    bike = (QUADINFO *)v->data;
    hfl_old = TestHeight_quadbike(v, 550, -260, &fl_old);
    hfr_old = TestHeight_quadbike(v, 550, 260, &fr_old);
    hbl_old = TestHeight_quadbike(v, -550, -260, &bl_old);
    hbr_old = TestHeight_quadbike(v, -550, 260, &br_old);
    hmml_old = TestHeight_quadbike(v, 0, -260, &mml_old);
    hmmr_old = TestHeight_quadbike(v, 0, 260, &mmr_old);
    hmtl_old = TestHeight_quadbike(v, 550 >> 1, -260, &mtl_old);
    hmtr_old = TestHeight_quadbike(v, 550 >> 1, 260, &mtr_old);
    hmbl_old = TestHeight_quadbike(v, -550 >> 1, -260, &mbl_old);
    hmbr_old = TestHeight_quadbike(v, -550 >> 1, 260, &mbr_old);
    old.x = v->pos.x_pos;
    old.y = v->pos.y_pos;
    old.z = v->pos.z_pos;
    if (bl_old.y > hbl_old)
        bl_old.y = hbl_old;
    if (br_old.y > hbr_old)
        br_old.y = hbr_old;
    if (fl_old.y > hfl_old)
        fl_old.y = hfl_old;
    if (fr_old.y > hfr_old)
        fr_old.y = hfr_old;
    if (mbl_old.y > hmbl_old)
        mbl_old.y = hmbl_old;
    if (mbr_old.y > hmbr_old)
        mbr_old.y = hmbr_old;
    if (mtl_old.y > hmtl_old)
        mtl_old.y = hmtl_old;
    if (mtr_old.y > hmtr_old)
        mtr_old.y = hmtr_old;
    if (mml_old.y > hmml_old)
        mml_old.y = hmml_old;
    if (mmr_old.y > hmmr_old)
        mmr_old.y = hmmr_old;
    if (v->pos.y_pos > (v->floor - STEP_L)) {
        sint16 momentum;

        if (bike->skidoo_turn < -(ONE_DEGREE * 2))
            bike->skidoo_turn += (ONE_DEGREE * 2);
        else if (bike->skidoo_turn > (ONE_DEGREE * 2))
            bike->skidoo_turn -= (ONE_DEGREE * 2);
        else
            bike->skidoo_turn = 0;
        v->pos.y_rot += bike->skidoo_turn + bike->extra_rotation;
        rot = v->pos.y_rot - bike->momentum_angle;
        momentum = (ONE_DEGREE * 3) - ((((((ONE_DEGREE * 3) - (ONE_DEGREE + (ONE_DEGREE / 2))) << 8) / 0xa000) * bike->Velocity) >> 8);
        if (!(input & IN_ACTION) && bike->Velocity > 0)
            momentum += momentum >> 2;
        if (rot < -(ONE_DEGREE + (ONE_DEGREE / 2))) {
            if (rot < -(ONE_DEGREE * 150)) {
                rot = -(ONE_DEGREE * 150);
                bike->momentum_angle = v->pos.y_rot - rot;
            } else
                bike->momentum_angle -= momentum;
        } else if (rot > (ONE_DEGREE + (ONE_DEGREE / 2))) {
            if (rot > (ONE_DEGREE * 150)) {
                rot = (ONE_DEGREE * 150);
                bike->momentum_angle = v->pos.y_rot - rot;
            } else
                bike->momentum_angle += momentum;
        } else
            bike->momentum_angle = v->pos.y_rot;
    } else
        v->pos.y_rot += bike->skidoo_turn + bike->extra_rotation;
    room_number = v->room_number;
    floor = GetFloor(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, &room_number);
    height = GetHeight(floor, v->pos.x_pos, v->pos.y_pos, v->pos.z_pos);
    if (v->pos.y_pos >= height) {
        speed = (v->speed * phd_cos(v->pos.x_rot)) >> W2V_SHIFT;
    } else
        speed = v->speed;
    v->pos.z_pos += (speed * phd_cos(bike->momentum_angle)) >> W2V_SHIFT;
    v->pos.x_pos += (speed * phd_sin(bike->momentum_angle)) >> W2V_SHIFT;
    slip = 100 * phd_sin(v->pos.x_rot) >> W2V_SHIFT;
    if (ABS(slip) > 100 / 2) {
        dont_exit_quad = 1;
        if (slip > 0)
            slip -= 10;
        else
            slip += 10;
        v->pos.z_pos -= slip * phd_cos(v->pos.y_rot) >> W2V_SHIFT;
        v->pos.x_pos -= slip * phd_sin(v->pos.y_rot) >> W2V_SHIFT;
    }
    slip = 50 * phd_sin(v->pos.z_rot) >> W2V_SHIFT;
    if (ABS(slip) > 50 / 2) {
        dont_exit_quad = 1;
        v->pos.z_pos -= slip * phd_sin(v->pos.y_rot) >> W2V_SHIFT;
        v->pos.x_pos += slip * phd_cos(v->pos.y_rot) >> W2V_SHIFT;
    }
    moved.x = v->pos.x_pos;
    moved.z = v->pos.z_pos;
    if (!(v->flags & ONESHOT))
        SkidooBaddieCollision(v);
    rot = 0;
    hfl = TestHeight_quadbike(v, 550, -260, &fl);
    if (hfl < fl_old.y - STEP_L)
        rot = DoShift_boat(v, &fl, &fl_old);
    hmtl = TestHeight_quadbike(v, 550 >> 1, -260, &mtl);
    if (hmtl < mtl_old.y - STEP_L)
        DoShift_boat(v, &mtl, &mtl_old);
    hmml = TestHeight_quadbike(v, 0, -260, &mml);
    if (hmml < mml_old.y - STEP_L)
        DoShift_boat(v, &mml, &mml_old);
    hmbl = TestHeight_quadbike(v, -550 >> 1, -260, &mbl);
    if (hmbl < mbl_old.y - STEP_L)
        DoShift_boat(v, &mbl, &mbl_old);
    hbl = TestHeight_quadbike(v, -550, -260, &bl);
    if (hbl < bl_old.y - STEP_L) {
        rotadd = DoShift_boat(v, &bl, &bl_old);
        if ((rotadd > 0 && rot >= 0) || (rotadd < 0 && rot <= 0))
            rot += rotadd;
    }
    hfr = TestHeight_quadbike(v, 550, 260, &fr);
    if (hfr < fr_old.y - STEP_L) {
        rotadd = DoShift_boat(v, &fr, &fr_old);
        if ((rotadd > 0 && rot >= 0) || (rotadd < 0 && rot <= 0))
            rot += rotadd;
    }
    hmtr = TestHeight_quadbike(v, 550 >> 1, 260, &mtr);
    if (hmtr < mtr_old.y - STEP_L)
        DoShift_boat(v, &mtr, &mtr_old);
    hmmr = TestHeight_quadbike(v, 0, 260, &mmr);
    if (hmmr < mmr_old.y - STEP_L)
        DoShift_boat(v, &mmr, &mmr_old);
    hmbr = TestHeight_quadbike(v, -550 >> 1, 260, &mbr);
    if (hmbr < mbr_old.y - STEP_L)
        DoShift_boat(v, &mbr, &mbr_old);
    hbr = TestHeight_quadbike(v, -550, 260, &br);
    if (hbr < br_old.y - STEP_L) {
        rotadd = DoShift_boat(v, &br, &br_old);
        if ((rotadd > 0 && rot >= 0) || (rotadd < 0 && rot <= 0))
            rot += rotadd;
    }
    room_number = v->room_number;
    floor = GetFloor(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, &room_number);
    height = GetHeight(floor, v->pos.x_pos, v->pos.y_pos, v->pos.z_pos);
    if (height < v->pos.y_pos - STEP_L)
        DoShift_boat(v, (PHD_VECTOR *)&v->pos, &old);
    bike->extra_rotation = rot;
    collide = GetCollisionAnim_quadbike(v, &moved);
    if (collide) {
        newspeed = ((v->pos.z_pos - old.z) * phd_cos(bike->momentum_angle) + (v->pos.x_pos - old.x) * phd_sin(bike->momentum_angle)) >> W2V_SHIFT;
        newspeed <<= 8;
        if ((&items[lara.skidoo] == v) && (bike->Velocity == 0xa000) && (newspeed < (bike->Velocity - 10))) {
            lara_item->hit_points -= (bike->Velocity - newspeed) >> 7;
            lara_item->hit_status = 1;
        }
        if (bike->Velocity > 0 && newspeed < bike->Velocity)
            bike->Velocity = (newspeed < 0) ? 0 : newspeed;
        else if (bike->Velocity < 0 && newspeed > bike->Velocity)
            bike->Velocity = (newspeed > 0) ? 0 : newspeed;
        if (bike->Velocity < -0x3000)
            bike->Velocity = -0x3000;
    }
    return collide;
}

#ifdef TR3_EXE
BITE_INFO quad_bites[6] = {
    { -56, -32, -380, 0 },
    { 56, -32, -380, 0 },
    { -8, 180, -48, 3 },
    { 8, 180, -48, 4 },
    { 90, 180, -32, 6 },
    { -90, 180, -32, 7 },
};
char tunes[4] = {
    9, 12, 4, 12,
};
#endif
