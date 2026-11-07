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

typedef struct {
    sint16 x;
    sint16 y;
    sint16 z;
    uint8 rsub;
    uint8 gsub;
    uint8 bsub;
    uint8 pad[3];
    sint32 rgb;
} SHIELD_POINTS;

extern SHIELD_POINTS LondonBossShield[40];
extern SHIELD_POINTS TonyBossShield[40];
extern SHIELD_POINTS TribeBossShield[40];
extern SHIELD_POINTS WillBossShield[40];
extern char shield_active;
extern float f_centerx;
extern float f_centery;
extern sint32 phd_znear;
extern sint32 wibble;

void phd_PushMatrix(void);
void phd_TranslateAbs(sint32 x, sint32 y, sint32 z);
void HWI_InsertGT4_Sorted(PHD_VBUF *v1, PHD_VBUF *v2, PHD_VBUF *v3, PHD_VBUF *v4, PHDTEXTURESTRUCT *pTex, int nSortType, unsigned short dbl);

static __inline void ClipCheckPoint(PHD_VBUF *v, long x, long y, long z)
{
    long clipFlag;
    clipFlag = 0;
    if (x < phd_winxmin)
        clipFlag++;
    else if (x > phd_winxmax)
        clipFlag += 2;
    if (y < phd_winymin)
        clipFlag += 4;
    else if (y > phd_winymax)
        clipFlag += 8;
    v->clip = clipFlag;
    v->xs = (float)x;
    v->ys = (float)y;
    v->ooz = f_persp / (float)z * f_oneopersp;
}
static __inline void setColor(PHD_VBUF *v, long c, char flag)
{
    long r, g, b;
    if (!flag) {
        r = (c >> 3) & 0x1f;
        g = (c >> 11) & 0x1f;
        b = (c >> 19) & 0x1f;
        v->g = short(r << 10 | g << 5 | b);
    } else
        v->g = (short)c;
}
static __inline void setXYZ3(PHD_VBUF *v, char cFlag, long x1, long y1, long z1, long c1, long x2, long y2, long z2, long c2, long x3, long y3, long z3, long c3)
{
    ClipCheckPoint(&v[0], x1, y1, z1);
    ClipCheckPoint(&v[1], x2, y2, z2);
    ClipCheckPoint(&v[2], x3, y3, z3);
    setColor(&v[0], c1, cFlag);
    setColor(&v[1], c2, cFlag);
    setColor(&v[2], c3, cFlag);
}
static __inline void setXYZ4(PHD_VBUF *v, char cFlag, long x1, long y1, long z1, long c1, long x2, long y2, long z2, long c2, long x3, long y3, long z3, long c3, long x4, long y4, long z4, long c4)
{
    ClipCheckPoint(&v[0], x1, y1, z1);
    setColor(&v[0], c1, cFlag);
    ClipCheckPoint(&v[1], x2, y2, z2);
    setColor(&v[1], c2, cFlag);
    ClipCheckPoint(&v[3], x4, y4, z4);
    setColor(&v[3], c4, cFlag);
    ClipCheckPoint(&v[2], x3, y3, z3);
    v[0].zv = (float)z1;
    v[1].zv = (float)z2;
    v[3].zv = (float)z4;
    v[2].zv = (float)z3;
}

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

bool ClipLine(long &x1, long &y1, long &x2, long &y2, long w, long h)
{
    float clip;
    long xMin, yMin;
    w = phd_winxmin + phd_winxmax;
    h = phd_winymin + phd_winymax;
    xMin = phd_winxmin;
    yMin = phd_winymin;
    if (x1 < xMin && x2 < xMin)
        return 0;
    if (y1 < yMin && y2 < yMin)
        return 0;
    if (x1 > w && x2 > w)
        return 0;
    if (y1 > h && y2 > h)
        return 0;
    if (x1 > w) {
        clip = ((float)w - x2) / float(x1 - x2);
        x1 = w;
        y1 = long((y1 - y2) * clip + y2);
    }
    if (x2 > w) {
        clip = ((float)w - x1) / float(x2 - x1);
        x2 = w;
        y2 = long((y2 - y1) * clip + y1);
    }
    if (x1 < xMin) {
        clip = ((float)xMin - x1) / float(x2 - x1);
        x1 = xMin;
        y1 = long((y2 - y1) * clip + y1);
    }
    if (x2 < xMin) {
        clip = ((float)xMin - x2) / float(x1 - x2);
        x2 = xMin;
        y2 = long((y1 - y2) * clip + y2);
    }
    if (y1 > h) {
        clip = ((float)h - y2) / float(y1 - y2);
        y1 = h;
        x1 = long((x1 - x2) * clip + x2);
    }
    if (y2 > h) {
        clip = ((float)h - y1) / float(y2 - y1);
        y2 = h;
        x2 = long((x2 - x1) * clip + x1);
    }
    if (y1 < yMin) {
        clip = ((float)yMin - y1) / float(y2 - y1);
        y1 = yMin;
        x1 = long((x2 - x1) * clip + x1);
    }
    if (y2 < yMin) {
        clip = ((float)yMin - y2) / float(y1 - y2);
        y2 = yMin;
        x2 = long((x1 - x2) * clip + x2);
    }
    return 1;
}

void DrawLondonBossShield(ITEM_INFO *item)
{
    DISPLAYMODE *dm;
    SHIELD_POINTS *s0;
    SHIELD_POINTS *s1;
    PHDSPRITESTRUCT *sprite;
    PHD_VBUF v[4];
    PHDTEXTURESTRUCT tex;
    long *pZ0;
    long *pZ1;
    short *pXY0;
    short *pXY1;
    float zv;
    long w, h, r, g, b, rgb;
    long i, j;
    short x, y, z;
    long x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, c1, c2, c3, c4;
    long Z[150];
    long u1, v1, u2, v2;
    short XY[150];
    dm = &App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode];
    w = dm->w - 1;
    h = dm->h - 1;
    phd_PushMatrix();
    phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    pXY0 = XY;
    pZ0 = Z;
    s0 = &LondonBossShield[0];
    for (i = 0; i < 40; i++) {
        x = s0->x;
        y = s0->y;
        z = s0->z;
        if (i >= 8 && i <= 31) {
            rgb = s0->rgb;
            r = rgb & 0xff;
            g = (rgb >> 8) & 0xff;
            b = (rgb >> 16) & 0xff;
            if (rgb) {
                r -= s0->rsub;
                g -= s0->gsub;
                b -= s0->bsub;
                if (r < 0)
                    r = 0;
                if (g < 0)
                    g = 0;
                if (b < 0)
                    b = 0;
                s0->rgb = r | (g << 8) | (b << 16);
            }
        }
        s0++;
        z1 = x * phd_mxptr[8] + y * phd_mxptr[9] + z * phd_mxptr[10] + phd_mxptr[11];
        zv = f_persp / (float)z1;
        x1 = x * phd_mxptr[0] + y * phd_mxptr[1] + z * phd_mxptr[2] + phd_mxptr[3];
        pXY0[0] = (short)(x1 * zv + f_centerx);
        y1 = x * phd_mxptr[4] + y * phd_mxptr[5] + z * phd_mxptr[6] + phd_mxptr[7];
        pXY0[1] = (short)(y1 * zv + f_centery);
        *pZ0++ = z1;
        pXY0 += 2;
    }
    pXY0 = &XY[0];
    pZ0 = &Z[0];
    pXY1 = &XY[16];
    pZ1 = &Z[8];
    s0 = &LondonBossShield[0];
    s1 = &LondonBossShield[8];
    for (i = 0; i < 4; i++) {
        x1 = *pXY0++;
        y1 = *pXY0++;
        z1 = *pZ0++;
        x3 = *pXY1++;
        y3 = *pXY1++;
        z3 = *pZ1++;
        c1 = s0->rgb;
        c3 = s1->rgb;
        s0++;
        s1++;
        sprite = &phdspriteinfo[objects[296].mesh_index + 18 + ((i + (wibble >> 3)) & 7)];
        u1 = (sprite->offset << 8) & 0xff00;
        v1 = sprite->offset & 0xff00;
        u2 = u1 + sprite->width;
        v2 = v1 + sprite->height;
        u1 += App.nUVAdd;
        u2 -= App.nUVAdd;
        v1 += App.nUVAdd;
        v2 -= App.nUVAdd;
        for (j = 0; j < 8; j++) {
            if (j != 7) {
                x2 = *pXY0++;
                y2 = *pXY0++;
                z2 = *pZ0++;
                x4 = *pXY1++;
                y4 = *pXY1++;
                z4 = *pZ1++;
                c2 = s0->rgb;
                c4 = s1->rgb;
                s0++;
                s1++;
            } else {
                x2 = pXY0[-16];
                y2 = pXY0[-15];
                z2 = pZ0[-8];
                x4 = pXY1[-16];
                y4 = pXY1[-15];
                z4 = pZ1[-8];
                c2 = s0[-8].rgb;
                c4 = s1->rgb;
            }
            if ((z1 + z2 + z3 + z4) >> 2 > phd_znear && (c1 || c2 || c3 || c4) && x1 > -128 && x2 > -128 && x3 > -128 && x4 > -128 && x1 < w + 128 && x2 < w + 128 && x3 < w + 128 && x4 < w + 128 && y1 > -128 && y2 > -128 && y3 > -128 && y4 > -128 && y1 < h + 128 && y2 < h + 128 && y3 < h + 128 && y4 < h + 128) {
                setXYZ4(v, 0, x1, y1, z1, c1, x2, y2, z2, c2, x4, y4, z4, c4, x3, y3, z3, c3);
                tex.u1 = u1;
                tex.u2 = u2;
                tex.u3 = u2;
                tex.u4 = u1;
                tex.v1 = v1;
                tex.v2 = v1;
                tex.v3 = v2;
                tex.v4 = v2;
                tex.tpage = sprite->tpage;
                tex.drawtype = 2;
                HWI_InsertGT4_Sorted(&v[0], &v[1], &v[2], &v[3], &tex, 0, 1);
            }
            x1 = x2;
            y1 = y2;
            z1 = z2;
            x3 = x4;
            y3 = y4;
            z3 = z4;
            c1 = c2;
            c3 = c4;
        }
    }
    phd_PopMatrix();
}

void DrawTonyBossShield(ITEM_INFO *item)
{
    DISPLAYMODE *dm;
    SHIELD_POINTS *s0;
    SHIELD_POINTS *s1;
    PHDSPRITESTRUCT *sprite;
    PHD_VBUF v[4];
    PHDTEXTURESTRUCT tex;
    long *pZ0;
    long *pZ1;
    short *pXY0;
    short *pXY1;
    float zv;
    long w, h, r, g, b, rgb;
    long i, j;
    short x, y, z;
    long x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, c1, c2, c3, c4;
    long Z[150];
    long u1, v1, u2, v2;
    short XY[150];
    dm = &App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode];
    w = dm->w - 1;
    h = dm->h - 1;
    phd_PushMatrix();
    phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    pXY0 = XY;
    pZ0 = Z;
    s0 = &TonyBossShield[0];
    for (i = 0; i < 40; i++) {
        x = s0->x;
        y = s0->y;
        z = s0->z;
        if (i >= 8 && i <= 31) {
            rgb = s0->rgb;
            r = rgb & 0xff;
            g = (rgb >> 8) & 0xff;
            b = (rgb >> 16) & 0xff;
            if (rgb) {
                r -= s0->rsub;
                g -= s0->gsub;
                b -= s0->bsub;
                if (r < 0)
                    r = 0;
                if (g < 0)
                    g = 0;
                if (b < 0)
                    b = 0;
                s0->rgb = r | (g << 8) | (b << 16);
            }
        }
        s0++;
        z1 = x * phd_mxptr[8] + y * phd_mxptr[9] + z * phd_mxptr[10] + phd_mxptr[11];
        zv = f_persp / (float)z1;
        x1 = x * phd_mxptr[0] + y * phd_mxptr[1] + z * phd_mxptr[2] + phd_mxptr[3];
        pXY0[0] = (short)(x1 * zv + f_centerx);
        y1 = x * phd_mxptr[4] + y * phd_mxptr[5] + z * phd_mxptr[6] + phd_mxptr[7];
        pXY0[1] = (short)(y1 * zv + f_centery);
        *pZ0++ = z1;
        pXY0 += 2;
    }
    pXY0 = &XY[0];
    pZ0 = &Z[0];
    pXY1 = &XY[16];
    pZ1 = &Z[8];
    s0 = &TonyBossShield[0];
    s1 = &TonyBossShield[8];
    for (i = 0; i < 4; i++) {
        x1 = *pXY0++;
        y1 = *pXY0++;
        z1 = *pZ0++;
        x3 = *pXY1++;
        y3 = *pXY1++;
        z3 = *pZ1++;
        c1 = s0->rgb;
        c3 = s1->rgb;
        s0++;
        s1++;
        sprite = &phdspriteinfo[objects[296].mesh_index + 18 + ((i + (wibble >> 3)) & 7)];
        u1 = (sprite->offset << 8) & 0xff00;
        v1 = sprite->offset & 0xff00;
        u2 = u1 + sprite->width;
        v2 = v1 + sprite->height;
        u1 += App.nUVAdd;
        u2 -= App.nUVAdd;
        v1 += App.nUVAdd;
        v2 -= App.nUVAdd;
        for (j = 0; j < 8; j++) {
            if (j != 7) {
                x2 = *pXY0++;
                y2 = *pXY0++;
                z2 = *pZ0++;
                x4 = *pXY1++;
                y4 = *pXY1++;
                z4 = *pZ1++;
                c2 = s0->rgb;
                c4 = s1->rgb;
                s0++;
                s1++;
            } else {
                x2 = pXY0[-16];
                y2 = pXY0[-15];
                z2 = pZ0[-8];
                x4 = pXY1[-16];
                y4 = pXY1[-15];
                z4 = pZ1[-8];
                c2 = s0[-8].rgb;
                c4 = s1->rgb;
            }
            if ((z1 + z2 + z3 + z4) >> 2 > phd_znear && (c1 || c2 || c3 || c4) && x1 > -128 && x2 > -128 && x3 > -128 && x4 > -128 && x1 < w + 128 && x2 < w + 128 && x3 < w + 128 && x4 < w + 128 && y1 > -128 && y2 > -128 && y3 > -128 && y4 > -128 && y1 < h + 128 && y2 < h + 128 && y3 < h + 128 && y4 < h + 128) {
                setXYZ4(v, 0, x1, y1, z1, c1, x2, y2, z2, c2, x4, y4, z4, c4, x3, y3, z3, c3);
                tex.u1 = u1;
                tex.u2 = u2;
                tex.u3 = u2;
                tex.u4 = u1;
                tex.v1 = v1;
                tex.v2 = v1;
                tex.v3 = v2;
                tex.v4 = v2;
                tex.tpage = sprite->tpage;
                tex.drawtype = 2;
                HWI_InsertGT4_Sorted(&v[0], &v[1], &v[2], &v[3], &tex, 0, 1);
            }
            x1 = x2;
            y1 = y2;
            z1 = z2;
            x3 = x4;
            y3 = y4;
            z3 = z4;
            c1 = c2;
            c3 = c4;
        }
    }
    phd_PopMatrix();
}

void DrawTribeBossShield(ITEM_INFO *item)
{
    DISPLAYMODE *dm;
    SHIELD_POINTS *s0;
    SHIELD_POINTS *s1;
    PHDSPRITESTRUCT *sprite;
    PHD_VBUF v[4];
    PHDTEXTURESTRUCT tex;
    long *pZ0;
    long *pZ1;
    short *pXY0;
    short *pXY1;
    float zv;
    long w, h, r, g, b, rgb;
    long i, j;
    short x, y, z;
    long x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, c1, c2, c3, c4;
    long Z[150];
    long u1, v1, u2, v2;
    short XY[150];
    dm = &App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode];
    w = dm->w - 1;
    h = dm->h - 1;
    phd_PushMatrix();
    phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    pXY0 = XY;
    pZ0 = Z;
    shield_active = 0;
    s0 = &TribeBossShield[0];
    for (i = 0; i < 40; i++) {
        x = s0->x;
        y = s0->y;
        z = s0->z;
        if (i >= 8 && i <= 31) {
            rgb = s0->rgb;
            r = rgb & 0xff;
            g = (rgb >> 8) & 0xff;
            b = (rgb >> 16) & 0xff;
            if (rgb) {
                r -= s0->rsub;
                g -= s0->gsub;
                b -= s0->bsub;
                if (r < 0)
                    r = 0;
                if (g < 0)
                    g = 0;
                if (b < 0)
                    b = 0;
                s0->rgb = r | (g << 8) | (b << 16);
            }
        }
        s0++;
        z1 = x * phd_mxptr[8] + y * phd_mxptr[9] + z * phd_mxptr[10] + phd_mxptr[11];
        zv = f_persp / (float)z1;
        x1 = x * phd_mxptr[0] + y * phd_mxptr[1] + z * phd_mxptr[2] + phd_mxptr[3];
        pXY0[0] = (short)(x1 * zv + f_centerx);
        y1 = x * phd_mxptr[4] + y * phd_mxptr[5] + z * phd_mxptr[6] + phd_mxptr[7];
        pXY0[1] = (short)(y1 * zv + f_centery);
        *pZ0++ = z1;
        pXY0 += 2;
    }
    pXY0 = &XY[0];
    pZ0 = &Z[0];
    pXY1 = &XY[16];
    pZ1 = &Z[8];
    s0 = &TribeBossShield[0];
    s1 = &TribeBossShield[8];
    for (i = 0; i < 4; i++) {
        x1 = *pXY0++;
        y1 = *pXY0++;
        z1 = *pZ0++;
        x3 = *pXY1++;
        y3 = *pXY1++;
        z3 = *pZ1++;
        c1 = s0->rgb;
        c3 = s1->rgb;
        s0++;
        s1++;
        sprite = &phdspriteinfo[objects[296].mesh_index + 18 + ((i + (wibble >> 3)) & 7)];
        u1 = (sprite->offset << 8) & 0xff00;
        v1 = sprite->offset & 0xff00;
        u2 = u1 + sprite->width;
        v2 = v1 + sprite->height;
        u1 += App.nUVAdd;
        u2 -= App.nUVAdd;
        v1 += App.nUVAdd;
        v2 -= App.nUVAdd;
        for (j = 0; j < 8; j++) {
            if (j != 7) {
                x2 = *pXY0++;
                y2 = *pXY0++;
                z2 = *pZ0++;
                x4 = *pXY1++;
                y4 = *pXY1++;
                z4 = *pZ1++;
                c2 = s0->rgb;
                c4 = s1->rgb;
                s0++;
                s1++;
            } else {
                x2 = pXY0[-16];
                y2 = pXY0[-15];
                z2 = pZ0[-8];
                x4 = pXY1[-16];
                y4 = pXY1[-15];
                z4 = pZ1[-8];
                c2 = s0[-8].rgb;
                c4 = s1->rgb;
            }
            if ((z1 + z2 + z3 + z4) >> 2 > phd_znear && (c1 || c2 || c3 || c4) && x1 > -128 && x2 > -128 && x3 > -128 && x4 > -128 && x1 < w + 128 && x2 < w + 128 && x3 < w + 128 && x4 < w + 128 && y1 > -128 && y2 > -128 && y3 > -128 && y4 > -128 && y1 < h + 128 && y2 < h + 128 && y3 < h + 128 && y4 < h + 128) {
                setXYZ4(v, 0, x1, y1, z1, c1, x2, y2, z2, c2, x4, y4, z4, c4, x3, y3, z3, c3);
                tex.u1 = u1;
                tex.u2 = u2;
                tex.u3 = u2;
                tex.u4 = u1;
                tex.v1 = v1;
                tex.v2 = v1;
                tex.v3 = v2;
                tex.v4 = v2;
                tex.tpage = sprite->tpage;
                tex.drawtype = 2;
                HWI_InsertGT4_Sorted(&v[0], &v[1], &v[2], &v[3], &tex, 0, 1);
            }
            x1 = x2;
            y1 = y2;
            z1 = z2;
            x3 = x4;
            y3 = y4;
            z3 = z4;
            c1 = c2;
            c3 = c4;
        }
    }
    phd_PopMatrix();
}

void DrawWillBossShield(ITEM_INFO *item)
{
    DISPLAYMODE *dm;
    SHIELD_POINTS *s0;
    SHIELD_POINTS *s1;
    PHDSPRITESTRUCT *sprite;
    PHD_VBUF v[4];
    PHDTEXTURESTRUCT tex;
    long *pZ0;
    long *pZ1;
    short *pXY0;
    short *pXY1;
    float zv;
    long w, h, r, g, b, rgb;
    long i, j;
    short x, y, z;
    long x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, c1, c2, c3, c4;
    long Z[150];
    long u1, v1, u2, v2;
    short XY[150];
    dm = &App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode];
    w = dm->w - 1;
    h = dm->h - 1;
    phd_PushMatrix();
    phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    pXY0 = XY;
    pZ0 = Z;
    s0 = &WillBossShield[0];
    for (i = 0; i < 40; i++) {
        x = s0->x;
        y = s0->y;
        z = s0->z;
        if (i >= 8 && i <= 31) {
            rgb = s0->rgb;
            r = rgb & 0xff;
            g = (rgb >> 8) & 0xff;
            b = (rgb >> 16) & 0xff;
            if (rgb) {
                r -= s0->rsub;
                g -= s0->gsub;
                b -= s0->bsub;
                if (r < 0)
                    r = 0;
                if (g < 0)
                    g = 0;
                if (b < 0)
                    b = 0;
                s0->rgb = r | (g << 8) | (b << 16);
            }
        }
        s0++;
        z1 = x * phd_mxptr[8] + y * phd_mxptr[9] + z * phd_mxptr[10] + phd_mxptr[11];
        zv = f_persp / (float)z1;
        x1 = x * phd_mxptr[0] + y * phd_mxptr[1] + z * phd_mxptr[2] + phd_mxptr[3];
        pXY0[0] = (short)(x1 * zv + f_centerx);
        y1 = x * phd_mxptr[4] + y * phd_mxptr[5] + z * phd_mxptr[6] + phd_mxptr[7];
        pXY0[1] = (short)(y1 * zv + f_centery);
        *pZ0++ = z1;
        pXY0 += 2;
    }
    pXY0 = &XY[0];
    pZ0 = &Z[0];
    pXY1 = &XY[16];
    pZ1 = &Z[8];
    s0 = &WillBossShield[0];
    s1 = &WillBossShield[8];
    for (i = 0; i < 4; i++) {
        x1 = *pXY0++;
        y1 = *pXY0++;
        z1 = *pZ0++;
        x3 = *pXY1++;
        y3 = *pXY1++;
        z3 = *pZ1++;
        c1 = s0->rgb;
        c3 = s1->rgb;
        s0++;
        s1++;
        sprite = &phdspriteinfo[objects[296].mesh_index + 18 + ((i + (wibble >> 3)) & 7)];
        u1 = (sprite->offset << 8) & 0xff00;
        v1 = sprite->offset & 0xff00;
        u2 = u1 + sprite->width;
        v2 = v1 + sprite->height;
        u1 += App.nUVAdd;
        u2 -= App.nUVAdd;
        v1 += App.nUVAdd;
        v2 -= App.nUVAdd;
        for (j = 0; j < 8; j++) {
            if (j != 7) {
                x2 = *pXY0++;
                y2 = *pXY0++;
                z2 = *pZ0++;
                x4 = *pXY1++;
                y4 = *pXY1++;
                z4 = *pZ1++;
                c2 = s0->rgb;
                c4 = s1->rgb;
                s0++;
                s1++;
            } else {
                x2 = pXY0[-16];
                y2 = pXY0[-15];
                z2 = pZ0[-8];
                x4 = pXY1[-16];
                y4 = pXY1[-15];
                z4 = pZ1[-8];
                c2 = s0[-8].rgb;
                c4 = s1->rgb;
            }
            if ((z1 + z2 + z3 + z4) >> 2 > phd_znear && (c1 || c2 || c3 || c4) && x1 > -128 && x2 > -128 && x3 > -128 && x4 > -128 && x1 < w + 128 && x2 < w + 128 && x3 < w + 128 && x4 < w + 128 && y1 > -128 && y2 > -128 && y3 > -128 && y4 > -128 && y1 < h + 128 && y2 < h + 128 && y3 < h + 128 && y4 < h + 128) {
                setXYZ4(v, 0, x1, y1, z1, c1, x2, y2, z2, c2, x4, y4, z4, c4, x3, y3, z3, c3);
                tex.u1 = u1;
                tex.u2 = u2;
                tex.u3 = u2;
                tex.u4 = u1;
                tex.v1 = v1;
                tex.v2 = v1;
                tex.v3 = v2;
                tex.v4 = v2;
                tex.tpage = sprite->tpage;
                tex.drawtype = 2;
                HWI_InsertGT4_Sorted(&v[0], &v[1], &v[2], &v[3], &tex, 0, 1);
            }
            x1 = x2;
            y1 = y2;
            z1 = z2;
            x3 = x4;
            y3 = y4;
            z3 = z4;
            c1 = c2;
            c3 = c4;
        }
    }
    phd_PopMatrix();
}

extern BITE_INFO NodeOffsets[16];
extern int distanceFogValue;
extern sint32 phd_zfar;
void HWI_InsertAlphaSprite_Sorted(long x1, long y1, long z1, long shade1, long x2, long y2, long z2, long shade2, long x3, long y3, long z3, long shade3, long x4, long y4, long z4, long shade4, long nSprite, long nDrawtype, long double_sided);

void S_DrawSparks(void)
{
    DISPLAYMODE *dm;
    SPARKS *sptr;
    FX_INFO *fx;
    ITEM_INFO *item;
    PHD_VECTOR pos;
    long w, h, x, y, z, sw, sh;
    long x1, y1, x2, y2, x3, y3, x4, y4, r, g, b, f, c;
    long vpos[3][3];
    long point[3];
    long i;
    dm = &App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode];
    w = dm->w;
    h = dm->h;
    for (i = 0, sptr = &sparks[0]; i < 192; i++, sptr++) {
        if (!sptr->On)
            continue;
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
            item = &items[sptr->FxObj];
            if (sptr->Flags & 1024) {
                x = item->pos.x_pos;
                y = item->pos.y_pos;
                z = item->pos.z_pos;
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
                y = sptr->y + item->pos.y_pos;
                z = sptr->z + item->pos.z_pos;
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
                continue;
            x = vpos[0][0];
            y = vpos[0][1];
            if (x + (sw >> 1) < 0 || x - (sw >> 1) > w || y + (sh >> 1) < 0 || y - (sh >> 1) > h)
                continue;
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
                continue;
            x = vpos[0][0];
            y = vpos[0][1];
            if (x + (sw >> 1) < 0 || x - (sw >> 1) > w || y + (sh >> 1) < 0 || y - (sh >> 1) > h)
                continue;
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
    }
}


typedef struct {
    sint32 x;
    sint32 old_roomflags : 1;
    sint32 stopped : 1;
    sint32 y : 30;
    sint32 z;
    sint8 xv;
    uint8 yv;
    sint8 zv;
    uint8 life;
} SNOWFLAKE;

static SNOWFLAKE raindrops[256];

extern sint16 IsRoomOutsideNo;
extern sint32 SmokeWindX;
extern sint32 SmokeWindZ;
extern PHD_VECTOR CamPos;
extern int GlobalAlpha;

sint32 IsRoomOutside(sint32 x, sint32 y, sint32 z);
void HWI_InsertLine_Sorted(long x1, long y1, long x2, long y2, long z, long c0, long c1);

int abs(int x);
#pragma intrinsic(abs)

void DoRain(void)
{
    SNOWFLAKE *rainptr;
    PHD_VECTOR pos;
    long *pZ;
    short *pXY;
    float zv;
    long x, y, z, radius, lp, angle, randval, out, triggered;
    long x1, y1, x2, y2, alpha;
    short tx, ty, tz;
    long Z[50];
    short XY[50];

    pXY = XY;
    pZ = Z;
    triggered = 0;
    rainptr = &raindrops[0];
    for (lp = 0; lp < 256; lp++) {
        if (rainptr->x == 0 && triggered < 8) {
            triggered++;
            y = -1024 - (GetRandomDraw() & 2047);
            radius = GetRandomDraw() & 4095;
            angle = GetRandomDraw() & 8190;
            x = (rcossin_tbl[angle] * radius) >> 12;
            z = (rcossin_tbl[angle + 1] * radius) >> 12;
            rainptr->x = lara_item->pos.x_pos + x;
            rainptr->y = lara_item->pos.y_pos + y;
            rainptr->z = lara_item->pos.z_pos + z;
            if (IsRoomOutside(rainptr->x, rainptr->y, rainptr->z) < 0) {
                rainptr->x = 0;
                rainptr++;
                continue;
            }
            rainptr->xv = (GetRandomDraw() & 7) - 4;
            rainptr->yv = (GetRandomDraw() & 7) + 16;
            rainptr->zv = (GetRandomDraw() & 7) - 4;
            rainptr->life = 32 + (56 - (rainptr->yv << 1));
        }
        if (rainptr->x != 0) {
            out = IsRoomOutside(rainptr->x, rainptr->y, rainptr->z);
            if (out == -2 || room[IsRoomOutsideNo].flags & UNDERWATER) {
                rainptr->x = 0;
                rainptr++;
                continue;
            }
            if (rainptr->life > 240 || abs(CamPos.x - rainptr->x) > 6000 || abs(CamPos.z - rainptr->z) > 6000) {
                rainptr->x = 0;
                rainptr++;
                continue;
            }
            rainptr->x += rainptr->xv + (SmokeWindX << 2);
            rainptr->y += rainptr->yv << 3;
            rainptr->z += rainptr->zv + (SmokeWindZ << 2);
            randval = GetRandomDraw();
            if ((randval & 3) != 3) {
                rainptr->xv += (randval & 3) - 1;
                if (rainptr->xv < -4)
                    rainptr->xv = -4;
                else if (rainptr->xv > 4)
                    rainptr->xv = 4;
            }
            randval = (randval >> 2) & 3;
            if (randval != 3) {
                rainptr->zv += randval - 1;
                if (rainptr->zv < -4)
                    rainptr->zv = -4;
                else if (rainptr->zv > 4)
                    rainptr->zv = 4;
            }
            rainptr->life -= 2;
            if (rainptr->life > 240)
                rainptr->x = 0;
        }
        rainptr++;
    }
    phd_PushMatrix();
    phd_TranslateAbs(lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos);
    rainptr = &raindrops[0];
    for (lp = 0; lp < 256; lp++) {
        if (rainptr->x == 0) {
            rainptr++;
            continue;
        }
        tx = rainptr->x - (SmokeWindX << 2) - lara_item->pos.x_pos;
        ty = rainptr->y - (rainptr->yv << 3) - lara_item->pos.y_pos;
        tz = rainptr->z - (SmokeWindZ << 2) - lara_item->pos.z_pos;
        pos.x = tx * phd_mxptr[0] + ty * phd_mxptr[1] + tz * phd_mxptr[2] + phd_mxptr[3];
        pos.y = tx * phd_mxptr[4] + ty * phd_mxptr[5] + tz * phd_mxptr[6] + phd_mxptr[7];
        pos.z = tx * phd_mxptr[8] + ty * phd_mxptr[9] + tz * phd_mxptr[10] + phd_mxptr[11];
        zv = f_persp / (float)pos.z;
        pXY[0] = (short)(pos.x * zv + f_centerx);
        pXY[1] = (short)(pos.y * zv + f_centery);
        pZ[0] = pos.z;
        pXY += 2;
        pZ++;
        tx = rainptr->x - lara_item->pos.x_pos;
        ty = rainptr->y - lara_item->pos.y_pos;
        tz = rainptr->z - lara_item->pos.z_pos;
        pos.x = tx * phd_mxptr[0] + ty * phd_mxptr[1] + tz * phd_mxptr[2] + phd_mxptr[3];
        pos.y = tx * phd_mxptr[4] + ty * phd_mxptr[5] + tz * phd_mxptr[6] + phd_mxptr[7];
        pos.z = tx * phd_mxptr[8] + ty * phd_mxptr[9] + tz * phd_mxptr[10] + phd_mxptr[11];
        zv = f_persp / (float)pos.z;
        pXY[0] = (short)(pos.x * zv + f_centerx);
        pXY[1] = (short)(pos.y * zv + f_centery);
        pZ[0] = pos.z;
        pXY -= 2;
        pZ--;
        x1 = pXY[0];
        y1 = pXY[1];
        x2 = pXY[2];
        y2 = pXY[3];
        z = pZ[0];
        rainptr++;
        if (z < 32 || x1 < phd_winxmin || x1 > phd_winxmin + phd_winxmax || y1 < phd_winymin || y1 > phd_winymin + phd_winymax)
            continue;
        if (ClipLine(x1, y1, x2, y2, 0, 0)) {
            alpha = GlobalAlpha;
            GlobalAlpha = 0x80000000;
            HWI_InsertLine_Sorted(x1 - phd_winxmin, y1 - phd_winymin, x2 - phd_winxmin, y2 - phd_winymin, z, 32, 0x304060);
            GlobalAlpha = alpha;
        }
    }
    phd_PopMatrix();
}


typedef struct {
    sint16 x;
    sint16 z;
    sint32 rgb;
} EXPLOSION_VERTS;

typedef struct {
    sint16 on;
    sint16 life;
    sint16 speed;
    sint16 radius;
    sint16 xrot;
    sint16 zrot;
    sint32 x;
    sint32 y;
    sint32 z;
    EXPLOSION_VERTS verts[16];
} EXPLOSION_RING;

extern EXPLOSION_RING ExpRings[];

void phd_RotX(sint16 rx);
void phd_RotZ(sint16 rz);

void DrawExplosionRings(void)
{
    DISPLAYMODE *dm;
    EXPLOSION_RING *ring;
    EXPLOSION_VERTS *vtx;
    EXPLOSION_VERTS *vtx2;
    PHDSPRITESTRUCT *sprite;
    PHD_VBUF v[4];
    PHD_VECTOR pos;
    PHDTEXTURESTRUCT tex;
    long *pZ;
    long *pZ2;
    short *pXY;
    short *pXY2;
    float zv;
    long w, h, rad, ang, r, g, b, x, y, z;
    long x1, x2, x3, x4, y1, y2, y3, y4, z1, z2, z3, z4, col1, col2, col3, col4;
    long i, j, k;
    long Z[50];
    short XY[50];
    long u1, u2, v1, v2;
    dm = &App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode];
    w = dm->w - 1;
    h = dm->h - 1;
    for (i = 0; i < 6; i++) {
        ring = &ExpRings[i];
        if (!ring->on)
            continue;
        ring->life--;
        if (!ring->life) {
            ring->on = 0;
            continue;
        }
        phd_PushMatrix();
        phd_TranslateAbs(ring->x, ring->y, ring->z);
        phd_RotZ(ring->zrot << 4);
        phd_RotX(ring->xrot << 4);
        ring->radius += ring->speed;
        rad = ring->radius;
        vtx = ring->verts;
        pXY = XY;
        pZ = Z;
        for (j = 0; j < 2; j++) {
            ang = (wibble & 0x3f) << 3;
            for (k = 0; k < 8; k++) {
                vtx->x = short((rad * rcossin_tbl[ang << 1]) >> (12));
                vtx->z = short((rad * rcossin_tbl[(ang << 1) + 1]) >> (12));
                r = GetRandomDraw() & 0x1f;
                g = (GetRandomDraw() & 0x3f) + 224;
                b = (g >> 2) + (GetRandomDraw() & 0x3f);
                r = (r * ring->life) >> 5;
                g = (g * ring->life) >> 5;
                b = (b * ring->life) >> 5;
                vtx->rgb = r | (g << 8) | (b << 16);
                ang = (ang + 512) & 0xfff;
                x = vtx->x;
                y = 0;
                z = vtx->z;
                pos.z = phd_mxptr[8] * x + phd_mxptr[9] * y + phd_mxptr[10] * z + phd_mxptr[11];
                zv = f_persp / (float)pos.z;
                pos.x = phd_mxptr[0] * x + phd_mxptr[1] * y + phd_mxptr[2] * z + phd_mxptr[3];
                pXY[0] = (short)(pos.x * zv + f_centerx);
                pos.y = phd_mxptr[4] * x + phd_mxptr[5] * y + phd_mxptr[6] * z + phd_mxptr[7];
                pXY[1] = (short)(pos.y * zv + f_centery);
                pZ[0] = pos.z;
                vtx++;
                pXY += 2;
                pZ++;
            }
            rad >>= 1;
        }
        vtx = ring->verts;
        vtx2 = &ring->verts[8];
        pXY = XY;
        pZ = Z;
        pXY2 = &XY[16];
        pZ2 = &Z[8];
        x1 = *pXY++;
        y1 = *pXY++;
        z1 = *pZ++;
        x3 = *pXY2++;
        y3 = *pXY2++;
        z3 = *pZ2++;
        col1 = vtx->rgb;
        col3 = vtx2->rgb;
        sprite = &phdspriteinfo[objects[296].mesh_index + 4 + ((wibble >> 4) & 3)];
        u1 = (sprite->offset << 8) & 0xff00;
        v1 = sprite->offset & 0xff00;
        u2 = u1 + sprite->width;
        v2 = v1 + sprite->height;
        u1 += App.nUVAdd;
        u2 -= App.nUVAdd;
        v1 += App.nUVAdd;
        v2 -= App.nUVAdd;
        for (j = 0; j < 8; j++) {
            if (j != 7) {
                x2 = *pXY++;
                y2 = *pXY++;
                z2 = *pZ++;
                x4 = *pXY2++;
                y4 = *pXY2++;
                z4 = *pZ2++;
                col2 = vtx->rgb;
                col4 = vtx2->rgb;
                vtx++;
                vtx2++;
            } else {
                x2 = pXY[-16];
                y2 = pXY[-15];
                z2 = pZ[-8];
                x4 = pXY2[-16];
                y4 = pXY2[-15];
                z4 = pZ2[-8];
                col2 = vtx[-8].rgb;
                col4 = vtx->rgb;
            }
            if (((z1 + z2 + z3 + z4) >> 2) > phd_znear && (col1 || col2 || col3 || col4) && x1 > -128 && x2 > -128 && x3 > -128 && x4 > -128 && x1 < w + 128 && x2 < w + 128 && x3 < w + 128 && x4 < w + 128 && y1 > -128 && y2 > -128 && y3 > -128 && y4 > -128 && y1 < h + 128 && y2 < h + 128 && y3 < h + 128 && y4 < h + 128) {
                setXYZ4(v, 0, x1, y1, z1, col1, x2, y2, z2, col2, x4, y4, z4, col4, x3, y3, z3, col3);
                tex.u1 = u1;
                tex.u2 = u2;
                tex.u3 = u1;
                tex.u4 = u2;
                tex.v1 = v1;
                tex.v2 = v1;
                tex.v3 = v2;
                tex.v4 = v2;
                tex.tpage = sprite->tpage;
                tex.drawtype = 2;
                HWI_InsertGT4_Sorted(&v[0], &v[1], &v[2], &v[3], &tex, 0, 1);
            }
            x1 = x2;
            y1 = y2;
            z1 = z2;
            x3 = x4;
            y3 = y4;
            z3 = z4;
            col1 = col2;
            col3 = col4;
        }
        phd_PopMatrix();
    }
}

void HWI_InsertLine_Sorted(long x1, long y1, long x2, long y2, long z, long c0, long c1);

void S_DrawDarts(ITEM_INFO *item)
{
    DISPLAYMODE *dm;
    sint32 *mptr;
    float zv;
    long w, h, size;
    sint16 x, y, z;
    sint16 sxy[4];
    long sz[2];
    sint16 *p;
    long *pz;
    long x1, y1, x2, y2, z1;

    dm = &App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode];
    w = dm->w - 1;
    h = dm->h - 1;
    phd_PushMatrix();
    phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    mptr = phd_mxptr;
    z1 = mptr[11];
    zv = f_persp / (float)z1;
    sxy[0] = (sint16)(float)(mptr[3] * zv + f_centerx);
    sxy[1] = (sint16)(float)(mptr[7] * zv + f_centery);
    sz[0] = z1;
    size = (-96 * phd_cos(item->pos.x_rot)) >> W2V_SHIFT;
    z = (sint16)((size * phd_cos(item->pos.y_rot)) >> W2V_SHIFT);
    x = (sint16)((size * phd_sin(item->pos.y_rot)) >> W2V_SHIFT);
    y = (sint16)((96 * phd_sin(item->pos.x_rot)) >> W2V_SHIFT);
    z1 = mptr[8] * x + mptr[9] * y + mptr[10] * z + mptr[11];
    zv = f_persp / (float)z1;
    sxy[2] = (sint16)(float)((mptr[0] * x + mptr[1] * y + mptr[2] * z + mptr[3]) * zv + f_centerx);
    sxy[3] = (sint16)(float)((mptr[4] * x + mptr[5] * y + mptr[6] * z + mptr[7]) * zv + f_centery);
    sz[1] = z1;
    p = sxy;
    pz = sz;
    x1 = p[0];
    y1 = p[1];
    x2 = p[2];
    y2 = p[3];
    if (pz[0] > 32 && pz[1] > 32 && ClipLine(x1, y1, x2, y2, w, h))
        HWI_InsertLine_Sorted(x1 - phd_winxmin, y1 - phd_winymin, x2 - phd_winxmin, y2 - phd_winymin, pz[0], 0, 0x783c14);
    phd_PopMatrix();
}

extern unsigned char SplashLinks[];

void HWI_InsertAlphaSprite_Sorted(long x1, long y1, long z1, long shade1, long x2, long y2, long z2, long shade2, long x3, long y3, long z3, long shade3, long x4, long y4, long z4, long shade4, long nSprite, long nDrawtype, long double_sided);

void S_DrawSplashes(void)
{
    RIPPLE_STRUCT *rptr;
    SPLASH_STRUCT *sptr;
    SPLASH_VERTS *svptr;
    long *result;
    long *scrpoints;
    unsigned char *pptr;
    long lp, vlp, olp;
    long wx, wy, wz, linkadd;
    long x1, x2, x3, x4, y1, y2, y3, y4;
    long z1, z2, z3, z4;
    long mscrpoints[48 * 4];
    long mresult[3];
    int nSprite;
    int sw = App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].w;
    int sh = App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].h;

    sptr = &splashes[0];
    for (lp = 0; lp < 4; lp++) {
        if ((sptr->flags & 1) == 0) {
            sptr++;
            continue;
        }
        svptr = &sptr->sv[0];
        scrpoints = (long *)&mscrpoints;
        result = (long *)&mresult;
        for (vlp = 0; vlp < 48; vlp++) {
            wx = (svptr->wx >> 4) + sptr->x;
            wy = svptr->wy + sptr->y;
            wz = (svptr->wz >> 4) + sptr->z;
            mCalcPoint(wx, wy, wz, result);
            ProjectPCoord(result[0], result[1], result[2], scrpoints, sw >> 1, sh >> 1, phd_persp);
            scrpoints += 4;
            svptr++;
        }
        scrpoints = (long *)&mscrpoints;
        for (olp = 0; olp < 3; olp++) {
            if (olp == 2 || (olp == 0 && (sptr->flags & 4)) || (olp == 1 && (sptr->flags & 8)))
                nSprite = objects[296].mesh_index + 4 + ((wibble >> 4) & 3);
            else
                nSprite = objects[296].mesh_index + 8;
            pptr = &SplashLinks[0];
            linkadd = olp << 6;
            for (vlp = 0; vlp < 8; vlp++) {
                x1 = scrpoints[(*pptr) + linkadd];
                y1 = scrpoints[(*pptr) + 1 + linkadd];
                z1 = scrpoints[(*pptr++) + 2 + linkadd];
                x2 = scrpoints[(*pptr) + linkadd];
                y2 = scrpoints[(*pptr) + 1 + linkadd];
                z2 = scrpoints[(*pptr++) + 2 + linkadd];
                x3 = scrpoints[(*pptr) + linkadd];
                y3 = scrpoints[(*pptr) + 1 + linkadd];
                z3 = scrpoints[(*pptr++) + 2 + linkadd];
                x4 = scrpoints[(*pptr) + linkadd];
                y4 = scrpoints[(*pptr) + 1 + linkadd];
                z4 = scrpoints[(*pptr++) + 2 + linkadd];
                if ((x1 < 0 && x2 < 0 && x3 < 0 && x4 < 0) || (x1 >= sw && x2 >= sw && x3 >= sw && x4 >= sw) || (y1 < 0 && y2 < 0 && y3 < 0 && y4 < 0) || (y1 >= sh && y2 >= sh && y3 >= sh && y4 >= sh)) {
                    continue;
                }
                z1 <<= W2V_SHIFT;
                if (z1 < phd_znear || z1 > phd_zfar)
                    continue;
                z2 <<= W2V_SHIFT;
                if (z2 < phd_znear || z2 > phd_zfar)
                    continue;
                z3 <<= W2V_SHIFT;
                if (z3 < phd_znear || z3 > phd_zfar)
                    continue;
                z4 <<= W2V_SHIFT;
                if (z4 < phd_znear || z4 > phd_zfar)
                    continue;
                int a, b;
                a = (sptr->life >> 2);
                b = (sptr->life >> 2) - (sptr->life >> 4);
                int nShade1 = a << 10 | a << 5 | a;
                int nShade2 = b << 10 | b << 5 | b;
                if ((((x3 - x2) * (y1 - y2)) - ((x1 - x2) * (y3 - y2))) < 0)
                    HWI_InsertAlphaSprite_Sorted(x1, y1, z1, nShade1, x3, y3, z3, nShade2, x4, y4, z4, nShade2, x2, y2, z2, nShade1, nSprite, 14, 1);
                else
                    HWI_InsertAlphaSprite_Sorted(x1, y1, z1, nShade1, x2, y2, z2, nShade1, x4, y4, z4, nShade2, x3, y3, z3, nShade2, nSprite, 14, 0);
            }
        }
        sptr++;
    }
    result = (long *)&mresult;
    rptr = &ripples[0];
    for (lp = 0; lp < 16; lp++) {
        if (rptr->flags & 1) {
            long size = rptr->size << 2;
            nSprite = objects[296].mesh_index + 9;
            scrpoints = (long *)&mscrpoints;
            mCalcPoint(rptr->x - size, rptr->y, rptr->z - size, result);
            ProjectPCoord(result[0], result[1], result[2], scrpoints, sw >> 1, sh >> 1, phd_persp);
            scrpoints += 3;
            mCalcPoint(rptr->x + size, rptr->y, rptr->z - size, result);
            ProjectPCoord(result[0], result[1], result[2], scrpoints, sw >> 1, sh >> 1, phd_persp);
            scrpoints += 3;
            mCalcPoint(rptr->x - size, rptr->y, rptr->z + size, result);
            ProjectPCoord(result[0], result[1], result[2], scrpoints, sw >> 1, sh >> 1, phd_persp);
            scrpoints += 3;
            mCalcPoint(rptr->x + size, rptr->y, rptr->z + size, result);
            ProjectPCoord(result[0], result[1], result[2], scrpoints, sw >> 1, sh >> 1, phd_persp);
            scrpoints -= 9;
            x1 = *scrpoints++;
            y1 = *scrpoints++;
            z1 = *scrpoints++;
            x2 = *scrpoints++;
            y2 = *scrpoints++;
            z2 = *scrpoints++;
            x3 = *scrpoints++;
            y3 = *scrpoints++;
            z3 = *scrpoints++;
            x4 = *scrpoints++;
            y4 = *scrpoints++;
            z4 = *scrpoints;
            if ((x1 < 0 && x2 < 0 && x3 < 0 && x4 < 0) || (x1 >= sw && x2 >= sw && x3 >= sw && x4 >= sw) || (y1 < 0 && y2 < 0 && y3 < 0 && y4 < 0) || (y1 >= sh && y2 >= sh && y3 >= sh && y4 >= sh)) {
                rptr++;
                continue;
            }
            z1 <<= W2V_SHIFT;
            if (z1 < phd_znear || z1 > phd_zfar) {
                rptr++;
                continue;
            }
            z2 <<= W2V_SHIFT;
            if (z2 < phd_znear || z2 > phd_zfar) {
                rptr++;
                continue;
            }
            z3 <<= W2V_SHIFT;
            if (z3 < phd_znear || z3 > phd_zfar) {
                rptr++;
                continue;
            }
            z4 <<= W2V_SHIFT;
            if (z4 < phd_znear || z4 > phd_zfar) {
                rptr++;
                continue;
            }
            int nShade = 0;
            if (rptr->flags & 0x10) {
                if (rptr->flags & 0x20) {
                    nSprite = objects[296].mesh_index;
                    if (rptr->init)
                        nShade = (rptr->init >> 1) << 10;
                    else
                        nShade = (rptr->life >> 1) << 10;
                } else {
                    if (rptr->init)
                        nShade = (rptr->init >> 2) << 10 | (rptr->init >> 2) << 5 | rptr->init >> 2;
                    else
                        nShade = (rptr->life >> 2) << 10 | (rptr->life >> 2) << 5 | rptr->life >> 2;
                }
            } else {
                if (rptr->init)
                    nShade = (rptr->init >> 1) << 10 | (rptr->init >> 1) << 5 | rptr->init >> 1;
                else
                    nShade = (rptr->life >> 1) << 10 | (rptr->life >> 1) << 5 | rptr->life >> 1;
            }
            if ((((x3 - x2) * (y1 - y2)) - ((x1 - x2) * (y3 - y2))) < 0)
                HWI_InsertAlphaSprite_Sorted(x1, y1, z1, nShade, x3, y3, z3, nShade, x4, y4, z4, nShade, x2, y2, z2, nShade, nSprite, 14, 1);
            else
                HWI_InsertAlphaSprite_Sorted(x1, y1, z1, nShade, x2, y2, z2, nShade, x4, y4, z4, nShade, x3, y3, z3, nShade, nSprite, 14, 0);
        }
        rptr++;
    }
}

extern char LaserShades[];

void UpdateLaserShades(void);
void HWR_SetCurrentTexture(DWORD handle);
void HWR_EnableZBuffer(bool tWrite, bool tCompare);

void S_DrawLaserBeam(GAME_VECTOR *src, GAME_VECTOR *dest, unsigned char r, unsigned char g, unsigned char b)
{
    long x1, x2, y1, y2, z1, z2, r1, g1, b1, r2, g2, b2;
    long wx, wy, wz, dx, dy, dz, lp, segments, alpha;
    long result[3 * 200];
    long rgbs[3 * 200];
    long screencoords[3 * 200];
    int sw = App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].w - 1;
    int sh = App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].h - 1;

    UpdateLaserShades();
    dx = (src->x - dest->x);
    dz = (src->z - dest->z);
    dx = phd_sqrt((dx * dx) + (dz * dz));
    segments = dx >> 9;
    if (segments < 8)
        segments = 8;
    else if (segments > 32)
        segments = 32;
    dx = (dest->x - src->x) / segments;
    dy = (dest->y - src->y) / segments;
    dz = (dest->z - src->z) / segments;
    wx = wy = wz = 0;
    for (lp = 0; lp < segments + 1; lp++) {
        mCalcPoint(src->x + wx, src->y + wy, src->z + wz, result);
        ProjectPCoord(result[0], result[1], result[2], &screencoords[lp * 3], sw >> 1, sh >> 1, phd_persp);
        wx += dx;
        wy += dy;
        wz += dz;
        if (lp == 0 || lp == segments) {
            rgbs[(lp * 3) + 0] = 0;
            rgbs[(lp * 3) + 1] = 0;
            rgbs[(lp * 3) + 2] = 0;
        } else {
            long rnd, tr, tg, tb;
            rnd = LaserShades[lp];
            if (r == 255)
                tr = 32 + rnd;
            else
                tr = rnd >> r;
            if (g == 255)
                tg = 32 + rnd;
            else
                tg = rnd >> g;
            if (b == 255)
                tb = 32 + rnd;
            else
                tb = rnd >> b;
            rgbs[(lp * 3) + 0] = tr;
            rgbs[(lp * 3) + 1] = tg;
            rgbs[(lp * 3) + 2] = tb;
        }
    }
    x1 = screencoords[0];
    y1 = screencoords[1];
    z1 = screencoords[2];
    r1 = rgbs[0];
    g1 = rgbs[1];
    b1 = rgbs[2];
    r1 <<= 1;
    g1 <<= 1;
    b1 <<= 1;
    if (r1 > 255)
        r1 = 255;
    if (g1 > 255)
        g1 = 255;
    if (b1 > 255)
        b1 = 255;
    HWR_SetCurrentTexture(0);
    HWR_EnableZBuffer(true, true);
    for (lp = 0; lp < segments; lp++) {
        x2 = screencoords[((lp + 1) * 3) + 0];
        y2 = screencoords[((lp + 1) * 3) + 1];
        z2 = screencoords[((lp + 1) * 3) + 2];
        r2 = rgbs[((lp + 1) * 3) + 0];
        g2 = rgbs[((lp + 1) * 3) + 1];
        b2 = rgbs[((lp + 1) * 3) + 2];
        r2 <<= 1;
        g2 <<= 1;
        b2 <<= 1;
        if (r2 > 255)
            r2 = 255;
        if (g2 > 255)
            g2 = 255;
        if (b2 > 255)
            b2 = 255;
        if (z1 > 32 && z2 > 32) {
            if (ClipLine(x1, y1, x2, y2, sw, sh)) {
                alpha = GlobalAlpha;
                GlobalAlpha = 0xb0000000;
                HWI_InsertLine_Sorted(x1 - phd_winxmin, y1 - phd_winymin, x2 - phd_winxmin, y2 - phd_winymin, z1 << W2V_SHIFT, r1 << 16 | g1 << 8 | b1, r2 << 16 | g2 << 8 | b2);
                GlobalAlpha = alpha;
            }
        }
        x1 = x2;
        y1 = y2;
        z1 = z2;
        r1 = r2;
        g1 = g2;
        b1 = b2;
    }
}
}
