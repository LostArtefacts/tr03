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

extern uint32 CurrentLevel;
extern int level_complete;
extern int title_loaded;
extern sint16 Inventory_Chosen;
extern sint32 Inventory_ExtraData[8];
void ModifyStartInfo(int level_number);
void InitialiseLevelFlags(void);
void S_FadeInInventory(int fade);
void StartGameStub1(void);
int S_LoadGame(LPVOID data, int size, int slot);
int GameLoop(int demo_mode);

sint32 StartGame(int level_number, int type)
{
    sint32 result;

    if (type == 1 || type == 2 || type == 3)
        CurrentLevel = level_number;
    if (type != 2)
        ModifyStartInfo(level_number);
    title_loaded = 0;
    if (type != 2)
        InitialiseLevelFlags();
    if (!InitialiseLevel(level_number, type)) {
        CurrentLevel = 0;
        return 0x700;
    }

    result = GameLoop(0);
    if (result == 0x500 || result == 0x400)
        return result;
    if (result == 0x700) {
        CurrentLevel = 0;
        return result;
    }

    if (level_complete) {
        if (gameflow.demoversion && gameflow.singlelevel)
            return 0x500;
        if (CurrentLevel == 0) {
            StartGameStub1();
            return 0x500;
        }
        S_FadeInInventory(1);
        return CurrentLevel | 0x600;
    }

    if (Inventory_Chosen) {
        if (Inventory_ExtraData[0] == 0) {
            S_LoadGame(&savegame, sizeof(SAVEGAME_INFO), Inventory_ExtraData[1]);
            return Inventory_ExtraData[1] | 0x100;
        }
        if (Inventory_ExtraData[0] == 1) {
            if (gameflow.play_any_level)
                return Inventory_ExtraData[1] + 1;
            return 1;
        }
    }
    return 0x500;
}

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

extern "C" {
extern int GF_PlayingFMV;
extern char **GF_picfilenames;
extern sint16 GF_LoadingPic;
extern char **GF_titlefilenames;
extern int HiResFlag;
extern unsigned char game_palette[768];
void TempVideoAdjust(int hires, double sizer);
void TempVideoRemove(void);
void FadeToPal(int nFrames, unsigned char *pPal);
void S_CDPlay(sint16 track, int mode);
void CreateMonoScreen(void);
void DrawMonoScreen(int r, int g, int b);
void DXTextureSetGreyScale(bool set);
void ShowStatsText(char *time, int type);
int S_LoadLevelFile(char *name, int number, int type);
int Level2World(int level);
int World2Level(int world);
void CreateStartInfo(int level_number);
void SortOutAdventureSave(int world);
int Inv_RemoveItem(int item_num);
void T_InitPrint(void);
int S_UpdateInput(void);
void S_InitialisePolyList(int clear);
sint32 GetDebouncedInput(sint32 in);
void ShowEndStatsText(void);
void T_DrawText(void);
void S_OutputPolyList(void);
int S_DumpScreen(void);
void FreePictureTextures(int *indices);
extern int input;
extern sint32 inputDB;
extern int reset_flag;
extern sint16 LevelSecrets[];
extern int GF_BonusLevelEnabled;
extern int CurPicTexIndices[5];
extern int OldPicTexIndices[5];
extern int nLoadedPictures;
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

extern "C" int S_FrontEndCheck(void)
{
    int i;
    REQUEST_INFO *req;
    int count;
    HANDLE file;
    char filename[80];
    char count_str[16];
    DWORD bytes;

    req = &Load_Game_Requester;
    Init_Requester(req);
    SavedGames = 0;
    for (i = 0; i < 16; i++) {
        wsprintf(filename, "savegame.%d", i);
        file = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (file != INVALID_HANDLE_VALUE) {
            ReadFile(file, filename, 75, &bytes, NULL);
            ReadFile(file, &count, sizeof(sint32), &bytes, NULL);
            CloseHandle(file);
            wsprintf(count_str, "%d", count);
            AddRequesterItem(req, filename, 2, count_str, 4);
            if (count > save_counter) {
                save_counter = count;
                req->selected = i;
            }
            saved_levels[i] = 1;
            SavedGames++;
        } else {
            AddRequesterItem(req, GF_PCStrings[15], 0, NULL, 0);
            saved_levels[i] = 0;
        }
    }
    memcpy(SaveGameReqFlags1, RequesterFlags1, 24 * sizeof(uint32));
    memcpy(SaveGameReqFlags2, RequesterFlags2, 24 * sizeof(uint32));
    save_counter++;
    return 1;
}

extern "C" int S_SaveGame(LPVOID data, int size, int slot)
{
    HANDLE file;
    char filename[80];
    char count_str[16];
    DWORD bytes;

    wsprintf(filename, "savegame.%d", slot);
    file = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file != INVALID_HANDLE_VALUE) {
        wsprintf(filename, "%s", GF_Level_Names[savegame.current_level]);
        WriteFile(file, filename, 75, &bytes, NULL);
        WriteFile(file, &save_counter, sizeof(sint32), &bytes, NULL);
        WriteFile(file, data, size, &bytes, NULL);
        CloseHandle(file);
        wsprintf(count_str, "%d", save_counter);
        ChangeRequesterItem(&Load_Game_Requester, slot, filename, 2, count_str, 4);
        saved_levels[slot] = 1;
        save_counter++;
        SavedGames++;
        return 1;
    }
    return 0;
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
