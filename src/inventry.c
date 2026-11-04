#include "game.h"

#define PASSPORT_OPTION 145
#define MAP_OPTION 146
#define GAMMA_OPTION 184

extern sint32 CurPicTexIndices[5];
extern float f_zfar;

void HWR_EnableZBuffer(int write, int compare);
void TRDrawPicture(sint32 col, sint32 *indices, float z);

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

void DoInventoryPicture(void)
{
    HWR_EnableZBuffer(0, 0);
    TRDrawPicture(0, CurPicTexIndices, f_zfar);
}
