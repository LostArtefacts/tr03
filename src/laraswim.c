#include "game.h"

void lara_as_dive(ITEM_INFO *item, COLL_INFO *coll)
{
    if (input & IN_FORWARD)
        item->pos.x_rot -= ONE_DEGREE;
}

void lara_as_waterroll(ITEM_INFO *item, COLL_INFO *coll)
{
    item->fallspeed = 0;
}

void lara_col_swim(ITEM_INFO *item, COLL_INFO *coll)
{
    LaraSwimCollision(item, coll);
}
