#include "game.h"


void InitialiseLaraLoad(sint16 item_num)
{
    lara.item_number = item_num;
    lara_item = &items[item_num];
}

extern sint16 cheat_hit_points;

void LaraUnderWater(ITEM_INFO *item, COLL_INFO *coll);
void LaraInitialiseMeshes(int level_number);

void LaraCheat(ITEM_INFO *item, COLL_INFO *coll)
{
    lara_item->hit_points = 1000;
    LaraUnderWater(item, coll);
    if ((input & IN_SLOW) && !(input & IN_LOOK)) {
        lara.water_status = LARA_ABOVEWATER;
        item->frame_number = STOP_F;
        item->anim_number = STOP_A;
        item->pos.x_rot = item->pos.z_rot = 0;
        lara.torso_x_rot = lara.torso_y_rot = 0;
        lara.head_x_rot = lara.head_y_rot = 0;
        lara.gun_status = LG_ARMLESS;
        LaraInitialiseMeshes(1);
        lara.mesh_effects = 0;
        lara_item->hit_points = cheat_hit_points;
    }
}

void ControlLaraExtra(sint16 item_num)
{
    AnimateItem(&items[item_num]);
}
