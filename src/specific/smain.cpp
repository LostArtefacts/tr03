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

sint32 GameMain(void)
{
    sint32 option, level_number, ok;

    game_hires = HiResFlag = 0;
    game_sizer = screen_sizer = 1.0;

    if (!S_InitialiseSystem())
        return 0;

    if (!GF_LoadScriptFile("data\\tombPC.dat"))
        S_ExitSystem("GameMain: could not load script file");

    SOUND_Init();

    InitialiseStartInfo();

    S_FrontEndCheck(&savegame, sizeof(SAVEGAME_INFO));

    HiResFlag = -1;
    malloc_buffer = (char *)GlobalAlloc(GMEM_FIXED, 0x380000);
    if (!malloc_buffer) {
        lstrcpy(exit_message, "GameMain: could not allocate malloc_buffer");
        return 0;
    }

    HiResFlag = 0;
    TempVideoAdjust(1, 1.0);

    S_UpdateInput();
    LoadPicture("pix/legal.bmp", App.lpPictureBuffer, 1);
    FadePictureUp(32);
    S_Wait(300, 1);
    ForceFadeDown(1);
    FadePictureDown(32);

    int n = GF_DoFrontEndSequence();
    if (GtWindowClosed)
        return 1;
    if (n == 1) {
        lstrcpy(exit_message, "GameMain: failed in GF_DoFrontEndSequence()");
        return 0;
    }

    title_loaded = 0;

    option = gameflow.firstOption;

    ok = 1;
    while (ok) {
        level_number = option & 0xff;
        option &= ~0xff;

        switch (option) {
        case EXIT_TO_OPTION:
        case EXIT_TO_TITLE:
            if (!gameflow.title_disabled) {
                option = TitleSequence();
                GF_StartGame = 1;
            } else if (gameflow.title_replace >= 0 && gameflow.title_replace != EXIT_TO_TITLE) {
                option = gameflow.title_replace;
            } else {
                lstrcpy(exit_message, "GameMain Failed: Title disabled & no replacement");
                return 0;
            }
            break;
        case STARTGAME:
            if (gameflow.singlelevel >= 0)
                option = GF_DoLevelSequence(gameflow.singlelevel, IL_NORMAL);
            else {
                if (level_number <= gameflow.num_levels) {
                    option = GF_DoLevelSequence(level_number, IL_NORMAL);
                    break;
                } else {
                    wsprintf(exit_message, "GameMain: STARTGAME with invalid level number (%d)", level_number);
                    return 0;
                }
            }
            break;

        case STARTSAVEDGAME:
            S_LoadGame(&savegame, sizeof(SAVEGAME_INFO), level_number);
            level_number = savegame.current_level;
            if (level_number <= gameflow.num_levels)
                option = GF_DoLevelSequence(level_number, IL_SAVED);
            else {
                wsprintf(exit_message, "GameMain: STARTSAVEDGAME with invalid level number (%d)", level_number);
                return 0;
            }
            break;
        case STARTCINE:
            StartCinematic(level_number);
            option = EXIT_TO_TITLE;
            break;
        case STARTDEMO:
            option = DoDemoSequence(-1);
            break;
        case LEVELCOMPLETE:
            option = LevelCompleteSequence(level_number);
            break;
        case EXITGAME:
            ok = 0;
            break;
        default:
            ok = 0;
            break;
        }
    }

    if (nLoadedPictures) {
        ForceFadeDown(1);
        FadePictureDown(32);
    }

    S_SaveSettings();

    ShutdownGame();

    return 1;
}

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

extern uint32 CurrentLevel;
extern int FinishLevelCheat;
int Inv_AddItem(int item_num);
int ExplodingDeath(sint16 item_number, sint32 mesh_bits, sint16 damage);

void CheckCheatMode(void)
{
    static int cheat_mode = 0, cheat_flare, cheat_turn;
    static sint16 as, cheat_angle;

    if (CurrentLevel == 0 || CurrentLevel == (uint32)(gameflow.num_levels - gameflow.num_demos - 1))
        return;

    as = lara_item->current_anim_state;
    switch (cheat_mode) {
    case 0:
        if (as == AS_WALK)
            cheat_mode = 1;
        break;
    case 1:
        cheat_flare = (lara.gun_type == LG_PISTOLS);

        if (as != AS_WALK)
            cheat_mode = (as == AS_STOP) ? 2 : 0;
        break;
    case 2:
        if (as != AS_STOP)
            cheat_mode = (as == AS_DUCK) ? 3 : 0;
        break;
    case 3:
        if (as != AS_DUCK)
            cheat_mode = (as == AS_STOP) ? 4 : 0;
        break;
    case 4:
        if (as != AS_STOP) {
            cheat_angle = lara_item->pos.y_rot;
            cheat_turn = 0;
            if (as == AS_TURN_L)
                cheat_mode = 5;
            else if (as == AS_TURN_R)
                cheat_mode = 6;
            else
                cheat_mode = 0;
        }
        break;
    case 5:
        if (as != AS_TURN_L && as != AS_FASTTURN)
            cheat_mode = (cheat_turn < -0x17000) ? 7 : 0;
        else {
            cheat_turn += (sint16)(lara_item->pos.y_rot - cheat_angle);
            cheat_angle = lara_item->pos.y_rot;
        }
        break;
    case 6:
        if (as != AS_TURN_R && as != AS_FASTTURN)
            cheat_mode = (cheat_turn > 0x17000) ? 7 : 0;
        else {
            cheat_turn += (sint16)(lara_item->pos.y_rot - cheat_angle);
            cheat_angle = lara_item->pos.y_rot;
        }
        break;
    case 7:
        if (as != AS_STOP) {
            if (as == AS_COMPRESS) {
                cheat_mode = 8;
            } else
                cheat_mode = 0;
        }
        break;
    case 8:
        if (lara_item->fallspeed > 0) {
            if (cheat_flare)
                cheat_flare = (lara.gun_type == LG_PISTOLS);
            if (as == AS_FORWARDJUMP && cheat_flare) {
                FinishLevelCheat = 1;
            } else if (as == AS_BACKJUMP && cheat_flare) {
                int i;
                Inv_AddItem(165);
                Inv_AddItem(161);
                Inv_AddItem(163);
                Inv_AddItem(162);
                Inv_AddItem(160);
                Inv_AddItem(166);
                Inv_AddItem(167);
                Inv_AddItem(164);
                lara.magnums.ammo = 1000;
                lara.uzis.ammo = 1000;
                lara.shotgun.ammo = 1000;
                lara.harpoon.ammo = 1000;
                lara.rocket.ammo = 1000;
                lara.grenade.ammo = 1000;
                lara.m16.ammo = 1000;
                for (i = 0; i < 50; i++) {
                    Inv_AddItem(176);
                    Inv_AddItem(177);
                    Inv_AddItem(179);
                }
                SoundEffect(7, NULL, 2);
            } else if (as == AS_FORWARDJUMP || as == AS_BACKJUMP) {
                ExplodingDeath(lara.item_number, 0xffffffff, 1);
                lara_item->hit_points = 0;
                lara_item->flags |= ONESHOT;
            }
            cheat_mode = 0;
        }
        break;
    default:
        cheat_mode = 0;
    }
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
