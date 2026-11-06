#include "standard.h"
#include "specific.h"
#include "winmain.h"

extern "C" {

#include "../game.h"

#define MAX_FISH 8

typedef struct {
    sint16 x;
    sint16 y;
    sint16 z;
    uint16 angle;
    sint16 desty;
    sint8 angadd;
    uint8 speed;
    uint8 acc;
    uint8 swim;
} FISH_INFO;

typedef struct {
    sint16 angle;
    uint8 speed;
    uint8 on;
    sint16 angle_time;
    sint16 speed_time;
    sint16 Xrange;
    sint16 Zrange;
    sint16 Yrange;
} LEADER_INFO;

typedef struct phd_vbuf {
    float xv;
    float yv;
    float zv;
    float ooz;
    float xs;
    float ys;
    sint32 z;
    char clip;
    char fog;
    sint16 g;
    unsigned short u;
    unsigned short v;
    int dynamic;
} PHD_VBUF;

typedef struct phdtexturestruct {
    unsigned short drawtype;
    unsigned short tpage;
    unsigned short u1;
    unsigned short v1;
    unsigned short u2;
    unsigned short v2;
    unsigned short u3;
    unsigned short v3;
    unsigned short u4;
    unsigned short v4;
} PHDTEXTURESTRUCT;

typedef struct phdspritestruct {
    unsigned short tpage;
    unsigned short offset;
    unsigned short width;
    unsigned short height;
    sint16 x1;
    sint16 y1;
    sint16 x2;
    sint16 y2;
} PHDSPRITESTRUCT;

extern FISH_INFO fish[];
extern LEADER_INFO lead_info[];
extern PHDSPRITESTRUCT phdspriteinfo[];
extern sint16 rcossin_tbl[];
extern sint16 phd_winxmin;
extern sint16 phd_winymin;
extern sint16 phd_winxmax;
extern sint16 phd_winymax;
extern int phd_persp;
extern float f_persp;
extern float f_oneopersp;

void mCalcPoint(sint32 x, sint32 y, sint32 z, sint32 *result);
void ProjectPCoord(sint32 x, sint32 y, sint32 z, sint32 *result, sint32 cx, sint32 cy, sint32 fov);
void HWI_InsertGT3_Poly(PHD_VBUF *v1, PHD_VBUF *v2, PHD_VBUF *v3, PHDTEXTURESTRUCT *pTex, unsigned short *uv1, unsigned short *uv2, unsigned short *uv3, int nSortType, unsigned short dbl);

void S_DrawFish(ITEM_INFO *item)
{
    long lp, leader;
    long xoff, yoff, zoff;
    unsigned short u1, v1, u2, v2;
    long angle;
    long result[3];
    long scr[3][3];
    long x, y, z, size;
    long x1, x2, x3, y1, y2, y3, z1, z2, z3;
    FISH_INFO *fptr;
    PHDSPRITESTRUCT *pSpriteInfo;
    PHD_VBUF v[3];
    PHDTEXTURESTRUCT Tex;
    int clipflag;
    int rgb;
    int sw, sh;
    int clipx, clipy;

    sw = App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].w;
    sh = App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].h;
    clipx = phd_winxmin + phd_winxmax;
    clipy = phd_winymin + phd_winymax;
    if (!item->active)
        return;
    leader = item->hit_points;
    if (leader == -1)
        return;
    if (!lead_info[leader].on)
        return;
    xoff = item->pos.x_pos;
    yoff = item->pos.y_pos;
    zoff = item->pos.z_pos;
    if (item->object_number == 338)
        pSpriteInfo = phdspriteinfo + objects[296].mesh_index + 10;
    else
        pSpriteInfo = phdspriteinfo + objects[296].mesh_index + 11;
    fptr = &fish[MAX_FISH + leader * 24];
    for (lp = 0; lp < 24; lp++) {
        angle = ((rcossin_tbl[fptr->swim << 7] >> 5) + fptr->angle - 2048) & 4095;
        x = fptr->x + xoff;
        y = fptr->y + yoff;
        z = fptr->z + zoff;
        mCalcPoint(x, y, z, result);
        ProjectPCoord(result[0], result[1], result[2], scr[0], sw >> 1, sh >> 1, phd_persp);
        size = (rcossin_tbl[lp << 7] >> 5) + 192;
        z += (rcossin_tbl[(angle << 1) + 1] * size) >> 12;
        x += -((rcossin_tbl[angle << 1] * size) >> 12);
        y -= size;
        mCalcPoint(x, y, z, result);
        ProjectPCoord(result[0], result[1], result[2], scr[1], sw >> 1, sh >> 1, phd_persp);
        y += size << 1;
        mCalcPoint(x, y, z, result);
        ProjectPCoord(result[0], result[1], result[2], scr[2], sw >> 1, sh >> 1, phd_persp);
        x1 = scr[0][0];
        x2 = scr[1][0];
        x3 = scr[2][0];
        y1 = scr[0][1];
        y2 = scr[1][1];
        y3 = scr[2][1];
        z1 = scr[0][2];
        z2 = scr[1][2];
        z3 = scr[2][2];
        if (z1 > 0x5000 || z1 < 32 || z2 < 32 || z3 < 32
            || (x1 < phd_winxmin && x2 < phd_winxmin && x3 < phd_winxmin)
            || (x1 >= clipx && x2 >= clipx && x3 >= clipx)
            || (y1 < phd_winymin && y2 < phd_winymin && y3 < phd_winymin)
            || (y1 >= clipy && y2 >= clipy && y3 >= clipy)) {
            fptr++;
            continue;
        }
        if (angle < 1024)
            angle -= 512;
        else if (angle < 2048)
            angle -= 1536;
        else if (angle < 3072)
            angle -= 2560;
        else
            angle -= 3584;
        if (angle > 512 || angle < 0)
            angle = 0;
        else if (angle < 256)
            angle >>= 2;
        else
            angle = (512 - angle) >> 2;
        angle += lp;
        if (angle > 128)
            angle = 128;
        z1 <<= W2V_SHIFT;
        z2 <<= W2V_SHIFT;
        z3 <<= W2V_SHIFT;
        rgb = 80 + angle;
        rgb = (rgb >> 3) << 10 | (rgb >> 3) << 5 | (rgb >> 3);
        clipflag = 0;
        if (x1 < phd_winxmin)
            clipflag++;
        else if (x1 > clipx)
            clipflag += 2;
        if (y1 < phd_winymin)
            clipflag += 4;
        else if (y1 > clipy)
            clipflag += 8;
        v[0].xs = float(x1);
        v[0].ys = float(y1);
        v[0].ooz = (f_persp / (float)z1) * f_oneopersp;
        v[0].clip = clipflag;
        v[0].g = rgb;
        clipflag = 0;
        if (x2 < phd_winxmin)
            clipflag++;
        else if (x2 > clipx)
            clipflag += 2;
        if (y2 < phd_winymin)
            clipflag += 4;
        else if (y2 > clipy)
            clipflag += 8;
        v[1].xs = float(x2);
        v[1].ys = float(y2);
        v[1].ooz = (f_persp / (float)z2) * f_oneopersp;
        v[1].clip = clipflag;
        v[1].g = rgb;
        clipflag = 0;
        if (x3 < phd_winxmin)
            clipflag++;
        else if (x3 > clipx)
            clipflag += 2;
        if (y3 < phd_winymin)
            clipflag += 4;
        else if (y3 > clipy)
            clipflag += 8;
        v[2].xs = float(x3);
        v[2].ys = float(y3);
        v[2].ooz = (f_persp / (float)z2) * f_oneopersp;
        v[2].clip = clipflag;
        v[2].g = rgb;
        v[0].zv = float(z1);
        v[1].zv = float(z2);
        v[2].zv = float(z3);
        u1 = (pSpriteInfo->offset << 8) & 0xff00;
        v1 = pSpriteInfo->offset & 0xff00;
        u2 = u1 + pSpriteInfo->width;
        v2 = v1 + pSpriteInfo->height;
        u1 += App.nUVAdd;
        u2 -= App.nUVAdd;
        v1 += App.nUVAdd;
        v2 -= App.nUVAdd;
        Tex.drawtype = 1;
        Tex.tpage = pSpriteInfo->tpage;
        if (item->object_number == 338) {
            if (lp & 1) {
                v[0].u = u1;
                v[0].v = v1;
                v[1].u = u2;
                v[1].v = v1;
                v[2].u = u1;
                v[2].v = v2;
            } else {
                v[0].u = u2;
                v[0].v = v2;
                v[1].u = u1;
                v[1].v = v2;
                v[2].u = u2;
                v[2].v = v1;
            }
        } else {
            if (leader & 1) {
                v[0].u = u1;
                v[0].v = v1;
                v[1].u = u2;
                v[1].v = v1;
                v[2].u = u1;
                v[2].v = v2;
            } else {
                v[0].u = u2;
                v[0].v = v2;
                v[1].u = u1;
                v[1].v = v2;
                v[2].u = u2;
                v[2].v = v1;
            }
        }
        HWI_InsertGT3_Poly(&v[0], &v[1], &v[2], &Tex, &v[0].u, &v[1].u, &v[2].u, 0, 1);
        fptr++;
    }
}

}
