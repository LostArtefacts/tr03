#include "standard.h"
#include "specific.h"
#include "winmain.h"

extern "C" sint32 rand_1;
extern "C" sint32 rand_2;

extern "C" {

#include "../invdata.h"
#include "../gameflow.h"
#include "../savegame.h"

extern int overlay_flag;
extern int noinput_count;
extern int GnGameMode;
extern bool GtWindowClosed;
extern sint16 Option_Music_Volume;

void InitialiseCamera(void);
sint32 ControlPhase(sint32 nframes, int demo_mode);
sint32 DrawPhaseGame(void);
void S_FadeToBlack(void);
void S_SoundStopAllSamples(void);
void S_CDStop(void);
void S_CDVolume(int volume);
void AddRequesterItem(REQUEST_INFO *req, char *text1, uint32 flags1, char *text2, uint32 flags2);
void SetPCRequesterSize(REQUEST_INFO *req, int maxlines, int ypos);
extern char **GF_Level_Names;
extern uint32 RequesterFlags1[];
extern uint32 RequesterFlags2[];
extern uint32 SaveGameReqFlags1[];
extern uint32 SaveGameReqFlags2[];
void ChangeRequesterItem(REQUEST_INFO *req, int item, char *text1, uint32 flags1, char *text2, uint32 flags2);
extern REQUEST_INFO Load_Game_Requester;
extern int save_counter;
extern sint16 saved_levels[];
extern int SavedGames;
void Init_Requester(REQUEST_INFO *req);
extern char **GF_PCStrings;
void S_UnloadLevelFile(void);
sint32 InitialiseLevel(sint32 level, sint32 type);
sint32 S_StartSyncedAudio(sint32 track);
bool LoadPicture(const char *name, LPDIRECTDRAWSURFACE3 surf, int a);
void FadePictureUp(int steps);
void FadePictureDown(int steps);
void S_Wait(int nFrames, int skip);

int GameLoop(int demo_mode)
{
    int nframes;
    int lp;

    overlay_flag = 1;
    InitialiseCamera();
    noinput_count = 0;
    if (demo_mode)
        GnGameMode = 2;
    else
        GnGameMode = 1;
    lp = ControlPhase(1, demo_mode);
    while (!lp) {
        nframes = DrawPhaseGame();
        if (GtWindowClosed)
            lp = 0x700;
        else
            lp = ControlPhase(nframes, demo_mode);
    }
    if (lp != 1)
        S_FadeToBlack();
    GnGameMode = 0;
    S_SoundStopAllSamples();
    S_CDStop();
    if (Option_Music_Volume)
        S_CDVolume(25 * Option_Music_Volume + 5);
    return lp;
}

}

extern "C" sint32 LevelCompleteSequence(int level_number)
{
    return 0x500;
}

sint32 GetRandomControl(void)
{
    rand_1 = rand_1 * 0x41c64e6d + 0x3039;
    return (rand_1 >> 10) & 0x7fff;
}

void SeedRandomControl(sint32 seed)
{
    rand_1 = seed;
}

sint32 GetRandomDraw(void)
{
    rand_2 = rand_2 * 0x41c64e6d + 0x3039;
    return (rand_2 >> 10) & 0x7fff;
}

void SeedRandomDraw(sint32 seed)
{
    rand_2 = seed;
}

extern "C" void GetValidLevelsList(REQUEST_INFO *req)
{
    int i;

    RemoveAllReqItems(req);
    for (i = 1; i < gameflow.num_levels; i++)
        AddRequesterItem(req, GF_Level_Names[i], 0, NULL, 0);
}

extern "C" void GetSavedGamesList(REQUEST_INFO *req)
{
    SetPCRequesterSize(req, 10, -32);
    if (req->selected >= req->vis_lines)
        req->line_offset = req->selected - req->vis_lines + 1;
    memcpy(RequesterFlags1, SaveGameReqFlags1, 24 * sizeof(uint32));
    memcpy(RequesterFlags2, SaveGameReqFlags2, 24 * sizeof(uint32));
}

extern "C" void DisplayCredits(void)
{
    char buf[64] = "pix\\credit0?.bmp";
    int i;

    S_UnloadLevelFile();
    if (!InitialiseLevel(0, 0))
        return;
    S_StartSyncedAudio(121);
    LoadPicture("pix\\theend.bmp", App.lpPictureBuffer, 1);
    FadePictureUp(32);
    S_Wait(300, 0);
    FadePictureDown(32);
    for (i = 1; i < 10; i++) {
        buf[11] = i + '0';
        LoadPicture(buf, App.lpPictureBuffer, 1);
        FadePictureUp(32);
        S_Wait(300, 0);
        FadePictureDown(32);
    }
    LoadPicture("pix\\theend2.bmp", App.lpPictureBuffer, 1);
    FadePictureUp(32);
}

extern "C" int S_LoadGame(LPVOID data, int size, int slot)
{
    HANDLE file;
    DWORD bytes;
    int value;
    char buffer[80];

    wsprintf(buffer, "savegame.%d", slot);
    file = CreateFile(buffer, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (file != INVALID_HANDLE_VALUE) {
        ReadFile(file, buffer, 75, &bytes, 0);
        ReadFile(file, &value, sizeof(int), &bytes, 0);
        ReadFile(file, data, size, &bytes, 0);
        CloseHandle(file);
        return 1;
    }
    return 0;
}
