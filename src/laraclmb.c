#include "game.h"

#define CAM_E_CLIMBU (30 * ONE_DEGREE)
#define CAM_E_CLIMBD (-45 * ONE_DEGREE)

void lara_as_climbing(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->enable_spaz = 0;
    coll->enable_baddie_push = 0;
    camera.target_elevation = CAM_E_CLIMBU;
}
