#include "standard.h"
#include "specific.h"
#include "winmain.h"
#include "texture.h"

extern "C" {

extern char *malloc_buffer;
extern int malloc_size;
extern char *malloc_ptr;
extern int malloc_free;
extern int malloc_used;
extern char *game_malloc_types[];
extern char exit_message[128];
void S_ExitSystem(char *message);
extern void *TLVertexBuffer;
extern void *TLUnRollBuffer;
extern void *VertexBuffer;
extern void *UnRollBuffer;
extern sint16 DumpX;
extern sint16 DumpY;
extern sint16 DumpWidth;
extern sint16 DumpHeight;
extern int SqrtTable[1024];
extern int RColorTable[33][33][33];
extern int GColorTable[33][33][33];
extern int BColorTable[33][33][33];
void InitZTable(void);
void InitUVTable(void);
extern float wibble_table[32];
extern sint16 shade_table[32];
extern sint32 rand_table[32];
extern sint32 wibble_light[32][32];

sint32 __fastcall phd_sin(sint32 angle);

void DXFreeTPages(void);
void ACMClose(void);
bool DXSetCooperativeLevel(LPDIRECTDRAW2 lpDD2, HWND WindowHandle, int Flags);
void DXClearAllTextures(DXTEXTURE *list);
void DI_Finish(void);
void DS_Finish(void);
void CloseDrawPrimitive(void);
void WinFreeDX(bool free_dd);
void DXFreeDeviceInfo(DEVICEINFO *dd);

typedef struct watertab {
    char shimmer;
    char choppy;
    unsigned char random;
    unsigned char abs;
} WATERTAB;

unsigned short GetRandom(WATERTAB *w, int n);

extern WATERTAB WaterTable[22][64];
extern sint16 rcossin_tbl[];
extern unsigned char water_off[4];
extern sint16 water_shim[4];
extern sint16 water_chop[4];

void init_water_table(void)
{
    int lp;
    int i;
    int j;
    int k;
    sint16 sin;

    srand(121197);

    for (lp = 0; lp < 64; lp++) {
        sin = rcossin_tbl[lp << 7];

        WaterTable[0][lp].shimmer = (sin * 63) >> 15;
        WaterTable[0][lp].choppy = (sin * 16) >> 12;
        WaterTable[0][lp].random = GetRandom(WaterTable[0], lp);
        WaterTable[0][lp].abs = 0;

        WaterTable[1][lp].shimmer = (sin * 32) >> 15;
        WaterTable[1][lp].choppy = 0;
        WaterTable[1][lp].random = GetRandom(WaterTable[1], lp);
        WaterTable[1][lp].abs = -3;

        WaterTable[2][lp].shimmer = (sin * 64) >> 15;
        WaterTable[2][lp].choppy = 0;
        WaterTable[2][lp].random = GetRandom(WaterTable[2], lp);
        WaterTable[2][lp].abs = 0;

        WaterTable[3][lp].shimmer = (sin * 96) >> 15;
        WaterTable[3][lp].choppy = 0;
        WaterTable[3][lp].random = GetRandom(WaterTable[3], lp);
        WaterTable[3][lp].abs = 4;

        WaterTable[4][lp].shimmer = (sin * 127) >> 15;
        WaterTable[4][lp].choppy = 0;
        WaterTable[4][lp].random = GetRandom(WaterTable[4], lp);
        WaterTable[4][lp].abs = 8;

        for (i = 0, j = 5; i < 4; i++, j += 4) {
            for (k = 0; k < 4; k++) {
                WaterTable[j + k][lp].shimmer = -((sin * water_shim[k]) >> 15);
                WaterTable[j + k][lp].choppy = (sin * water_chop[i]) >> 12;
                WaterTable[j + k][lp].random = GetRandom(WaterTable[j + k], lp);
                WaterTable[j + k][lp].abs = water_off[k];
            }
        }
    }
}

unsigned short GetRandom(WATERTAB *w, int n)
{
    int i;
    unsigned short random;

    do {
        random = rand() & 0xfc;
        for (i = 0; i < n; i++) {
            if (w[i].random == random)
                break;
        }
        if (i == n)
            return random;
    } while (1);
    return 0;
}

int S_InitialiseSystem(void)
{
    int cval;
    int i;
    int a;
    int b;
    int c;

    DumpX = 0;
    DumpY = 0;
    DumpWidth = (sint16)App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].w;
    DumpHeight = (sint16)App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].h;
    InitZTable();
    InitUVTable();

    TLVertexBuffer = GlobalAlloc(GMEM_FIXED, 0x48000);
    VertexBuffer = (void *)(((long)TLVertexBuffer + 32) & 0xFFFFFFE0);
    TLUnRollBuffer = GlobalAlloc(GMEM_FIXED, 0x48000);
    UnRollBuffer = (void *)(((long)TLUnRollBuffer + 32) & 0xFFFFFFE0);

    for (i = 0; i < 1024; i++)
        SqrtTable[i] = (int)sqrt((float)i);

    for (a = 1; a < 33; a++) {
        for (b = 1; b < 33; b++) {
            for (c = 1; c < 33; c++) {
                cval = ((a - b) * c) / a;
                RColorTable[a][b][c] = cval << 10;
                GColorTable[a][b][c] = cval << 5;
                BColorTable[a][b][c] = cval;
            }
        }
    }

    malloc_size = 0x380000;
    return 1;
}

void ShutdownGame(void)
{
    GlobalFree(TLVertexBuffer);
    GlobalFree(TLUnRollBuffer);
    DXFreeTPages();
    ACMClose();
    DXSetCooperativeLevel(App.lpDD, App.WindowHandle, DDSCL_NORMAL);
    if (malloc_buffer)
        GlobalFree(malloc_buffer);
    DXClearAllTextures(PictureTextures);
    DI_Finish();
    DS_Finish();
    if (!App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].bHardware)
        CloseDrawPrimitive();
    WinFreeDX(true);
    DXFreeDeviceInfo(&App.DeviceInfo);
}

void init_game_malloc(void)
{
    malloc_ptr = malloc_buffer;
    malloc_free = malloc_size;
    malloc_used = 0;
}

void *game_malloc(int size, int type)
{
    char *ptr;

    size = (size + 3) & ~3;
    if (size <= malloc_free) {
        ptr = malloc_ptr;
        malloc_free -= size;
        malloc_used += size;
        malloc_ptr += size;
        return ptr;
    }
    wsprintf(exit_message, "game_malloc(): OUT OF MEMORY %s %d", game_malloc_types[type], size);
    S_ExitSystem(exit_message);
    return NULL;
}

void game_free(int size, int type)
{
    size = (size + 3) & ~3;
    malloc_ptr -= size;
    malloc_free += size;
    malloc_used -= size;
}

void CalculateWibbleTable(void)
{
    int i;
    int j;
    int sn;
    sint16 angle;

    for (i = 0; i < 32; i++) {
        angle = (i * 65536) / 32;
        sn = phd_sin(angle);
        wibble_table[i] = (float)(sn * 2 >> 14);
        shade_table[i] = sn * 0x300 >> 14;
        rand_table[i] = (GetRandomDraw() >> 5) - 0x1ff;
        for (j = 0; j < 31; j++)
            wibble_light[i][j] = ((j - 16) * i << 9) / 31;
    }
}

}
