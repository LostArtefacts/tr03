#include "game.h"

void DrawTonyBossShield(ITEM_INFO *item);

typedef struct {
    sint16 x;
    sint16 y;
    sint16 z;
    uint8 rsub;
    uint8 gsub;
    uint8 bsub;
    uint8 pad[3];
    sint32 rgb;
} SHIELD_POINTS;

extern sint16 rcossin_tbl[];
extern sint32 TonyBossRadii[5];
extern sint32 TonyBossHeights[5];
extern SHIELD_POINTS TonyBossShield[40];

void S_DrawTonyBoss(ITEM_INFO *item)
{
    DrawAnimatingItem(item);
    if (bossdata.explode_count && item->hit_points <= 0)
        DrawExplosionRings();
    if (bossdata.explode_count && bossdata.explode_count <= 64)
        DrawTonyBossShield(item);
}

void TonyBossDie(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    item->collidable = 0;
    item->hit_points = DONT_TARGET;
    KillItem(item_number);
    DisableBaddieAI(item_number);
    item->flags |= ONESHOT;
}

void InitialiseTonyBoss(sint16 item_number)
{
    SHIELD_POINTS *shptr;
    sint32 lp, lp1, angle, y;

    items[item_number].item_flags[3] = bossdata.explode_count = bossdata.ring_count = bossdata.dropped_icon = bossdata.dead = 0;
    shptr = &TonyBossShield[0];
    for (lp = 0; lp < 5; lp++) {
        y = TonyBossHeights[lp];
        angle = 0;
        for (lp1 = 0; lp1 < 8; lp1++) {
            shptr->x = (rcossin_tbl[angle << 1] * TonyBossRadii[lp]) >> 11;
            shptr->y = y;
            shptr->z = (rcossin_tbl[(angle << 1) + 1] * TonyBossRadii[lp]) >> 11;
            shptr->rgb = 0;
            angle += 512;
            shptr++;
        }
    }
}

sint32 GetFreeSpark(void);

void TriggerTonyFlame(sint16 item_number, sint32 hand)
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
    sptr->sR = 255;
    sptr->sG = 48 + (GetRandomControl() & 31);
    sptr->sB = 48;
    sptr->dR = 192 + (GetRandomControl() & 63);
    sptr->dG = 128 + (GetRandomControl() & 63);
    sptr->dB = 32;
    sptr->ColFadeSpeed = 12 + (GetRandomControl() & 3);
    sptr->FadeToBlack = 8;
    sptr->sLife = sptr->Life = (GetRandomControl() & 7) + 24;
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = (GetRandomControl() & 15) - 8;
    sptr->y = 0;
    sptr->z = (GetRandomControl() & 15) - 8;
    sptr->Xvel = (GetRandomControl() & 255) - 128;
    sptr->Yvel = -(GetRandomControl() & 15) - 16;
    sptr->Zvel = (GetRandomControl() & 255) - 128;
    sptr->Friction = 5;
    if (GetRandomControl() & 1) {
        sptr->Flags = 2 | 8 | 16 | 128 | 512 | 4096;
        sptr->RotAng = GetRandomControl() & 4095;
        if (GetRandomControl() & 1)
            sptr->RotAdd = -(GetRandomControl() & 15) - 16;
        else
            sptr->RotAdd = (GetRandomControl() & 15) + 16;
    } else
        sptr->Flags = 2 | 8 | 128 | 512 | 4096;
    sptr->Gravity = -(GetRandomControl() & 31) - 16;
    sptr->MaxYvel = -(GetRandomControl() & 7) - 16;
    sptr->FxObj = item_number;
    sptr->NodeNumber = hand;
    sptr->Def = objects[296].mesh_index;
    sptr->Scalar = 1;
    size = (GetRandomControl() & 31) + 64;
    sptr->Width = sptr->sWidth = size;
    sptr->Height = sptr->sHeight = size;
    sptr->dWidth = size >> 2;
    sptr->dHeight = size >> 2;
}

void TriggerFireBall(ITEM_INFO *item, sint32 type, PHD_VECTOR *pos1, sint16 room_number, sint16 angle, sint32 zdspeed)
{
    FX_INFO *fx;
    PHD_VECTOR pos;
    sint16 fx_number;
    sint32 speed, fallspeed;

    if (type == 0) {
        pos.x = pos.y = pos.z = 0;
        GetJointAbsPosition(item, &pos, 10);
        angle = item->pos.y_rot;
        speed = 0;
        fallspeed = -16;
    } else if (type == 1) {
        pos.x = pos.y = pos.z = 0;
        GetJointAbsPosition(item, &pos, 13);
        angle = item->pos.y_rot;
        speed = 0;
        fallspeed = -16;
    } else if (type == 2) {
        pos.x = pos.y = pos.z = 0;
        GetJointAbsPosition(item, &pos, 13);
        speed = 160;
        fallspeed = -(GetRandomControl() & 7) - 32;
    } else if (type == 4) {
        pos.x = pos1->x;
        pos.y = pos1->y;
        pos.z = pos1->z;
        speed = zdspeed + (GetRandomControl() & 3);
        angle = GetRandomControl() << 1;
        fallspeed = (GetRandomControl() & 3) - 2;
    } else if (type == 5) {
        pos.x = pos1->x;
        pos.y = pos1->y;
        pos.z = pos1->z;
        speed = (GetRandomControl() & 7) + 48;
        angle += (GetRandomControl() & 0x1fff) - 0x9000;
        fallspeed = -(GetRandomControl() & 15) - 16;
    } else if (type == 3) {
        pos.x = pos1->x;
        pos.y = pos1->y;
        pos.z = pos1->z;
        speed = 0;
        fallspeed = (GetRandomControl() & 3) + 4;
    } else {
        pos.x = pos1->x;
        pos.y = pos1->y;
        pos.z = pos1->z;
        speed = (GetRandomControl() & 31) + 32;
        angle = GetRandomControl() << 1;
        fallspeed = -(GetRandomControl() & 31) - 32;
    }
    fx_number = CreateEffect(room_number);
    if (fx_number != NO_ITEM) {
        fx = &effects[fx_number];
        fx->pos.x_pos = pos.x;
        fx->pos.y_pos = pos.y;
        fx->pos.z_pos = pos.z;
        fx->pos.y_rot = angle;
        fx->object_number = 283;
        fx->speed = speed;
        fx->fallspeed = fallspeed;
        fx->flag1 = type;
        fx->flag2 = (GetRandomControl() & 3) + 1;
        if (type == 5)
            fx->flag2 <<= 1;
        else if (type == 2)
            fx->flag2 = 0;
    }
}

void TriggerFireBallFlame(sint16 fx_number, sint32 type, sint32 xv, sint32 yv, sint32 zv)
{
    SPARKS *sptr;
    sint32 size;
    sint32 dx, dz;

    dx = lara_item->pos.x_pos - effects[fx_number].pos.x_pos;
    dz = lara_item->pos.z_pos - effects[fx_number].pos.z_pos;
    if (dx < -0x4000 || dx > 0x4000 || dz < -0x4000 || dz > 0x4000)
        return;
    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sR = 255;
    sptr->sG = 48 + (GetRandomControl() & 31);
    sptr->sB = 48;
    sptr->dR = 192 + (GetRandomControl() & 63);
    sptr->dG = 128 + (GetRandomControl() & 63);
    sptr->dB = 32;
    sptr->ColFadeSpeed = 12 + (GetRandomControl() & 3);
    sptr->FadeToBlack = 8;
    sptr->sLife = sptr->Life = (GetRandomControl() & 7) + 24;
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = (GetRandomControl() & 15) - 8;
    sptr->y = 0;
    sptr->z = (GetRandomControl() & 15) - 8;
    sptr->Xvel = xv + ((GetRandomControl() & 255) - 128);
    sptr->Yvel = yv;
    sptr->Zvel = zv + ((GetRandomControl() & 255) - 128);
    sptr->Friction = 5;
    if (GetRandomControl() & 1) {
        sptr->Flags = 2 | 8 | 16 | 512 | 64;
        sptr->RotAng = GetRandomControl() & 4095;
        if (GetRandomControl() & 1)
            sptr->RotAdd = -(GetRandomControl() & 15) - 16;
        else
            sptr->RotAdd = (GetRandomControl() & 15) + 16;
    } else {
        sptr->Flags = 2 | 8 | 512 | 64;
    }
    sptr->FxObj = fx_number;
    sptr->Def = objects[296].mesh_index;
    sptr->Scalar = 1;
    size = (GetRandomControl() & 31) + 64;
    sptr->Width = sptr->sWidth = size;
    sptr->Height = sptr->sHeight = size;
    sptr->dWidth = size >> 2;
    sptr->dHeight = size >> 2;
    if (type == 0 || type == 1) {
        sptr->Gravity = (GetRandomControl() & 31) + 16;
        sptr->MaxYvel = (GetRandomControl() & 15) + 48;
        sptr->Yvel = -sptr->Yvel << 4;
        sptr->Scalar = 2;
    } else if (type == 4 || type == 5 || type == 6)
        sptr->Gravity = sptr->MaxYvel = 0;
    else if (type == 3) {
        sptr->Gravity = -(GetRandomControl() & 31) - 16;
        sptr->MaxYvel = -(GetRandomControl() & 31) - 64;
        sptr->Yvel = sptr->Yvel << 4;
        sptr->Scalar = 2;
    } else if (type == 2) {
        sptr->Gravity = sptr->MaxYvel = 0;
        sptr->Scalar = 2;
    }
}
