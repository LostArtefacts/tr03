#include "standard.h"
#include "specific.h"
#include "winmain.h"

extern "C" {

#include "../game.h"
#include "../text.h"

extern int noinput_count;
extern bool dontFadePicture;
extern char **GF_titlefilenames;
extern int title_loaded;
extern int reset_flag;
extern sint16 Inventory_Chosen;
extern sint32 Inventory_ExtraData[8];

void TempVideoAdjust(int hires, double sizer);
bool LoadPicture(const char *name, LPDIRECTDRAWSURFACE3 surf, int a);
void FadePictureUp(int steps);
void FadePictureDown(int steps);
sint32 InitialiseLevel(sint32 level, sint32 type);
void S_CDStop(void);
void S_CDPlay(sint16 track, int mode);
bool TIME_Init(void);
sint32 Display_Inventory(sint32 inventory_mode);
void Inv_RemoveAllItems(void);
int S_LoadGame(LPVOID data, int size, int slot);
sint32 TitleSequence(void);
void InitialiseStartInfo(void);
extern int HiResFlag;
extern int game_hires;
extern double screen_sizer;
extern double game_sizer;
extern char *malloc_buffer;
extern char exit_message[128];
extern bool GtWindowClosed;
extern char GF_StartGame;
extern int nLoadedPictures;
int S_InitialiseSystem(void);
sint32 GF_LoadScriptFile(char *fname);
void SOUND_Init(void);
int S_FrontEndCheck(void *data, int size);
int S_UpdateInput(void);
void S_Wait(int nFrames, int skip);
void ForceFadeDown(int fade);
sint32 GF_DoFrontEndSequence(void);
sint32 StartCinematic(sint32 level_number);
sint32 DoDemoSequence(int level_number);
sint32 LevelCompleteSequence(int level_number);
void S_SaveSettings(void);
void ShutdownGame(void);

sint32 TitleSequence(void)
{
    T_InitPrint();
    TempVideoAdjust(1, 1.0);
    noinput_count = 0;
    dontFadePicture = true;
    LoadPicture(GF_titlefilenames[1], App.lpPictureBuffer, 0);
    FadePictureUp(32);
    if (!title_loaded) {
        if (!InitialiseLevel(0, 0))
            return 0x700;
        title_loaded = 1;
    }
    S_CDStop();
    if (gameflow.title_track)
        S_CDPlay(gameflow.title_track, 1);
    TIME_Init();
    Display_Inventory(1);
    dontFadePicture = false;
    FadePictureDown(32);
    S_CDStop();
    if (reset_flag) {
        reset_flag = 0;
        return 0x400;
    }
    if (Inventory_Chosen == 147)
        return 0;
    if (Inventory_Chosen == 145) {
        if (Inventory_ExtraData[0] == 0) {
            Inv_RemoveAllItems();
            S_LoadGame(&savegame, sizeof(SAVEGAME_INFO), Inventory_ExtraData[1]);
            return Inventory_ExtraData[1] | 0x100;
        }
        if (Inventory_ExtraData[0] == 1) {
            InitialiseStartInfo();
            if (gameflow.play_any_level)
                return Inventory_ExtraData[1] + 1;
            return 1;
        }
    }
    return 0x700;
}

}
