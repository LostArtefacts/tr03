#include "game.h"

sint32 GetWaterHeight(sint32 x, sint32 y, sint32 z, sint16 room_number);
void LaraSwimCollision(ITEM_INFO *item, COLL_INFO *coll);

void lara_as_dive(ITEM_INFO *item, COLL_INFO *coll)
{
    if (input & IN_FORWARD)
        item->pos.x_rot -= ONE_DEGREE;
}

void lara_as_uwdeath(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.look = 0;
    item->fallspeed -= 8;
    if (item->fallspeed <= 0)
        item->fallspeed = 0;
    if (item->pos.x_rot >= -2 * ONE_DEGREE && item->pos.x_rot <= 2 * ONE_DEGREE)
        item->pos.x_rot = 0;
    else if (item->pos.x_rot < 0)
        item->pos.x_rot += 2 * ONE_DEGREE;
    else
        item->pos.x_rot -= 2 * ONE_DEGREE;
}

void lara_as_waterroll(ITEM_INFO *item, COLL_INFO *coll)
{
    item->fallspeed = 0;
}

void lara_col_swim(ITEM_INFO *item, COLL_INFO *coll)
{
    LaraSwimCollision(item, coll);
}

void lara_col_glide(ITEM_INFO *item, COLL_INFO *coll)
{
    LaraSwimCollision(item, coll);
}

void lara_col_tread(ITEM_INFO *item, COLL_INFO *coll)
{
    LaraSwimCollision(item, coll);
}

void lara_col_dive(ITEM_INFO *item, COLL_INFO *coll)
{
    LaraSwimCollision(item, coll);
}

void lara_col_uwdeath(ITEM_INFO *item, COLL_INFO *coll)
{
    int wh;

    item->hit_points = -1;
    lara.air = -1;
    lara.gun_status = LG_HANDSBUSY;
    wh = GetWaterHeight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);
    if (wh != NO_HEIGHT && wh < item->pos.y_pos - 100)
        item->pos.y_pos -= 5;
    LaraSwimCollision(item, coll);
}

void lara_col_waterroll(ITEM_INFO *item, COLL_INFO *coll)
{
    LaraSwimCollision(item, coll);
}
