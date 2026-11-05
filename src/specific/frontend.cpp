#include "specific.h"

extern "C" {

extern unsigned char game_palette[];
extern int Inventory_Mode;

unsigned char SWR_FindNearestPaletteEntry(unsigned char *pPal, int nR, int nG, int nB, bool tDontUseWindowsColours);
void TempVideoRemove(void);
int FMV_Play(char *fname);
int FMV_PlayIntro(char *movie1, char *movie2);

unsigned short S_COLOUR(int R, int G, int B)
{
    return SWR_FindNearestPaletteEntry(game_palette, R, G, B, false);
}

void S_FinishInventory(void)
{
    if (Inventory_Mode != 1)
        TempVideoRemove();
}

void DrawDummyItem(struct item_info *item)
{
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
