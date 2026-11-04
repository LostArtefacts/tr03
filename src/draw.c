#include "game.h"
#include <string.h>

#define S_LongMemCpy(d, s, n) memcpy(d, s, (n) * sizeof(sint32))

extern int IM_rate, IM_frac;
extern sint32 *IMptr;
extern sint32 IMstack[];

void phd_PushMatrix(void);
void phd_RotX(sint16 rx);
void phd_RotY(sint16 ry);
void phd_RotZ(sint16 rz);
void phd_RotYXZ(sint16 ry, sint16 rx, sint16 rz);
void phd_TranslateRel(sint32 x, sint32 y, sint32 z);
void phd_PutPolygons(sint16 *objptr, int clip);
void gar_RotYXZsuperpack(sint16 **pprot, int skip);
void InterpolateMatrix(void);
int GetFrames(ITEM_INFO *item, sint16 *frmptr[], int *rate);

void InitInterpolate(int frac, int rate)
{
    IM_frac = frac;
    IM_rate = rate;
    IMptr = IMstack;
    S_LongMemCpy(IMstack, phd_mxptr, 12);
}

void phd_PopMatrix_I(void)
{
    phd_PopMatrix();
    IMptr -= 12;
}
