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
extern sint16 Option_SFX_Volume;
extern sint16 Option_Music_Volume;
extern float GammaOption;
extern sint16 layout[2][14];
void S_CDVolume(int volume);
void S_SoundSetMasterVolume(int volume);

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

void S_SaveSettings(void)
{
    FILE *file;

    file = fopen("config.txt", "w+");
    if (file) {
        if (App.DeviceInfoPtr->nDDInfo)
            fprintf(file, "DD %d\n", App.DXConfigPtr->nDD);
        if (App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].nD3DInfo) {
            fprintf(file, "D3D %d\n", App.DXConfigPtr->nD3D);
            fprintf(file, "D3DTF %d\n", App.DXConfigPtr->D3DTF);
        }
        if (App.DeviceInfoPtr->nDSInfo)
            fprintf(file, "DS %d\n", App.DXConfigPtr->DS);
        if (App.DeviceInfoPtr->nDIInfo)
            fprintf(file, "DI %d\n", App.DXConfigPtr->DI);
        fprintf(file, "VM %d\n", App.DXConfigPtr->nVMode);
        fprintf(file, "ZBUFFER %d\n", App.DXConfigPtr->bZBuffer);
        fprintf(file, "DITHER %d\n", App.DXConfigPtr->Dither);
        fprintf(file, "FILTER %d\n", App.DXConfigPtr->Filter);
        fprintf(file, "AGP %d\n", App.DXConfigPtr->AGP);
        fprintf(file, "SOUND %d\n", App.DXConfigPtr->sound);
        fprintf(file, "JOYSTICK %d\n", App.DXConfigPtr->Joystick);
        fprintf(file, "MMX %d\n", App.DXConfigPtr->MMX);
        fprintf(file, "SFX %d\n", Option_SFX_Volume);
        fprintf(file, "MUSIC %d\n", Option_Music_Volume);
        fprintf(file, "GAMMA %f\n", GammaOption);
        fprintf(file, "DEFKEY %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
            layout[0][0],
            layout[0][1],
            layout[0][2],
            layout[0][3],
            layout[0][4],
            layout[0][5],
            layout[0][6],
            layout[0][7],
            layout[0][8],
            layout[0][9],
            layout[0][10],
            layout[0][11],
            layout[0][12],
            layout[0][13]);
        fprintf(file, "USERKEY %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
            layout[1][0],
            layout[1][1],
            layout[1][2],
            layout[1][3],
            layout[1][4],
            layout[1][5],
            layout[1][6],
            layout[1][7],
            layout[1][8],
            layout[1][9],
            layout[1][10],
            layout[1][11],
            layout[1][12],
            layout[1][13]);
        fclose(file);
    }
    file = fopen("data.bin", "wb+");
    if (file) {
        fwrite(savegame.best_assault_times, sizeof(uint32), 10, file);
        fwrite(savegame.best_quadbike_times, sizeof(uint32), 10, file);
        fwrite(&savegame.QuadbikeKeyFlag, sizeof(uint32), 1, file);
    }
}

bool S_LoadSettings(void)
{
    FILE *file;
    char s[80];

    file = fopen("data.bin", "rb");
    if (file) {
        fread(savegame.best_assault_times, sizeof(uint32), 10, file);
        fread(savegame.best_quadbike_times, sizeof(uint32), 10, file);
        fread(&savegame.QuadbikeKeyFlag, sizeof(uint32), 1, file);
        fclose(file);
    }
    file = fopen("config.txt", "r");
    if (file) {
        if (App.DeviceInfoPtr->nDDInfo) {
            fscanf(file, "%s", s);
            fscanf(file, "%d\n", &App.DXConfigPtr->nDD);
        }
        if (App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].nD3DInfo) {
            fscanf(file, "%s", s);
            fscanf(file, "%d\n", &App.DXConfigPtr->nD3D);
            fscanf(file, "%s", s);
            fscanf(file, "%d\n", &App.DXConfigPtr->D3DTF);
        }
        if (App.DeviceInfoPtr->nDSInfo) {
            fscanf(file, "%s", s);
            fscanf(file, "%d\n", &App.DXConfigPtr->DS);
        }
        if (App.DeviceInfoPtr->nDIInfo) {
            fscanf(file, "%s", s);
            fscanf(file, "%d\n", &App.DXConfigPtr->DI);
        }
        fscanf(file, "%s", s);
        fscanf(file, "%d\n", &App.DXConfigPtr->nVMode);
        fscanf(file, "%s", s);
        fscanf(file, "%d\n", &App.DXConfigPtr->bZBuffer);
        fscanf(file, "%s", s);
        fscanf(file, "%d\n", &App.DXConfigPtr->Dither);
        fscanf(file, "%s", s);
        fscanf(file, "%d\n", &App.DXConfigPtr->Filter);
        fscanf(file, "%s", s);
        fscanf(file, "%d\n", &App.DXConfigPtr->AGP);
        fscanf(file, "%s", s);
        fscanf(file, "%d\n", &App.DXConfigPtr->sound);
        fscanf(file, "%s", s);
        fscanf(file, "%d\n", &App.DXConfigPtr->Joystick);
        fscanf(file, "%s", s);
        fscanf(file, "%d\n", &App.DXConfigPtr->MMX);
        fscanf(file, "%s", s);
        fscanf(file, "%d\n", &Option_SFX_Volume);
        fscanf(file, "%s", s);
        fscanf(file, "%d\n", &Option_Music_Volume);
        fscanf(file, "%s", s);
        fscanf(file, "%f\n", &GammaOption);
        fscanf(file, "%s", s);
        fscanf(file, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
            &layout[0][0],
            &layout[0][1],
            &layout[0][2],
            &layout[0][3],
            &layout[0][4],
            &layout[0][5],
            &layout[0][6],
            &layout[0][7],
            &layout[0][8],
            &layout[0][9],
            &layout[0][10],
            &layout[0][11],
            &layout[0][12],
            &layout[0][13]);
        fscanf(file, "%s", s);
        fscanf(file, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
            &layout[1][0],
            &layout[1][1],
            &layout[1][2],
            &layout[1][3],
            &layout[1][4],
            &layout[1][5],
            &layout[1][6],
            &layout[1][7],
            &layout[1][8],
            &layout[1][9],
            &layout[1][10],
            &layout[1][11],
            &layout[1][12],
            &layout[1][13]);
        S_CDVolume(25 * Option_Music_Volume + 5);
        S_SoundSetMasterVolume(6 * Option_SFX_Volume + 4);
        fclose(file);
        return true;
    }
    Option_Music_Volume = 7;
    Option_SFX_Volume = 10;
    S_CDVolume(25 * Option_Music_Volume + 5);
    S_SoundSetMasterVolume(6 * Option_SFX_Volume + 4);
    return false;
}

}
