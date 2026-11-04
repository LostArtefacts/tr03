#include "game.h"
#include <stdlib.h>

#define DAMAGE_START 140
#define DAMAGE_LENGTH 14
#define FASTFALL_FRICTION item->speed = (item->speed * 95) / 100
#define STEPUP_HEIGHT ((STEP_L * 3) / 2)
#define CAM_A_HANG 0
#define CAM_E_HANG (-60 * ONE_DEGREE)

void GetLaraCollisionInfo(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->facing = lara.move_angle;
    GetCollisionInfo(coll, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, LARA_HITE);
}

void lara_col_all4turnl(ITEM_INFO *item, COLL_INFO *coll)
{
    GetCollisionInfo(coll, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, LARA_DUCK_HEIGHT);
}
