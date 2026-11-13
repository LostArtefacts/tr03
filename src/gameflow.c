#include "specific/standard.h"
#include "game.h"

extern sint16 *GF_level_sequence_list[];
extern int title_loaded;

extern char **GF_GameStrings;
extern char **GF_PCStrings;
extern sint16 GF_SunsetEnabled;
extern INVENTORY_ITEM icompass_option;
extern INVENTORY_ITEM igun_option;
extern INVENTORY_ITEM iflare_option;
extern INVENTORY_ITEM ishotgun_option;
extern INVENTORY_ITEM imagnum_option;
extern INVENTORY_ITEM iuzi_option;
extern INVENTORY_ITEM iharpoon_option;
extern INVENTORY_ITEM im16_option;
extern INVENTORY_ITEM irocket_option;
extern INVENTORY_ITEM igrenade_option;
extern INVENTORY_ITEM igunammo_option;
extern INVENTORY_ITEM isgunammo_option;
extern INVENTORY_ITEM imagammo_option;
extern INVENTORY_ITEM iuziammo_option;
extern INVENTORY_ITEM iharpoonammo_option;
extern INVENTORY_ITEM im16ammo_option;
extern INVENTORY_ITEM irocketammo_option;
extern INVENTORY_ITEM igrenadeammo_option;
extern INVENTORY_ITEM imedi_option;
extern INVENTORY_ITEM ibigmedi_option;
extern INVENTORY_ITEM ipickup1_option;
extern INVENTORY_ITEM ipickup2_option;
extern INVENTORY_ITEM ipuzzle1_option;
extern INVENTORY_ITEM ipuzzle2_option;
extern INVENTORY_ITEM ipuzzle3_option;
extern INVENTORY_ITEM ipuzzle4_option;
extern INVENTORY_ITEM ikey1_option;
extern INVENTORY_ITEM ikey2_option;
extern INVENTORY_ITEM ikey3_option;
extern INVENTORY_ITEM ikey4_option;
extern INVENTORY_ITEM ipassport_option;
extern INVENTORY_ITEM igamma_option;
extern INVENTORY_ITEM icon1_option;
extern INVENTORY_ITEM icon2_option;
extern INVENTORY_ITEM icon3_option;
extern INVENTORY_ITEM icon4_option;
extern INVENTORY_ITEM sgcrystal_option;
extern INVENTORY_ITEM idetail_option;
extern INVENTORY_ITEM isound_option;
extern INVENTORY_ITEM icontrol_option;
extern INVENTORY_ITEM iphoto_option;
extern REQUEST_INFO Load_Game_Requester;
extern REQUEST_INFO Level_Select_Requester;

int S_LoadGameFlow(char *name);
void SetRequesterHeading(REQUEST_INFO *req, char *text1, uint32 flags1, char *text2, uint32 flags2);

int GF_LoadScriptFile(char *fname)
{
    GF_SunsetEnabled = 0;
    if (!S_LoadGameFlow(fname))
        return 0;

    gameflow.dozy_cheat_enabled = 1;
    gameflow.stats_track = 14;

    icompass_option.itemText = GF_GameStrings[35];
    igun_option.itemText = GF_GameStrings[36];
    iflare_option.itemText = GF_GameStrings[44];
    ishotgun_option.itemText = GF_GameStrings[37];
    imagnum_option.itemText = GF_GameStrings[38];
    iuzi_option.itemText = GF_GameStrings[39];
    iharpoon_option.itemText = GF_GameStrings[40];
    im16_option.itemText = GF_GameStrings[41];
    irocket_option.itemText = GF_GameStrings[42];
    igrenade_option.itemText = GF_GameStrings[43];
    igunammo_option.itemText = GF_GameStrings[45];
    isgunammo_option.itemText = GF_GameStrings[46];
    imagammo_option.itemText = GF_GameStrings[47];
    iuziammo_option.itemText = GF_GameStrings[48];
    iharpoonammo_option.itemText = GF_GameStrings[49];
    im16ammo_option.itemText = GF_GameStrings[50];
    irocketammo_option.itemText = GF_GameStrings[51];
    igrenadeammo_option.itemText = GF_GameStrings[52];

    imedi_option.itemText = GF_GameStrings[53];
    ibigmedi_option.itemText = GF_GameStrings[54];
    ipickup1_option.itemText = GF_GameStrings[55];
    ipickup2_option.itemText = GF_GameStrings[55];
    ipuzzle1_option.itemText = GF_GameStrings[56];
    ipuzzle2_option.itemText = GF_GameStrings[56];
    ipuzzle3_option.itemText = GF_GameStrings[56];
    ipuzzle4_option.itemText = GF_GameStrings[56];
    ikey1_option.itemText = GF_GameStrings[57];
    ikey2_option.itemText = GF_GameStrings[57];
    ikey3_option.itemText = GF_GameStrings[57];
    ikey4_option.itemText = GF_GameStrings[57];
    ipassport_option.itemText = GF_GameStrings[58];
    igamma_option.itemText = GF_GameStrings[90];

    icon1_option.itemText = GF_GameStrings[80];
    icon2_option.itemText = GF_GameStrings[81];
    icon3_option.itemText = GF_GameStrings[82];
    icon4_option.itemText = GF_GameStrings[83];
    sgcrystal_option.itemText = GF_GameStrings[84];
    idetail_option.itemText = GF_PCStrings[0];
    isound_option.itemText = GF_PCStrings[2];
    icontrol_option.itemText = GF_PCStrings[3];
    iphoto_option.itemText = GF_GameStrings[59];
    SetRequesterHeading(&Load_Game_Requester, GF_GameStrings[11], 0, NULL, 0);
    SetRequesterHeading(&Level_Select_Requester, GF_GameStrings[11], 0, NULL, 0);
    return 1;
}

sint32 GF_DoFrontEndSequence(void)
{
    sint32 option;

    option = GF_InterpretSequence(GF_frontendSequence, IL_NORMAL, 1);
    if (option == EXITGAME)
        return 1;
    return 0;
}

sint32 GF_DoLevelSequence(int level, int type)
{
    sint16 *ptr;
    sint32 option;

    do
    {
        if (level > gameflow.num_levels - 1)
        {
            title_loaded = 0;
            return EXIT_TO_TITLE;
        }
        ptr = GF_level_sequence_list[level];
        option = GF_InterpretSequence(ptr, type, 0);
        level++;
        if (gameflow.singlelevel >= 0)
            return option;
    } while ((option & ~0xff) == LEVELCOMPLETE);
    return option;
}

#define MT_START 0
#define MT_SECRET 1

#define ADDINV_PISTOLS 0
#define ADDINV_SHOTGUN 1
#define ADDINV_AUTOPISTOLS 2
#define ADDINV_UZIS 3
#define ADDINV_HARPOON 4
#define ADDINV_M16 5
#define ADDINV_ROCKET 6
#define ADDINV_GRENADE 7
#define ADDINV_PISTOL_AMMO 8
#define ADDINV_SHOTGUN_AMMO 9
#define ADDINV_AUTOPISTOLS_AMMO 10
#define ADDINV_UZI_AMMO 11
#define ADDINV_HARPOON_AMMO 12
#define ADDINV_M16_AMMO 13
#define ADDINV_ROCKET_AMMO 14
#define ADDINV_GRENADE_AMMO 15
#define ADDINV_FLARES 16
#define ADDINV_MEDI 17
#define ADDINV_BIGMEDI 18
#define ADDINV_PICKUP1 19
#define ADDINV_PICKUP2 20
#define ADDINV_PUZZLE1 21
#define ADDINV_PUZZLE2 22
#define ADDINV_PUZZLE3 23
#define ADDINV_PUZZLE4 24
#define ADDINV_KEY1 25
#define ADDINV_KEY2 26
#define ADDINV_KEY3 27
#define ADDINV_KEY4 28
#define ADDINV_SAVEGAME_CRYSTAL 29
#define ADDINV_NUMBEROF 30

#define SHOTGUN_AMMO_QTY 12
#define MAGNUM_AMMO_QTY 10
#define UZI_AMMO_QTY 40
#define HARPOON_AMMO_QTY 3
#define M16_AMMO_QTY 60
#define ROCKET_AMMO_QTY 1
#define GRENADE_AMMO_QTY 2

#define GFE_PICTURE 0
#define GFE_LIST_START 1
#define GFE_LIST_END 2
#define GFE_PLAYFMV 3
#define GFE_STARTLEVEL 4
#define GFE_CUTSCENE 5
#define GFE_LEVCOMPLETE 6
#define GFE_DEMOPLAY 7
#define GFE_JUMPTO_SEQ 8
#define GFE_END_SEQ 9
#define GFE_SETTRACK 10
#define GFE_SUNSET 11
#define GFE_LOADINGPIC 12
#define GFE_DEADLY_WATER 13
#define GFE_REMOVE_WEAPONS 14
#define GFE_GAMECOMPLETE 15
#define GFE_CUTANGLE 16
#define GFE_NOFLOOR 17
#define GFE_ADD2INV 18
#define GFE_STARTANIM 19
#define GFE_NUMSECRETS 20
#define GFE_KILL2COMPLETE 21
#define GFE_REMOVE_AMMO 22

#define LV_JUNGLE 1
#define LV_SHORE 5
#define LV_DESERT 13
#define LV_ROOFTOPS 9
#define LV_ANTARC 16

extern char GF_Add2InvItems[];
extern char GF_SecretInvItems[];
extern char **GF_picfilenames;
extern char **GF_fmvfilenames;
extern char **GF_cutscenefilenames;
extern uint32 CurrentLevel;
extern char GF_StartGame;
extern int GF_PlayingFMV;
extern int GF_BonusLevelEnabled;
extern sint16 GF_LoadingPic;
extern sint16 GF_NumSecrets;
extern sint16 GF_CDtracks[];
extern sint16 GF_Cutscene_Orientation;
extern int GF_LaraStartAnim;
extern sint16 GF_SunsetEnabled;
extern sint16 GF_DeadlyWater;
extern sint16 GF_NoFloor;
extern sint16 GF_RemoveWeapons;
extern sint16 GF_RemoveAmmo;
extern char GF_Kill2Complete;
extern sint16 NextAdventure;

void ShowThis(char *string, int frames);
int S_IntroFMV(char *movie1, char *movie2);
int S_PlayFMV(char *fname, int sequence);
sint32 StartGame(int level_number, int type);
void S_FadeToBlack(void);
void SetPictureToFade(int fade);
sint32 InitialiseLevel(sint32 level, sint32 type);
sint32 StartCinematic(sint32 level_number);
int LevelStats(int level);
sint32 Display_Inventory(sint32 inventory_mode);
void CreateStartInfo(int level_number);
void FadePictureDown(int steps);
sint32 StartDemo(int level_number);
void SetCutsceneTrack(int track);
void DisplayCredits(void);
sint32 GameStats(int level_number, int type);

sint32 GF_InterpretSequence(sint16 *ptr, int type, int seq_type)
{
    int adv;
    sint32 ret_val;
    sint32 option = EXIT_TO_TITLE;
    sint16 level;
    sint16 ntracks = 0;
    char string[80];

    GF_CDtracks[0] = 2;
    GF_LaraStartAnim = GF_SunsetEnabled = GF_DeadlyWater = GF_NoFloor = 0;
    GF_RemoveWeapons = GF_RemoveAmmo = GF_Kill2Complete = 0;
    GF_NumSecrets = 3;
    memset(GF_Add2InvItems, 0, ADDINV_NUMBEROF);
    memset(GF_SecretInvItems, 0, ADDINV_NUMBEROF);

    while (*ptr != GFE_END_SEQ) {
        switch (*ptr) {
        case GFE_STARTLEVEL:
            level = *(ptr + 1);
            if (level <= gameflow.num_levels) {
                if (type == IL_STORY_SO_FAR) {
                    ptr += 2;
                    continue;
                }
                if (type == IL_STORY_SO_FAR_MIDLEVEL)
                    return EXIT_TO_TITLE;
                option = StartGame(level, type);
                GF_StartGame = 0;
                if (type == IL_SAVED)
                    type = IL_NORMAL;
                if ((option & ~0xff) != LEVELCOMPLETE)
                    return option;
            } else {
                sprintf(string, "INVALID LEVEL %d", level);
                option = EXIT_TO_TITLE;
            }
            ptr += 2;
            break;

        case GFE_DEMOPLAY:
            if (type != IL_SAVED && type != IL_STORY_SO_FAR && type != IL_STORY_SO_FAR_MIDLEVEL) {
                option = StartDemo(*(ptr + 1));
                return option;
            }
            ptr += 2;
            break;

        case GFE_CUTANGLE:
            if (type != IL_SAVED)
                GF_Cutscene_Orientation = *(ptr + 1);
            ptr += 2;
            break;

        case GFE_CUTSCENE:
            if (type != IL_SAVED) {
                sprintf(string, "CUTSCENE %d %s", *(ptr + 1), GF_cutscenefilenames[*(ptr + 1)]);
                level = (sint16)CurrentLevel;
                S_FadeToBlack();
                SetPictureToFade(1);
                if (!InitialiseLevel(*(ptr + 1), IL_CUTSCENE))
                    ret_val = 2;
                else
                    ret_val = StartCinematic(*(ptr + 1));
                CurrentLevel = level;
                if (ret_val == 2 && (type == IL_STORY_SO_FAR || type == IL_STORY_SO_FAR_MIDLEVEL))
                    return EXIT_TO_TITLE;
                if (ret_val == 3)
                    return EXITGAME;
            }
            ptr += 2;
            break;

        case GFE_PLAYFMV:
            if (type != IL_SAVED) {
                if (*(ptr + 2) == GFE_PLAYFMV) {
                    if (S_IntroFMV(GF_fmvfilenames[*(ptr + 1)], GF_fmvfilenames[*(ptr + 3)]))
                        return EXITGAME;
                    ptr += 2;
                } else {
                    GF_PlayingFMV = 1;
                    if (S_PlayFMV(GF_fmvfilenames[*(ptr + 1)], *(ptr + 1)))
                        return EXITGAME;
                }
            }
            ptr += 2;
            break;

        case GFE_LEVCOMPLETE:
            if (type == IL_STORY_SO_FAR || type == IL_STORY_SO_FAR_MIDLEVEL) {
                ptr++;
                continue;
            }
            if ((adv = LevelStats(CurrentLevel)) == -1)
                return EXIT_TO_TITLE;
            if (adv) {
                sint16 lev;
                char Adventure[] = { LV_JUNGLE, LV_SHORE, LV_DESERT, LV_JUNGLE, LV_ROOFTOPS, LV_ANTARC };

                Display_Inventory(7);
                lev = Adventure[NextAdventure];
                option = STARTGAME | lev;
                CreateStartInfo(lev);
                FadePictureDown(32);
            } else {
                CreateStartInfo(CurrentLevel + 1);
                option = STARTGAME | (CurrentLevel + 1);
                savegame.current_level = (sint16)(CurrentLevel + 1);
            }
            ptr++;
            break;

        case GFE_GAMECOMPLETE:
            DisplayCredits();
            GameStats(CurrentLevel, type);
            if (GF_BonusLevelEnabled) {
                CreateStartInfo(CurrentLevel + 1);
                option = STARTGAME | (CurrentLevel + 1);
                savegame.current_level = (sint16)(CurrentLevel + 1);
                FadePictureDown(32);
                return option;
            }
            return EXIT_TO_TITLE;

        case GFE_PICTURE:
            if (type != IL_SAVED) {
                sprintf(string, "PICTURE %s", GF_picfilenames[*(ptr + 1)]);
                ShowThis(string, 30 * 5);
            }
            ptr += 2;
            break;

        case GFE_JUMPTO_SEQ:
            sprintf(string, "JUMPSEQ %d", *(ptr + 1));
            ptr += 2;
            break;

        case GFE_LIST_START:
            ptr++;
            break;

        case GFE_LIST_END:
            ptr++;
            break;

        case GFE_SETTRACK:
            GF_CDtracks[ntracks] = *(ptr + 1);
            SetCutsceneTrack(*(ptr + 1));
            ntracks++;
            ptr += 2;
            break;

        case GFE_SUNSET:
            if (type == IL_STORY_SO_FAR || type == IL_STORY_SO_FAR_MIDLEVEL) {
                ptr++;
                continue;
            }
            GF_SunsetEnabled = 1;
            ptr++;
            break;

        case GFE_DEADLY_WATER:
            if (type == IL_STORY_SO_FAR || type == IL_STORY_SO_FAR_MIDLEVEL) {
                ptr++;
                continue;
            }
            GF_DeadlyWater = 1;
            ptr++;
            break;

        case GFE_NOFLOOR:
            if (type == IL_STORY_SO_FAR || type == IL_STORY_SO_FAR_MIDLEVEL) {
                ptr += 2;
                continue;
            }
            GF_NoFloor = *(ptr + 1);
            ptr += 2;
            break;

        case GFE_STARTANIM:
            if (type == IL_STORY_SO_FAR || type == IL_STORY_SO_FAR_MIDLEVEL) {
                ptr += 2;
                continue;
            }
            GF_LaraStartAnim = *(ptr + 1);
            ptr += 2;
            break;

        case GFE_NUMSECRETS:
            if (type == IL_STORY_SO_FAR || type == IL_STORY_SO_FAR_MIDLEVEL) {
                ptr += 2;
                continue;
            }
            GF_NumSecrets = *(ptr + 1);
            ptr += 2;
            break;

        case GFE_ADD2INV:
            if (type == IL_STORY_SO_FAR || type == IL_STORY_SO_FAR_MIDLEVEL) {
                ptr += 2;
                continue;
            }
            if (*(ptr + 1) < 1000) {
                GF_SecretInvItems[*(ptr + 1)]++;
            } else {
                if (type != IL_SAVED)
                    GF_Add2InvItems[*(ptr + 1) - 1000]++;
            }
            ptr += 2;
            break;

        case GFE_REMOVE_WEAPONS:
            if (type == IL_STORY_SO_FAR || type == IL_STORY_SO_FAR_MIDLEVEL) {
                ptr++;
                continue;
            }
            if (type != IL_SAVED)
                GF_RemoveWeapons = 1;
            ptr++;
            break;

        case GFE_REMOVE_AMMO:
            if (type == IL_STORY_SO_FAR || type == IL_STORY_SO_FAR_MIDLEVEL) {
                ptr++;
                continue;
            }
            if (type != IL_SAVED)
                GF_RemoveAmmo = 1;
            ptr++;
            break;

        case GFE_KILL2COMPLETE:
            if (type == IL_STORY_SO_FAR || type == IL_STORY_SO_FAR_MIDLEVEL) {
                ptr++;
                continue;
            }
            GF_Kill2Complete = 1;
            ptr++;
            break;

        case GFE_LOADINGPIC:
            GF_LoadingPic = *(ptr + 1);
            ptr += 2;
            break;

        default:
            return EXITGAME;
        }
    }
    if (type == IL_STORY_SO_FAR || type == IL_STORY_SO_FAR_MIDLEVEL)
        return 0;
    return option;
}

int Inv_AddItem(int itemNum);
int Inv_RequestItem(int item_num);
void AddDisplayPickup(sint16 objnum);

void GF_ModifyInventory(int level, int type)
{
    int i;
    START_INFO *start = &savegame.start[level];

    if (!start->got_pistols && GF_Add2InvItems[ADDINV_PISTOLS]) {
        start->got_pistols = 1;
        Inv_AddItem(160);
    }

    if (Inv_RequestItem(161)) {
        if (type == MT_START)
            lara.shotgun.ammo += GF_Add2InvItems[ADDINV_SHOTGUN_AMMO] * SHOTGUN_AMMO_QTY;
        else {
            lara.shotgun.ammo += GF_SecretInvItems[ADDINV_SHOTGUN_AMMO] * SHOTGUN_AMMO_QTY;
            for (i = 0; i < GF_SecretInvItems[ADDINV_SHOTGUN_AMMO]; i++)
                AddDisplayPickup(169);
        }
    } else if ((type == MT_START && GF_Add2InvItems[ADDINV_SHOTGUN]) || (type == MT_SECRET && GF_SecretInvItems[ADDINV_SHOTGUN])) {
        start->got_shotgun = 1;
        Inv_AddItem(161);
        if (type == MT_START)
            lara.shotgun.ammo += GF_Add2InvItems[ADDINV_SHOTGUN_AMMO] * SHOTGUN_AMMO_QTY;
        else {
            lara.shotgun.ammo += GF_SecretInvItems[ADDINV_SHOTGUN_AMMO] * SHOTGUN_AMMO_QTY;
            AddDisplayPickup(161);
        }
    } else {
        if (type == MT_START) {
            for (i = 0; i < GF_Add2InvItems[ADDINV_SHOTGUN_AMMO]; i++)
                Inv_AddItem(169);
        } else {
            for (i = 0; i < GF_SecretInvItems[ADDINV_SHOTGUN_AMMO]; i++) {
                AddDisplayPickup(169);
                Inv_AddItem(169);
            }
        }
    }

    if (Inv_RequestItem(162)) {
        if (type == MT_START)
            lara.magnums.ammo += GF_Add2InvItems[ADDINV_AUTOPISTOLS_AMMO] * MAGNUM_AMMO_QTY;
        else {
            lara.magnums.ammo += GF_SecretInvItems[ADDINV_AUTOPISTOLS_AMMO] * MAGNUM_AMMO_QTY;
            for (i = 0; i < GF_SecretInvItems[ADDINV_AUTOPISTOLS_AMMO]; i++)
                AddDisplayPickup(170);
        }
    } else if ((type == MT_START && GF_Add2InvItems[ADDINV_AUTOPISTOLS]) || (type == MT_SECRET && GF_SecretInvItems[ADDINV_AUTOPISTOLS])) {
        start->got_magnums = 1;
        Inv_AddItem(162);
        if (type == MT_START)
            lara.magnums.ammo += GF_Add2InvItems[ADDINV_AUTOPISTOLS_AMMO] * MAGNUM_AMMO_QTY;
        else {
            AddDisplayPickup(162);
            lara.magnums.ammo += GF_Add2InvItems[ADDINV_AUTOPISTOLS_AMMO] * MAGNUM_AMMO_QTY;
            for (i = 0; i < GF_SecretInvItems[ADDINV_AUTOPISTOLS]; i++)
                AddDisplayPickup(170);
        }
    } else {
        if (type == MT_START) {
            for (i = 0; i < GF_Add2InvItems[ADDINV_AUTOPISTOLS_AMMO]; i++)
                Inv_AddItem(170);
        } else {
            for (i = 0; i < GF_SecretInvItems[ADDINV_AUTOPISTOLS_AMMO]; i++) {
                Inv_AddItem(170);
                AddDisplayPickup(170);
            }
        }
    }

    if (Inv_RequestItem(163)) {
        if (type == MT_START)
            lara.uzis.ammo += GF_Add2InvItems[ADDINV_UZI_AMMO] * UZI_AMMO_QTY;
        else {
            lara.uzis.ammo += GF_SecretInvItems[ADDINV_UZI_AMMO] * UZI_AMMO_QTY;
            for (i = 0; i < GF_SecretInvItems[ADDINV_UZI_AMMO]; i++)
                AddDisplayPickup(171);
        }
    } else if ((type == MT_START && GF_Add2InvItems[ADDINV_UZIS]) || (type == MT_SECRET && GF_SecretInvItems[ADDINV_UZIS])) {
        start->got_uzis = 1;
        Inv_AddItem(163);
        if (type == MT_START)
            lara.uzis.ammo += GF_Add2InvItems[ADDINV_UZI_AMMO] * UZI_AMMO_QTY;
        else {
            AddDisplayPickup(163);
            lara.uzis.ammo += GF_SecretInvItems[ADDINV_UZI_AMMO] * UZI_AMMO_QTY;
            for (i = 0; i < GF_SecretInvItems[ADDINV_UZI_AMMO]; i++)
                AddDisplayPickup(171);
        }
    } else {
        if (type == MT_START) {
            for (i = 0; i < GF_Add2InvItems[ADDINV_UZI_AMMO]; i++)
                Inv_AddItem(171);
        } else {
            for (i = 0; i < GF_SecretInvItems[ADDINV_UZI_AMMO]; i++) {
                Inv_AddItem(171);
                AddDisplayPickup(171);
            }
        }
    }

    if (Inv_RequestItem(164)) {
        if (type == MT_START)
            lara.harpoon.ammo += GF_Add2InvItems[ADDINV_HARPOON_AMMO] * HARPOON_AMMO_QTY;
        else {
            lara.harpoon.ammo += GF_SecretInvItems[ADDINV_HARPOON_AMMO] * HARPOON_AMMO_QTY;
            for (i = 0; i < GF_SecretInvItems[ADDINV_HARPOON_AMMO]; i++)
                AddDisplayPickup(172);
        }
    } else if ((type == MT_START && GF_Add2InvItems[ADDINV_HARPOON]) || (type == MT_SECRET && GF_SecretInvItems[ADDINV_HARPOON])) {
        start->got_harpoon = 1;
        Inv_AddItem(164);
        if (type == MT_START)
            lara.harpoon.ammo += GF_Add2InvItems[ADDINV_HARPOON_AMMO] * HARPOON_AMMO_QTY;
        else {
            AddDisplayPickup(164);
            lara.harpoon.ammo += GF_SecretInvItems[ADDINV_HARPOON_AMMO] * HARPOON_AMMO_QTY;
            for (i = 0; i < GF_SecretInvItems[ADDINV_HARPOON_AMMO]; i++)
                AddDisplayPickup(172);
        }
    } else {
        if (type == MT_START) {
            for (i = 0; i < GF_Add2InvItems[ADDINV_HARPOON_AMMO]; i++) {
                if (i == 0) {
                    Inv_AddItem(172);
                    lara.harpoon.ammo -= HARPOON_AMMO_QTY;
                }
                lara.harpoon.ammo += HARPOON_AMMO_QTY;
            }
        } else {
            for (i = 0; i < GF_SecretInvItems[ADDINV_HARPOON_AMMO]; i++) {
                if (i == 0) {
                    Inv_AddItem(172);
                    lara.harpoon.ammo -= HARPOON_AMMO_QTY;
                }
                lara.harpoon.ammo += HARPOON_AMMO_QTY;
                AddDisplayPickup(172);
            }
        }
    }

    if (Inv_RequestItem(165)) {
        if (type == MT_START)
            lara.m16.ammo += GF_Add2InvItems[ADDINV_M16_AMMO] * M16_AMMO_QTY;
        else {
            lara.m16.ammo += GF_SecretInvItems[ADDINV_M16_AMMO] * M16_AMMO_QTY;
            for (i = 0; i < GF_SecretInvItems[ADDINV_M16_AMMO]; i++)
                AddDisplayPickup(173);
        }
    } else if ((type == MT_START && GF_Add2InvItems[ADDINV_M16]) || (type == MT_SECRET && GF_SecretInvItems[ADDINV_M16])) {
        start->got_m16 = 1;
        Inv_AddItem(165);
        if (type == MT_START)
            lara.m16.ammo += GF_Add2InvItems[ADDINV_M16_AMMO] * M16_AMMO_QTY;
        else {
            AddDisplayPickup(165);
            lara.m16.ammo += GF_SecretInvItems[ADDINV_M16_AMMO] * M16_AMMO_QTY;
            for (i = 0; i < GF_SecretInvItems[ADDINV_M16_AMMO]; i++)
                AddDisplayPickup(173);
        }
    } else {
        if (type == MT_START) {
            for (i = 0; i < GF_Add2InvItems[ADDINV_M16_AMMO]; i++)
                Inv_AddItem(173);
        } else {
            for (i = 0; i < GF_SecretInvItems[ADDINV_M16_AMMO]; i++) {
                Inv_AddItem(173);
                AddDisplayPickup(173);
            }
        }
    }

    if (Inv_RequestItem(166)) {
        if (type == MT_START)
            lara.rocket.ammo += GF_Add2InvItems[ADDINV_ROCKET_AMMO] * ROCKET_AMMO_QTY;
        else {
            lara.rocket.ammo += GF_SecretInvItems[ADDINV_ROCKET_AMMO] * ROCKET_AMMO_QTY;
            for (i = 0; i < GF_SecretInvItems[ADDINV_ROCKET_AMMO]; i++)
                AddDisplayPickup(174);
        }
    } else if ((type == MT_START && GF_Add2InvItems[ADDINV_ROCKET]) || (type == MT_SECRET && GF_SecretInvItems[ADDINV_ROCKET])) {
        start->got_rocket = 1;
        Inv_AddItem(166);
        if (type == MT_START)
            lara.rocket.ammo += GF_Add2InvItems[ADDINV_ROCKET_AMMO] * ROCKET_AMMO_QTY;
        else {
            lara.rocket.ammo += GF_SecretInvItems[ADDINV_ROCKET_AMMO] * ROCKET_AMMO_QTY;
            AddDisplayPickup(166);
            for (i = 0; i < GF_SecretInvItems[ADDINV_ROCKET_AMMO]; i++)
                AddDisplayPickup(174);
        }
    } else {
        if (type == MT_START) {
            for (i = 0; i < GF_Add2InvItems[ADDINV_ROCKET_AMMO]; i++)
                Inv_AddItem(174);
        } else {
            for (i = 0; i < GF_SecretInvItems[ADDINV_ROCKET_AMMO]; i++) {
                Inv_AddItem(174);
                AddDisplayPickup(174);
            }
        }
    }

    if (Inv_RequestItem(167)) {
        if (type == MT_START)
            lara.grenade.ammo += GF_Add2InvItems[ADDINV_GRENADE_AMMO] * GRENADE_AMMO_QTY;
        else {
            lara.grenade.ammo += GF_SecretInvItems[ADDINV_GRENADE_AMMO] * GRENADE_AMMO_QTY;
            for (i = 0; i < GF_SecretInvItems[ADDINV_GRENADE_AMMO]; i++)
                AddDisplayPickup(175);
        }
    } else if ((type == MT_START && GF_Add2InvItems[ADDINV_GRENADE]) || (type == MT_SECRET && GF_SecretInvItems[ADDINV_GRENADE])) {
        start->got_grenade = 1;
        Inv_AddItem(167);
        if (type == MT_START)
            lara.grenade.ammo += GF_Add2InvItems[ADDINV_GRENADE_AMMO] * GRENADE_AMMO_QTY;
        else {
            lara.grenade.ammo += GF_SecretInvItems[ADDINV_GRENADE_AMMO] * GRENADE_AMMO_QTY;
            AddDisplayPickup(167);
            for (i = 0; i < GF_SecretInvItems[ADDINV_GRENADE_AMMO]; i++)
                AddDisplayPickup(175);
        }
    } else {
        if (type == MT_START) {
            for (i = 0; i < GF_Add2InvItems[ADDINV_GRENADE_AMMO]; i++)
                Inv_AddItem(175);
        } else {
            for (i = 0; i < GF_SecretInvItems[ADDINV_GRENADE_AMMO]; i++) {
                Inv_AddItem(175);
                AddDisplayPickup(175);
            }
        }
    }

    if (type == MT_START) {
        for (i = 0; i < GF_Add2InvItems[ADDINV_FLARES]; i++)
            Inv_AddItem(179);
        for (i = 0; i < GF_Add2InvItems[ADDINV_MEDI]; i++)
            Inv_AddItem(176);
        for (i = 0; i < GF_Add2InvItems[ADDINV_BIGMEDI]; i++)
            Inv_AddItem(177);
        for (i = 0; i < GF_Add2InvItems[ADDINV_PICKUP1]; i++)
            Inv_AddItem(236);
        for (i = 0; i < GF_Add2InvItems[ADDINV_PICKUP2]; i++)
            Inv_AddItem(237);
        for (i = 0; i < GF_Add2InvItems[ADDINV_PUZZLE1]; i++)
            Inv_AddItem(205);
        for (i = 0; i < GF_Add2InvItems[ADDINV_PUZZLE2]; i++)
            Inv_AddItem(206);
        for (i = 0; i < GF_Add2InvItems[ADDINV_PUZZLE3]; i++)
            Inv_AddItem(207);
        for (i = 0; i < GF_Add2InvItems[ADDINV_PUZZLE4]; i++)
            Inv_AddItem(208);
        for (i = 0; i < GF_Add2InvItems[ADDINV_KEY1]; i++)
            Inv_AddItem(224);
        for (i = 0; i < GF_Add2InvItems[ADDINV_KEY2]; i++)
            Inv_AddItem(225);
        for (i = 0; i < GF_Add2InvItems[ADDINV_KEY3]; i++)
            Inv_AddItem(226);
        for (i = 0; i < GF_Add2InvItems[ADDINV_KEY4]; i++)
            Inv_AddItem(227);
        for (i = 0; i < GF_Add2InvItems[ADDINV_SAVEGAME_CRYSTAL]; i++)
            Inv_AddItem(180);
        for (i = 0; i < ADDINV_NUMBEROF; i++)
            GF_Add2InvItems[i] = 0;
    } else {
        for (i = 0; i < GF_SecretInvItems[ADDINV_FLARES]; i++) {
            Inv_AddItem(179);
            AddDisplayPickup(179);
        }
        for (i = 0; i < GF_SecretInvItems[ADDINV_MEDI]; i++) {
            Inv_AddItem(176);
            AddDisplayPickup(176);
        }
        for (i = 0; i < GF_SecretInvItems[ADDINV_BIGMEDI]; i++) {
            Inv_AddItem(177);
            AddDisplayPickup(177);
        }
        for (i = 0; i < GF_SecretInvItems[ADDINV_PICKUP1]; i++) {
            Inv_AddItem(236);
            AddDisplayPickup(236);
        }
        for (i = 0; i < GF_SecretInvItems[ADDINV_PICKUP2]; i++) {
            Inv_AddItem(237);
            AddDisplayPickup(237);
        }
        for (i = 0; i < GF_SecretInvItems[ADDINV_PUZZLE1]; i++) {
            Inv_AddItem(205);
            AddDisplayPickup(205);
        }
        for (i = 0; i < GF_SecretInvItems[ADDINV_PUZZLE2]; i++) {
            Inv_AddItem(206);
            AddDisplayPickup(206);
        }
        for (i = 0; i < GF_SecretInvItems[ADDINV_PUZZLE3]; i++) {
            Inv_AddItem(207);
            AddDisplayPickup(207);
        }
        for (i = 0; i < GF_SecretInvItems[ADDINV_PUZZLE4]; i++) {
            Inv_AddItem(208);
            AddDisplayPickup(208);
        }
        for (i = 0; i < GF_SecretInvItems[ADDINV_KEY1]; i++) {
            Inv_AddItem(224);
            AddDisplayPickup(224);
        }
        for (i = 0; i < GF_SecretInvItems[ADDINV_KEY2]; i++) {
            Inv_AddItem(225);
            AddDisplayPickup(225);
        }
        for (i = 0; i < GF_SecretInvItems[ADDINV_KEY3]; i++) {
            Inv_AddItem(226);
            AddDisplayPickup(226);
        }
        for (i = 0; i < GF_SecretInvItems[ADDINV_KEY4]; i++) {
            Inv_AddItem(227);
            AddDisplayPickup(227);
        }
        for (i = 0; i < ADDINV_NUMBEROF; i++)
            GF_SecretInvItems[i] = 0;
    }
}

#ifdef TR3_EXE
void ShowThis(char *string, int frames)
{
}
#endif
