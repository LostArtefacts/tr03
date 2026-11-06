#include "../specific/standard.h"

extern "C" {

#include "../3dglodef.h"

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

void HWI_InsertGT4_Poly(PHD_VBUF *v1, PHD_VBUF *v2, PHD_VBUF *v3, PHD_VBUF *v4, PHDTEXTURESTRUCT *pTex, int nSortType, unsigned short dbl);
void SubdivideGT4(PHD_VBUF *v1, PHD_VBUF *v2, PHD_VBUF *v3, PHD_VBUF *v4, PHDTEXTURESTRUCT *pTex, int nSortType, unsigned short dbl, int level);

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

short *HWI_InsertObjectG3_Sorted(short *pFaceInfo, long nFaces, int nSortType);

}
