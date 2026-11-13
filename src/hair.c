#include <windows.h>
#include "game.h"

#define HAIR_SEGMENTS 6

extern sint32 first_hair;
extern PHD_VECTOR hvel[HAIR_SEGMENTS + 1];
extern PHD_3DPOS hair[HAIR_SEGMENTS + 1];
extern sint32 *bones;

void InitialiseHair(void)
{
    int i;
    sint32 *bone;

    first_hair = 1;
    bone = bones + objects[HAIR].bone_index;
    hair[0].y_rot = 0;
    hair[0].x_rot = -0x4000;
    for (i = 1; i < HAIR_SEGMENTS + 1; i++, bone += 4)
    {
        hair[i].x_pos = *(bone + 1);
        hair[i].y_pos = *(bone + 2);
        hair[i].z_pos = *(bone + 3);
        hair[i].x_rot = -0x4000;
        hair[i].y_rot = hair[i].z_rot = 0;
        hvel[i].x = hvel[i].y = hvel[i].z = 0;
    }
}

#define NOT_INSIDE 32
#define SQUARE(x) ((x) * (x))
#define M03 3
#define M13 7
#define M23 11

typedef struct {
    sint32 x;
    sint32 y;
    sint32 z;
    sint32 r;
} SPHERE;

extern sint32 wind;
extern sint32 wind_angle;
extern sint32 dwind_angle;
extern sint32 SmokeWindX;
extern sint32 SmokeWindZ;
extern sint16 rcossin_tbl[];

void phd_PushUnitMatrix(void);
void phd_PushMatrix(void);
sint16 *GetBestFrame(ITEM_INFO *item);
void gar_RotYXZsuperpack(sint16 **pprot, int skip);
void phd_RotYXZ(sint16 ry, sint16 rx, sint16 rz);
void phd_TranslateRel(sint32 x, sint32 y, sint32 z);
sint32 GetWaterHeight(sint32 x, sint32 y, sint32 z, sint16 room_number);
FLOOR_INFO *GetFloor(sint32 x, sint32 y, sint32 z, sint16 *room_number);
sint32 GetHeight(FLOOR_INFO *floor, sint32 x, sint32 y, sint32 z);
uint32 __fastcall phd_sqrt(uint32 n);

void HairControl(int in_cutscene)
{
    OBJECT_INFO *object;
    sint32 *bone, distance;
    sint16 *rotation;
    sint16 *frame, *objptr, room_number;
    PHD_VECTOR pos;
    FLOOR_INFO *floor;
    sint32 i, water_level, height, size;
    SPHERE sphere[5];
    sint32 j, x, y, z;

    object = &objects[LARA];
    if (lara.hit_direction >= 0) {
        sint16 spaz;

        switch (lara.hit_direction) {
        case 0:
            if (lara.IsDucked)
                spaz = 294;
            else
                spaz = 125;
            break;
        case 2:
            if (lara.IsDucked)
                spaz = 293;
            else
                spaz = 126;
            break;
        case 1:
            if (lara.IsDucked)
                spaz = 295;
            else
                spaz = 127;
            break;
        default:
            if (lara.IsDucked)
                spaz = 296;
            else
                spaz = 128;
            break;
        }
        frame = anims[spaz].frame_ptr;
        size = anims[spaz].interpolation >> 8;
        frame += (int)(lara.hit_frame * size);
    } else
        frame = GetBestFrame(lara_item);

    phd_PushUnitMatrix();
    *(phd_mxptr + M03) = lara_item->pos.x_pos << W2V_SHIFT;
    *(phd_mxptr + M13) = lara_item->pos.y_pos << W2V_SHIFT;
    *(phd_mxptr + M23) = lara_item->pos.z_pos << W2V_SHIFT;
    phd_RotYXZ(lara_item->pos.y_rot, lara_item->pos.x_rot, lara_item->pos.z_rot);

    rotation = frame + 9;
    bone = bones + object->bone_index;
    phd_TranslateRel((sint32)*(frame + 6), (sint32)*(frame + 7), (sint32)*(frame + 8));
    gar_RotYXZsuperpack(&rotation, 0);

    phd_PushMatrix();
    objptr = lara.mesh_ptrs[HIPS];
    phd_TranslateRel(*objptr, *(objptr + 1), *(objptr + 2));
    sphere[0].x = *(phd_mxptr + M03) >> W2V_SHIFT;
    sphere[0].y = *(phd_mxptr + M13) >> W2V_SHIFT;
    sphere[0].z = *(phd_mxptr + M23) >> W2V_SHIFT;
    sphere[0].r = (sint32)*(objptr + 3);
    phd_PopMatrix();

    phd_TranslateRel(*(bone + 1 + 24), *(bone + 2 + 24), *(bone + 3 + 24));
    if (lara.weapon_item != NO_ITEM && lara.gun_type == LG_M16
        && (items[lara.weapon_item].current_anim_state == 0
            || items[lara.weapon_item].current_anim_state == 2
            || items[lara.weapon_item].current_anim_state == 4)) {
        rotation = lara.right_arm.frame_base
            + lara.right_arm.frame_number * (anims[lara.right_arm.anim_number].interpolation >> 8) + 9;
        gar_RotYXZsuperpack(&rotation, 7);
    } else
        gar_RotYXZsuperpack(&rotation, 6);
    phd_RotYXZ(lara.torso_y_rot, lara.torso_x_rot, lara.torso_z_rot);

    phd_PushMatrix();
    objptr = lara.mesh_ptrs[TORSO];
    phd_TranslateRel(*objptr, *(objptr + 1), *(objptr + 2));
    sphere[1].x = *(phd_mxptr + M03) >> W2V_SHIFT;
    sphere[1].y = *(phd_mxptr + M13) >> W2V_SHIFT;
    sphere[1].z = *(phd_mxptr + M23) >> W2V_SHIFT;
    sphere[1].r = (sint32)*(objptr + 3);
    phd_PopMatrix();

    phd_PushMatrix();
    phd_TranslateRel(*(bone + 1 + 28), *(bone + 2 + 28), *(bone + 3 + 28));
    gar_RotYXZsuperpack(&rotation, 0);
    objptr = lara.mesh_ptrs[UARM_R];
    phd_TranslateRel(*objptr, *(objptr + 1), *(objptr + 2));
    sphere[3].x = *(phd_mxptr + M03) >> W2V_SHIFT;
    sphere[3].y = *(phd_mxptr + M13) >> W2V_SHIFT;
    sphere[3].z = *(phd_mxptr + M23) >> W2V_SHIFT;
    sphere[3].r = (sint32)*(objptr + 3) * 3 / 2;
    phd_PopMatrix();

    phd_PushMatrix();
    phd_TranslateRel(*(bone + 1 + 40), *(bone + 2 + 40), *(bone + 3 + 40));
    gar_RotYXZsuperpack(&rotation, 2);
    objptr = lara.mesh_ptrs[UARM_R];
    phd_TranslateRel(*objptr, *(objptr + 1), *(objptr + 2));
    sphere[4].x = *(phd_mxptr + M03) >> W2V_SHIFT;
    sphere[4].y = *(phd_mxptr + M13) >> W2V_SHIFT;
    sphere[4].z = *(phd_mxptr + M23) >> W2V_SHIFT;
    sphere[4].r = (sint32)*(objptr + 3) * 3 / 2;
    phd_PopMatrix();

    phd_TranslateRel(*(bone + 1 + 52), *(bone + 2 + 52), *(bone + 3 + 52));
    gar_RotYXZsuperpack(&rotation, 2);
    phd_RotYXZ(lara.head_y_rot, lara.head_x_rot, lara.head_z_rot);

    phd_PushMatrix();
    objptr = lara.mesh_ptrs[HEAD];
    phd_TranslateRel(*objptr, *(objptr + 1), *(objptr + 2));
    sphere[2].x = *(phd_mxptr + M03) >> W2V_SHIFT;
    sphere[2].y = *(phd_mxptr + M13) >> W2V_SHIFT;
    sphere[2].z = *(phd_mxptr + M23) >> W2V_SHIFT;
    sphere[2].r = (sint32)*(objptr + 3);
    phd_PopMatrix();

    phd_TranslateRel(0, -23, -55);
    pos.x = *(phd_mxptr + M03) >> W2V_SHIFT;
    pos.y = *(phd_mxptr + M13) >> W2V_SHIFT;
    pos.z = *(phd_mxptr + M23) >> W2V_SHIFT;
    phd_PopMatrix();

    bone = bones + objects[HAIR].bone_index;

    if (first_hair) {
        first_hair = 0;
        hair[0].x_pos = pos.x;
        hair[0].y_pos = pos.y;
        hair[0].z_pos = pos.z;
        for (i = 0; i < HAIR_SEGMENTS; i++, bone += 4) {
            phd_PushUnitMatrix();
            *(phd_mxptr + M03) = hair[i].x_pos << W2V_SHIFT;
            *(phd_mxptr + M13) = hair[i].y_pos << W2V_SHIFT;
            *(phd_mxptr + M23) = hair[i].z_pos << W2V_SHIFT;
            phd_RotYXZ(hair[i].y_rot, hair[i].x_rot, 0);
            phd_TranslateRel(*(bone + 1), *(bone + 2), *(bone + 3));
            hair[i + 1].x_pos = *(phd_mxptr + M03) >> W2V_SHIFT;
            hair[i + 1].y_pos = *(phd_mxptr + M13) >> W2V_SHIFT;
            hair[i + 1].z_pos = *(phd_mxptr + M23) >> W2V_SHIFT;
            phd_PopMatrix();
        }
        wind = SmokeWindX = SmokeWindZ = 0;
        wind_angle = dwind_angle = 2048;
    } else {
        hair[0].x_pos = pos.x;
        hair[0].y_pos = pos.y;
        hair[0].z_pos = pos.z;

        room_number = lara_item->room_number;
        if (in_cutscene)
            water_level = NO_HEIGHT;
        else {
            x = lara_item->pos.x_pos + (frame[0] + frame[1]) / 2;
            y = lara_item->pos.y_pos + (frame[2] + frame[3]) / 2;
            z = lara_item->pos.z_pos + (frame[4] + frame[5]) / 2;
            water_level = GetWaterHeight(x, y, z, room_number);
        }

        {
            sint32 diff;

            i = GetRandomControl() & 7;
            wind += i - 3;
            if (wind <= -2)
                wind++;
            else if (wind >= 9)
                wind--;

            dwind_angle += ((GetRandomControl() & 63) - 32) << 1;
            dwind_angle &= 8190;
            if (dwind_angle < 1024)
                dwind_angle += (1024 - dwind_angle) << 1;
            else if (dwind_angle > 3072)
                dwind_angle -= (dwind_angle - 3072) << 1;

            diff = dwind_angle - wind_angle;
            wind_angle += diff >> 3;
            wind_angle &= 8190;
        }

        SmokeWindX = (rcossin_tbl[wind_angle] * wind) >> 12;
        SmokeWindZ = (rcossin_tbl[wind_angle + 1] * wind) >> 12;

        for (i = 1; i < HAIR_SEGMENTS + 1; i++, bone += 4) {
            hvel[0].x = hair[i].x_pos;
            hvel[0].y = hair[i].y_pos;
            hvel[0].z = hair[i].z_pos;

            if (!in_cutscene) {
                floor = GetFloor(hair[i].x_pos, hair[i].y_pos, hair[i].z_pos, &room_number);
                height = GetHeight(floor, hair[i].x_pos, hair[i].y_pos, hair[i].z_pos);
            } else
                height = 32767;

            hair[i].x_pos += hvel[i].x * 3 / 4;
            hair[i].y_pos += hvel[i].y * 3 / 4;
            hair[i].z_pos += hvel[i].z * 3 / 4;

            if (lara.water_status == LARA_ABOVEWATER && room[room_number].flags & NOT_INSIDE) {
                hair[i].x_pos += SmokeWindX;
                hair[i].z_pos += SmokeWindZ;
            }

            switch (lara.water_status) {
            case LARA_ABOVEWATER:
                hair[i].y_pos += 10;
                if (water_level != NO_HEIGHT && hair[i].y_pos > water_level)
                    hair[i].y_pos = water_level;
                else if (hair[i].y_pos > height) {
                    hair[i].x_pos = hvel[0].x;
                    hair[i].z_pos = hvel[0].z;
                }
                break;
            case LARA_UNDERWATER:
            case LARA_SURFACE:
            case LARA_WADE:
                if (hair[i].y_pos < water_level)
                    hair[i].y_pos = water_level;
                else if (hair[i].y_pos > height)
                    hair[i].y_pos = height;
                break;
            }

            for (j = 0; j < 5; j++) {
                x = hair[i].x_pos - sphere[j].x;
                y = hair[i].y_pos - sphere[j].y;
                z = hair[i].z_pos - sphere[j].z;
                distance = x * x + y * y + z * z;
                if (distance < SQUARE(sphere[j].r)) {
                    distance = phd_sqrt(distance);
                    if (distance == 0)
                        distance = 1;
                    hair[i].x_pos = sphere[j].x + x * sphere[j].r / distance;
                    hair[i].y_pos = sphere[j].y + y * sphere[j].r / distance;
                    hair[i].z_pos = sphere[j].z + z * sphere[j].r / distance;
                }
            }

            distance = phd_sqrt(SQUARE(hair[i].z_pos - hair[i - 1].z_pos) + SQUARE(hair[i].x_pos - hair[i - 1].x_pos));
            hair[i - 1].y_rot = phd_atan(hair[i].z_pos - hair[i - 1].z_pos, hair[i].x_pos - hair[i - 1].x_pos);
            hair[i - 1].x_rot = -phd_atan(distance, hair[i].y_pos - hair[i - 1].y_pos);

            phd_PushUnitMatrix();
            *(phd_mxptr + M03) = hair[i - 1].x_pos << W2V_SHIFT;
            *(phd_mxptr + M13) = hair[i - 1].y_pos << W2V_SHIFT;
            *(phd_mxptr + M23) = hair[i - 1].z_pos << W2V_SHIFT;
            phd_RotYXZ(hair[i - 1].y_rot, hair[i - 1].x_rot, 0);
            if (i == HAIR_SEGMENTS)
                phd_TranslateRel(*(bone - 3), *(bone - 2), *(bone - 1));
            else
                phd_TranslateRel(*(bone + 1), *(bone + 2), *(bone + 3));
            hair[i].x_pos = *(phd_mxptr + M03) >> W2V_SHIFT;
            hair[i].y_pos = *(phd_mxptr + M13) >> W2V_SHIFT;
            hair[i].z_pos = *(phd_mxptr + M23) >> W2V_SHIFT;

            hvel[i].x = hair[i].x_pos - hvel[0].x;
            hvel[i].y = hair[i].y_pos - hvel[0].y;
            hvel[i].z = hair[i].z_pos - hvel[0].z;
            phd_PopMatrix();
        }
    }
}

void phd_RotX(sint16 rx);
void phd_RotY(sint16 ry);
void phd_TranslateAbs(sint32 x, sint32 y, sint32 z);
void phd_PutPolygons(sint16 *objptr, int clip);

void DrawHair(void)
{
    int i;
    OBJECT_INFO *object;
    sint16 **mesh;

    object = &objects[HAIR];
    mesh = &meshes[object->mesh_index];
    for (i = 0; i < HAIR_SEGMENTS; i++) {
        phd_PushMatrix();
        phd_TranslateAbs(hair[i].x_pos, hair[i].y_pos, hair[i].z_pos);
        phd_RotY(hair[i].y_rot);
        phd_RotX(hair[i].x_rot);
        phd_PutPolygons(*mesh++, 1);
        phd_PopMatrix();
    }
}

#ifdef TR3_EXE
sint32 wind_angle = 2048;
sint32 dwind_angle = 2048;
#endif
