#include "game.h"

typedef struct {
    sint32 x;
    sint32 y;
    sint32 z;
    sint16 YRot;
    sint16 Active;
} FOOTPRINT;

extern sint16 FXType;
extern int OnObject;
extern sint32 FootPrintNum;
extern FOOTPRINT FootPrint[32];
extern char footsounds[14];

void GetLaraHandAbsPosition(PHD_VECTOR *vec, int hand);

void AddFootprint(ITEM_INFO *item)
{
    sint16 room_num;
    FLOOR_INFO *floor;
    FOOTPRINT *f;
    PHD_VECTOR pos;

    pos.x = 0;
    pos.y = 0;
    pos.z = 0;
    if (FXType == 0x4000)
        GetLaraHandAbsPosition(&pos, 5);
    else
        GetLaraHandAbsPosition(&pos, 6);
    room_num = item->room_number;
    floor = GetFloor(pos.x, pos.y, pos.z, &room_num);
    if (floor->fx != 6 && floor->fx != 5 && floor->fx != 9 && floor->fx != 11)
        SoundEffect(288 + footsounds[floor->fx], &lara_item->pos, 0);
    if (floor->fx <= 4 && !OnObject) {
        f = &FootPrint[FootPrintNum];
        f->x = pos.x;
        f->y = GetHeight(floor, pos.x, pos.y, pos.z);
        f->z = pos.z;
        f->YRot = item->pos.y_rot;
        f->Active = 0x200;
        FootPrintNum = (FootPrintNum + 1) & 31;
    }
}

#ifdef TR3_EXE
char footsounds[14] = {
    0, 5, 3, 2, 1, 9, 9, 4, 6, 9, 3, 9, 4, 6,
};
#endif
