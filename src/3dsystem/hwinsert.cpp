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

}
