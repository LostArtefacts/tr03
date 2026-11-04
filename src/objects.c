#include "game.h"

typedef struct {
    sint32 start_height;
    sint32 wait_time;
} LIFT_INFO;

enum height_types { WALL, SMALL_SLOPE, BIG_SLOPE, DIAGONAL, SPLIT_TRI };

extern int height_type;
extern int OnObject;

void LiftFloorCeiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *floor, sint32 *ceiling);
