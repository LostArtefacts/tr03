// cflags: /Ob2
#include "../specific/standard.h"

extern "C" {

#include "../3dglodef.h"

extern sint16 phd_winxmin;
extern sint16 phd_winymin;

extern float UVTable[65536];
extern unsigned char ColorTable[256];
extern bool bBlueEffect;

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

typedef struct texturebucket {
    DWORD tpage;
    int cnt;
    D3DTLVERTEX Vertex[288];
} TEXTUREBUCKET;

extern TEXTUREBUCKET Buckets[6];
extern HRESULT (*DrawPrimitive)(D3DPRIMITIVETYPE dptPrimitiveType, D3DVERTEXTYPE dvtVertexType, LPVOID lpvVertices, DWORD dwVertexCount, DWORD dwFlags);

void HWR_SetCurrentTexture(DWORD handle);
void HWR_EnableZBuffer(bool tWrite, bool tCompare);
extern int nDrawnPoints;

void InitUVTable(void)
{
    int n;

    for (n = 0; n < 65536; n++)
        UVTable[n] = float(n) * (1.0f / 65536.0f);
}

void InitBuckets(void)
{
    int n;

    for (n = 0; n < 6; n++) {
        Buckets[n].tpage = 0xffffffff;
        Buckets[n].cnt = 0;
    }
}

int FindBucket(DWORD tpage)
{
    int n;
    int cnt;
    int fullest;

    if (nDrawnPoints > 2700)
        return -1;

    for (n = 0; n < 6; n++) {
        if (Buckets[n].tpage == tpage && Buckets[n].cnt < 256)
            return n;
        if (Buckets[n].cnt > 256) {
            HWR_EnableZBuffer(true, true);
            HWR_SetCurrentTexture(Buckets[n].tpage);
            DrawPrimitive(D3DPT_TRIANGLELIST, D3DVT_TLVERTEX, Buckets[n].Vertex, Buckets[n].cnt, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
            Buckets[n].tpage = tpage;
            Buckets[n].cnt = 0;
            return n;
        }
    }

    fullest = 0;
    cnt = 0;
    for (n = 0; n < 6; n++) {
        if (Buckets[n].tpage == 0xffffffff) {
            Buckets[n].tpage = tpage;
            return n;
        }
        if (Buckets[n].cnt > cnt) {
            cnt = Buckets[n].cnt;
            fullest = n;
        }
    }

    HWR_EnableZBuffer(true, true);
    HWR_SetCurrentTexture(Buckets[fullest].tpage);
    DrawPrimitive(D3DPT_TRIANGLELIST, D3DVT_TLVERTEX, Buckets[fullest].Vertex, Buckets[fullest].cnt, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
    Buckets[fullest].tpage = tpage;
    Buckets[fullest].cnt = 0;
    return fullest;
}

void DrawBuckets(void)
{
    for (int n = 0; n < 6; n++) {
        if (Buckets[n].cnt != 0) {
            HWR_SetCurrentTexture(Buckets[n].tpage);
            DrawPrimitive(D3DPT_TRIANGLELIST, D3DVT_TLVERTEX, &Buckets[n].Vertex[0], Buckets[n].cnt, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
        }
    }
}

#include "../specific/winmain.h"

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

extern int nPolyType;

void HWI_InsertGT3_Poly(PHD_VBUF *v1, PHD_VBUF *v2, PHD_VBUF *v3, PHDTEXTURESTRUCT *pTex, unsigned short *uv1, unsigned short *uv2, unsigned short *uv3, int nSortType, unsigned short dbl);
void SubdivideGT3(PHD_VBUF *v1, PHD_VBUF *v2, PHD_VBUF *v3, PHDTEXTURESTRUCT *pTex, int nSortType, unsigned short dbl, int level);

void HWI_InsertGT3_Sorted(PHD_VBUF *v1, PHD_VBUF *v2, PHD_VBUF *v3, PHDTEXTURESTRUCT *pTex, unsigned short *uv1, unsigned short *uv2, unsigned short *uv3, int nSortType, unsigned short dbl)
{
    long zv;

    if (App.lpZBuffer) {
        HWI_InsertGT3_Poly(v1, v2, v3, pTex, &pTex->u1, &pTex->u2, &pTex->u3, nSortType, dbl);
        return;
    }
    if (nPolyType == 3 || nPolyType == 4) {
        zv = (long)v1->zv;
        if (zv < v2->zv)
            zv = (long)v2->zv;
        if (zv < v3->zv)
            zv = (long)v3->zv;
        if (zv < 0x1f40000)
            SubdivideGT3(v1, v2, v3, pTex, nSortType, dbl, 2);
        else if (zv < 0x36b0000)
            SubdivideGT3(v1, v2, v3, pTex, nSortType, dbl, 1);
        else
            HWI_InsertGT3_Poly(v1, v2, v3, pTex, &pTex->u1, &pTex->u2, &pTex->u3, nSortType, dbl);
        return;
    }
    HWI_InsertGT3_Poly(v1, v2, v3, pTex, &pTex->u1, &pTex->u2, &pTex->u3, nSortType, dbl);
}

int GETR(int rgb)
{
    int r;

    r = ColorTable[((rgb >> 10) & 0x1f) << 3];
    if (bBlueEffect) {
        r *= 128;
        r >>= 8;
    }
    return r;
}

int GETG(int rgb)
{
    int g;

    g = ColorTable[((rgb >> 5) & 0x1f) << 3];
    if (bBlueEffect) {
        g *= 224;
        g >>= 8;
    }
    return g;
}

typedef struct vertex_info {
    float x, y;
    float ooz;
    float u, v, g;
    int vr, vg, vb;
} VERTEX_INFO;

extern VERTEX_INFO v_buffer[];
extern int GlobalAlpha;
extern float f_a;
extern float f_boo;

void PHD_VBUF_To_D3DTLVTX(PHD_VBUF *phdV, D3DTLVERTEX *v, unsigned short *uv)
{
    long r, g, b;

    v->sx = phdV->xs;
    v->sy = phdV->ys;
    v->sz = f_a - f_boo * phdV->ooz;
    v->rhw = phdV->ooz;
    r = GETR(phdV->g);
    g = GETG(phdV->g);
    b = ColorTable[(phdV->g & 0x1f) << 3];
    v->color = GlobalAlpha | (r << 16) | (g << 8) | b;
    v->tu = UVTable[uv[0]];
    v->tv = UVTable[uv[1]];
    v->specular = 0;
}

extern int phd_znear;
extern float f_persp;
extern float f_oneopersp;
extern float f_centerx, f_centery;
extern float phd_leftfloat, phd_topfloat, phd_rightfloat, phd_bottomfloat;

void SubdivideEdge(PHD_VBUF *v0, PHD_VBUF *v1, PHD_VBUF *dest);

void HWI_InsertGT4_Poly(PHD_VBUF *v1, PHD_VBUF *v2, PHD_VBUF *v3, PHD_VBUF *v4, PHDTEXTURESTRUCT *pTex, int nSortType, unsigned short dbl);
void SubdivideGT4(PHD_VBUF *v1, PHD_VBUF *v2, PHD_VBUF *v3, PHD_VBUF *v4, PHDTEXTURESTRUCT *pTex, int nSortType, unsigned short dbl, int level);

void SubdivideGT4(PHD_VBUF *v1, PHD_VBUF *v2, PHD_VBUF *v3, PHD_VBUF *v4, PHDTEXTURESTRUCT *pTex, int nSortType, unsigned short dbl, int level)
{
    PHDTEXTURESTRUCT tex;
    PHD_VBUF v[5];

    if (level) {
        tex.drawtype = pTex->drawtype;
        tex.tpage = pTex->tpage;
        v1->u = pTex->u1;
        v1->v = pTex->v1;
        v2->u = pTex->u2;
        v2->v = pTex->v2;
        v3->u = pTex->u3;
        v3->v = pTex->v3;
        v4->u = pTex->u4;
        v4->v = pTex->v4;
        SubdivideEdge(v1, v2, &v[0]);
        SubdivideEdge(v3, v4, &v[1]);
        SubdivideEdge(v1, v4, &v[2]);
        SubdivideEdge(v2, v3, &v[3]);
        SubdivideEdge(v1, v3, &v[4]);
        tex.u1 = v1->u;
        tex.v1 = v1->v;
        tex.u2 = v[0].u;
        tex.v2 = v[0].v;
        tex.u3 = v[4].u;
        tex.v3 = v[4].v;
        tex.u4 = v[2].u;
        tex.v4 = v[2].v;
        SubdivideGT4(v1, &v[0], &v[4], &v[2], &tex, nSortType, dbl, level - 1);
        tex.u1 = v[0].u;
        tex.v1 = v[0].v;
        tex.u2 = v2->u;
        tex.v2 = v2->v;
        tex.u3 = v[3].u;
        tex.v3 = v[3].v;
        tex.u4 = v[4].u;
        tex.v4 = v[4].v;
        SubdivideGT4(&v[0], v2, &v[3], &v[4], &tex, nSortType, dbl, level - 1);
        tex.u1 = v[4].u;
        tex.v1 = v[4].v;
        tex.u2 = v[3].u;
        tex.v2 = v[3].v;
        tex.u3 = v3->u;
        tex.v3 = v3->v;
        tex.u4 = v[1].u;
        tex.v4 = v[1].v;
        SubdivideGT4(&v[4], &v[3], v3, &v[1], &tex, nSortType, dbl, level - 1);
        tex.u1 = v[2].u;
        tex.v1 = v[2].v;
        tex.u2 = v[4].u;
        tex.v2 = v[4].v;
        tex.u3 = v[1].u;
        tex.v3 = v[1].v;
        tex.u4 = v4->u;
        tex.v4 = v4->v;
        SubdivideGT4(&v[2], &v[4], &v[1], v4, &tex, nSortType, dbl, level - 1);
    } else {
        HWI_InsertGT4_Poly(v1, v2, v3, v4, pTex, nSortType, dbl);
    }
}

void SubdivideGT3(PHD_VBUF *v1, PHD_VBUF *v2, PHD_VBUF *v3, PHDTEXTURESTRUCT *pTex, int nSortType, unsigned short dbl, int level)
{
    PHD_VBUF v12;
    PHD_VBUF v23;
    PHD_VBUF v31;
    PHDTEXTURESTRUCT tex;

    if (level) {
        tex.drawtype = pTex->drawtype;
        tex.tpage = pTex->tpage;
        v1->u = pTex->u1;
        v1->v = pTex->v1;
        v2->u = pTex->u2;
        v2->v = pTex->v2;
        v3->u = pTex->u3;
        v3->v = pTex->v3;
        SubdivideEdge(v1, v2, &v12);
        SubdivideEdge(v2, v3, &v23);
        SubdivideEdge(v3, v1, &v31);
        tex.u1 = v1->u;
        tex.v1 = v1->v;
        tex.u2 = v12.u;
        tex.v2 = v12.v;
        tex.u3 = v31.u;
        tex.v3 = v31.v;
        SubdivideGT3(v1, &v12, &v31, &tex, nSortType, dbl, level - 1);
        tex.u1 = v12.u;
        tex.v1 = v12.v;
        tex.u2 = v2->u;
        tex.v2 = v2->v;
        tex.u3 = v23.u;
        tex.v3 = v23.v;
        SubdivideGT3(&v12, v2, &v23, &tex, nSortType, dbl, level - 1);
        tex.u1 = v31.u;
        tex.v1 = v31.v;
        tex.u2 = v12.u;
        tex.v2 = v12.v;
        tex.u3 = v23.u;
        tex.v3 = v23.v;
        tex.u4 = v3->u;
        tex.v4 = v3->v;
        SubdivideGT4(&v31, &v12, &v23, v3, &tex, nSortType, dbl, level - 1);
    } else {
        HWI_InsertGT3_Poly(v1, v2, v3, pTex, &pTex->u1, &pTex->u2, &pTex->u3, nSortType, dbl);
    }
}

void HWI_InsertGT4_Sorted(PHD_VBUF *v1, PHD_VBUF *v2, PHD_VBUF *v3, PHD_VBUF *v4, PHDTEXTURESTRUCT *pTex, int nSortType, unsigned short dbl)
{
    long zv;

    if (App.lpZBuffer) {
        HWI_InsertGT4_Poly(v1, v2, v3, v4, pTex, nSortType, dbl);
        return;
    }
    if (nPolyType != 3 && nPolyType != 4) {
        HWI_InsertGT4_Poly(v1, v2, v3, v4, pTex, nSortType, dbl);
        return;
    }
    zv = (long)v1->zv;
    if (zv < v2->zv)
        zv = (long)v2->zv;
    if (zv < v3->zv)
        zv = (long)v3->zv;
    if (zv < v4->zv)
        zv = (long)v4->zv;
    if (zv < 0x1f40000)
        SubdivideGT4(v1, v2, v3, v4, pTex, nSortType, dbl, 2);
    else if (zv < 0x36b0000)
        SubdivideGT4(v1, v2, v3, v4, pTex, nSortType, dbl, 1);
    else
        HWI_InsertGT4_Poly(v1, v2, v3, v4, pTex, nSortType, dbl);
}

extern PHD_VBUF vbuf[];
extern PHDTEXTURESTRUCT phdtextinfo[];

short *HWI_InsertObjectG4_Sorted(short *pFaceInfo, long nFaces, int nSortType);

short *HWI_InsertObjectGT4_Sorted(short *pFaceInfo, long nFaces, int nSortType)
{
    PHDTEXTURESTRUCT *pTex;
    long nDrawType;
    unsigned short double_sided;

    while (nFaces) {
        double_sided = (pFaceInfo[4] >> 15) & 1;
        pTex = &phdtextinfo[pFaceInfo[4] & 0x7fff];
        nDrawType = pTex->drawtype;
        if (nDrawType <= 1)
            nDrawType += 9;
        else
            nDrawType = 14;
        if (nDrawType == 10 || nDrawType == 14)
            HWI_InsertGT4_Sorted(&vbuf[pFaceInfo[0]], &vbuf[pFaceInfo[1]], &vbuf[pFaceInfo[2]], &vbuf[pFaceInfo[3]], pTex, nSortType, double_sided);
        else if (vbuf[pFaceInfo[0]].g || vbuf[pFaceInfo[1]].g || vbuf[pFaceInfo[2]].g || vbuf[pFaceInfo[3]].g)
            HWI_InsertGT4_Sorted(&vbuf[pFaceInfo[0]], &vbuf[pFaceInfo[1]], &vbuf[pFaceInfo[2]], &vbuf[pFaceInfo[3]], pTex, nSortType, double_sided);
        else
            HWI_InsertObjectG4_Sorted(pFaceInfo, 1, nSortType);
        pFaceInfo += 5;
        nFaces--;
    }
    return pFaceInfo;
}

short *HWI_InsertObjectG3_Sorted(short *pFaceInfo, long nFaces, int nSortType);

short *HWI_InsertObjectGT3_Sorted(short *pFaceInfo, long nFaces, int nSortType)
{
    PHDTEXTURESTRUCT *pTex;
    long nDrawType;
    unsigned short double_sided;

    while (nFaces) {
        pTex = &phdtextinfo[pFaceInfo[3] & 0x7fff];
        double_sided = (pFaceInfo[3] >> 15) & 1;
        nDrawType = pTex->drawtype;
        if (nDrawType <= 1)
            nDrawType += 9;
        else
            nDrawType = 14;
        if (nDrawType == 10 || nDrawType == 14)
            HWI_InsertGT3_Sorted(&vbuf[pFaceInfo[0]], &vbuf[pFaceInfo[1]], &vbuf[pFaceInfo[2]], pTex, &pTex->u1, &pTex->u2, &pTex->u3, nSortType, double_sided);
        else if (vbuf[pFaceInfo[0]].g || vbuf[pFaceInfo[1]].g || vbuf[pFaceInfo[2]].g)
            HWI_InsertGT3_Sorted(&vbuf[pFaceInfo[0]], &vbuf[pFaceInfo[1]], &vbuf[pFaceInfo[2]], pTex, &pTex->u1, &pTex->u2, &pTex->u3, nSortType, double_sided);
        else
            HWI_InsertObjectG3_Sorted(pFaceInfo, 1, nSortType);
        pFaceInfo += 4;
        nFaces--;
    }
    return pFaceInfo;
}

void PHD_VBUF_To_VERTEX_INFO(PHD_VBUF *phdV, VERTEX_INFO *v)
{
    v->x = phdV->xs;
    v->y = phdV->ys;
    v->ooz = phdV->ooz;
    v->vr = GETR(phdV->g);
    v->vg = GETG(phdV->g);
    v->vb = ColorTable[(phdV->g & 0x1f) << 3];
}

typedef struct point_info {
    float xv, yv, zv;
    float ooz;
    float xs, ys;
    float u, v;
    float g;
    int vr, vg, vb;
} POINT_INFO;

extern float f_znear;
extern float f_oneoznear;
extern float f_perspoznear;
extern float f_centerx, f_centery;

#define MAGICNUM float((1 << 26) * ((1 << 26) * 1.5))
#define FTOL(x) ((fconv = (x) + (MAGICNUM)), *(int *)(&fconv))

int RoomZedClipper(int number, POINT_INFO *input, VERTEX_INFO *output)
{
    POINT_INFO *A, *B;
    VERTEX_INFO *pOut;
    float clipper, fADiff, fBDiff;
    int nVertices;
    double fconv;

    A = input + number - 1;
    B = input;
    pOut = output;
    nVertices = 0;
    for (; number--; A = B++) {
        fADiff = f_znear - A->zv;
        fBDiff = f_znear - B->zv;
        if (((*(long *)&fADiff) | (*(long *)&fBDiff)) >= 0)
            continue;
        if (((*(long *)&fADiff) ^ (*(long *)&fBDiff)) < 0) {
            clipper = fBDiff / (A->zv - B->zv);
            pOut->x = (B->xv + (A->xv - B->xv) * clipper) * f_perspoznear + f_centerx;
            pOut->y = (B->yv + (A->yv - B->yv) * clipper) * f_perspoznear + f_centery;
            pOut->ooz = f_oneoznear;
            pOut->u = (B->u + (A->u - B->u) * clipper) * f_oneoznear;
            pOut->v = (B->v + (A->v - B->v) * clipper) * f_oneoznear;
            pOut->vr = FTOL((B->vr + (A->vr - B->vr) * clipper));
            pOut->vg = FTOL((B->vg + (A->vg - B->vg) * clipper));
            pOut->vb = FTOL((B->vb + (A->vb - B->vb) * clipper));
            ++pOut;
            ++nVertices;
        }
        if ((*(long *)&fBDiff) < 0) {
            pOut->x = B->xs;
            pOut->y = B->ys;
            pOut->ooz = B->ooz;
            pOut->u = B->u * B->ooz;
            pOut->v = B->v * B->ooz;
            pOut->vr = B->vr;
            pOut->vg = B->vg;
            pOut->vb = B->vb;
            ++pOut;
            ++nVertices;
        }
    }
    if (nVertices < 3)
        return 0;
    return nVertices;
}

extern float phd_leftfloat, phd_topfloat, phd_rightfloat, phd_bottomfloat;

static VERTEX_INFO XYGUV_output[8];

int RoomXYGUVClipper(int number, VERTEX_INFO *input)
{
    VERTEX_INFO *A, *B;
    int i, j;
    float clipper;
    double fconv;

    B = &input[number - 1];
    for (i = j = 0; i < number; i++) {
        A = B;
        B = &input[i];
        if (A->x < phd_leftfloat) {
            if (B->x < phd_leftfloat)
                continue;
            clipper = (phd_leftfloat - B->x) / (A->x - B->x);
            XYGUV_output[j].ooz = B->ooz + (A->ooz - B->ooz) * clipper;
            XYGUV_output[j].vr = FTOL(B->vr + (A->vr - B->vr) * clipper);
            XYGUV_output[j].u = B->u + (A->u - B->u) * clipper;
            XYGUV_output[j].vg = FTOL(B->vg + (A->vg - B->vg) * clipper);
            XYGUV_output[j].v = B->v + (A->v - B->v) * clipper;
            XYGUV_output[j].vb = FTOL(B->vb + (A->vb - B->vb) * clipper);
            XYGUV_output[j].x = phd_leftfloat;
            XYGUV_output[j++].y = B->y + (A->y - B->y) * clipper;
        } else if (A->x > phd_rightfloat) {
            if (B->x > phd_rightfloat)
                continue;
            clipper = (phd_rightfloat - B->x) / (A->x - B->x);
            XYGUV_output[j].ooz = B->ooz + (A->ooz - B->ooz) * clipper;
            XYGUV_output[j].vr = FTOL(B->vr + (A->vr - B->vr) * clipper);
            XYGUV_output[j].u = B->u + (A->u - B->u) * clipper;
            XYGUV_output[j].vg = FTOL(B->vg + (A->vg - B->vg) * clipper);
            XYGUV_output[j].v = B->v + (A->v - B->v) * clipper;
            XYGUV_output[j].vb = FTOL(B->vb + (A->vb - B->vb) * clipper);
            XYGUV_output[j].x = phd_rightfloat;
            XYGUV_output[j++].y = B->y + (A->y - B->y) * clipper;
        }
        if (B->x < phd_leftfloat) {
            clipper = (phd_leftfloat - B->x) / (A->x - B->x);
            XYGUV_output[j].ooz = B->ooz + (A->ooz - B->ooz) * clipper;
            XYGUV_output[j].vr = FTOL(B->vr + (A->vr - B->vr) * clipper);
            XYGUV_output[j].u = B->u + (A->u - B->u) * clipper;
            XYGUV_output[j].vg = FTOL(B->vg + (A->vg - B->vg) * clipper);
            XYGUV_output[j].v = B->v + (A->v - B->v) * clipper;
            XYGUV_output[j].vb = FTOL(B->vb + (A->vb - B->vb) * clipper);
            XYGUV_output[j].x = phd_leftfloat;
            XYGUV_output[j++].y = B->y + (A->y - B->y) * clipper;
        } else if (B->x > phd_rightfloat) {
            clipper = (phd_rightfloat - B->x) / (A->x - B->x);
            XYGUV_output[j].ooz = B->ooz + (A->ooz - B->ooz) * clipper;
            XYGUV_output[j].vr = FTOL(B->vr + (A->vr - B->vr) * clipper);
            XYGUV_output[j].u = B->u + (A->u - B->u) * clipper;
            XYGUV_output[j].vg = FTOL(B->vg + (A->vg - B->vg) * clipper);
            XYGUV_output[j].v = B->v + (A->v - B->v) * clipper;
            XYGUV_output[j].vb = FTOL(B->vb + (A->vb - B->vb) * clipper);
            XYGUV_output[j].x = phd_rightfloat;
            XYGUV_output[j++].y = B->y + (A->y - B->y) * clipper;
        } else {
            XYGUV_output[j].x = B->x;
            XYGUV_output[j].y = B->y;
            XYGUV_output[j].ooz = B->ooz;
            XYGUV_output[j].vr = B->vr;
            XYGUV_output[j].vg = B->vg;
            XYGUV_output[j].vb = B->vb;
            XYGUV_output[j].u = B->u;
            XYGUV_output[j++].v = B->v;
        }
    }
    if (j < 3)
        return 0;
    number = j;
    B = &XYGUV_output[number - 1];
    for (i = j = 0; i < number; i++) {
        A = B;
        B = &XYGUV_output[i];
        if (A->y < phd_topfloat) {
            if (B->y < phd_topfloat)
                continue;
            clipper = (phd_topfloat - B->y) / (A->y - B->y);
            input[j].ooz = B->ooz + (A->ooz - B->ooz) * clipper;
            input[j].vr = FTOL(B->vr + (A->vr - B->vr) * clipper);
            input[j].u = B->u + (A->u - B->u) * clipper;
            input[j].vg = FTOL(B->vg + (A->vg - B->vg) * clipper);
            input[j].v = B->v + (A->v - B->v) * clipper;
            input[j].vb = FTOL(B->vb + (A->vb - B->vb) * clipper);
            input[j].x = B->x + (A->x - B->x) * clipper;
            input[j++].y = phd_topfloat;
        } else if (A->y > phd_bottomfloat) {
            if (B->y > phd_bottomfloat)
                continue;
            clipper = (phd_bottomfloat - B->y) / (A->y - B->y);
            input[j].ooz = B->ooz + (A->ooz - B->ooz) * clipper;
            input[j].vr = FTOL(B->vr + (A->vr - B->vr) * clipper);
            input[j].u = B->u + (A->u - B->u) * clipper;
            input[j].vg = FTOL(B->vg + (A->vg - B->vg) * clipper);
            input[j].v = B->v + (A->v - B->v) * clipper;
            input[j].vb = FTOL(B->vb + (A->vb - B->vb) * clipper);
            input[j].x = B->x + (A->x - B->x) * clipper;
            input[j++].y = phd_bottomfloat;
        }
        if (B->y < phd_topfloat) {
            clipper = (phd_topfloat - B->y) / (A->y - B->y);
            input[j].ooz = B->ooz + (A->ooz - B->ooz) * clipper;
            input[j].vr = FTOL(B->vr + (A->vr - B->vr) * clipper);
            input[j].u = B->u + (A->u - B->u) * clipper;
            input[j].vg = FTOL(B->vg + (A->vg - B->vg) * clipper);
            input[j].v = B->v + (A->v - B->v) * clipper;
            input[j].vb = FTOL(B->vb + (A->vb - B->vb) * clipper);
            input[j].x = B->x + (A->x - B->x) * clipper;
            input[j++].y = phd_topfloat;
        } else if (B->y > phd_bottomfloat) {
            clipper = (phd_bottomfloat - B->y) / (A->y - B->y);
            input[j].ooz = B->ooz + (A->ooz - B->ooz) * clipper;
            input[j].vr = FTOL(B->vr + (A->vr - B->vr) * clipper);
            input[j].u = B->u + (A->u - B->u) * clipper;
            input[j].vg = FTOL(B->vg + (A->vg - B->vg) * clipper);
            input[j].v = B->v + (A->v - B->v) * clipper;
            input[j].vb = FTOL(B->vb + (A->vb - B->vb) * clipper);
            input[j].x = B->x + (A->x - B->x) * clipper;
            input[j++].y = phd_bottomfloat;
        } else {
            input[j].x = B->x;
            input[j].y = B->y;
            input[j].ooz = B->ooz;
            input[j].vr = B->vr;
            input[j].vg = B->vg;
            input[j].vb = B->vb;
            input[j].u = B->u;
            input[j++].v = B->v;
        }
    }
    if (j < 3)
        return 0;
    return j;
}

extern D3DTLVERTEX *VertexBuffer;
extern D3DTLVERTEX *CurrentTLVertex;
extern bool bAlphaTesting;
extern sint32 *sort3dptrbf;
extern sint32 *sort3dptrfb;
extern sint16 *info3dptrbf;
extern sint16 *info3dptrfb;
extern int surfacenumbf;
extern int surfacenumfb;
extern int GlobalAlpha;
extern float one;
extern float f_a;
extern float f_boo;

void HWI_InsertLine_Sorted(long x1, long y1, long x2, long y2, long z, long c0, long c1)
{
    D3DTLVERTEX *v;
    sint32 *sort;
    sint16 *info;
    float zv;

    if (CurrentTLVertex - VertexBuffer > 9184)
        return;
    if (!App.lpZBuffer || !bAlphaTesting) {
        sort = sort3dptrbf;
        info = info3dptrbf;
        sort[2] = nPolyType;
        surfacenumbf++;
        sort3dptrbf += 3;
        info3dptrbf += 5;
    } else {
        sort = sort3dptrfb;
        info = info3dptrfb;
        sort[2] = nPolyType;
        surfacenumfb++;
        sort3dptrfb += 3;
        info3dptrfb += 5;
    }
    sort[0] = (sint32)info;
    sort[1] = z;
    info[0] = 12;
    info[1] = 0;
    info[2] = 2;
    v = CurrentTLVertex;
    *((D3DTLVERTEX **)(info + 3)) = v;
    v[0].sx = float(phd_winxmin + x1);
    v[0].sy = float(phd_winymin + y1);
    v[1].sx = float(phd_winxmin + x2);
    v[1].sy = float(phd_winymin + y2);
    v[0].color = c0 | GlobalAlpha;
    v[1].color = c1 | GlobalAlpha;
    zv = one / (float)z;
    v[1].rhw = zv;
    v[0].rhw = zv;
    zv = f_a - zv * f_boo;
    v[0].specular = 0;
    v[1].specular = 0;
    v[1].sz = zv;
    v[0].sz = zv;
    CurrentTLVertex = v + 2;
}

void HWI_InsertTransQuad_Sorted(long x, long y, long w, long h, long z)
{
    D3DTLVERTEX *v;
    sint32 *sort;
    sint16 *info;
    float zv;

    if (CurrentTLVertex - VertexBuffer > 9184)
        return;
    if (!App.lpZBuffer || !bAlphaTesting) {
        sort = sort3dptrbf;
        info = info3dptrbf;
        sort[2] = nPolyType;
        surfacenumbf++;
        sort3dptrbf += 3;
        info3dptrbf += 5;
    } else {
        sort = sort3dptrfb;
        info = info3dptrfb;
        sort[2] = nPolyType;
        surfacenumfb++;
        sort3dptrfb += 3;
        info3dptrfb += 5;
    }
    sort[0] = (sint32)info;
    sort[1] = z;
    info[0] = 13;
    info[1] = 0;
    info[2] = 4;
    v = CurrentTLVertex;
    *((D3DTLVERTEX **)(info + 3)) = v;
    v[0].sx = v[3].sx = (float)x;
    v[0].sy = v[1].sy = (float)y;
    v[1].sx = v[2].sx = float(x + w);
    v[3].sy = v[2].sy = float(y + h);
    v[0].color = 0x50003fff;
    v[1].color = 0x50003fff;
    v[2].color = 0x50003f1f;
    v[3].color = 0x50003f1f;
    zv = one / (float)z;
    v[3].rhw = zv;
    v[2].rhw = zv;
    v[1].rhw = zv;
    v[0].rhw = zv;
    zv = f_a - zv * f_boo;
    v[0].specular = 0;
    v[1].specular = 0;
    v[2].specular = 0;
    v[3].specular = 0;
    v[3].sz = zv;
    v[2].sz = zv;
    v[1].sz = zv;
    v[0].sz = zv;
    CurrentTLVertex = v + 4;
}


void HWI_InsertPoly_GouraudRGB(int nPoints, float zdepth, int nDrawType)
{
    D3DTLVERTEX *pTLV;
    unsigned long dwAnd;
    VERTEX_INFO *pVI;
    int b;
    int n, v;
    sint32 *sort;
    sint16 *info;
    double fconv;

    dwAnd = (nDrawType == 13 || nDrawType == 16) ? 0x80ffffff : 0xffffffff;
    pVI = v_buffer;
    if (App.lpZBuffer && nDrawType != 13 && nDrawType != 16) {
        b = FindBucket(0);
        if (b == -1)
            return;
        for (n = 0; n < 3; n++) {
            v = Buckets[b].cnt;
            Buckets[b].Vertex[v].sx = pVI->x;
            Buckets[b].Vertex[v].sy = pVI->y;
            Buckets[b].Vertex[v].sz = f_a - f_boo * pVI->ooz;
            Buckets[b].Vertex[v].rhw = pVI->ooz;
            Buckets[b].Vertex[v].color = (0xff000000 | (pVI->vr << 16) | (pVI->vg << 8) | pVI->vb) & dwAnd;
            pVI++;
            Buckets[b].cnt++;
        }
        nDrawnPoints++;
        nPoints -= 3;
        pVI--;
        for (n = nPoints; n > 0; n--) {
            nDrawnPoints++;
            v = Buckets[b].cnt;
            Buckets[b].Vertex[v + 0].sx = v_buffer->x;
            Buckets[b].Vertex[v + 0].sy = v_buffer->y;
            Buckets[b].Vertex[v + 0].sz = f_a - f_boo * v_buffer->ooz;
            Buckets[b].Vertex[v + 0].rhw = v_buffer->ooz;
            Buckets[b].Vertex[v + 0].color = (0xff000000 | (v_buffer->vr << 16) | (v_buffer->vg << 8) | v_buffer->vb) & dwAnd;
            Buckets[b].Vertex[v + 1].sx = pVI->x;
            Buckets[b].Vertex[v + 1].sy = pVI->y;
            Buckets[b].Vertex[v + 1].sz = f_a - f_boo * pVI->ooz;
            Buckets[b].Vertex[v + 1].rhw = pVI->ooz;
            Buckets[b].Vertex[v + 1].color = (0xff000000 | (pVI->vr << 16) | (pVI->vg << 8) | pVI->vb) & dwAnd;
            pVI++;
            Buckets[b].Vertex[v + 2].sx = pVI->x;
            Buckets[b].Vertex[v + 2].sy = pVI->y;
            Buckets[b].Vertex[v + 2].sz = f_a - f_boo * pVI->ooz;
            Buckets[b].Vertex[v + 2].rhw = pVI->ooz;
            Buckets[b].Vertex[v + 2].color = (0xff000000 | (pVI->vr << 16) | (pVI->vg << 8) | pVI->vb) & dwAnd;
            Buckets[b].cnt += 3;
        }
    } else {
        if (CurrentTLVertex - VertexBuffer > 9184)
            return;
        if (!App.lpZBuffer || !bAlphaTesting) {
            sort = sort3dptrbf;
            info = info3dptrbf;
            sort[2] = nPolyType;
            surfacenumbf++;
            sort3dptrbf += 3;
            info3dptrbf += 5;
        } else if (nDrawType == 14 || nDrawType == 13 || nDrawType == 12 || nDrawType == 16) {
            sort = sort3dptrfb;
            info = info3dptrfb;
            sort[2] = nPolyType;
            surfacenumfb++;
            sort3dptrfb += 3;
            info3dptrfb += 5;
        } else {
            sort = sort3dptrbf;
            info = info3dptrbf;
            sort[2] = nPolyType;
            surfacenumbf++;
            sort3dptrbf += 3;
            info3dptrbf += 5;
        }
        sort[0] = (sint32)info;
        sort[1] = FTOL(zdepth);
        info[0] = (sint16)nDrawType;
        info[1] = 0;
        info[2] = (sint16)nPoints;
        pTLV = CurrentTLVertex;
        *((D3DTLVERTEX **)(info + 3)) = pTLV;
        for (n = nPoints; n--;) {
            pTLV->sx = pVI->x;
            pTLV->sy = pVI->y;
            pTLV->sz = f_a - f_boo * pVI->ooz;
            pTLV->rhw = pVI->ooz;
            pTLV->color = (0xff000000 | (pVI->vr << 16) | (pVI->vg << 8) | pVI->vb) & dwAnd;
            pVI++;
            pTLV++;
        }
        CurrentTLVertex = pTLV;
    }
}

}
