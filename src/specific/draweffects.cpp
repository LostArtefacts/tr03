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

}
