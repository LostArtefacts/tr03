#include "standard.h"
#include "specific.h"
#include "winmain.h"
extern "C" {
#include "../game.h"
}

extern "C" {

extern sint16 rcossin_tbl[];
extern int phd_persp;

void mCalcPoint(sint32 x, sint32 y, sint32 z, sint32 *result);
void ProjectPCoord(sint32 x, sint32 y, sint32 z, sint32 *result, sint32 cx, sint32 cy, sint32 fov);

extern sint32 phd_znear;

extern BITE_INFO NodeOffsets[16];
extern int distanceFogValue;
extern sint32 phd_zfar;
void HWI_InsertAlphaSprite_Sorted(long x1, long y1, long z1, long shade1, long x2, long y2, long z2, long shade2, long x3, long y3, long z3, long shade3, long x4, long y4, long z4, long shade4, long nSprite, long nDrawtype, long double_sided);

void S_DrawSparks(void)
{
    SPARKS *sptr;
    PHD_VECTOR pos;
    long w;
    FX_INFO *fx;
    long h;
    long x;
    long y;
    long z;
    long sw;
    long sh;
    long x1;
    long y1;
    long x2;
    long y2;
    long x3;
    long y3;
    long x4;
    long y4;
    long r;
    long g;
    long b;
    long f;
    long c;
    long vpos[3][3];
    long point[3];
    long i;
    w = App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].w;
    h = App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].h;
    sptr = &sparks[0];
    for (i = 0; i < 192; i++) {
        if (!sptr->On)
            { sptr++; continue; }
        if (sptr->Flags & 64) {
            fx = &effects[sptr->FxObj];
            if (sptr->Flags & 1024) {
                x = fx->pos.x_pos;
                y = fx->pos.y_pos;
                z = fx->pos.z_pos;
            } else {
                x = sptr->x + fx->pos.x_pos;
                y = sptr->y + fx->pos.y_pos;
                z = sptr->z + fx->pos.z_pos;
            }
        } else if (sptr->Flags & 128) {
            ITEM_INFO *item = &items[sptr->FxObj];
            const sint32 &itemPositionY = item->pos.y_pos;
            const sint32 &itemPositionZ = item->pos.z_pos;
            if (sptr->Flags & 1024) {
                x = item->pos.x_pos;
                y = itemPositionY;
                z = itemPositionZ;
            } else if (sptr->Flags & 4096) {
                pos.x = NodeOffsets[sptr->NodeNumber].x;
                pos.y = NodeOffsets[sptr->NodeNumber].y;
                pos.z = NodeOffsets[sptr->NodeNumber].z;
                GetJointAbsPosition(item, &pos, NodeOffsets[sptr->NodeNumber].mesh_num);
                x = sptr->x + pos.x;
                y = sptr->y + pos.y;
                z = sptr->z + pos.z;
                if (sptr->NodeNumber == 2 || sptr->NodeNumber == 3) {
                    b = sptr->NodeNumber == 3 ? (GetRandomDraw() & 3) + 12 : (GetRandomDraw() & 3) + 8;
                    if (sptr->sLife - sptr->Life > b) {
                        sptr->Flags &= ~(4096 | 128);
                        sptr->x = x;
                        sptr->y = y;
                        sptr->z = z;
                    }
                }
            } else {
                x = sptr->x + item->pos.x_pos;
                y = sptr->y + itemPositionY;
                z = sptr->z + itemPositionZ;
            }
        } else {
            x = sptr->x;
            y = sptr->y;
            z = sptr->z;
        }
        mCalcPoint(x, y, z, point);
        ProjectPCoord(point[0], point[1], point[2], vpos[0], w >> 1, h >> 1, phd_persp);
        if (sptr->Flags & 8) {
            if (sptr->Flags & 2) {
                if (!vpos[0][2])
                    vpos[0][2] = 1;
                x = 4;
                if (sptr->Flags & 4096 && !sptr->NodeNumber)
                    x = 2;
                sw = ((sptr->Width * phd_persp) << sptr->Scalar) / vpos[0][2];
                sh = ((sptr->Height * phd_persp) << sptr->Scalar) / vpos[0][2];
                if (sw > sptr->Width << sptr->Scalar)
                    sw = sptr->Width << sptr->Scalar;
                else if (sw < x)
                    sw = x;
                if (sh > sptr->Height << sptr->Scalar)
                    sh = sptr->Height << sptr->Scalar;
                else if (sh < x)
                    sh = x;
            } else {
                sw = sptr->Width;
                sh = sptr->Height;
            }
            z = vpos[0][2] << 14;
            if (z < phd_znear || z > phd_zfar)
                { sptr++; continue; }
            x = vpos[0][0];
            y = vpos[0][1];
            if (x + (sw >> 1) < 0 || x - (sw >> 1) > w || y + (sh >> 1) < 0 || y - (sh >> 1) > h)
                { sptr++; continue; }
            if (sptr->Flags & 16) {
                long sin, cos;
                long sx1, sx2, sy1, sy2;
                long cx1, cx2, cy1, cy2;
                sin = rcossin_tbl[sptr->RotAng << 1];
                cos = rcossin_tbl[(sptr->RotAng << 1) + 1];
                sx1 = ((-sw >> 1) * sin) >> 12;
                sx2 = ((sw >> 1) * sin) >> 12;
                sy1 = ((-sh >> 1) * sin) >> 12;
                sy2 = ((sh >> 1) * sin) >> 12;
                cx1 = ((-sw >> 1) * cos) >> 12;
                cx2 = ((sw >> 1) * cos) >> 12;
                cy1 = ((-sh >> 1) * cos) >> 12;
                cy2 = ((sh >> 1) * cos) >> 12;
                x1 = sx1 - cy1 + x;
                x2 = sx2 - cy1 + x;
                x3 = sx2 - cy2 + x;
                x4 = sx1 - cy2 + x;
                y1 = cx1 + sy1 + y;
                y2 = cx2 + sy1 + y;
                y3 = cx2 + sy2 + y;
                y4 = cx1 + sy2 + y;
                g = sptr->G;
                r = sptr->R;
                b = sptr->B;
                c = (sptr->R >> 3) << 10 | (sptr->G >> 3) << 5 | (sptr->B >> 3);
                if (z > distanceFogValue << 14) {
                    f = 2048 - ((z - (distanceFogValue << 14)) >> 16);
                    r = (r * f) >> 14;
                    g = (g * f) >> 14;
                    b = (b * f) >> 14;
                    if (r < 0)
                        r = 0;
                    if (g < 0)
                        g = 0;
                    if (b < 0)
                        b = 0;
                    c = r << 10 | g << 5 | b;
                }
                if (sptr->TransType == 2 || sptr->TransType == 3)
                    HWI_InsertAlphaSprite_Sorted(x1, y1, z, c, x2, y2, z, c, x3, y3, z, c, x4, y4, z, c, sptr->Def, 14, 0);
                else
                    HWI_InsertAlphaSprite_Sorted(x1, y1, z, c, x2, y2, z, c, x3, y3, z, c, x4, y4, z, c, sptr->Def, 10, 0);
                sptr->RotAng = (sptr->RotAng + sptr->RotAdd) & 0xFFF;
            } else {
                x1 = x - (sw >> 1);
                y1 = y - (sh >> 1);
                x2 = x + (sw >> 1);
                y2 = y + (sh >> 1);
                g = sptr->G;
                r = sptr->R;
                b = sptr->B;
                c = (sptr->R >> 3) << 10 | (sptr->G >> 3) << 5 | (sptr->B >> 3);
                if (z > distanceFogValue << 14) {
                    f = 2048 - ((z - (distanceFogValue << 14)) >> 16);
                    r = (r * f) >> 14;
                    g = (g * f) >> 14;
                    b = (b * f) >> 14;
                    if (r < 0)
                        r = 0;
                    if (g < 0)
                        g = 0;
                    if (b < 0)
                        b = 0;
                    c = r << 10 | g << 5 | b;
                }
                if (sptr->TransType == 2 || sptr->TransType == 3)
                    HWI_InsertAlphaSprite_Sorted(x1, y1, z, c, x2, y1, z, c, x2, y2, z, c, x1, y2, z, c, sptr->Def, 14, 0);
                else
                    HWI_InsertAlphaSprite_Sorted(x1, y1, z, c, x2, y1, z, c, x2, y2, z, c, x1, y2, z, c, sptr->Def, 10, 0);
            }
        } else {
            if (sptr->Flags & 2) {
                if (!vpos[0][2])
                    vpos[0][2] = 1;
                sw = ((sptr->Width * phd_persp) << sptr->Scalar) / vpos[0][2];
                sh = ((sptr->Height * phd_persp) << sptr->Scalar) / vpos[0][2];
                if (sw > sptr->Width << 2)
                    sw = sptr->Width << 2;
                else if (sw < 1)
                    sw = 1;
                if (sh > sptr->Height << 2)
                    sh = sptr->Height << 2;
                else if (sh < 1)
                    sh = 1;
            } else {
                sw = sptr->Width;
                sh = sptr->Height;
            }
            z = vpos[0][2] << 14;
            if (z < phd_znear || z > phd_zfar)
                { sptr++; continue; }
            x = vpos[0][0];
            y = vpos[0][1];
            if (x + (sw >> 1) < 0 || x - (sw >> 1) > w || y + (sh >> 1) < 0 || y - (sh >> 1) > h)
                { sptr++; continue; }
            x1 = x - (sw >> 1);
            y1 = y - (sh >> 1);
            x2 = x1 + sw;
            y2 = y1 + sh;
            g = sptr->G;
            r = sptr->R;
            b = sptr->B;
            c = (sptr->R >> 3) << 10 | (sptr->G >> 3) << 5 | (sptr->B >> 3);
            if (z > distanceFogValue << 14) {
                f = 2048 - ((z - (distanceFogValue << 14)) >> 16);
                r = (r * f) >> 14;
                g = (g * f) >> 14;
                b = (b * f) >> 14;
                if (r < 0)
                    r = 0;
                if (g < 0)
                    g = 0;
                if (b < 0)
                    b = 0;
                c = r << 10 | g << 5 | b;
            }
            if (sptr->TransType == 2 || sptr->TransType == 3)
                HWI_InsertAlphaSprite_Sorted(x1, y1, z, c, x2, y1, z, c, x2, y2, z, c, x1, y2, z, c, -1, 13, 0);
            else
                HWI_InsertAlphaSprite_Sorted(x1, y1, z, c, x2, y1, z, c, x2, y2, z, c, x1, y2, z, c, -1, 11, 0);
        }
        sptr++;
    }
}
}
