#include "specific.h"

extern "C" {

extern unsigned char game_palette[];
extern int Inventory_Mode;
extern int phd_znear;
extern sint16 phd_winxmin;
extern sint16 phd_winymin;
extern int input;
extern void (*InsertLine)(int x1, int y1, int x2, int y2, int z, int color0, int color1);
extern void (*InsertTransQuad)(int x, int y, int w, int h, int z);

int Sync(void);
int S_UpdateInput(void);

unsigned char SWR_FindNearestPaletteEntry(unsigned char *pPal, int nR, int nG, int nB, bool tDontUseWindowsColours);
void TempVideoRemove(void);
int FMV_Play(char *fname);
int FMV_PlayIntro(char *movie1, char *movie2);

unsigned short S_COLOUR(int R, int G, int B)
{
    return SWR_FindNearestPaletteEntry(game_palette, R, G, B, false);
}

void S_DrawScreenLine(int x, int y, int z, int w, int h, int col, unsigned short *grdptr, unsigned short flags)
{
    InsertLine(x, y, x + w, y + h, phd_znear + z * 8, (char)col, col);
}

void S_FinishInventory(void)
{
    if (Inventory_Mode != 1)
        TempVideoRemove();
}

void DrawDummyItem(struct item_info *item)
{
}

void S_Wait(int nFrames, int skip)
{
    int s;

    if (skip) {
        while (nFrames > 0) {
            if (!input)
                break;
            S_UpdateInput();
            do {
                s = Sync();
            } while (!s);
            nFrames -= s;
        }
    }
    while (nFrames > 0) {
        S_UpdateInput();
        if (skip && input)
            break;
        do {
            s = Sync();
        } while (!s);
        nFrames -= s;
    }
}

int S_PlayFMV(char *fname, int sequence)
{
    return FMV_Play(fname);
}

int S_IntroFMV(char *movie1, char *movie2)
{
    return FMV_PlayIntro(movie1, movie2);
}

}
