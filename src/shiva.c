#include "game.h"

#define SHIVA_START_ANIM 14
#define SHIVA_START 4

sint32 GetFreeSpark(void);

extern sint32 effect_mesh;
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

void TriggerShivaSmoke(sint32 x, sint32 y, sint32 z, sint32 uw);

void DrawShiva(ITEM_INFO *item)
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
    PHD_VECTOR pos;

    frac = GetFrames(item, frmptr, &rate);
    if (item->hit_points <= 0 && item->status != ACTIVE && item->mesh_bits != 0) {
        item->mesh_bits = item->mesh_bits >> 1;
        pos.x = 0;
        pos.y = 0;
        pos.z = 256;
        GetJointAbsPosition(item, &pos, effect_mesh++);
        TriggerShivaSmoke(pos.x, pos.y, pos.z, 1);
    }

    object = &objects[item->object_number];
    if (object->shadow_size)
        S_PrintShadow(object->shadow_size, frmptr[0], item, 0);

    phd_PushMatrix();
    phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);
    if ((clip = S_GetObjectBounds(frmptr[0]))) {
        CalculateObjectLighting(item, frmptr[0]);
        extra_rotation = (sint16 *)item->data;
        meshpp = &meshes[objects[70].mesh_index];
        swappp = &meshes[objects[122].mesh_index];
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

void TriggerShivaSmoke(sint32 x, sint32 y, sint32 z, sint32 uw)
{
    sint32 size;
    SPARKS *sptr;
    sint32 dx, dz;

    dx = lara_item->pos.x_pos - x;
    dz = lara_item->pos.z_pos - z;
    if (dx < -0x4000 || dx > 0x4000 || dz < -0x4000 || dz > 0x4000)
        return;
    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    if (uw) {
        sptr->sR = 0;
        sptr->sG = 0;
        sptr->sB = 0;
        sptr->dR = 192;
        sptr->dG = 192;
        sptr->dB = 208;
    } else {
        sptr->sR = 144;
        sptr->sG = 144;
        sptr->sB = 144;
        sptr->dR = 64;
        sptr->dG = 64;
        sptr->dB = 64;
    }
    sptr->ColFadeSpeed = 8;
    sptr->FadeToBlack = 64;
    sptr->sLife = sptr->Life = (GetRandomControl() & 31) + 96;
    if (uw)
        sptr->TransType = 2;
    else
        sptr->TransType = 3;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = x + (GetRandomControl() & 31) - 16;
    sptr->y = y + (GetRandomControl() & 31) - 16;
    sptr->z = z + (GetRandomControl() & 31) - 16;
    sptr->Xvel = ((GetRandomControl() & 4095) - 2048) >> 2;
    sptr->Yvel = (GetRandomControl() & 255) - 128;
    sptr->Zvel = ((GetRandomControl() & 4095) - 2048) >> 2;
    if (uw) {
        sptr->Yvel >>= 4;
        sptr->y += 32;
        sptr->Friction = 4 | (1 << 4);
    } else
        sptr->Friction = 6;
    sptr->Flags = 2 | 8 | 16 | 512;
    sptr->RotAng = GetRandomControl() & 4095;
    if (GetRandomControl() & 1)
        sptr->RotAdd = -(GetRandomControl() & 15) - 16;
    else
        sptr->RotAdd = (GetRandomControl() & 15) + 16;
    sptr->Def = objects[296].mesh_index;
    sptr->Scalar = 3;
    if (uw)
        sptr->Gravity = sptr->MaxYvel = 0;
    else {
        sptr->Gravity = -(GetRandomControl() & 3) - 3;
        sptr->MaxYvel = -(GetRandomControl() & 3) - 4;
    }
    size = (GetRandomControl() & 31) + 128;
    sptr->Width = sptr->sWidth = size >> 2;
    sptr->dWidth = size;
    size += (GetRandomControl() & 31) + 32;
    sptr->Height = sptr->sHeight = size >> 3;
    sptr->dHeight = size;
}

void InitialiseShiva(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    if (item->object_number == SHIVA)
    {
        item->anim_number = objects[SHIVA].anim_index + SHIVA_START_ANIM;
        item->frame_number = anims[item->anim_number].frame_base;
        item->current_anim_state = item->goal_anim_state = SHIVA_START;
    }
    item->status = NOT_ACTIVE;
    item->mesh_bits = 0;
}

#define SHIVA_TOUCHL 0x2400
#define SHIVA_TOUCHR 0x2400000

extern BITE_INFO shiva_left;
extern BITE_INFO shiva_right;

sint16 CreatureEffect(ITEM_INFO *item, BITE_INFO *bite, sint16 (*generate)(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 yrot, sint16 room_number));
sint16 DoBloodSplat(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num);

void ShivaDamage(ITEM_INFO *item, CREATURE_INFO *shiva, int damage)
{
    if (!shiva->flags && (item->touch_bits & SHIVA_TOUCHR)) {
        lara_item->hit_points -= damage;
        lara_item->hit_status = 1;
        CreatureEffect(item, &shiva_right, DoBloodSplat);
        shiva->flags = 1;
        SoundEffect(318, &item->pos, 0);
    }
    if (!shiva->flags && (item->touch_bits & SHIVA_TOUCHL)) {
        lara_item->hit_points -= damage;
        lara_item->hit_status = 1;
        CreatureEffect(item, &shiva_left, DoBloodSplat);
        shiva->flags = 1;
        SoundEffect(318, &item->pos, 0);
    }
}
