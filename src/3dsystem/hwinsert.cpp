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

void HWI_InsertGT3_Poly(PHD_VBUF *v0, PHD_VBUF *v1, PHD_VBUF *v2, PHDTEXTURESTRUCT *pTex, unsigned short *uv0, unsigned short *uv1, unsigned short *uv2, int nSortType, unsigned short dbl);
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

inline unsigned long VertexRGB(int rgb)
{
    int r, g, b;

    r = GETR(rgb);
    g = GETG(rgb);
    b = ColorTable[(rgb & 0x1f) << 3];
    return GlobalAlpha | (r << 16) | (g << 8) | b;
}

inline void PHD_VBUF_To_D3DTLVTX(PHD_VBUF *phdV, D3DTLVERTEX *v, unsigned short *uv)
{
    v->sx = phdV->xs;
    v->sy = phdV->ys;
    v->sz = f_a - f_boo * phdV->ooz;
    v->rhw = phdV->ooz;
    v->color = VertexRGB(phdV->g);
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


void HWI_InsertGT4_Poly(PHD_VBUF *v1, PHD_VBUF *v2, PHD_VBUF *v3, PHD_VBUF *v4, PHDTEXTURESTRUCT *pTex, int nSortType, unsigned short dbl);

void HWI_InsertPoly_Gouraud(int nPoints, float zdepth, int nR, int nG, int nB, int nDrawType)
{
    D3DTLVERTEX *pTLV;
    unsigned long dwAnd;
    VERTEX_INFO *pVI;
    int b;
    int n, v;
    sint32 *sort;
    sint16 *info;
    double fconv;

    dwAnd = nDrawType == 13 ? 0x80ffffff : 0xffffffff;
    pVI = v_buffer;
    if (App.lpZBuffer && nDrawType != 13) {
        b = FindBucket(0);
        if (b == -1)
            return;
        for (n = 0; n < 3; n++) {
            v = Buckets[b].cnt;
            pVI->vr = (pVI->vr * nR) >> 8;
            pVI->vg = (pVI->vg * nG) >> 8;
            pVI->vb = (pVI->vb * nB) >> 8;
            Buckets[b].Vertex[v].sx = pVI->x;
            Buckets[b].Vertex[v].sy = pVI->y;
            Buckets[b].Vertex[v].sz = f_a - f_boo * pVI->ooz;
            Buckets[b].Vertex[v].rhw = pVI->ooz;
            Buckets[b].Vertex[v].color = (0xff000000 | (pVI->vr << 16) | (pVI->vg << 8) | pVI->vb) & dwAnd;
            Buckets[b].Vertex[v].specular = 0;
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
            Buckets[b].Vertex[v + 0].specular = 0;
            Buckets[b].Vertex[v + 1].sx = pVI->x;
            Buckets[b].Vertex[v + 1].sy = pVI->y;
            Buckets[b].Vertex[v + 1].sz = f_a - f_boo * pVI->ooz;
            Buckets[b].Vertex[v + 1].rhw = pVI->ooz;
            Buckets[b].Vertex[v + 1].color = (0xff000000 | (pVI->vr << 16) | (pVI->vg << 8) | pVI->vb) & dwAnd;
            Buckets[b].Vertex[v + 1].specular = 0;
            pVI++;
            pVI->vr = (pVI->vr * nR) >> 8;
            pVI->vg = (pVI->vg * nG) >> 8;
            pVI->vb = (pVI->vb * nB) >> 8;
            Buckets[b].Vertex[v + 2].sx = pVI->x;
            Buckets[b].Vertex[v + 2].sy = pVI->y;
            Buckets[b].Vertex[v + 2].sz = f_a - f_boo * pVI->ooz;
            Buckets[b].Vertex[v + 2].rhw = pVI->ooz;
            Buckets[b].Vertex[v + 2].color = (0xff000000 | (pVI->vr << 16) | (pVI->vg << 8) | pVI->vb) & dwAnd;
            Buckets[b].Vertex[v + 2].specular = 0;
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
            pVI->vr = (pVI->vr * nR) >> 8;
            pVI->vg = (pVI->vg * nG) >> 8;
            pVI->vb = (pVI->vb * nB) >> 8;
            pTLV->color = (0xff000000 | (pVI->vr << 16) | (pVI->vg << 8) | pVI->vb) & dwAnd;
            pTLV->specular = 0;
            pVI++;
            pTLV++;
        }
        CurrentTLVertex = pTLV;
    }
}

extern unsigned char game_palette[];
extern int phd_winwidth;
extern int phd_winheight;

inline DWORD ColorFromRGB(int nR, int nG, int nB)
{
    if (bBlueEffect) {
        nR *= 128;
        nR >>= 8;
        nG *= 224;
        nG >>= 8;
    }
    return ((nR << 16) | (nG << 8) | nB) | GlobalAlpha;
}

void HWI_InsertFlatRect_Sorted(int nX1, int nY1, int nX2, int nY2, int nZDepth, int nColour)
{
    sint32 *sort;
    sint16 *info;
    unsigned char *pPE = game_palette + nColour * 3;
    D3DTLVERTEX *pTLV;

    if (nX2 <= nX1 || nY2 <= nY1)
        return;
    if (nX1 < phd_winxmin)
        nX1 = phd_winxmin;
    if (nY1 < phd_winymin)
        nY1 = phd_winymin;
    if (nX2 > phd_winxmin + phd_winwidth)
        nX2 = phd_winxmin + phd_winwidth;
    if (nY2 > phd_winymin + phd_winheight)
        nX2 = phd_winymin + phd_winheight;
    if (App.lpZBuffer == 0) {
        if (CurrentTLVertex - VertexBuffer > 9184)
            return;
        sort = sort3dptrbf;
        info = info3dptrbf;
        sort[2] = nPolyType;
        surfacenumbf++;
        sort3dptrbf += 3;
        info3dptrbf += 5;
        sort[0] = (sint32)info;
        sort[1] = nZDepth;
        info[0] = 11;
        info[1] = 0;
        info[2] = 4;
        pTLV = CurrentTLVertex;
        *((D3DTLVERTEX **)(info + 3)) = pTLV;
        pTLV[0].sx = pTLV[3].sx = float(nX1);
        pTLV[0].sy = pTLV[1].sy = float(nY1);
        pTLV[1].sx = pTLV[2].sx = float(nX2);
        pTLV[3].sy = pTLV[2].sy = float(nY2);
        pTLV[0].color = pTLV[1].color = pTLV[2].color = pTLV[3].color = ColorFromRGB(pPE[0], pPE[1], pPE[2]);
        float fooz = one / float(nZDepth);
        float z = f_a - fooz * f_boo;
        pTLV[0].sz = z;
        pTLV[1].sz = z;
        pTLV[2].sz = z;
        pTLV[3].sz = z;
        pTLV[0].rhw = fooz;
        pTLV[1].rhw = fooz;
        pTLV[2].rhw = fooz;
        pTLV[3].rhw = fooz;
        pTLV[0].specular = 0;
        pTLV[1].specular = 0;
        pTLV[2].specular = 0;
        pTLV[3].specular = 0;
        CurrentTLVertex = pTLV + 4;
    } else {
        int b;
        int v;
        b = FindBucket(0);
        if (b == -1)
            return;
        v = Buckets[b].cnt;
        float fooz = one / float(nZDepth);
        float z = f_a - fooz * f_boo;
        Buckets[b].Vertex[v + 0].specular = 0;
        Buckets[b].Vertex[v + 1].specular = 0;
        Buckets[b].Vertex[v + 2].specular = 0;
        Buckets[b].Vertex[v + 3].specular = 0;
        Buckets[b].Vertex[v + 4].specular = 0;
        Buckets[b].Vertex[v + 5].specular = 0;
        Buckets[b].Vertex[v + 6].specular = 0;
        Buckets[b].Vertex[v + 0].sx = float(nX1);
        Buckets[b].Vertex[v + 0].sy = float(nY1);
        Buckets[b].Vertex[v + 0].sz = z;
        Buckets[b].Vertex[v + 0].rhw = fooz;
        Buckets[b].Vertex[v + 0].color = ColorFromRGB(pPE[0], pPE[1], pPE[2]);
        Buckets[b].Vertex[v + 1].sx = float(nX2);
        Buckets[b].Vertex[v + 1].sy = float(nY1);
        Buckets[b].Vertex[v + 1].sz = z;
        Buckets[b].Vertex[v + 1].rhw = fooz;
        Buckets[b].Vertex[v + 1].color = ColorFromRGB(pPE[0], pPE[1], pPE[2]);
        Buckets[b].Vertex[v + 2].sx = float(nX2);
        Buckets[b].Vertex[v + 2].sy = float(nY2);
        Buckets[b].Vertex[v + 2].sz = z;
        Buckets[b].Vertex[v + 2].rhw = fooz;
        Buckets[b].Vertex[v + 2].color = ColorFromRGB(pPE[0], pPE[1], pPE[2]);
        Buckets[b].Vertex[v + 3].sx = float(nX1);
        Buckets[b].Vertex[v + 3].sy = float(nY1);
        Buckets[b].Vertex[v + 3].sz = z;
        Buckets[b].Vertex[v + 3].rhw = fooz;
        Buckets[b].Vertex[v + 3].color = ColorFromRGB(pPE[0], pPE[1], pPE[2]);
        Buckets[b].Vertex[v + 4].sx = float(nX2);
        Buckets[b].Vertex[v + 4].sy = float(nY2);
        Buckets[b].Vertex[v + 4].sz = z;
        Buckets[b].Vertex[v + 4].rhw = fooz;
        Buckets[b].Vertex[v + 4].color = ColorFromRGB(pPE[0], pPE[1], pPE[2]);
        Buckets[b].Vertex[v + 5].sx = float(nX1);
        Buckets[b].Vertex[v + 5].sy = float(nY2);
        Buckets[b].Vertex[v + 5].sz = z;
        Buckets[b].Vertex[v + 5].rhw = fooz;
        Buckets[b].Vertex[v + 5].color = ColorFromRGB(pPE[0], pPE[1], pPE[2]);
        Buckets[b].cnt += 6;
        nDrawnPoints += 2;
    }
}

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
extern sint16 phd_winxmax;
extern sint16 phd_winymax;
extern int phd_zfar;

void HWI_InsertClippedPoly_Textured(int nPoints, float zdepth, int nDrawType, int nTPage);
int XYGClipper(int n, VERTEX_INFO *in);

void HWI_InsertAlphaSprite_Sorted(int nX1, int nY1, int nZ1, int nShade1, int nX2, int nY2, int nZ2, int nShade2, int nX3, int nY3, int nZ3, int nShade3, int nX4, int nY4, int nZ4, int nShade4, int nSprite, int type, int dbl)
{
    bool tOldBlueEffect = bBlueEffect;
    bBlueEffect = false;
    PHDSPRITESTRUCT *pSprite;
    float u1, v1, u2, v2;
    VERTEX_INFO swap;

    if (nSprite != -1) {
        pSprite = phdspriteinfo + nSprite;
        u1 = float((pSprite->offset << 8) & 0xff00);
        v1 = float(pSprite->offset & 0xff00);
        u2 = u1 + pSprite->width;
        v2 = v1 + pSprite->height;
        u1 += App.nUVAdd;
        u2 -= App.nUVAdd;
        v1 += App.nUVAdd;
        v2 -= App.nUVAdd;
    }
    v_buffer[0].x = float(nX1);
    v_buffer[0].y = float(nY1);
    v_buffer[0].ooz = one / float(nZ1);
    v_buffer[0].vr = GETR(nShade1);
    v_buffer[0].vg = GETG(nShade1);
    v_buffer[0].vb = ColorTable[(nShade1 & 0x1f) << 3];
    v_buffer[1].x = float(nX2);
    v_buffer[1].y = float(nY2);
    v_buffer[1].ooz = one / float(nZ2);
    v_buffer[1].vr = GETR(nShade2);
    v_buffer[1].vg = GETG(nShade2);
    v_buffer[1].vb = ColorTable[(nShade2 & 0x1f) << 3];
    v_buffer[2].x = float(nX3);
    v_buffer[2].y = float(nY3);
    v_buffer[2].ooz = one / float(nZ3);
    v_buffer[2].vr = GETR(nShade3);
    v_buffer[2].vg = GETG(nShade3);
    v_buffer[2].vb = ColorTable[(nShade3 & 0x1f) << 3];
    v_buffer[3].x = float(nX4);
    v_buffer[3].y = float(nY4);
    v_buffer[3].ooz = one / float(nZ4);
    v_buffer[3].vr = GETR(nShade4);
    v_buffer[3].vg = GETG(nShade4);
    v_buffer[3].vb = ColorTable[(nShade4 & 0x1f) << 3];
    if (nSprite != -1) {
        if (dbl == 0) {
            v_buffer[0].u = u1 * v_buffer[0].ooz;
            v_buffer[0].v = v1 * v_buffer[0].ooz;
            v_buffer[1].u = u2 * v_buffer[1].ooz;
            v_buffer[1].v = v1 * v_buffer[1].ooz;
            v_buffer[2].u = u2 * v_buffer[2].ooz;
            v_buffer[2].v = v2 * v_buffer[2].ooz;
            v_buffer[3].u = u1 * v_buffer[3].ooz;
            v_buffer[3].v = v2 * v_buffer[3].ooz;
        } else {
            v_buffer[0].u = u1 * v_buffer[0].ooz;
            v_buffer[0].v = v1 * v_buffer[0].ooz;
            v_buffer[3].u = u2 * v_buffer[3].ooz;
            v_buffer[3].v = v1 * v_buffer[3].ooz;
            v_buffer[2].u = u2 * v_buffer[2].ooz;
            v_buffer[2].v = v2 * v_buffer[2].ooz;
            v_buffer[1].u = u1 * v_buffer[1].ooz;
            v_buffer[1].v = v2 * v_buffer[1].ooz;
        }
    }
    if (nSprite == -1 && dbl && (long)((v_buffer[0].y - v_buffer[1].y) * (v_buffer[2].x - v_buffer[1].x) - (v_buffer[0].x - v_buffer[1].x) * (v_buffer[2].y - v_buffer[1].y)) < 0) {
        swap.x = v_buffer[2].x;
        swap.y = v_buffer[2].y;
        swap.ooz = v_buffer[2].ooz;
        swap.u = v_buffer[2].u;
        swap.v = v_buffer[2].v;
        swap.vr = v_buffer[2].vr;
        swap.vg = v_buffer[2].vg;
        swap.vb = v_buffer[2].vb;
        v_buffer[2].x = v_buffer[0].x;
        v_buffer[2].y = v_buffer[0].y;
        v_buffer[2].ooz = v_buffer[0].ooz;
        v_buffer[2].u = v_buffer[0].u;
        v_buffer[2].v = v_buffer[0].v;
        v_buffer[2].vr = v_buffer[0].vr;
        v_buffer[2].vg = v_buffer[0].vg;
        v_buffer[2].vb = v_buffer[0].vb;
        v_buffer[0].x = swap.x;
        v_buffer[0].y = swap.y;
        v_buffer[0].ooz = swap.ooz;
        v_buffer[0].u = swap.u;
        v_buffer[0].v = swap.v;
        v_buffer[0].vr = swap.vr;
        v_buffer[0].vg = swap.vg;
        v_buffer[0].vb = swap.vb;
    }
    int nPoints = 4;
    phd_leftfloat = (float)phd_winxmin;
    phd_rightfloat = (float)(phd_winxmin + phd_winwidth);
    phd_topfloat = (float)phd_winymin;
    phd_bottomfloat = (float)(phd_winymin + phd_winheight);
    if (nSprite == -1)
        nPoints = XYGClipper(nPoints, v_buffer);
    else
        nPoints = RoomXYGUVClipper(nPoints, v_buffer);
    if (nPoints) {
        float fZDepth = float((nZ1 + nZ2 + nZ3 + nZ4) >> 2);
        if (nSprite == -1)
            HWI_InsertPoly_GouraudRGB(nPoints, fZDepth, type);
        else
            HWI_InsertClippedPoly_Textured(nPoints, fZDepth, type, pSprite->tpage);
    }
    bBlueEffect = tOldBlueEffect;
}

void HWI_InsertGT3_Sorted(PHD_VBUF *v1, PHD_VBUF *v2, PHD_VBUF *v3, PHDTEXTURESTRUCT *pTex, unsigned short *uv1, unsigned short *uv2, unsigned short *uv3, int nSortType, unsigned short dbl);

#pragma auto_inline(off)
int visible_zclip(PHD_VBUF *vn1, PHD_VBUF *vn2, PHD_VBUF *vn3)
{
    float a, b, c;

    a = (vn1->yv * vn3->zv) - (vn3->yv * vn1->zv);
    b = (vn1->zv * vn3->xv) - (vn3->zv * vn1->xv);
    c = (vn1->xv * vn3->yv) - (vn3->xv * vn1->yv);
    if ((a * vn2->xv) + (b * vn2->yv) + (c * vn2->zv) < 0.0f)
        return 1;
    return 0;
}
#pragma auto_inline(on)


extern int outside;
extern float outsideBackgroundTop;
extern float f_zfar;
extern int phd_winwidth;
extern int phd_winheight;
extern unsigned char G_GouraudPalette[];

int visible_zclip(PHD_VBUF *v1, PHD_VBUF *v2, PHD_VBUF *v3);
int XYGClipper(int n, VERTEX_INFO *in);

inline void PHD_VBUF_To_POINT_INFO(PHD_VBUF *pV, POINT_INFO *p)
{
    p->xv = pV->xv;
    p->yv = pV->yv;
    p->zv = pV->zv;
    p->ooz = pV->ooz;
    p->xs = pV->xs;
    p->ys = pV->ys;
    int col = pV->g;
    p->vr = GETR(col);
    p->vg = GETG(col);
    p->vb = ColorTable[(col & 0x1f) << 3];
}

void HWI_InsertPoly_Gouraud(int nPoints, float zdepth, int nR, int nG, int nB, int nDrawType);

void HWI_InsertTransQuad_Sorted(long x, long y, long w, long h, long z);

short *HWI_InsertObjectG3_Sorted(short *pFaceInfo, long nFaces, int nSortType)
{
    long nPoints;
    unsigned char *pC;
    float zdepth;

    for (; nFaces; nFaces--, pFaceInfo += 4) {
        PHD_VBUF *v2;
        PHD_VBUF *v0;
        PHD_VBUF *v1;

        v0 = &vbuf[pFaceInfo[0]];
        v1 = &vbuf[pFaceInfo[1]];
        v2 = &vbuf[pFaceInfo[2]];
        if (outside && nPolyType != 1 && v0->zv == f_zfar && v1->zv == f_zfar && v2->zv == f_zfar && v0->ys > outsideBackgroundTop && v1->ys > outsideBackgroundTop && v2->ys > outsideBackgroundTop)
            continue;
        if (v0->clip & v1->clip & v2->clip)
            continue;
        if ((v0->clip | v1->clip | v2->clip) < 0) {
            if (!visible_zclip(v0, v1, v2))
                continue;
            POINT_INFO points[3];
            PHD_VBUF_To_POINT_INFO(v0, &points[0]);
            PHD_VBUF_To_POINT_INFO(v1, &points[1]);
            PHD_VBUF_To_POINT_INFO(v2, &points[2]);
            nPoints = RoomZedClipper(3, points, v_buffer);
            if (nPoints) {
                phd_leftfloat = (float)phd_winxmin;
                phd_topfloat = (float)phd_winymin;
                phd_rightfloat = float(phd_winxmin + phd_winwidth);
                phd_bottomfloat = float(phd_winymin + phd_winheight);
                nPoints = XYGClipper(nPoints, v_buffer);
            }
        } else {
            if ((long)((v2->xs - v1->xs) * (v0->ys - v1->ys) - (v2->ys - v1->ys) * (v0->xs - v1->xs)) < 0)
                continue;
            PHD_VBUF_To_VERTEX_INFO(v0, &v_buffer[0]);
            PHD_VBUF_To_VERTEX_INFO(v1, &v_buffer[1]);
            PHD_VBUF_To_VERTEX_INFO(v2, &v_buffer[2]);
            nPoints = 3;
            if (v0->clip | v1->clip | v2->clip) {
                phd_leftfloat = (float)phd_winxmin;
                phd_topfloat = (float)phd_winymin;
                phd_rightfloat = float(phd_winxmin + phd_winwidth);
                phd_bottomfloat = float(phd_winymin + phd_winheight);
                nPoints = XYGClipper(nPoints, v_buffer);
            }
        }
        if (nPoints) {
            pC = &G_GouraudPalette[(pFaceInfo[3] >> 6) & 0x3fc];
            switch (nSortType) {
            case 1:
                zdepth = v0->zv;
                if (v1->zv > zdepth)
                    zdepth = v1->zv;
                if (v2->zv > zdepth)
                    zdepth = v2->zv;
                break;
            case 0:
                zdepth = (v0->zv + v1->zv + v2->zv) * 0.33333334f;
                break;
            default:
                zdepth = 1000000000.0f;
            }
            HWI_InsertPoly_Gouraud(nPoints, zdepth, pC[0], pC[1], pC[2], 11);
        }
    }
    return pFaceInfo;
}

extern VERTEX_INFO v_buffer[];
extern int phd_winwidth;
extern int phd_winheight;

int XYClipper(int number, VERTEX_INFO *input);
void HWI_InsertPoly_Gouraud(int nPoints, float zdepth, int nR, int nG, int nB, int nDrawType);

void HWI_InsertTrans8_Sorted(PHD_VBUF *buf, sint16 shade)
{
    float z;
    long nPoints, nVtx;
    char clipO, clipA;
    int i;

    nVtx = 8;
    clipO = buf[7].clip;
    clipO |= buf[6].clip;
    clipO |= buf[5].clip;
    clipO |= buf[4].clip;
    clipO |= buf[3].clip;
    clipO |= buf[2].clip;
    clipO |= buf[1].clip;
    clipO |= buf[0].clip;
    if (clipO < 0)
        return;
    clipA = ((((((buf[6].clip & buf[7].clip) & buf[0].clip) & buf[1].clip) & buf[2].clip) & buf[3].clip) & buf[4].clip) & buf[5].clip;
    if (clipA)
        return;
    if ((long)((buf[2].xs - buf[1].xs) * (buf->ys - buf[1].ys) - (buf[2].ys - buf[1].ys) * (buf->xs - buf[1].xs)) < 0)
        return;
    for (i = 0; i < nVtx; i++) {
        v_buffer[i].x = buf[i].xs;
        v_buffer[i].y = buf[i].ys;
        v_buffer[i].ooz = one / (buf[i].zv - 131072);
        v_buffer[i].vr = 0;
        v_buffer[i].vg = 0;
        v_buffer[i].vb = 0;
    }
    nPoints = nVtx;
    if (clipO) {
        phd_leftfloat = (float)phd_winxmin;
        phd_topfloat = (float)phd_winymin;
        phd_rightfloat = float(phd_winxmin + phd_winwidth);
        phd_bottomfloat = float(phd_winymin + phd_winheight);
        nPoints = XYClipper(nPoints, v_buffer);
    }
    if (nPoints) {
        z = (((((((buf[6].zv + buf[5].zv) + buf[4].zv) + buf[3].zv) + buf[2].zv) + buf[1].zv) + buf[0].zv) + buf[7].zv) * 0.125F - 131072;
        HWI_InsertPoly_Gouraud(nPoints, z, 0, 0, 0, 13);
    }
}

void HWI_InsertGT4_Poly(PHD_VBUF *v1, PHD_VBUF *v2, PHD_VBUF *v3, PHD_VBUF *v4, PHDTEXTURESTRUCT *pTex, int nSortType, unsigned short dbl);
void SubdivideGT4(PHD_VBUF *v1, PHD_VBUF *v2, PHD_VBUF *v3, PHD_VBUF *v4, PHDTEXTURESTRUCT *pTex, int nSortType, unsigned short dbl, int level);
int GETR(int rgb);

void SubdivideEdge(PHD_VBUF *v0, PHD_VBUF *v1, PHD_VBUF *dest)
{
    float zv;
    sint16 r, g, b, r0, g0, b0, r1, g1, b1;

    dest->xv = (v1->xv + v0->xv) * 0.5F;
    dest->yv = (v0->yv + v1->yv) * 0.5F;
    dest->zv = (v0->zv + v1->zv) * 0.5F;
    if (dest->zv > phd_znear) {
        zv = f_persp / dest->zv;
        dest->xs = zv * dest->xv + f_centerx;
        dest->ys = dest->yv * zv + f_centery;
        dest->ooz = zv * f_oneopersp;
        dest->clip = 0;
        if (dest->xs < phd_leftfloat)
            dest->clip++;
        else if (dest->xs > phd_rightfloat)
            dest->clip += 2;
        if (dest->ys < phd_topfloat)
            dest->clip += 4;
        else if (dest->ys > phd_bottomfloat)
            dest->clip += 8;
    } else {
        dest->clip = -128;
    }
    r0 = (v0->g >> 10) & 31;
    r1 = (v1->g >> 10) & 31;
    g0 = (v0->g >> 5) & 31;
    g1 = (v1->g >> 5) & 31;
    b0 = v0->g & 31;
    b1 = v1->g & 31;
    r = (r0 + r1) >> 1;
    g = (g0 + g1) >> 1;
    b = (b0 + b1) >> 1;
    dest->g = (sint16)(r << 10 | g << 5 | b);
    dest->u = (v0->u + v1->u) >> 1;
    dest->v = (v0->v + v1->v) >> 1;
}

void HWI_InsertPoly_Gouraud(int nPoints, float zdepth, int nR, int nG, int nB, int nDrawType);

short *HWI_InsertObjectG4_Sorted(short *pFaceInfo, long nFaces, int nSortType)
{
    long nPoints;
    unsigned char *pC;
    float zdepth;

    for (; nFaces; nFaces--, pFaceInfo += 5) {
        PHD_VBUF *v0;
        PHD_VBUF *v1;
        PHD_VBUF *v2;
        PHD_VBUF *v3;

        v0 = &vbuf[pFaceInfo[0]];
        v1 = &vbuf[pFaceInfo[1]];
        v2 = &vbuf[pFaceInfo[2]];
        v3 = &vbuf[pFaceInfo[3]];
        if (outside && nPolyType != 1 && v0->zv == f_zfar && v1->zv == f_zfar && v2->zv == f_zfar && v3->zv == f_zfar && v0->ys > outsideBackgroundTop && v1->ys > outsideBackgroundTop && v2->ys > outsideBackgroundTop && v3->ys > outsideBackgroundTop)
            continue;
        if (v0->clip & v1->clip & v2->clip & v3->clip)
            continue;
        if ((v0->clip | v1->clip | v2->clip | v3->clip) < 0) {
            if (!visible_zclip(v0, v1, v2))
                continue;
            POINT_INFO points[4];
            PHD_VBUF_To_POINT_INFO(v0, &points[0]);
            PHD_VBUF_To_POINT_INFO(v1, &points[1]);
            PHD_VBUF_To_POINT_INFO(v2, &points[2]);
            PHD_VBUF_To_POINT_INFO(v3, &points[3]);
            nPoints = RoomZedClipper(4, points, v_buffer);
            if (nPoints) {
                phd_leftfloat = (float)phd_winxmin;
                phd_topfloat = (float)phd_winymin;
                phd_rightfloat = float(phd_winxmin + phd_winwidth);
                phd_bottomfloat = float(phd_winymin + phd_winheight);
                nPoints = XYGClipper(nPoints, v_buffer);
            }
        } else {
            if ((long)((v0->ys - v1->ys) * (v2->xs - v1->xs) - (v0->xs - v1->xs) * (v2->ys - v1->ys)) < 0)
                continue;
            PHD_VBUF_To_VERTEX_INFO(v0, &v_buffer[0]);
            PHD_VBUF_To_VERTEX_INFO(v1, &v_buffer[1]);
            PHD_VBUF_To_VERTEX_INFO(v2, &v_buffer[2]);
            PHD_VBUF_To_VERTEX_INFO(v3, &v_buffer[3]);
            nPoints = 4;
            if (v0->clip | v1->clip | v2->clip | v3->clip) {
                phd_leftfloat = (float)phd_winxmin;
                phd_topfloat = (float)phd_winymin;
                phd_rightfloat = float(phd_winxmin + phd_winwidth);
                phd_bottomfloat = float(phd_winymin + phd_winheight);
                nPoints = XYGClipper(nPoints, v_buffer);
            }
        }
        if (nPoints) {
            pC = &G_GouraudPalette[(pFaceInfo[4] >> 6) & 0x3fc];
            switch (nSortType) {
            case 1:
                zdepth = v0->zv;
                if (v1->zv > zdepth)
                    zdepth = v1->zv;
                if (v2->zv > zdepth)
                    zdepth = v2->zv;
                if (v3->zv > zdepth)
                    zdepth = v3->zv;
                break;
            case 0:
                zdepth = (v0->zv + v1->zv + v2->zv + v3->zv) * 0.25f;
                break;
            default:
                zdepth = 1000000000.0f;
            }
            HWI_InsertPoly_Gouraud(nPoints, zdepth, pC[0], pC[1], pC[2], 11);
        }
    }
    return pFaceInfo;
}

extern DWORD TPages[];

}

template <class T> inline void swap(T &a, T &b)
{
    a ^= b;
    b ^= a;
    a ^= b;
}

extern "C" {

inline void PHD_VBUF_To_TLVERT(PHD_VBUF *pV, D3DTLVERTEX *p)
{
    long r, g, b;
    int rgb;

    p->sx = pV->xs;
    p->sy = pV->ys;
    p->sz = f_a - f_boo * pV->ooz;
    p->rhw = pV->ooz;
    rgb = pV->g;
    r = GETR(rgb);
    g = GETG(rgb);
    b = ColorTable[(rgb & 0x1f) << 3];
    p->color = GlobalAlpha | (r << 16) | (g << 8) | b;
    p->specular = 0;
}

int XYGClipper(int n, VERTEX_INFO *in);
void HWI_InsertGT4_Poly(PHD_VBUF *v1, PHD_VBUF *v2, PHD_VBUF *v3, PHD_VBUF *v4, PHDTEXTURESTRUCT *pTex, int nSortType, unsigned short dbl);

void HWI_InsertGT4_Poly(PHD_VBUF *v0, PHD_VBUF *v1, PHD_VBUF *v2, PHD_VBUF *v3, PHDTEXTURESTRUCT *pTex, int nSortType, unsigned short dbl)
{
    float zdepth;
    D3DTLVERTEX *pTLV;
    int drawtype;

    if (outside && nPolyType != 1 && v0->zv == f_zfar && v1->zv == f_zfar && v2->zv == f_zfar && v3->zv == f_zfar && v0->ys > outsideBackgroundTop && v1->ys > outsideBackgroundTop && v2->ys > outsideBackgroundTop && v3->ys > outsideBackgroundTop)
        return;
    if (v0->clip & v1->clip & v2->clip & v3->clip)
        return;
    if ((v0->clip | v1->clip | v2->clip | v3->clip) < 0) {
        if (!dbl) {
            if (!visible_zclip(v0, v1, v2))
                return;
            HWI_InsertGT3_Poly(v0, v1, v2, pTex, &pTex->u1, &pTex->u2, &pTex->u3, nSortType, 0);
            HWI_InsertGT3_Poly(v0, v2, v3, pTex, &pTex->u1, &pTex->u3, &pTex->u4, nSortType, 0);
        } else {
            if (visible_zclip(v0, v1, v2)) {
                HWI_InsertGT3_Poly(v0, v1, v2, pTex, &pTex->u1, &pTex->u2, &pTex->u3, nSortType, 0);
                HWI_InsertGT3_Poly(v0, v2, v3, pTex, &pTex->u1, &pTex->u3, &pTex->u4, nSortType, 0);
            }
            swap((long &)v0, (long &)v2);
            if (visible_zclip(v0, v1, v2)) {
                HWI_InsertGT3_Poly(v0, v1, v2, pTex, &pTex->u3, &pTex->u2, &pTex->u1, nSortType, 0);
                HWI_InsertGT3_Poly(v0, v2, v3, pTex, &pTex->u3, &pTex->u1, &pTex->u4, nSortType, 0);
            }
        }
        return;
    }
    if ((long)((v2->xs - v1->xs) * (v0->ys - v1->ys) - (v0->xs - v1->xs) * (v2->ys - v1->ys)) < 0) {
        if (dbl) {
            swap((long &)v0, (long &)v2);
            if (v0->clip | v1->clip | v2->clip | v3->clip) {
                HWI_InsertGT3_Poly(v0, v1, v2, pTex, &pTex->u3, &pTex->u2, &pTex->u1, nSortType, 0);
                HWI_InsertGT3_Poly(v0, v2, v3, pTex, &pTex->u3, &pTex->u1, &pTex->u4, nSortType, 0);
                return;
            }
        } else
            return;
    } else
        dbl = 0;
    if (v0->clip | v1->clip | v2->clip | v3->clip) {
        HWI_InsertGT3_Poly(v0, v1, v2, pTex, &pTex->u1, &pTex->u2, &pTex->u3, nSortType, dbl);
        HWI_InsertGT3_Poly(v0, v2, v3, pTex, &pTex->u1, &pTex->u3, &pTex->u4, nSortType, dbl);
        return;
    }
    drawtype = pTex->drawtype;
    if (drawtype <= 1)
        drawtype += 9;
    else
        drawtype = 14;
    if (nPolyType == 1) {
        if (v0->ys > outsideBackgroundTop)
            outsideBackgroundTop = v0->ys;
        if (v1->ys > outsideBackgroundTop)
            outsideBackgroundTop = v1->ys;
        if (v2->ys > outsideBackgroundTop)
            outsideBackgroundTop = v2->ys;
        if (v3->ys > outsideBackgroundTop)
            outsideBackgroundTop = v3->ys;
    }
    if (!App.lpZBuffer || drawtype == 14 || drawtype == 10) {
        sint32 *sort;
        sint16 *info;
        double fconv;

        switch (nSortType) {
        case 1:
            zdepth = v0->zv;
            if (v1->zv > zdepth)
                zdepth = v1->zv;
            if (v2->zv > zdepth)
                zdepth = v2->zv;
            if (v3->zv > zdepth)
                zdepth = v3->zv;
            break;
        case 0:
            zdepth = (v0->zv + v1->zv + v2->zv + v3->zv) * 0.25f;
            break;
        default:
            zdepth = 1000000000.0f;
        }
        if (CurrentTLVertex - VertexBuffer > 9184)
            return;
        if (!App.lpZBuffer || !bAlphaTesting) {
            sort = sort3dptrbf;
            info = info3dptrbf;
            sort[2] = nPolyType;
            surfacenumbf++;
            sort3dptrbf += 3;
            info3dptrbf += 5;
        } else if (drawtype == 14 || drawtype == 13 || drawtype == 12 || drawtype == 16) {
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
        info[0] = (sint16)drawtype;
        info[1] = pTex->tpage;
        info[2] = 4;
        pTLV = CurrentTLVertex;
        *((D3DTLVERTEX **)(info + 3)) = pTLV;
        if (!dbl) {
            PHD_VBUF_To_TLVERT(v0, pTLV);
            pTLV->tu = UVTable[pTex->u1];
            pTLV->tv = UVTable[pTex->v1];
            pTLV++;
            PHD_VBUF_To_TLVERT(v1, pTLV);
            pTLV->tu = UVTable[pTex->u2];
            pTLV->tv = UVTable[pTex->v2];
            pTLV++;
            PHD_VBUF_To_TLVERT(v2, pTLV);
            pTLV->tu = UVTable[pTex->u3];
            pTLV->tv = UVTable[pTex->v3];
        } else {
            PHD_VBUF_To_TLVERT(v0, pTLV);
            pTLV->tu = UVTable[pTex->u3];
            pTLV->tv = UVTable[pTex->v3];
            pTLV++;
            PHD_VBUF_To_TLVERT(v1, pTLV);
            pTLV->tu = UVTable[pTex->u2];
            pTLV->tv = UVTable[pTex->v2];
            pTLV++;
            PHD_VBUF_To_TLVERT(v2, pTLV);
            pTLV->tu = UVTable[pTex->u1];
            pTLV->tv = UVTable[pTex->v1];
        }
        pTLV++;
        PHD_VBUF_To_TLVERT(v3, pTLV);
        pTLV->tu = UVTable[pTex->u4];
        pTLV->tv = UVTable[pTex->v4];
        pTLV++;
        CurrentTLVertex = pTLV;
    } else {
        int b;

        b = FindBucket(TPages[pTex->tpage]);
        if (b == -1)
            return;
        nDrawnPoints += 2;
        if (!dbl) {
            PHD_VBUF_To_TLVERT(v0, &Buckets[b].Vertex[Buckets[b].cnt]);
            Buckets[b].Vertex[Buckets[b].cnt].tu = UVTable[pTex->u1];
            Buckets[b].Vertex[Buckets[b].cnt].tv = UVTable[pTex->v1];
            Buckets[b].cnt++;
            PHD_VBUF_To_TLVERT(v1, &Buckets[b].Vertex[Buckets[b].cnt]);
            Buckets[b].Vertex[Buckets[b].cnt].tu = UVTable[pTex->u2];
            Buckets[b].Vertex[Buckets[b].cnt].tv = UVTable[pTex->v2];
            Buckets[b].cnt++;
            PHD_VBUF_To_TLVERT(v2, &Buckets[b].Vertex[Buckets[b].cnt]);
            Buckets[b].Vertex[Buckets[b].cnt].tu = UVTable[pTex->u3];
            Buckets[b].Vertex[Buckets[b].cnt].tv = UVTable[pTex->v3];
            Buckets[b].cnt++;
            PHD_VBUF_To_TLVERT(v0, &Buckets[b].Vertex[Buckets[b].cnt]);
            Buckets[b].Vertex[Buckets[b].cnt].tu = UVTable[pTex->u1];
            Buckets[b].Vertex[Buckets[b].cnt].tv = UVTable[pTex->v1];
            Buckets[b].cnt++;
            PHD_VBUF_To_TLVERT(v2, &Buckets[b].Vertex[Buckets[b].cnt]);
            Buckets[b].Vertex[Buckets[b].cnt].tu = UVTable[pTex->u3];
            Buckets[b].Vertex[Buckets[b].cnt].tv = UVTable[pTex->v3];
            Buckets[b].cnt++;
            PHD_VBUF_To_TLVERT(v3, &Buckets[b].Vertex[Buckets[b].cnt]);
            Buckets[b].Vertex[Buckets[b].cnt].tu = UVTable[pTex->u4];
            Buckets[b].Vertex[Buckets[b].cnt].tv = UVTable[pTex->v4];
            Buckets[b].cnt++;
        } else {
            PHD_VBUF_To_TLVERT(v0, &Buckets[b].Vertex[Buckets[b].cnt]);
            Buckets[b].Vertex[Buckets[b].cnt].tu = UVTable[pTex->u3];
            Buckets[b].Vertex[Buckets[b].cnt].tv = UVTable[pTex->v3];
            Buckets[b].cnt++;
            PHD_VBUF_To_TLVERT(v1, &Buckets[b].Vertex[Buckets[b].cnt]);
            Buckets[b].Vertex[Buckets[b].cnt].tu = UVTable[pTex->u2];
            Buckets[b].Vertex[Buckets[b].cnt].tv = UVTable[pTex->v2];
            Buckets[b].cnt++;
            PHD_VBUF_To_TLVERT(v2, &Buckets[b].Vertex[Buckets[b].cnt]);
            Buckets[b].Vertex[Buckets[b].cnt].tu = UVTable[pTex->u1];
            Buckets[b].Vertex[Buckets[b].cnt].tv = UVTable[pTex->v1];
            Buckets[b].cnt++;
            PHD_VBUF_To_TLVERT(v0, &Buckets[b].Vertex[Buckets[b].cnt]);
            Buckets[b].Vertex[Buckets[b].cnt].tu = UVTable[pTex->u3];
            Buckets[b].Vertex[Buckets[b].cnt].tv = UVTable[pTex->v3];
            Buckets[b].cnt++;
            PHD_VBUF_To_TLVERT(v2, &Buckets[b].Vertex[Buckets[b].cnt]);
            Buckets[b].Vertex[Buckets[b].cnt].tu = UVTable[pTex->u1];
            Buckets[b].Vertex[Buckets[b].cnt].tv = UVTable[pTex->v1];
            Buckets[b].cnt++;
            PHD_VBUF_To_TLVERT(v3, &Buckets[b].Vertex[Buckets[b].cnt]);
            Buckets[b].Vertex[Buckets[b].cnt].tu = UVTable[pTex->u4];
            Buckets[b].Vertex[Buckets[b].cnt].tv = UVTable[pTex->v4];
            Buckets[b].cnt++;
        }
    }
}

inline void PHD_VBUF_To_POINT_INFO_UV(PHD_VBUF *pV, POINT_INFO *p, unsigned short *pUV)
{
    p->xv = pV->xv;
    p->yv = pV->yv;
    p->zv = pV->zv;
    p->ooz = pV->ooz;
    p->xs = pV->xs;
    p->ys = pV->ys;
    p->vr = GETR(pV->g);
    p->vg = GETG(pV->g);
    p->vb = ColorTable[(pV->g & 0x1f) << 3];
    p->u = float(pUV[0]);
    p->v = float(pUV[1]);
}

inline void PHD_VBUF_To_VERTEX_INFO_UV(PHD_VBUF *pV, VERTEX_INFO *p, unsigned short *pUV)
{
    p->x = pV->xs;
    p->y = pV->ys;
    p->ooz = pV->ooz;
    p->u = pUV[0] * pV->ooz;
    p->v = pUV[1] * pV->ooz;
    p->vr = GETR(pV->g);
    p->vg = GETG(pV->g);
    p->vb = ColorTable[(pV->g & 0x1f) << 3];
}

int GETG(int rgb);

void HWI_InsertGT3_Poly(PHD_VBUF *v0, PHD_VBUF *v1, PHD_VBUF *v2, PHDTEXTURESTRUCT *pTex, unsigned short *uv0, unsigned short *uv1, unsigned short *uv2, int nSortType, unsigned short dbl)
{
    int nPoints = 0;
    float zdepth = 0;
    D3DTLVERTEX *pTLV = 0;
    POINT_INFO points[3];
    int drawtype = 0;

    if (outside && nPolyType != 1 && v0->zv == f_zfar && v1->zv == f_zfar && v2->zv == f_zfar && v0->ys > outsideBackgroundTop && v1->ys > outsideBackgroundTop && v2->ys > outsideBackgroundTop)
        return;
    if ((v0->clip | v1->clip | v2->clip) < 0) {
        PHD_VBUF_To_POINT_INFO_UV(v0, &points[0], uv0);
        PHD_VBUF_To_POINT_INFO_UV(v1, &points[1], uv1);
        PHD_VBUF_To_POINT_INFO_UV(v2, &points[2], uv2);
        nPoints = RoomZedClipper(3, points, v_buffer);
        if (nPoints)
            goto doxyclip;
        return;
    }
    if ((long)((v2->xs - v1->xs) * (v0->ys - v1->ys) - (v0->xs - v1->xs) * (v2->ys - v1->ys)) < 0) {
        if (dbl) {
            swap((long &)v1, (long &)v2);
            swap((long &)uv1, (long &)uv2);
        } else
            return;
    }
    if (v0->clip | v1->clip | v2->clip) {
        PHD_VBUF_To_VERTEX_INFO_UV(v0, &v_buffer[0], uv0);
        PHD_VBUF_To_VERTEX_INFO_UV(v1, &v_buffer[1], uv1);
        PHD_VBUF_To_VERTEX_INFO_UV(v2, &v_buffer[2], uv2);
        nPoints = 3;
doxyclip:
        phd_leftfloat = (float)phd_winxmin;
        phd_rightfloat = float(phd_winxmin + phd_winwidth);
        phd_topfloat = (float)phd_winymin;
        phd_bottomfloat = float(phd_winymin + phd_winheight);
        nPoints = RoomXYGUVClipper(nPoints, v_buffer);
        if (nPoints == 0)
            return;
        switch (nSortType) {
        case 1:
            zdepth = v0->zv;
            if (v1->zv > zdepth)
                zdepth = v1->zv;
            if (v2->zv > zdepth)
                zdepth = v2->zv;
            break;
        case 0:
            zdepth = (v0->zv + v1->zv + v2->zv) * 0.33333334f;
            break;
        default:
            zdepth = 1000000000.0f;
        }
        drawtype = pTex->drawtype;
        if (drawtype <= 1)
            drawtype += 9;
        else
            drawtype = 14;
        if (nPolyType == 1) {
            if (v0->ys > outsideBackgroundTop)
                outsideBackgroundTop = v0->ys;
            if (v1->ys > outsideBackgroundTop)
                outsideBackgroundTop = v1->ys;
            if (v2->ys > outsideBackgroundTop)
                outsideBackgroundTop = v2->ys;
        }
        HWI_InsertClippedPoly_Textured(nPoints, zdepth, drawtype, pTex->tpage);
        return;
    }
    drawtype = pTex->drawtype;
    if (drawtype <= 1)
        drawtype += 9;
    else
        drawtype = 14;
    if (nPolyType == 1) {
        if (v0->ys > outsideBackgroundTop)
            outsideBackgroundTop = v0->ys;
        if (v1->ys > outsideBackgroundTop)
            outsideBackgroundTop = v1->ys;
        if (v2->ys > outsideBackgroundTop)
            outsideBackgroundTop = v2->ys;
    }
    if (!App.lpZBuffer || drawtype == 14 || drawtype == 10) {
        sint32 *sort = 0;
        sint16 *info = 0;
        double fconv;

        switch (nSortType) {
        case 1:
            zdepth = v0->zv;
            if (zdepth < v1->zv)
                zdepth = v1->zv;
            if (zdepth < v2->zv)
                zdepth = v2->zv;
            break;
        case 0:
            zdepth = (v0->zv + v1->zv + v2->zv) * 0.33333334f;
            break;
        default:
            zdepth = 1000000000.0f;
        }
        if (CurrentTLVertex - VertexBuffer > 9184)
            return;
        if (!App.lpZBuffer || !bAlphaTesting) {
            sort = sort3dptrbf;
            info = info3dptrbf;
            sort[2] = nPolyType;
            surfacenumbf++;
            sort3dptrbf += 3;
            info3dptrbf += 5;
        } else if (drawtype == 14 || drawtype == 13 || drawtype == 12 || drawtype == 16) {
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
        info[0] = (sint16)drawtype;
        info[1] = pTex->tpage;
        info[2] = 3;
        pTLV = CurrentTLVertex;
        *((D3DTLVERTEX **)(info + 3)) = pTLV;
        PHD_VBUF_To_D3DTLVTX(v0, pTLV, uv0);
        pTLV++;
        PHD_VBUF_To_D3DTLVTX(v1, pTLV, uv1);
        pTLV++;
        PHD_VBUF_To_D3DTLVTX(v2, pTLV, uv2);
        pTLV++;
        CurrentTLVertex = pTLV;
    } else {
        int b;

        b = FindBucket(TPages[pTex->tpage]);
        if (b == -1)
            return;
        nDrawnPoints++;
        PHD_VBUF_To_D3DTLVTX(v0, &Buckets[b].Vertex[Buckets[b].cnt], uv0);
        Buckets[b].cnt++;
        PHD_VBUF_To_D3DTLVTX(v1, &Buckets[b].Vertex[Buckets[b].cnt], uv1);
        Buckets[b].cnt++;
        PHD_VBUF_To_D3DTLVTX(v2, &Buckets[b].Vertex[Buckets[b].cnt], uv2);
        Buckets[b].cnt++;
    }
}
}
