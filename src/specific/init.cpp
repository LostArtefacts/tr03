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
