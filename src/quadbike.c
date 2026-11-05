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

extern char HandbrakeStarting;
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
