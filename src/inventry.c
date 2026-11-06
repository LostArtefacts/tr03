#include "specific/standard.h"
#include "game.h"

#define PASSPORT_OPTION 145
#define MAP_OPTION 146
#define GAMMA_OPTION 184

extern sint32 CurPicTexIndices[5];
extern float f_zfar;

void HWR_EnableZBuffer(int write, int compare);
void TRDrawPicture(sint32 col, sint32 *indices, float z);

extern sint32 Inventory_Mode;
extern sint16 Inventory_Displaying;
extern sint16 Inventory_Chosen;
extern sint32 Inventory_ExtraData[8];
extern sint16 inv_main_objects, inv_main_current;
extern sint16 inv_option_objects, inv_option_current;
extern INVENTORY_ITEM *inv_main_list[];
extern INVENTORY_ITEM *inv_option_list[];
extern INVENTORY_ITEM *inv_levelselect_list[];
extern sint32 OpenInvOnGym;
extern sint16 item_data;
extern sint32 HiResFlag;
extern sint32 phd_left, phd_right, phd_top, phd_bottom;
extern sint16 phd_winxmax, phd_winymax;

void S_SetupAboveWater(int underwater);
void TempVideoAdjust(sint32 a, double sizer);

void Construct_Inventory(void)
{
    int n;
    INVENTORY_ITEM *inv_item;

    S_SetupAboveWater(0);
    if (Inventory_Mode != 1)
        TempVideoAdjust(HiResFlag, 1.0);
    phd_left = 0;
    phd_top = 0;
    phd_right = phd_winxmax;
    phd_bottom = phd_winymax;
    Inventory_Displaying = 1;
    Inventory_Chosen = 0;
    for (n = 0; n < 8; n++)
        Inventory_ExtraData[n] = 0;
    if (Inventory_Mode == 1) {
        if (gameflow.gym_enabled)
            inv_option_objects = 5;
        else
            inv_option_objects = 5;
    } else
        inv_option_objects = 4;
    for (n = 0; n < inv_main_objects; n++) {
        inv_item = inv_main_list[n];
        inv_item->drawn_meshes = inv_item->which_meshes;
        inv_item->current_frame = 0;
        inv_item->goal_frame = inv_item->current_frame;
        inv_item->anim_count = 0;
        inv_item->y_rot = 0;
    }
    for (n = 0; n < inv_option_objects; n++) {
        inv_item = inv_option_list[n];
        inv_item->current_frame = 0;
        inv_item->goal_frame = inv_item->current_frame;
        inv_item->anim_count = 0;
        inv_item->y_rot = 0;
    }
    for (n = 0; n < 1; n++) {
        inv_item = inv_levelselect_list[n];
        if (n == 0)
            inv_item->drawn_meshes = 0xffffffff;
        else
            inv_item->drawn_meshes = 0xf81;
        inv_item->current_frame = 0;
        inv_item->goal_frame = inv_item->current_frame;
        inv_item->anim_count = 0;
        inv_item->y_rot = 0;
    }
    inv_main_current = 0;
    if (OpenInvOnGym && Inventory_Mode == 1 && !gameflow.loadsave_disabled && gameflow.gym_enabled)
        inv_option_current = 4;
    else {
        inv_option_current = 0;
        OpenInvOnGym = 0;
    }
    item_data = 0;
}

void SelectMeshes(INVENTORY_ITEM *inv_item)
{
    int frame;

    if (inv_item->object_number == PASSPORT_OPTION) {
        frame = inv_item->current_frame;
        if (frame < 4)
            inv_item->drawn_meshes = 0x17;
        else if (frame <= 16)
            inv_item->drawn_meshes = 0x57;
        else if (frame < 19)
            inv_item->drawn_meshes = 0x5f;
        else if (frame == 19)
            inv_item->drawn_meshes = 0x5b;
        else if (frame < 24)
            inv_item->drawn_meshes = 0x7b;
        else if (frame < 29)
            inv_item->drawn_meshes = 0x3b;
        else if (frame == 29)
            inv_item->drawn_meshes = 0x13;
    } else if (inv_item->object_number == MAP_OPTION) {
        frame = inv_item->current_frame;
        if (frame && frame < 18)
            inv_item->drawn_meshes = 0xffffffff;
        else
            inv_item->drawn_meshes = inv_item->which_meshes;
    } else if (inv_item->object_number != GAMMA_OPTION) {
        inv_item->drawn_meshes = 0xffffffff;
    }
}

sint32 AnimateInventoryItem(INVENTORY_ITEM *inv_item)
{
    if (inv_item->current_frame != inv_item->goal_frame) {
        if (inv_item->anim_count) {
            inv_item->anim_count--;
        } else {
            inv_item->anim_count = inv_item->anim_speed;
            inv_item->current_frame += inv_item->anim_direction;
            if (inv_item->current_frame >= inv_item->frames_total)
                inv_item->current_frame = 0;
            else if (inv_item->current_frame < 0)
                inv_item->current_frame = inv_item->frames_total - 1;
        }
        SelectMeshes(inv_item);
        return 1;
    }
    SelectMeshes(inv_item);
    return 0;
}

extern uint8 GlobeLevelComplete[];
extern sint16 GlobeLevel;

extern char bInvItemsOff;
extern sint32 nPolyType;
extern sint16 GlobeXRot, GlobeYRot, GlobeZRot;
extern sint32 *bones;
extern sint32 phd_znear, phd_zfar, phd_persp, phd_centerx, phd_centery;
#define M03 3
#define M13 7
#define M23 11

typedef struct static_info {
    sint16 mesh_number;
    sint16 flags;
    sint16 x_minp;
    sint16 x_maxp;
    sint16 y_minp;
    sint16 y_maxp;
    sint16 z_minp;
    sint16 z_maxp;
    sint16 x_minc;
    sint16 x_maxc;
    sint16 y_minc;
    sint16 y_maxc;
    sint16 z_minc;
    sint16 z_maxc;
} STATIC_INFO;

extern STATIC_INFO static_objects[];
void phd_PushMatrix(void);
void phd_TranslateRel(sint32 x, sint32 y, sint32 z);
void phd_RotZ(sint16 rz);
void phd_RotYXZ(sint16 ry, sint16 rx, sint16 rz);
void gar_RotYXZsuperpack(sint16 **pprot, int skip);
void phd_PutPolygons(sint16 *objptr, int clip);
int S_GetObjectBounds(sint16 *bptr);
void S_DrawSprite(uint32 flags, sint32 x, sint32 y, sint32 z, sint16 sprnum, sint16 shade, sint16 scale);
void S_DrawScreenSprite(sint32 sx, sint32 sy, sint32 z, sint32 scaleH, sint32 scaleV, sint16 sprnum, sint16 shade, uint16 flags);
void S_DrawScreenLine(sint32 sx, sint32 sy, sint32 z, sint32 w, sint32 h, sint32 col, uint16 *grdptr, uint16 flags);
void S_DrawScreenBox(sint32 sx, sint32 sy, sint32 z, sint32 w, sint32 h, sint32 col, uint16 *grdptr, uint16 flags);
void S_DrawScreenFBox(sint32 sx, sint32 sy, sint32 z, sint32 w, sint32 h, sint32 col, uint16 *grdptr, uint16 flags);
void Inv_GlobeLight(sint16 mesh_num);

void DrawInventoryItem(INVENTORY_ITEM *inv_item)
{
    int hours, minutes, seconds, totsec;
    int clip, i, poppush;
    sint16 mesh, mesh_num;
    OBJECT_INFO *object;
    sint32 *bone;
    sint16 *rotation;
    sint16 *frame;
    INVENTORY_SPRITE *isprite, **isprite_list;
    sint32 xv, yv, zv, zp;
    sint32 sx, sy;

    nPolyType = 7;
    if (bInvItemsOff)
        return;
    if (inv_item->object_number == 146) {
        totsec = savegame.timer / 30;
        seconds = totsec % 60;
        minutes = ((totsec % 3600) * -91) / 5;
        hours = ((totsec / 12) * -91) / 5;
        seconds *= -1092;
    } else
        hours = minutes = seconds = totsec = 0;
    phd_TranslateRel(0, inv_item->ytrans, inv_item->ztrans);
    phd_RotYXZ(inv_item->y_rot, inv_item->x_rot, 0);
    if (inv_item->object_number == 184)
        phd_RotYXZ(GlobeYRot, GlobeXRot, GlobeZRot);
    object = &objects[inv_item->object_number];
    if (!object->loaded)
        return;
    if (object->nmeshes < 0) {
        S_DrawSprite(0, 0, 0, 0, object->mesh_index, 0, 0);
        return;
    }
    if ((isprite_list = inv_item->sprlist)) {
        zv = *(phd_mxptr + M23);
        xv = *(phd_mxptr + M03);
        yv = *(phd_mxptr + M13);
        zp = zv / phd_persp;
        sx = xv / zp + phd_centerx;
        sy = yv / zp + phd_centery;
        while ((isprite = *isprite_list++)) {
            if (zv < phd_znear || zv > phd_zfar)
                break;
            while (isprite->shape) {
                switch (isprite->shape) {
                case 1:
                    S_DrawScreenSprite(sx + isprite->x, sy + isprite->y, isprite->z, isprite->param1, isprite->param2, (sint16)(static_objects[356].mesh_number + isprite->sprnum), 4096, 0);
                    break;
                case 2:
                    S_DrawScreenLine(sx + isprite->x, sy + isprite->y, isprite->z, isprite->param1, isprite->param2, isprite->sprnum, (uint16 *)isprite->grdptr, 0);
                    break;
                case 3:
                    S_DrawScreenBox(sx + isprite->x, sy + isprite->y, isprite->z, isprite->param1, isprite->param2, isprite->sprnum, (uint16 *)isprite->grdptr, 0);
                    break;
                case 4:
                    S_DrawScreenFBox(sx + isprite->x, sy + isprite->y, isprite->z, isprite->param1, isprite->param2, isprite->sprnum, (uint16 *)isprite->grdptr, 0);
                    break;
                }
                isprite++;
            }
        }
    }
    frame = object->frame_base + inv_item->current_frame * (anims[object->anim_index].interpolation >> 8);
    phd_PushMatrix();
    if ((clip = S_GetObjectBounds(frame))) {
        phd_TranslateRel(*(frame + 6), *(frame + 7), *(frame + 8));
        rotation = frame + 9;
        gar_RotYXZsuperpack(&rotation, 0);
        mesh = object->mesh_index;
        bone = bones + object->bone_index;
        mesh_num = 1;
        if (inv_item->object_number == 184)
            inv_item->drawn_meshes = ~8;
        if (mesh_num & inv_item->drawn_meshes) {
            if (inv_item->object_number == 184) {
                Inv_GlobeLight(mesh_num);
                phd_PutPolygons(meshes[mesh], clip);
            } else
                phd_PutPolygons(meshes[mesh], clip);
        }
        for (i = object->nmeshes - 1; i > 0; i--) {
            mesh++;
            mesh_num <<= 1;
            poppush = *(bone++);
            if (poppush & 1)
                phd_PopMatrix();
            if (poppush & 2)
                phd_PushMatrix();
            phd_TranslateRel(*bone, *(bone + 1), *(bone + 2));
            gar_RotYXZsuperpack(&rotation, 0);
            if (inv_item->object_number == 146) {
                if (i == 1) {
                    phd_RotZ((sint16)seconds);
                    inv_item->misc_data[1] = inv_item->misc_data[0];
                    inv_item->misc_data[0] = seconds;
                }
                if (i == 2)
                    phd_RotZ((sint16)minutes);
                if (i == 3)
                    phd_RotZ((sint16)hours);
            }
            if (inv_item->object_number == 184)
                Inv_GlobeLight(mesh_num);
            if (mesh_num & inv_item->drawn_meshes) {
                if (inv_item->object_number == 184)
                    phd_PutPolygons(meshes[mesh], clip);
                else
                    phd_PutPolygons(meshes[mesh], clip);
            }
            bone += 3;
        }
    }
    phd_PopMatrix();
}

sint32 GetDebouncedInput(sint32 input)
{
    int j;

    j = input ^ (input & oldInputDB);
    oldInputDB = input;
    input &= j;
    return input;
}

void GetGlobeMeshFlags(void)
{
    GlobeLevelComplete[0] = savegame.IndiaComplete;
    GlobeLevelComplete[1] = savegame.SPacificComplete;
    GlobeLevelComplete[2] = savegame.NevadaComplete;
    GlobeLevelComplete[3] = 255;
    GlobeLevelComplete[4] = savegame.LondonComplete;
    if (savegame.IndiaComplete && savegame.SPacificComplete && savegame.NevadaComplete && savegame.LondonComplete)
        GlobeLevelComplete[5] = savegame.AntarcticaComplete;
    else
        GlobeLevelComplete[5] = 255;
    GlobeLevel = 0;
    while (GlobeLevelComplete[GlobeLevel])
        GlobeLevel++;
}

void DoInventoryPicture(void)
{
    HWR_EnableZBuffer(0, 0);
    TRDrawPicture(0, CurPicTexIndices, f_zfar);
}
