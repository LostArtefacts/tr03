#ifndef _3DGLODEF_H
#define _3DGLODEF_H

#include "types.h"

typedef struct phd_vector {
    sint32 x;
    sint32 y;
    sint32 z;
} PHD_VECTOR;

typedef sint16 PHD_ANGLE;

#define W2V_SHIFT 14

#define ABS(x) (((x) < 0) ? (-(x)) : (x))

sint32 __fastcall phd_sin(sint32 angle);
sint32 __fastcall phd_cos(sint32 angle);
sint16 __fastcall phd_atan(sint32 x, sint32 y);
uint32 __fastcall phd_sqrt(uint32 n);

#define phd_PopMatrix() phd_mxptr -= 12

extern sint32 *phd_mxptr;

void AlterFOV(sint16 fov);

#ifndef PHD_3DPOS_H
#define PHD_3DPOS_H
typedef struct phd_3dpos {
    sint32 x_pos;
    sint32 y_pos;
    sint32 z_pos;
    PHD_ANGLE x_rot;
    PHD_ANGLE y_rot;
    PHD_ANGLE z_rot;
} PHD_3DPOS;
#endif

#endif
