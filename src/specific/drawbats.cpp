#include "standard.h"
#include "winmain.h"
extern "C" {
#include "../game.h"
}
#include "specific.h"


extern "C" {

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

extern PHDSPRITESTRUCT phdspriteinfo[];
extern sint16 rcossin_tbl[];
extern sint16 phd_winxmin;
extern sint16 phd_winymin;
extern sint16 phd_winxmax;
extern sint16 phd_winymax;
extern float f_persp;
extern float f_oneopersp;

void HWI_InsertGT3_Poly(PHD_VBUF *v1, PHD_VBUF *v2, PHD_VBUF *v3, PHDTEXTURESTRUCT *pTex, unsigned short *uv1, unsigned short *uv2, unsigned short *uv3, int nSortType, unsigned short dbl);

extern float f_centerx;
extern float f_centery;

void phd_PushMatrix(void);
void phd_TranslateAbs(sint32 x, sint32 y, sint32 z);

extern short BatMesh[5][3];
extern unsigned char BatLinks[9];

void phd_RotY(sint16 ry);

void S_DrawBat(void)
{
    PHDSPRITESTRUCT *pSpriteInfo;
    BAT_STRUCT *bptr;
    short *scrxy;
    long *scrz;
    unsigned char *pptr;
    short *TempMesh;
    long lp, vlp;
    unsigned short u1, u2, v1, v2;
    long x1, x2, x3, y1, y2, y3, z1, z2, z3;
    short myscrxy[10];
    short myTempMesh[3];
    long myscrz[10];
    long result[3];
    sint32 *mptr;
    int sw = App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].w;
    int sh = App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].h;

    scrxy = (short *)&myscrxy;
    scrz = (long *)&myscrz;
    TempMesh = (short *)&myTempMesh;
    bptr = &bats[0];
    for (lp = 0; lp < 32; lp++) {
        if (bptr->flags & 1) {
            phd_PushMatrix();
            phd_TranslateAbs(bptr->x, bptr->y, bptr->z);
            phd_RotY(bptr->angle << 4);
            mptr = phd_mxptr;
            for (vlp = 0; vlp < 5; vlp++) {
                TempMesh[0] = BatMesh[vlp][0];
                if (vlp >= 3)
                    TempMesh[1] = BatMesh[vlp][1] - 512 + ((rcossin_tbl[bptr->WingYoff << 7]) >> 4);
                else
                    TempMesh[1] = BatMesh[vlp][1] - 512 + ((rcossin_tbl[((bptr->WingYoff + 32) & 63) << 7]) >> 8);
                TempMesh[2] = BatMesh[vlp][2];
                result[0] = (mptr[0] * TempMesh[0] + mptr[1] * TempMesh[1] + mptr[2] * TempMesh[2] + mptr[3]);
                result[1] = (mptr[4] * TempMesh[0] + mptr[5] * TempMesh[1] + mptr[6] * TempMesh[2] + mptr[7]);
                result[2] = (mptr[8] * TempMesh[0] + mptr[9] * TempMesh[1] + mptr[10] * TempMesh[2] + mptr[11]);
                float zv = f_persp / (float)result[2];
                scrxy[0] = short(float(result[0] * zv + f_centerx));
                scrxy[1] = short(float(result[1] * zv + f_centery));
                scrz[0] = result[2];
                scrxy += 2;
                scrz += 2;
            }
            phd_PopMatrix();
            scrxy = (short *)&myscrxy;
            scrz = (long *)&myscrz;
            pptr = &BatLinks[0];
            for (vlp = 0; vlp < 3; vlp++) {
                x1 = scrxy[(*pptr)];
                y1 = scrxy[(*pptr) + 1];
                z1 = scrz[(*pptr++)];
                x2 = scrxy[(*pptr)];
                y2 = scrxy[(*pptr) + 1];
                z2 = scrz[(*pptr++)];
                x3 = scrxy[(*pptr)];
                y3 = scrxy[(*pptr) + 1];
                z3 = scrz[(*pptr++)];
                if ((z1 >> W2V_SHIFT < 32 || z2 >> W2V_SHIFT < 32 || z3 >> W2V_SHIFT < 32) || (x1 < 0 && x2 < 0 && x3 < 0) || (x1 >= sw && x2 >= sw && x3 >= sw) || (y1 < 0 && y2 < 0 && y3 < 0) || (y1 >= sh && y2 >= sh && y3 >= sh)) {
                    continue;
                }
                PHD_VBUF v[3];
                PHDTEXTURESTRUCT Tex;
                int clipflag;
                clipflag = 0;
                if (x1 < phd_winxmin)
                    clipflag++;
                else if (x1 > phd_winxmax)
                    clipflag += 2;
                if (y1 < phd_winymin)
                    clipflag += 4;
                else if (y1 > phd_winymax)
                    clipflag += 8;
                v[0].xs = float(x1);
                v[0].ys = float(y1);
                v[0].ooz = (f_persp / (float)z1) * f_oneopersp;
                v[0].clip = clipflag;
                v[0].g = 0x7e8c;
                clipflag = 0;
                if (x2 < phd_winxmin)
                    clipflag++;
                else if (x2 > phd_winxmax)
                    clipflag += 2;
                if (y2 < phd_winymin)
                    clipflag += 4;
                else if (y2 > phd_winymax)
                    clipflag += 8;
                v[1].xs = float(x2);
                v[1].ys = float(y2);
                v[1].ooz = (f_persp / (float)z2) * f_oneopersp;
                v[1].clip = clipflag;
                v[1].g = 0x7e8c;
                clipflag = 0;
                if (x3 < phd_winxmin)
                    clipflag++;
                else if (x3 > phd_winxmax)
                    clipflag += 2;
                if (y3 < phd_winymin)
                    clipflag += 4;
                else if (y3 > phd_winymax)
                    clipflag += 8;
                v[2].xs = float(x3);
                v[2].ys = float(y3);
                v[2].ooz = (f_persp / (float)z3) * f_oneopersp;
                v[2].clip = clipflag;
                v[2].g = 0x7e8c;
                pSpriteInfo = phdspriteinfo + objects[296].mesh_index + 12;
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
                v[0].zv = float(z1);
                v[1].zv = float(z2);
                v[2].zv = float(z3);
                if (!vlp) {
                    v[0].u = u1;
                    v[0].v = v1 + App.nUVAdd;
                    v[1].u = u2 - App.nUVAdd;
                    v[1].v = v2;
                    v[2].u = u1;
                    v[2].v = v2;
                } else if (vlp == 1) {
                    v[0].u = u2;
                    v[0].v = v1;
                    v[1].u = u1;
                    v[1].v = v1;
                    v[2].u = u2;
                    v[2].v = v2;
                } else {
                    v[0].u = u1;
                    v[0].v = v1;
                    v[1].u = u2;
                    v[1].v = v1;
                    v[2].u = u2;
                    v[2].v = v2;
                }
                HWI_InsertGT3_Poly(&v[0], &v[1], &v[2], &Tex, &v[0].u, &v[1].u, &v[2].u, 0, 1);
            }
        }
        bptr++;
    }
}

}
