#include "standard.h"
#include "specific.h"
#include "winmain.h"

extern "C" {

#include "../game.h"
#include "../text.h"

TEXTSTRING *T_Print(sint32 xpos, sint32 ypos, sint32 zpos, char *string);
void T_AddBackground(TEXTSTRING *textString, sint16 xsize, sint16 ysize, sint16 xoff, sint16 yoff, sint16 zoff, sint16 colour, GOURAUD_FILL *gourptr, uint16 flags);

__declspec(dllimport) int __cdecl wsprintfA(char *, const char *, ...);
int sprintf(char *buf, const char *fmt, ...);

extern sint16 layout[2][14];
extern char *kname[];
extern int iconfig;
extern int keychange;
extern int select_key;
extern uint8 keymap[];
extern int joy_fire;
void S_ShowControls(void);
void S_ChangeCtrlText(void);
void S_RemoveCtrlText(void);
void FlashConflicts(void);
void DefaultConflict(void);
extern int conflict[14];
extern TEXTSTRING *ctext[14];
extern TEXTSTRING *btext[14];
extern TEXTSTRING *ctrltext[2];
extern sint32 inputDB;
extern int CurrentLevel;
extern char **GF_PCStrings;
extern TEXTSTRING *sa_text[4];
extern sint16 item_data;
extern sint16 Option_Music_Volume;
extern sint16 Option_SFX_Volume;
extern int idelay;
extern int idcount;
extern INVENTORY_SPRITE is_sound_muslevel[];
extern INVENTORY_SPRITE is_sound_sfxlevel[];
extern INVENTORY_SPRITE is_sound_muslevellow[];
extern INVENTORY_SPRITE is_sound_sfxlevellow[];
void S_CDVolume(int volume);
extern TEXTSTRING *AdventureText;
extern TEXTSTRING *LeftArrow;
extern TEXTSTRING *RightArrow;
extern sint32 GlobeButton;
extern sint16 GlobeLevelAngles[7][4];
extern sint16 GlobeXRot;
extern sint16 GlobeYRot;
extern sint16 GlobeZRot;
extern sint16 GlobeLevel;
extern sint16 NextAdventure;
extern uint8 GlobeLevelComplete[];
extern char **GF_GameStrings;
void S_SoundSetMasterVolume(int volume);
extern TEXTSTRING *Inv_tagText;
extern TEXTSTRING *Inv_ringText;
extern TEXTSTRING *passport_text1;
extern sint32 passport_mode;
extern sint32 Inventory_Mode;
extern sint32 Inventory_ExtraData[8];
extern int SavedGames;
extern REQUEST_INFO Load_Game_Requester;
extern REQUEST_INFO Level_Select_Requester;
void Init_Requester(REQUEST_INFO *req);
void Remove_Requester(REQUEST_INFO *req);
sint32 Display_Requester(REQUEST_INFO *req, int des, int backgrounds);
void SetRequesterHeading(REQUEST_INFO *req, char *text1, uint32 flags1, char *text2, uint32 flags2);
void SetPCRequesterSize(REQUEST_INFO *req, int maxlines, int ypos);
void GetSavedGamesList(REQUEST_INFO *req);
void GetValidLevelsList(REQUEST_INFO *req);

void do_levelselect_option(INVENTORY_ITEM *inv_item);
void do_passport_option(INVENTORY_ITEM *inv_item);
void do_detail_option(INVENTORY_ITEM *inv_item);
void do_sound_option(INVENTORY_ITEM *inv_item);
void do_compass_option(INVENTORY_ITEM *inv_item);
void do_control_option(INVENTORY_ITEM *inv_item);
void ShowGymStatsText(char *time, int type);
void ShowStatsText(char *time, int type);

void do_pickup_option(INVENTORY_ITEM *inv_item)
{
    if (inputDB & (IN_DESELECT | IN_SELECT)) {
        inv_item->goal_frame = inv_item->frames_total - 1;
        inv_item->anim_direction = 1;
    }
}

void do_inventory_options(INVENTORY_ITEM *inv_item)
{
    switch (inv_item->object_number) {
    case 146:
        do_compass_option(inv_item);
        break;
    case 185:
    case 186:
    case 187:
    case 188:
    case 189:
    case 190:
    case 191:
    case 192:
        inputDB |= IN_SELECT;
        break;
    case 193:
    case 194:
    case 195:
    case 196:
    case 197:
    case 198:
    case 199:
        break;
    case 201:
    case 202:
    case 209:
    case 210:
    case 211:
    case 212:
    case 228:
    case 229:
    case 230:
    case 231:
        inputDB |= IN_SELECT;
        break;
    case 238:
    case 239:
        do_pickup_option(inv_item);
        break;
    case 145:
        do_passport_option(inv_item);
        break;
    case 181:
        do_detail_option(inv_item);
        break;
    case 182:
        do_sound_option(inv_item);
        break;
    case 183:
        do_control_option(inv_item);
        break;
    case 184:
        do_levelselect_option(inv_item);
        break;
    default:
        if (inputDB & (IN_SELECT | IN_DESELECT)) {
            inv_item->goal_frame = 0;
            inv_item->anim_direction = -1;
        }
        break;
    }
}

void do_levelselect_option(INVENTORY_ITEM *inv_item)
{
    sint16 diff;
    sint16 set;
    int advs;
    int lp;
    int w;
    char la[2] = { 17, 0 };
    char ra[2] = { 18, 0 };

    T_RemovePrint(AdventureText);
    AdventureText = NULL;
    T_RemovePrint(LeftArrow);
    LeftArrow = NULL;
    T_RemovePrint(RightArrow);
    RightArrow = NULL;
    set = 0;
    diff = GlobeLevelAngles[GlobeLevel][0] - GlobeXRot;
    if (diff < 128 && diff > -128) {
        GlobeXRot = GlobeLevelAngles[GlobeLevel][0];
        set++;
    } else {
        GlobeXRot += diff >> 3;
    }
    diff = GlobeLevelAngles[GlobeLevel][1] - GlobeYRot;
    if (diff < 128 && diff > -128) {
        GlobeYRot = GlobeLevelAngles[GlobeLevel][1];
        set++;
    } else {
        GlobeYRot += diff >> 3;
    }
    diff = GlobeLevelAngles[GlobeLevel][2] - GlobeZRot;
    if (diff < 128 && diff > -128) {
        GlobeZRot = GlobeLevelAngles[GlobeLevel][2];
        set++;
    } else {
        GlobeZRot += diff >> 3;
    }
    if (set == 3 && GlobeLevel != 6) {
        AdventureText = T_Print(0, -16, 5, GF_GameStrings[GlobeLevelAngles[GlobeLevel][3]]);
        T_CentreH(AdventureText, 1);
        T_BottomAlign(AdventureText, 1);
        if (input & IN_LEFT) {
            do {
                if (GlobeLevel == 0)
                    GlobeLevel = 5;
                else
                    GlobeLevel--;
            } while (GlobeLevelComplete[GlobeLevel]);
        } else if (input & IN_RIGHT) {
            do {
                if (GlobeLevel == 5)
                    GlobeLevel = 0;
                else
                    GlobeLevel++;
            } while (GlobeLevelComplete[GlobeLevel]);
        }
    }
    if (GlobeLevel == 6)
        inv_item->drawn_meshes = 0xf81;
    if (set == 3 && GlobeLevel == 6) {
        GlobeLevel = 0;
        while (GlobeLevelComplete[GlobeLevel])
            GlobeLevel++;
        inputDB |= GlobeButton;
        inv_item->drawn_meshes = 0xf81;
        return;
    } else if (GlobeLevel != 6) {
        advs = 0;
        for (lp = 0; lp < 6; lp++) {
            if (!GlobeLevelComplete[lp])
                advs++;
        }
        if (set == 3 && advs > 1) {
            w = App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].w / 2;
            LeftArrow = T_Print(w - 120, -16, 2, la);
            T_BottomAlign(LeftArrow, 1);
            RightArrow = T_Print(w + 120, -16, 2, ra);
            T_BottomAlign(RightArrow, 1);
        }
        inv_item->drawn_meshes = 0xfff;
        if (GlobeLevelComplete[3] == 255)
            inv_item->drawn_meshes = 0xff7;
        if (GlobeLevelComplete[5] == 255)
            inv_item->drawn_meshes &= ~0x40;
    }
    inputDB &= ~IN_DESELECT;
    if ((inputDB & (IN_DESELECT | IN_SELECT)) && GlobeLevel != 6) {
        if (inputDB & IN_SELECT)
            NextAdventure = GlobeLevel;
        GlobeLevel = 6;
        GlobeButton = inputDB & (IN_DESELECT | IN_SELECT);
        inputDB &= ~(IN_DESELECT | IN_SELECT);
    }
    if ((inputDB & (IN_DESELECT | IN_SELECT)) && GlobeLevel == 6)
        inputDB &= ~(IN_DESELECT | IN_SELECT);
}

void do_sound_option(INVENTORY_ITEM *inv_item)
{
    int i;
    char string[20];

    if (!sa_text[0]) {
        if (Option_Music_Volume > 10)
            Option_Music_Volume = 10;
        wsprintfA(string, "| %2d", Option_Music_Volume);
        sa_text[0] = T_Print(0, 0, 0, string);
        T_AddBackground(sa_text[0], 168, 0, 0, 0, 8, 0, 0, 0);
        T_AddOutline(sa_text[0], 1, 4, 0, 0);
        if (Option_SFX_Volume > 10)
            Option_SFX_Volume = 10;
        wsprintfA(string, "} %2d", Option_SFX_Volume);
        sa_text[1] = T_Print(0, 25, 0, string);
        sa_text[2] = T_Print(0, -32, 0, " ");
        T_AddBackground(sa_text[2], 180, 85, 0, 0, 48, 0, 0, 0);
        T_AddOutline(sa_text[2], 1, 15, 0, 0);
        sa_text[3] = T_Print(0, -30, 0, GF_PCStrings[5]);
        T_AddBackground(sa_text[3], 176, 0, 0, 0, 8, 0, 0, 0);
        T_AddOutline(sa_text[3], 1, 15, 0, 0);
        for (i = 0; i < 4; i++) {
            T_CentreH(sa_text[i], 1);
            T_CentreV(sa_text[i], 1);
        }
    }
    if ((inputDB & IN_FORWARD) && item_data > 0) {
        T_RemoveOutline(sa_text[item_data]);
        T_RemoveBackground(sa_text[item_data]);
        item_data--;
        T_AddBackground(sa_text[item_data], 168, 0, 0, 0, 8, 0, 0, 0);
        T_AddOutline(sa_text[item_data], 1, 4, 0, 0);
    }
    if ((inputDB & IN_BACK) && item_data < 1) {
        T_RemoveOutline(sa_text[item_data]);
        T_RemoveBackground(sa_text[item_data]);
        item_data++;
        T_AddBackground(sa_text[item_data], 168, 0, 0, 0, 8, 0, 0, 0);
        T_AddOutline(sa_text[item_data], 1, 4, 0, 0);
    }
    if (!item_data) {
        if ((input & IN_LEFT) && Option_Music_Volume > 0) {
            idelay = 1;
            idcount = 10;
            Option_Music_Volume--;
            wsprintfA(string, "| %2d", Option_Music_Volume);
            T_ChangeText(sa_text[0], string);
            if (Option_Music_Volume)
                S_CDVolume(Option_Music_Volume * 25 + 5);
            else
                S_CDVolume(0);
            SoundEffect(115, 0, 2);
        } else if ((input & IN_RIGHT) && Option_Music_Volume < 10) {
            idelay = 1;
            idcount = 10;
            Option_Music_Volume++;
            wsprintfA(string, "| %2d", Option_Music_Volume);
            T_ChangeText(sa_text[0], string);
            if (Option_Music_Volume)
                S_CDVolume(Option_Music_Volume * 25 + 5);
            else
                S_CDVolume(0);
            SoundEffect(115, 0, 2);
        }
    } else {
        if ((input & IN_LEFT) && Option_SFX_Volume > 0) {
            idelay = 1;
            idcount = 10;
            Option_SFX_Volume--;
            wsprintfA(string, "} %2d", Option_SFX_Volume);
            T_ChangeText(sa_text[1], string);
            if (Option_SFX_Volume)
                S_SoundSetMasterVolume(Option_SFX_Volume * 6 + 4);
            else
                S_SoundSetMasterVolume(0);
            SoundEffect(115, 0, 2);
        } else if ((input & IN_RIGHT) && Option_SFX_Volume < 10) {
            idelay = 1;
            idcount = 10;
            Option_SFX_Volume++;
            wsprintfA(string, "} %2d", Option_SFX_Volume);
            T_ChangeText(sa_text[1], string);
            if (Option_SFX_Volume)
                S_SoundSetMasterVolume(Option_SFX_Volume * 6 + 4);
            else
                S_SoundSetMasterVolume(0);
            SoundEffect(115, 0, 2);
        }
    }
    is_sound_sfxlevel[6].param1 = Option_SFX_Volume;
    is_sound_sfxlevellow[6].param1 = Option_SFX_Volume;
    is_sound_muslevel[6].param1 = Option_Music_Volume;
    is_sound_muslevellow[6].param1 = Option_Music_Volume;
    if (inputDB & (IN_SELECT | IN_DESELECT)) {
        T_RemovePrint(sa_text[0]);
        sa_text[0] = NULL;
        T_RemovePrint(sa_text[1]);
        sa_text[1] = NULL;
        T_RemovePrint(sa_text[2]);
        sa_text[2] = NULL;
        T_RemovePrint(sa_text[3]);
        sa_text[3] = NULL;
    }
}

void do_compass_option(INVENTORY_ITEM *inv_item)
{
    int h;
    int m;
    int s;
    char time_str[32];

    s = savegame.timer / 30;
    h = s / 3600;
    m = (s / 60) % 60;
    s = s % 60;
    sprintf(time_str, "%02d:%02d:%02d", h, m, s);
    if (!CurrentLevel)
        ShowGymStatsText(time_str, 1);
    else
        ShowStatsText(time_str, 1);
    if (inputDB & (IN_DESELECT | IN_SELECT)) {
        inv_item->goal_frame = inv_item->frames_total - 1;
        inv_item->anim_direction = 1;
    }
    SoundEffect(113, 0, 2);
}

void FlashConflicts(void)
{
    int i;
    int j;
    sint16 key;

    for (i = 0; i < 14; i++) {
        key = layout[iconfig][i];
        T_FlashText(btext[i], 0, 0);
        for (j = 0; j < 14; j++) {
            if (i != j && key == layout[iconfig][j]) {
                T_FlashText(btext[i], 1, 20);
                break;
            }
        }
    }
}

void DefaultConflict(void)
{
    int i;
    int j;
    sint16 key;

    for (i = 0; i < 14; i++) {
        key = layout[0][i];
        conflict[i] = 0;
        for (j = 0; j < 14; j++) {
            if (key == layout[1][j]) {
                conflict[i] = 1;
                break;
            }
        }
    }
}

void do_control_option(INVENTORY_ITEM *inv_item)
{
    sint16 key;

    if (!ctrltext[0]) {
        if (iconfig)
            ctrltext[0] = T_Print(0, -50, 0, GF_PCStrings[6]);
        else
            ctrltext[0] = T_Print(0, -50, 0, GF_PCStrings[19]);
        ctrltext[0]->zpos = 16;
        T_CentreH(ctrltext[0], 1);
        T_CentreV(ctrltext[0], 1);
        S_ShowControls();
        keychange = -1;
        T_AddBackground(ctrltext[0], 0, 0, 0, 0, 48, 0, NULL, 0);
        T_AddOutline(ctrltext[0], 1, 15, NULL, 0);
    }
    switch (select_key) {
    case 0:
        if (inputDB & (IN_RIGHT | IN_LEFT)) {
            if (keychange == -1) {
                iconfig = !iconfig;
                S_ChangeCtrlText();
                FlashConflicts();
            } else {
                ctext[keychange]->zpos = 16;
                T_RemoveBackground(ctext[keychange]);
                T_RemoveOutline(ctext[keychange]);
                if (keychange <= 6)
                    keychange += 7;
                else if (keychange == 14)
                    keychange = 7;
                else
                    keychange -= 7;
                ctext[keychange]->zpos = 16;
                T_AddBackground(ctext[keychange], 0, 0, 0, 0, 0, 0, NULL, 0);
                T_AddOutline(ctext[keychange], 1, 15, NULL, 0);
            }
        } else if ((inputDB & IN_DESELECT) || ((inputDB & IN_SELECT) && keychange == -1)) {
            T_RemovePrint(ctrltext[0]);
            ctrltext[0] = NULL;
            T_RemovePrint(ctrltext[1]);
            ctrltext[1] = NULL;
            S_RemoveCtrlText();
            DefaultConflict();
            return;
        }
        if (iconfig) {
            if (inputDB & IN_SELECT) {
                select_key = 1;
                ctext[keychange]->zpos = 16;
                T_RemoveBackground(ctext[keychange]);
                T_RemoveOutline(ctext[keychange]);
                btext[keychange]->zpos = 16;
                T_AddBackground(btext[keychange], 0, 0, 0, 0, 0, 0, NULL, 0);
                T_AddOutline(btext[keychange], 1, 15, NULL, 0);
            } else if (inputDB & IN_FORWARD) {
                if (keychange == -1) {
                    T_RemoveBackground(ctrltext[0]);
                    T_RemoveOutline(ctrltext[0]);
                } else {
                    ctext[keychange]->zpos = 16;
                    T_RemoveBackground(ctext[keychange]);
                    T_RemoveOutline(ctext[keychange]);
                }
                keychange--;
                if (keychange < -1)
                    keychange = 13;
                if (keychange == -1) {
                    T_AddBackground(ctrltext[0], 0, 0, 0, 0, 0, 0, NULL, 0);
                    T_AddOutline(ctrltext[0], 1, 15, NULL, 0);
                } else {
                    ctext[keychange]->zpos = 16;
                    T_AddBackground(ctext[keychange], 0, 0, 0, 0, 0, 0, NULL, 0);
                    T_AddOutline(ctext[keychange], 1, 15, NULL, 0);
                }
            } else if (inputDB & IN_BACK) {
                if (keychange == -1) {
                    T_RemoveBackground(ctrltext[0]);
                    T_RemoveOutline(ctrltext[0]);
                } else {
                    ctext[keychange]->zpos = 16;
                    T_RemoveBackground(ctext[keychange]);
                    T_RemoveOutline(ctext[keychange]);
                }
                keychange++;
                if (keychange > 13)
                    keychange = -1;
                if (keychange == -1) {
                    T_AddBackground(ctrltext[0], 0, 0, 0, 0, 0, 0, NULL, 0);
                    T_AddOutline(ctrltext[0], 1, 15, NULL, 0);
                } else {
                    ctext[keychange]->zpos = 16;
                    T_AddBackground(ctext[keychange], 0, 0, 0, 0, 0, 0, NULL, 0);
                    T_AddOutline(ctext[keychange], 1, 15, NULL, 0);
                }
            }
        }
        break;
    case 1:
        if (!(inputDB & IN_SELECT))
            select_key = 2;
        break;
    case 2:
        if (joy_fire) {
            for (key = 0; key < 32; key++) {
                if ((1 << key) & joy_fire)
                    break;
            }
            if (key == 32)
                key = 0;
            else
                key += 256;
        } else {
            for (key = 0; key < 256; key++) {
                if (keymap[key] & 0x80)
                    break;
            }
            if (key == 256)
                key = 0;
        }
        if (key && kname[key]) {
            if (key == DIK_RETURN || key == DIK_LEFT || key == DIK_RIGHT || key == DIK_UP || key == DIK_DOWN)
                break;
            if (key != DIK_ESCAPE) {
                layout[iconfig][keychange] = key;
                T_ChangeText(btext[keychange], kname[key]);
            }
            btext[keychange]->zpos = 16;
            T_RemoveBackground(btext[keychange]);
            T_RemoveOutline(btext[keychange]);
            ctext[keychange]->zpos = 16;
            T_AddBackground(ctext[keychange], 0, 0, 0, 0, 0, 0, NULL, 0);
            T_AddOutline(ctext[keychange], 1, 15, NULL, 0);
            select_key = 3;
            FlashConflicts();
        }
        break;
    case 3:
        if (layout[iconfig][keychange] & 256) {
            if (!((1 << (layout[iconfig][keychange] & 0xff)) & joy_fire))
                select_key = 0;
        } else if (!(keymap[layout[iconfig][keychange]] & 0x80)) {
            select_key = 0;
            if (layout[iconfig][keychange] == DIK_LCONTROL)
                layout[iconfig][keychange] = DIK_RCONTROL;
            if (layout[iconfig][keychange] == DIK_LSHIFT)
                layout[iconfig][keychange] = DIK_RSHIFT;
            if (layout[iconfig][keychange] == DIK_LMENU)
                layout[iconfig][keychange] = DIK_RMENU;
            FlashConflicts();
        }
        break;
    }
    input = 0;
    inputDB = 0;
}

void S_ChangeCtrlText(void)
{
    int i;
    char str[40];

    if (iconfig) {
        wsprintfA(str, GF_PCStrings[6], iconfig);
        T_ChangeText(ctrltext[0], str);
    } else {
        T_ChangeText(ctrltext[0], GF_PCStrings[19]);
    }
    for (i = 0; i < 14; i++) {
        if (kname[layout[iconfig][i]])
            T_ChangeText(btext[i], kname[layout[iconfig][i]]);
        else
            T_ChangeText(btext[i], "BAD");
    }
}

void S_RemoveCtrlText(void)
{
    int i;

    for (i = 0; i < 14; i++) {
        if (ctext[i])
            T_RemovePrint(ctext[i]);
        ctext[i] = 0;
        if (btext[i])
            T_RemovePrint(btext[i]);
        btext[i] = 0;
    }
}

extern int phd_winheight;
extern int phd_winwidth;

int GetRenderHeight(void)
{
    return phd_winheight;
}

int GetRenderWidth(void)
{
    return phd_winwidth;
}

}
