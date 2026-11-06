#include "standard.h"
#include "specific.h"
#include "winmain.h"

extern "C" {

#include "../room.h"

extern int water_effect;
extern int shade_effect;
extern bool bBlueEffect;
extern int framedump;
extern RECT phd_WindowRect;
extern sint16 phd_winxmin;
extern sint16 phd_winymin;
extern int phd_zfar;
extern unsigned short inv_colours[17];
extern void (*InsertFlatRect)(int x1, int y1, int x2, int y2, int zdepth, int col);

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

extern PHDTEXTURESTRUCT phdtextinfo[];
extern sint16 *aranges;
extern int light_level[4];
extern sint16 GF_SunsetEnabled;
extern int SunsetTimer;

sint32 __fastcall phd_sin(sint32 angle);

extern unsigned char game_palette[768];
extern unsigned char water_palette[768];
extern int wet;

void FadeToPal(int nFrames, unsigned char *pPal);
void TempVideoRemove(void);
void HWR_InitState(void);
int Sync(void);
void ScreenPartialDump(void);

void DXSaveScreen(LPDIRECTDRAWSURFACE3 lpSurface);
void DXUpdateFrame(bool runMessageLoop, LPRECT rect);
void DXClearBuffers(DWORD dwClrFlags, DWORD dwColour);
void ScreenClear(bool tWindow);

extern sint32 nPolyType;
extern bool GtFullScreenClearNeeded;
extern int objbcnt;
bool DXCheckForLostSurfaces(void);
bool DD_SpinMessageLoop(bool tWait);
void DXDoFlipWait(void);
void HWR_BeginScene(void);
void phd_InitPolyList(void);

void S_InitialisePolyList(int clear)
{
    DWORD flags;

    nPolyType = 0;
    if (GtFullScreenClearNeeded) {
        DXCheckForLostSurfaces();
        DD_SpinMessageLoop(false);
        DXDoFlipWait();
        DXClearBuffers(3, 0);
        clear = 0;
        GtFullScreenClearNeeded = false;
    }
    if (!App.nRenderMode) {
        DXClearBuffers(0x110, 0);
    } else {
        flags = 0x100;
        if (clear || HWConfig.nFillMode < D3DFILL_SOLID)
            flags = 0x102;
        DXClearBuffers(flags | 8, 0);
    }
    HWR_BeginScene();
    phd_InitPolyList();
    objbcnt = 0;
}

int S_DumpCine(void)
{
    static int nf = 0;

    if (framedump) {
        nf++;
        DXSaveScreen(App.lpFrontBuffer);
        return 1;
    }
    return 0;
}

int S_DumpScreen(void)
{
    int nFrames;

    if (!framedump) {
        nFrames = Sync();
        while (nFrames < 2) {
            while (!Sync())
                ;
            nFrames++;
        }
    } else {
        nFrames = 2;
    }
    ScreenPartialDump();
    return nFrames;
}

void S_ClearScreen(void)
{
    ScreenClear(false);
}

void S_InitialiseScreen(int type)
{
    if (type >= 0) {
        if (type != 0)
            TempVideoRemove();
        FadeToPal(30, game_palette);
    } else {
        FadeToPal(0, game_palette);
    }
    if (App.nRenderMode != 0)
        HWR_InitState();
}

void S_LightRoom(ROOM_INFO *r)
{
    sint16 *ptr;
    int level;
    int i;

    if (!r->lighting)
        return;
    level = light_level[r->lighting];
    ptr = r->data;
    for (i = *ptr++; i > 0; i--, ptr += 6)
        ((unsigned char *)ptr)[2] += (unsigned char)((level * (((unsigned char *)ptr)[3] & 0x1f)) >> 6);
}

void AnimateTextures(int n)
{
    static int comp;
    PHDTEXTURESTRUCT tex;
    sint16 *range;
    int nRanges;
    int nFrames;
    int i;

    for (comp += n; comp > 5; comp -= 5) {
        range = aranges;
        nRanges = *range++;
        for (i = 0; i < nRanges; i++) {
            nFrames = *range++;
            tex = phdtextinfo[*range];
            while (nFrames > 0) {
                phdtextinfo[range[0]] = phdtextinfo[range[1]];
                range++;
                nFrames--;
            }
            phdtextinfo[*range] = tex;
            range++;
        }
    }
}

void S_SetupBelowWater(int underwater)
{
    if (underwater ^ wet) {
        if (underwater)
            FadeToPal(1, water_palette);
        else
            FadeToPal(1, game_palette);
        wet = underwater;
    }
    water_effect = !underwater;
    shade_effect = 1;
    bBlueEffect = true;
}

void S_SetupAboveWater(int underwater)
{
    water_effect = underwater;
    shade_effect = 0;
    bBlueEffect = underwater;
}

static int pulse;

void S_AnimateTextures(int n)
{
    pulse = (n + pulse) & 31;
    light_level[1] = GetRandomDraw() & 31;
    light_level[2] = ((phd_sin((pulse << 16) / 32) + 0x4000) * 31) >> 15;
    if (GF_SunsetEnabled) {
        SunsetTimer += n;
        if (SunsetTimer < 72000)
            light_level[3] = 31 * SunsetTimer / 72000;
        else
            light_level[3] = 31;
    }
    AnimateTextures(n);
}

void ScreenPartialDump(void)
{
    DXUpdateFrame(true, &phd_WindowRect);
}

void ScreenClear(bool tWindow)
{
    DXClearBuffers(2, 0);
}

extern sint32 w2v_matrix[12];

void mCalcPoint(sint32 x, sint32 y, sint32 z, sint32 *result)
{
    x -= w2v_matrix[3];
    y -= w2v_matrix[7];
    z -= w2v_matrix[11];
    result[0] = (w2v_matrix[0] * x + w2v_matrix[1] * y + w2v_matrix[2] * z) >> 14;
    result[1] = (w2v_matrix[4] * x + w2v_matrix[5] * y + w2v_matrix[6] * z) >> 14;
    result[2] = (w2v_matrix[8] * x + w2v_matrix[9] * y + w2v_matrix[10] * z) >> 14;
}

void ProjectPCoord(sint32 x, sint32 y, sint32 z, sint32 *result, sint32 cx, sint32 cy, sint32 fov)
{
    if (z > 0) {
        result[0] = cx + x * fov / z;
        result[1] = cy + y * fov / z;
        result[2] = z;
    } else if (z < 0) {
        result[0] = cx - x * fov / z;
        result[1] = cy - y * fov / z;
        result[2] = z;
    } else {
        result[0] = cx + x * fov;
        result[1] = cy + y * fov;
        result[2] = z;
    }
}

#include "../items.h"

extern sint16 shadowman[];
extern sint16 phd_winxmin;
extern sint16 phd_winymin;
extern sint32 phd_left, phd_right, phd_top, phd_bottom;
extern sint32 phd_centerx, phd_centery;
extern float phd_leftfloat, phd_topfloat, phd_rightfloat, phd_bottomfloat;
extern float f_centerx, f_centery;
extern sint32 *phd_mxptr;
extern char vbuf[];
extern void (*InsertTrans8)(void *vbuf, sint16 shade);
void phd_PushMatrix(void);
void phd_TranslateAbs(sint32 x, sint32 y, sint32 z);
void phd_RotY(PHD_ANGLE ry);
sint16 *calc_object_vertices(sint16 *objptr);
void phd_RotY(PHD_ANGLE ry);

void S_PrintShadow(sint16 size, sint16 *bptr, ITEM_INFO *iptr)
{
    int midx;
    int xadd;
    int xadd2;
    int midz;
    int zadd;
    int zadd2;

    midx = (bptr[0] + bptr[1]) / 2;
    midz = (bptr[4] + bptr[5]) / 2;
    xadd = ((bptr[1] - bptr[0]) * size) / 1024;
    zadd = ((bptr[5] - bptr[4]) * size) / 1024;
    xadd2 = xadd * 2;
    zadd2 = zadd * 2;

    shadowman[6] = midx - xadd;
    shadowman[8] = midz + zadd2;
    shadowman[9] = midx + xadd;
    shadowman[11] = midz + zadd2;
    shadowman[12] = midx + xadd2;
    shadowman[14] = midz + zadd;
    shadowman[15] = midx + xadd2;
    shadowman[17] = midz - zadd;
    shadowman[18] = midx + xadd;
    shadowman[20] = midz - zadd * 2;
    shadowman[21] = midx - xadd;
    shadowman[23] = midz - zadd * 2;
    shadowman[24] = midx - xadd2;
    shadowman[26] = midz - zadd;
    shadowman[27] = midx - xadd2;
    shadowman[29] = midz + zadd;

    phd_leftfloat = (float)(phd_winxmin + phd_left);
    phd_topfloat = (float)(phd_winymin + phd_top);
    phd_rightfloat = (float)(phd_winxmin + phd_right + 1);
    phd_bottomfloat = (float)(phd_winymin + phd_bottom + 1);
    f_centerx = (float)(phd_winxmin + phd_centerx);
    f_centery = (float)(phd_winymin + phd_centery);

    phd_PushMatrix();
    phd_TranslateAbs(iptr->pos.x_pos, iptr->floor, iptr->pos.z_pos);
    phd_RotY(iptr->pos.y_rot);
    if (calc_object_vertices(&shadowman[4]))
        InsertTrans8(vbuf, 32);
    phd_mxptr -= 12;
}

}
