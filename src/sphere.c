#include "game.h"
#include <string.h>

extern int IM_rate, IM_frac;
extern sint32 *IMptr;
extern sint32 IMstack[];

typedef struct {
    sint32 x;
    sint32 y;
    sint32 z;
    sint32 r;
} SPHERE;

int GetSpheres(ITEM_INFO *item, SPHERE *ptr, int WorldSpace);

int TestCollision(ITEM_INFO *item, ITEM_INFO *laraitem)
{
    int num1;
    int num2;
    int x1, y1, z1, r1;
    int x, y, z, r;
    int i, j;
    uint32 flags;
    SPHERE *ptr1;
    SPHERE *ptr2;
    SPHERE slist_baddie[34];
    SPHERE slist_lara[34];

    flags = 0;
    num1 = GetSpheres(item, &slist_baddie[0], 1);
    num2 = GetSpheres(laraitem, &slist_lara[0], 1);
    ptr1 = &slist_baddie[0];
    for (i = 0; i < num1; i++, ptr1++) {
        x1 = ptr1->x;
        y1 = ptr1->y;
        z1 = ptr1->z;
        r1 = ptr1->r;
        if (r1 > 0) {
            ptr2 = &slist_lara[0];
            for (j = 0; j < num2; j++, ptr2++) {
                if (ptr2->r > 0) {
                    x = ptr2->x - x1;
                    y = ptr2->y - y1;
                    z = ptr2->z - z1;
                    r = ptr2->r + r1;
                    if (x * x + y * y + z * z < r * r) {
                        flags |= 1 << i;
                        break;
                    }
                }
            }
        }
    }
    item->touch_bits = flags;
    return flags;
}

extern sint32 *bones;

void phd_PushMatrix(void);
void phd_PushUnitMatrix(void);
void phd_TranslateAbs(sint32 x, sint32 y, sint32 z);
void phd_TranslateRel(sint32 x, sint32 y, sint32 z);
void phd_RotX(sint16 rx);
void phd_RotY(sint16 ry);
void phd_RotZ(sint16 rz);
void phd_RotYXZ(sint16 ry, sint16 rx, sint16 rz);
void gar_RotYXZsuperpack(sint16 **pprot, int skip);
sint16 *GetBestFrame(ITEM_INFO *item);

int GetSpheres(ITEM_INFO *item, SPHERE *ptr, int WorldSpace)
{
    int i, poppush;
    OBJECT_INFO *object;
    sint32 *bone;
    sint16 *rotation;
    sint16 *frame;
    sint32 x, y, z;
    sint16 *objptr, **meshpp;
    sint16 *extra_rotation;

    if (item == NULL)
        return 0;

    if (WorldSpace) {
        x = item->pos.x_pos;
        y = item->pos.y_pos;
        z = item->pos.z_pos;
        phd_PushUnitMatrix();
        phd_mxptr[3] = 0;
        phd_mxptr[7] = 0;
        phd_mxptr[11] = 0;
    } else {
        x = y = z = 0;
        phd_PushMatrix();
        phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    }
    phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);
    frame = GetBestFrame(item);
    phd_TranslateRel((sint32)*(frame + 6), (sint32)*(frame + 7), (sint32)*(frame + 8));
    rotation = frame + 9;
    gar_RotYXZsuperpack(&rotation, 0);

    object = &objects[item->object_number];
    meshpp = &meshes[object->mesh_index];
    bone = bones + object->bone_index;

    objptr = *(meshpp++);
    phd_PushMatrix();
    phd_TranslateRel((sint32)*objptr, (sint32)*(objptr + 1), (sint32)*(objptr + 2));
    ptr->x = x + (phd_mxptr[3] >> W2V_SHIFT);
    ptr->y = y + (phd_mxptr[7] >> W2V_SHIFT);
    ptr->z = z + (phd_mxptr[11] >> W2V_SHIFT);
    ptr->r = (sint32)*(objptr + 3);
    ptr++;
    phd_PopMatrix();

    extra_rotation = (sint16 *)item->data;
    for (i = object->nmeshes - 1; i > 0; i--, bone += 3) {
        poppush = *(bone++);
        if (poppush & 1)
            phd_PopMatrix();
        if (poppush & 2)
            phd_PushMatrix();

        phd_TranslateRel(*bone, *(bone + 1), *(bone + 2));
        gar_RotYXZsuperpack(&rotation, 0);

        if ((poppush & (4 | 8 | 16)) && extra_rotation) {
            if (poppush & 8)
                phd_RotY(*(extra_rotation++));
            if (poppush & 4)
                phd_RotX(*(extra_rotation++));
            if (poppush & 16)
                phd_RotZ(*(extra_rotation++));
        }

        objptr = *(meshpp++);
        phd_PushMatrix();
        phd_TranslateRel((sint32)*objptr, (sint32)*(objptr + 1), (sint32)*(objptr + 2));
        ptr->x = x + (phd_mxptr[3] >> W2V_SHIFT);
        ptr->y = y + (phd_mxptr[7] >> W2V_SHIFT);
        ptr->z = z + (phd_mxptr[11] >> W2V_SHIFT);
        ptr->r = (sint32)*(objptr + 3);
        ptr++;
        phd_PopMatrix();
    }
    phd_PopMatrix();
    return object->nmeshes;
}

extern sint16 null_rotations[12];

int GetFrames(ITEM_INFO *item, sint16 *frmptr[], int *rate);
void InitInterpolate2(int frac, int rate);
void phd_TranslateRel_ID(sint32 x, sint32 y, sint32 z, sint32 x2, sint32 y2, sint32 z2);
void phd_TranslateRel_I(sint32 x, sint32 y, sint32 z);
void gar_RotYXZsuperpack_I(sint16 **pprot1, sint16 **pprot2, int skip);
void phd_PopMatrix_I(void);
void phd_PushMatrix_I(void);
void phd_RotX_I(sint16 ang);
void phd_RotY_I(sint16 ang);
void phd_RotZ_I(sint16 ang);
void InterpolateMatrix(void);

void GetJointAbsPosition(ITEM_INFO *item, PHD_VECTOR *vec, int joint)
{
    int i, poppush;
    OBJECT_INFO *object;
    sint32 *bone;
    sint16 *rotation1, *rotation2;
    sint16 *extra_rotation;
    sint16 *frmptr[2];
    int rate, frac;
    sint32 *MatrixStash = phd_mxptr;
    sint32 *IMStash = IMptr;

    object = &objects[item->object_number];
    frac = GetFrames(item, frmptr, &rate);

    phd_PushUnitMatrix();
    phd_mxptr[3] = 0;
    phd_mxptr[7] = 0;
    phd_mxptr[11] = 0;
    phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);

    if (item->data)
        extra_rotation = (sint16 *)item->data;
    else
        extra_rotation = null_rotations;

    bone = bones + object->bone_index;

    if (!frac) {
        phd_TranslateRel((sint32)*(frmptr[0] + 6), (sint32)*(frmptr[0] + 7), (sint32)*(frmptr[0] + 8));
        rotation1 = frmptr[0] + 9;
        gar_RotYXZsuperpack(&rotation1, 0);

        for (i = 0; i < joint; i++, bone += 4) {
            poppush = *bone;
            if (poppush & 1)
                phd_PopMatrix();
            if (poppush & 2)
                phd_PushMatrix();

            phd_TranslateRel(*(bone + 1), *(bone + 2), *(bone + 3));
            gar_RotYXZsuperpack(&rotation1, 0);

            if (poppush & (4 | 8 | 16)) {
                if (poppush & 8)
                    phd_RotY(*(extra_rotation++));
                if (poppush & 4)
                    phd_RotX(*(extra_rotation++));
                if (poppush & 16)
                    phd_RotZ(*(extra_rotation++));
            }
        }

        phd_TranslateRel(vec->x, vec->y, vec->z);
        vec->x = (phd_mxptr[3] >> W2V_SHIFT) + item->pos.x_pos;
        vec->y = (phd_mxptr[7] >> W2V_SHIFT) + item->pos.y_pos;
        vec->z = (phd_mxptr[11] >> W2V_SHIFT) + item->pos.z_pos;
    } else {
        InitInterpolate2(frac, rate);
        rotation1 = frmptr[0] + 9;
        rotation2 = frmptr[1] + 9;
        phd_TranslateRel_ID((sint32)*(frmptr[0] + 6), (sint32)*(frmptr[0] + 7), (sint32)*(frmptr[0] + 8),
                            (sint32)*(frmptr[1] + 6), (sint32)*(frmptr[1] + 7), (sint32)*(frmptr[1] + 8));
        gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);

        for (i = 0; i < joint; i++, bone += 4) {
            poppush = *bone;
            if (poppush & 1)
                phd_PopMatrix_I();
            if (poppush & 2)
                phd_PushMatrix_I();

            phd_TranslateRel_I(*(bone + 1), *(bone + 2), *(bone + 3));
            gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);

            if (poppush & (4 | 8 | 16)) {
                if (poppush & 8)
                    phd_RotY_I(*(extra_rotation++));
                if (poppush & 4)
                    phd_RotX_I(*(extra_rotation++));
                if (poppush & 16)
                    phd_RotZ_I(*(extra_rotation++));
            }
        }

        phd_TranslateRel_I(vec->x, vec->y, vec->z);
        InterpolateMatrix();
        vec->x = (phd_mxptr[3] >> W2V_SHIFT) + item->pos.x_pos;
        vec->y = (phd_mxptr[7] >> W2V_SHIFT) + item->pos.y_pos;
        vec->z = (phd_mxptr[11] >> W2V_SHIFT) + item->pos.z_pos;
    }

    phd_mxptr = MatrixStash;
    IMptr = IMStash;
}

void InitInterpolate2(int frac, int rate)
{
    IM_frac = frac;
    IM_rate = rate;
    IMptr = &IMstack[12 * 32];
    memcpy(IMptr, phd_mxptr, 12 * sizeof(sint32));
}
