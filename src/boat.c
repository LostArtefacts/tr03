#include "game.h"

#define GRAVITY 6

typedef struct {
    sint32 boat_turn;
    sint32 left_fallspeed;
    sint32 right_fallspeed;
    sint16 tilt_angle;
    sint16 extra_rotation;
    sint32 water;
    sint32 pitch;
    sint16 prop_rot;
} BOAT_INFO;

int DoBoatDynamics(int height, int fallspeed, int *y)
{
    if (height > *y)
    {
        *y += fallspeed;
        if (*y > height)
        {
            *y = height;
            fallspeed = 0;
        }
        else
            fallspeed += GRAVITY;
    }
    else
    {
        fallspeed += (height - *y - fallspeed) >> 3;
        if (fallspeed < -20)
            fallspeed = -20;
        if (*y > height)
            *y = height;
    }
    return fallspeed;
}

void DrawBoat(ITEM_INFO *item)
{
    BOAT_INFO *b;

    b = (BOAT_INFO *)item->data;
    item->data = &b->prop_rot;
    DrawAnimatingItem(item);
    item->data = b;
    S_DrawWakeFX(item);
}
