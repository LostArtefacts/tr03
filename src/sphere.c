#include "game.h"
#include <string.h>

extern int IM_rate, IM_frac;
extern sint32 *IMptr;
extern sint32 IMstack[];

typedef struct {
    sint32 x;
    sint32 y;
    sint32 z;
    sint32 r;
} SPHERE;

int GetSpheres(ITEM_INFO *item, SPHERE *ptr, int WorldSpace);

int TestCollision(ITEM_INFO *item, ITEM_INFO *laraitem)
{
    int num1;
    int num2;
    int x1, y1, z1, r1;
    int x, y, z, r;
    int i, j;
    uint32 flags;
    SPHERE *ptr1;
    SPHERE *ptr2;
    SPHERE slist_baddie[34];
    SPHERE slist_lara[34];

    flags = 0;
    num1 = GetSpheres(item, &slist_baddie[0], 1);
    num2 = GetSpheres(laraitem, &slist_lara[0], 1);
    ptr1 = &slist_baddie[0];
    for (i = 0; i < num1; i++, ptr1++) {
        x1 = ptr1->x;
        y1 = ptr1->y;
        z1 = ptr1->z;
        r1 = ptr1->r;
        if (r1 > 0) {
            ptr2 = &slist_lara[0];
            for (j = 0; j < num2; j++, ptr2++) {
                if (ptr2->r > 0) {
                    x = ptr2->x - x1;
                    y = ptr2->y - y1;
                    z = ptr2->z - z1;
                    r = ptr2->r + r1;
                    if (x * x + y * y + z * z < r * r) {
                        flags |= 1 << i;
                        break;
                    }
                }
            }
        }
    }
    item->touch_bits = flags;
    return flags;
}

void InitInterpolate2(int frac, int rate)
{
    IM_frac = frac;
    IM_rate = rate;
    IMptr = &IMstack[12 * 32];
    memcpy(IMptr, phd_mxptr, 12 * sizeof(sint32));
}
