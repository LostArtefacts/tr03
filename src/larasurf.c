#include "game.h"

void lara_col_surfswim(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->bad_neg = -STEPUP_HEIGHT;
    lara.move_angle = item->pos.y_rot;
    LaraSurfaceCollision(item, coll);
    LaraTestWaterClimbOut(item, coll);
}

void lara_col_surfback(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.move_angle = item->pos.y_rot - 32768;
    LaraSurfaceCollision(item, coll);
}

void lara_col_surfleft(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.move_angle = item->pos.y_rot - 16384;
    LaraSurfaceCollision(item, coll);
}

void lara_col_surfright(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.move_angle = item->pos.y_rot + 16384;
    LaraSurfaceCollision(item, coll);
}

void lara_col_surftread(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->goal_anim_state == AS_SWIM)
    {
        item->current_anim_state = AS_DIVE;
        item->anim_number = SURFDIVE_A;
        item->frame_number = SURFDIVE_F;
        item->pos.x_rot = -45 * ONE_DEGREE;
        item->fallspeed = 20 * 4;
        lara.water_status = LARA_UNDERWATER;
    }
    lara.move_angle = item->pos.y_rot;
    LaraSurfaceCollision(item, coll);
}
