#include "game.h"

#define MONKEY_SIT_ANIM 2
#define MONKEY_SIT 6

void InitialiseMonkey(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    InitialiseCreature(item_number);
    item->anim_number = objects[MONKEY].anim_index + MONKEY_SIT_ANIM;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = item->goal_anim_state = MONKEY_SIT;
}

extern sint32 *bones;

int GetFrames(ITEM_INFO *item, sint16 *frmptr[], int *rate);
void S_PrintShadow(int size, sint16 *bptr, ITEM_INFO *item, int unknown);
void phd_PushMatrix(void);
void phd_TranslateAbs(sint32 x, sint32 y, sint32 z);
void phd_TranslateRel(sint32 x, sint32 y, sint32 z);
void phd_RotX(sint16 rx);
void phd_RotY(sint16 ry);
void phd_RotZ(sint16 rz);
void phd_RotYXZ(sint16 ry, sint16 rx, sint16 rz);
int S_GetObjectBounds(sint16 *bptr);
void CalculateObjectLighting(ITEM_INFO *item, sint16 *frame);
void gar_RotYXZsuperpack(sint16 **pprot, int skip);
void phd_PutPolygons(sint16 *objptr, int clip);
void InitInterpolate(int frac, int rate);
void phd_TranslateRel_ID(sint32 x, sint32 y, sint32 z, sint32 x2, sint32 y2, sint32 z2);
void gar_RotYXZsuperpack_I(sint16 **pprot1, sint16 **pprot2, int skip);
void phd_PutPolygons_I(sint16 *ptr, int clip);
void phd_PopMatrix_I(void);
void phd_PushMatrix_I(void);
void phd_TranslateRel_I(sint32 x, sint32 y, sint32 z);
void phd_RotX_I(sint16 ang);
void phd_RotY_I(sint16 ang);
void phd_RotZ_I(sint16 ang);

void DrawMonkey(ITEM_INFO *item)
{
    int clip, i, poppush;
    OBJECT_INFO *object;
    sint32 *bone;
    sint16 *rotation1, *rotation2;
    sint16 *extra_rotation;
    sint16 **meshpp, **swappp;
    uint32 bit;
    sint16 *frmptr[2];
    int rate, frac;

    frac = GetFrames(item, frmptr, &rate);
    object = &objects[item->object_number];
    if (object->shadow_size)
        S_PrintShadow(object->shadow_size, frmptr[0], item, 0);

    phd_PushMatrix();
    phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);
    if ((clip = S_GetObjectBounds(frmptr[0]))) {
        CalculateObjectLighting(item, frmptr[0]);
        extra_rotation = (sint16 *)item->data;
        meshpp = &meshes[objects[71].mesh_index];
        if (item->ai_bits != 8)
            swappp = &meshes[objects[123].mesh_index];
        else
            swappp = &meshes[objects[124].mesh_index];
        bone = bones + object->bone_index;
        bit = 1;
        if (!frac) {
            phd_TranslateRel((sint32)*(frmptr[0] + 6), (sint32)*(frmptr[0] + 7), (sint32)*(frmptr[0] + 8));
            rotation1 = frmptr[0] + 9;
            gar_RotYXZsuperpack(&rotation1, 0);
            if (bit & item->mesh_bits)
                phd_PutPolygons(*meshpp, clip);
            else
                phd_PutPolygons(*swappp, clip);
            meshpp++;
            swappp++;
            for (i = object->nmeshes - 1; i > 0; i--, bone += 4, meshpp++, swappp++) {
                poppush = *bone;
                if (poppush & 1)
                    phd_PopMatrix();
                if (poppush & 2)
                    phd_PushMatrix();
                phd_TranslateRel(*(bone + 1), *(bone + 2), *(bone + 3));
                gar_RotYXZsuperpack(&rotation1, 0);
                if (extra_rotation && (poppush & 0x1c)) {
                    if (poppush & 8)
                        phd_RotY(*(extra_rotation++));
                    if (poppush & 4)
                        phd_RotX(*(extra_rotation++));
                    if (poppush & 16)
                        phd_RotZ(*(extra_rotation++));
                }
                bit <<= 1;
                if (bit & item->mesh_bits)
                    phd_PutPolygons(*meshpp, clip);
                else
                    phd_PutPolygons(*swappp, clip);
            }
        } else {
            InitInterpolate(frac, rate);
            phd_TranslateRel_ID((sint32)*(frmptr[0] + 6), (sint32)*(frmptr[0] + 7), (sint32)*(frmptr[0] + 8),
                                (sint32)*(frmptr[1] + 6), (sint32)*(frmptr[1] + 7), (sint32)*(frmptr[1] + 8));
            rotation1 = frmptr[0] + 9;
            rotation2 = frmptr[1] + 9;
            gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
            if (bit & item->mesh_bits)
                phd_PutPolygons_I(*meshpp, clip);
            else
                phd_PutPolygons_I(*swappp, clip);
            meshpp++;
            swappp++;
            for (i = object->nmeshes - 1; i > 0; i--, bone += 4, meshpp++, swappp++) {
                poppush = *bone;
                if (poppush & 1)
                    phd_PopMatrix_I();
                if (poppush & 2)
                    phd_PushMatrix_I();
                phd_TranslateRel_I(*(bone + 1), *(bone + 2), *(bone + 3));
                gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
                if (extra_rotation && (poppush & 0x1c)) {
                    if (poppush & 8)
                        phd_RotY_I(*(extra_rotation++));
                    if (poppush & 4)
                        phd_RotX_I(*(extra_rotation++));
                    if (poppush & 16)
                        phd_RotZ_I(*(extra_rotation++));
                }
                bit <<= 1;
                if (bit & item->mesh_bits)
                    phd_PutPolygons_I(*meshpp, clip);
                else if (item->hit_points > 0 || item->status == ACTIVE || bit != 0x400 || item->carried_item == -1)
                    phd_PutPolygons_I(*swappp, clip);
            }
        }
    }
    phd_PopMatrix();
}
