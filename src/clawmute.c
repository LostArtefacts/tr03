#include "game.h"

sint32 GetFreeSpark(void);

void TriggerPlasma_clawmute(sint16 item_number)
{
    sint32 size;
    SPARKS *sptr;
    sint32 dx, dz;

    dx = lara_item->pos.x_pos - items[item_number].pos.x_pos;
    dz = lara_item->pos.z_pos - items[item_number].pos.z_pos;
    if (dx < -0x4000 || dx > 0x4000 || dz < -0x4000 || dz > 0x4000)
        return;
    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sB = 255;
    sptr->sG = 48 + (GetRandomControl() & 31);
    sptr->sR = 48;
    sptr->dB = 192 + (GetRandomControl() & 63);
    sptr->dG = 128 + (GetRandomControl() & 63);
    sptr->dR = 32;
    sptr->ColFadeSpeed = 12 + (GetRandomControl() & 3);
    sptr->FadeToBlack = 8;
    sptr->sLife = sptr->Life = (GetRandomControl() & 7) + 24;
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = (GetRandomControl() & 15) - 8;
    sptr->y = 0;
    sptr->z = (GetRandomControl() & 15) - 8;
    sptr->Xvel = (GetRandomControl() & 31) - 16;
    sptr->Yvel = (GetRandomControl() & 15) + 16;
    sptr->Zvel = (GetRandomControl() & 31) - 16;
    sptr->Friction = 3;
    if (GetRandomControl() & 1) {
        sptr->Flags = 2 | 8 | 16 | 128 | 512 | 4096;
        sptr->RotAng = GetRandomControl() & 4095;
        if (GetRandomControl() & 1)
            sptr->RotAdd = -(GetRandomControl() & 15) - 16;
        else
            sptr->RotAdd = (GetRandomControl() & 15) + 16;
    } else
        sptr->Flags = 2 | 8 | 128 | 512 | 4096;
    sptr->Gravity = (GetRandomControl() & 31) + 16;
    sptr->MaxYvel = (GetRandomControl() & 7) + 16;
    sptr->FxObj = item_number;
    sptr->NodeNumber = 6;
    sptr->Def = objects[296].mesh_index;
    sptr->Scalar = 1;
    size = (GetRandomControl() & 31) + 64;
    sptr->Width = sptr->sWidth = size;
    sptr->Height = sptr->sHeight = size;
    sptr->dWidth = size >> 2;
    sptr->dHeight = size >> 2;
}

void phd_GetVectorAngles(sint32 x, sint32 y, sint32 z, PHD_ANGLE *dest);
sint16 CreateEffect(sint16 room_num);

void TriggerPlasmaBall_clawmute(ITEM_INFO *item, sint32 type, PHD_VECTOR *pos1, sint16 room_number, sint16 angle)
{
    FX_INFO *fx;
    PHD_VECTOR pos;
    sint16 fx_number;
    PHD_ANGLE angles[2];
    sint32 speed;

    if (type == 0) {
        pos.x = -32;
        pos.y = -16;
        pos.z = -192;
        GetJointAbsPosition(item, &pos, 13);
        speed = (GetRandomControl() & 7) + 8;
        phd_GetVectorAngles(lara_item->pos.x_pos - pos.x, lara_item->pos.y_pos - pos.y - 256, lara_item->pos.z_pos - pos.z, angles);
        angles[0] = item->pos.y_rot;
    } else {
        pos.x = pos1->x;
        pos.y = pos1->y;
        pos.z = pos1->z;
        speed = (GetRandomControl() & 15) + 16;
        angles[0] = GetRandomControl() << 1;
        angles[1] = 0x2000;
    }
    fx_number = CreateEffect(room_number);
    if (fx_number != NO_ITEM) {
        fx = &effects[fx_number];
        fx->pos.x_pos = pos.x;
        fx->pos.y_pos = pos.y;
        fx->pos.z_pos = pos.z;
        fx->pos.y_rot = angles[0];
        fx->pos.x_rot = angles[1];
        fx->object_number = 368;
        fx->speed = speed;
        fx->fallspeed = 0;
        fx->flag1 = type;
    }
}
