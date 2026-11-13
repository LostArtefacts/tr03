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
extern TEXTSTRING *ControlTextInfo[2];
extern sint32 inputDB;
extern int CurrentLevel;
extern char **GF_PCStrings;
extern TEXTSTRING *sa_text[4];
extern sint16 item_data;
extern sint16 Option_Music_Volume;
extern sint16 Option_SoundFX_Volume;
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

void do_passport_option(INVENTORY_ITEM *inv_item)
{
    sint16 page;
    sint32 select;

    T_RemovePrint(Inv_itemText[0]);
    Inv_itemText[0] = NULL;
    page = (inv_item->goal_frame - inv_item->open_frame) / 5;
    if ((inv_item->goal_frame - inv_item->open_frame) % 5)
        page = -1;
    if (Inventory_Mode == 4 || Inventory_Mode == 3 || gameflow.loadsave_disabled)
        inputDB &= ~(IN_LEFT | IN_RIGHT);
    switch (page) {
    case 0:
        if ((CurrentLevel == 0 && Inventory_Mode != 1) || gameflow.loadsave_disabled) {
            inputDB = IN_RIGHT;
            break;
        }
        if (passport_mode == 1) {
            SetPCRequesterSize(&Load_Game_Requester, 10, -32);
            select = Display_Requester(&Load_Game_Requester, 1, 1);
            if (select) {
                if (select > 0)
                    Inventory_ExtraData[1] = select - 1;
                passport_mode = 0;
            } else if (inputDB & IN_RIGHT) {
                Remove_Requester(&Load_Game_Requester);
                passport_mode = 0;
            } else {
                input = inputDB = 0;
            }
        } else if (passport_mode == 0) {
            if (!SavedGames || Inventory_Mode == 3) {
                inputDB = IN_RIGHT;
            } else {
                if (!passport_text1) {
                    passport_text1 = T_Print(0, -16, 5, GF_GameStrings[4]);
                    T_BottomAlign(passport_text1, 1);
                    T_CentreH(passport_text1, 1);
                }
                T_RemovePrint(Inv_itemText[2]);
                Inv_itemText[2] = NULL;
                T_RemovePrint(Inv_tagText);
                Inv_tagText = NULL;
                T_RemovePrint(Inv_ringText);
                Inv_ringText = NULL;
                T_RemovePrint(Inv_itemText[0]);
                Inv_itemText[0] = NULL;
                GetSavedGamesList(&Load_Game_Requester);
                SetRequesterHeading(&Load_Game_Requester, GF_GameStrings[4], 0, NULL, 0);
                passport_mode = 1;
                input = 0;
                inputDB = 0;
            }
        }
        break;
    case 1:
        if ((CurrentLevel == 0 && Inventory_Mode != 1) || gameflow.loadsave_disabled) {
            inputDB = IN_RIGHT;
            break;
        }
        if (passport_mode == 1 || passport_mode == 2) {
            if (passport_mode == 1) {
                SetPCRequesterSize(&Load_Game_Requester, 10, -32);
                select = Display_Requester(&Load_Game_Requester, 1, 1);
            } else {
                SetPCRequesterSize(&Level_Select_Requester, 10, -32);
                select = Display_Requester(&Level_Select_Requester, 1, 1);
            }
            if (select) {
                if (select > 0)
                    Inventory_ExtraData[1] = select - 1;
                passport_mode = 0;
            } else if (inputDB & (IN_LEFT | IN_RIGHT)) {
                if (passport_mode == 1)
                    Remove_Requester(&Load_Game_Requester);
                else
                    Remove_Requester(&Level_Select_Requester);
                passport_mode = 0;
            } else {
                input = inputDB = 0;
            }
        } else if (passport_mode == 0) {
            if (Inventory_Mode == 5) {
                if (inv_item->anim_direction == -1)
                    inputDB = IN_LEFT;
                else
                    inputDB = IN_RIGHT;
            } else {
                if (!passport_text1) {
                    if (Inventory_Mode == 1 || CurrentLevel == 0)
                        passport_text1 = T_Print(0, -16, 5, GF_GameStrings[6]);
                    else
                        passport_text1 = T_Print(0, -16, 5, GF_GameStrings[5]);
                    T_BottomAlign(passport_text1, 1);
                    T_CentreH(passport_text1, 1);
                }
                if (Inventory_Mode != 1 && CurrentLevel != 0) {
                    T_RemovePrint(Inv_itemText[2]);
                    Inv_itemText[2] = NULL;
                    T_RemovePrint(Inv_tagText);
                    Inv_tagText = NULL;
                    T_RemovePrint(Inv_ringText);
                    Inv_ringText = NULL;
                    T_RemovePrint(Inv_itemText[0]);
                    Inv_itemText[0] = NULL;
                    GetSavedGamesList(&Load_Game_Requester);
                    SetRequesterHeading(&Load_Game_Requester, GF_GameStrings[5], 0, NULL, 0);
                    passport_mode = 1;
                    input = 0;
                    inputDB = 0;
                } else if (gameflow.play_any_level) {
                    T_RemovePrint(Inv_itemText[2]);
                    Inv_itemText[2] = NULL;
                    T_RemovePrint(Inv_tagText);
                    Inv_tagText = NULL;
                    T_RemovePrint(Inv_itemText[0]);
                    Inv_itemText[0] = NULL;
                    Init_Requester(&Level_Select_Requester);
                    GetValidLevelsList(&Level_Select_Requester);
                    SetRequesterHeading(&Level_Select_Requester, GF_GameStrings[11], 0, NULL, 0);
                    passport_mode = 2;
                    input = 0;
                    inputDB = 0;
                } else if (inputDB & IN_SELECT) {
                    Inventory_ExtraData[1] = 1;
                }
            }
        }
        break;
    case 2:
        if (!passport_text1) {
            if (Inventory_Mode == 1)
                passport_text1 = T_Print(0, -16, 5, GF_GameStrings[10]);
            else if (!gameflow.demoversion)
                passport_text1 = T_Print(0, -16, 5, GF_GameStrings[8]);
            else
                passport_text1 = T_Print(0, -16, 5, GF_GameStrings[9]);
            T_BottomAlign(passport_text1, 1);
            T_CentreH(passport_text1, 1);
        }
        break;
    }
    if (inputDB & IN_LEFT) {
        if (Inventory_Mode != 5 || SavedGames) {
            inv_item->anim_direction = -1;
            inv_item->goal_frame -= 5;
            if (!SavedGames) {
                if (inv_item->goal_frame < inv_item->open_frame + 5) {
                    inv_item->goal_frame = inv_item->open_frame + 5;
                } else {
                    T_RemovePrint(passport_text1);
                    passport_text1 = NULL;
                }
            } else {
                if (inv_item->goal_frame < inv_item->open_frame) {
                    inv_item->goal_frame = inv_item->open_frame;
                } else {
                    SoundEffect(115, NULL, 2);
                    T_RemovePrint(passport_text1);
                    passport_text1 = NULL;
                }
            }
            input = 0;
            inputDB = 0;
        }
    }
    if (inputDB & IN_RIGHT) {
        inv_item->anim_direction = 1;
        inv_item->goal_frame += 5;
        if (inv_item->goal_frame > inv_item->frames_total - 6) {
            inv_item->goal_frame = inv_item->frames_total - 6;
        } else {
            SoundEffect(115, NULL, 2);
            T_RemovePrint(passport_text1);
            passport_text1 = NULL;
        }
        input = 0;
        inputDB = 0;
    }
    if (inputDB & IN_DESELECT) {
        if (Inventory_Mode == 5) {
            input = 0;
            inputDB = 0;
        } else {
            if (page == 2) {
                inv_item->anim_direction = 1;
                inv_item->goal_frame = inv_item->frames_total - 1;
            } else {
                inv_item->goal_frame = 0;
                inv_item->anim_direction = -1;
            }
            T_RemovePrint(passport_text1);
            passport_text1 = NULL;
        }
    }
    if (inputDB & IN_SELECT) {
        Inventory_ExtraData[0] = page;
        if (page == 2) {
            inv_item->anim_direction = 1;
            inv_item->goal_frame = inv_item->frames_total - 1;
        } else {
            inv_item->goal_frame = 0;
            inv_item->anim_direction = -1;
        }
        T_RemovePrint(passport_text1);
        passport_text1 = NULL;
    }
}

typedef struct res_txt {
    char res[20];
} RES_TXT;

extern TEXTSTRING *dtext[14];
extern char available[6];
extern int selected_res;
extern RES_TXT resolutions[30];
extern int selection;
extern float GammaOption;
bool DXSwitchVideoMode(long needed, long current, bool disableZBuffer);
void HWR_InitState(void);
void CloseDrawPrimitive(void);
void InitDrawPrimitive(LPDIRECT3DDEVICE2 lpD3D, LPDIRECTDRAWSURFACE3 lpSurface, bool HW);

void do_detail_option(INVENTORY_ITEM *inv_item)
{
    int i, w, bpp, old_res;
    char gtxt[8];

    old_res = GetRenderWidth() / 2;
    w = old_res - 115;
    if (!dtext[0]) {
        bpp = App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].bpp;
        for (i = 0; i < App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].nDisplayMode; i++) {
            if (App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[i].bpp == bpp) {
                sprintf(resolutions[i].res, "%dx%d", App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[i].w, App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[i].h);
                if (App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[i].w == App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].w && App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[i].h == App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].h)
                    selected_res = i;
            }
        }
        dtext[5] = T_Print(w, -45, 0, GF_PCStrings[22]);
        dtext[4] = T_Print(w, -25, 0, GF_PCStrings[23]);
        dtext[3] = T_Print(w, -5, 0, GF_PCStrings[24]);
        dtext[2] = T_Print(w, 15, 0, GF_PCStrings[25]);
        dtext[1] = T_Print(w, 35, 0, GF_PCStrings[26]);
        dtext[0] = T_Print(w, 55, 0, GF_PCStrings[27]);
        dtext[6] = T_Print(0, -72, 0, " ");
        dtext[7] = T_Print(0, -70, 0, GF_PCStrings[21]);
        T_AddBackground(dtext[7], 236, 0, 0, 0, 48, 0, 0, 0);
        T_AddOutline(dtext[7], 1, 4, 0, 0);
        T_AddBackground(dtext[6], 240, 150, 0, 0, 48, 0, 0, 0);
        T_AddOutline(dtext[6], 1, 15, 0, 0);
        T_CentreV(dtext[0], 1);
        T_CentreV(dtext[1], 1);
        T_CentreV(dtext[2], 1);
        T_CentreV(dtext[3], 1);
        T_CentreV(dtext[4], 1);
        T_CentreV(dtext[5], 1);
        T_CentreH(dtext[6], 1);
        T_CentreV(dtext[6], 1);
        T_CentreH(dtext[7], 1);
        T_CentreV(dtext[7], 1);
        dtext[13] = T_Print(w + 130, -45, 0, resolutions[selected_res].res);
        if (App.lpZBuffer)
            dtext[12] = T_Print(w + 130, -25, 0, GF_PCStrings[17]);
        else
            dtext[12] = T_Print(w + 130, -25, 0, GF_PCStrings[16]);
        if (HWConfig.nFilter == D3DFILTER_LINEAR)
            dtext[11] = T_Print(w + 130, -5, 0, GF_PCStrings[17]);
        else
            dtext[11] = T_Print(w + 130, -5, 0, GF_PCStrings[16]);
        if (HWConfig.Dither)
            dtext[10] = T_Print(w + 130, 15, 0, GF_PCStrings[17]);
        else
            dtext[10] = T_Print(w + 130, 15, 0, GF_PCStrings[16]);
        if (HWConfig.TrueAlpha)
            dtext[9] = T_Print(w + 130, 35, 0, GF_PCStrings[16]);
        else
            dtext[9] = T_Print(w + 130, 35, 0, GF_PCStrings[17]);
        available[0] = 1;
        available[1] = 1;
        available[2] = 1;
        available[3] = 1;
        available[4] = 1;
        available[5] = 1;
        if (App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].bHardware) {
            T_ChangeText(dtext[9], GF_PCStrings[28]);
            available[1] = 0;
        } else {
            T_ChangeText(dtext[12], GF_PCStrings[28]);
            T_ChangeText(dtext[10], GF_PCStrings[28]);
            available[4] = 0;
            available[2] = 0;
            if (App.DXConfigPtr->MMX) {
                available[1] = 0;
                T_ChangeText(dtext[9], GF_PCStrings[28]);
            } else {
                T_ChangeText(dtext[11], GF_PCStrings[28]);
                available[3] = 0;
            }
        }
        sprintf(gtxt, "%d", (long)GammaOption);
        dtext[8] = T_Print(w + 130, 55, 0, gtxt);
        T_CentreV(dtext[8], 1);
        T_CentreV(dtext[9], 1);
        T_CentreV(dtext[10], 1);
        T_CentreV(dtext[11], 1);
        T_CentreV(dtext[12], 1);
        T_CentreV(dtext[13], 1);
        T_AddBackground(dtext[selection + 8], (sint16)T_GetTextWidth(dtext[selection + 8]), 0, 0, 0, 48, 0, 0, 0);
        T_AddOutline(dtext[selection + 8], 1, 4, 0, 0);
    }
    if ((inputDB & IN_LEFT) && selection == 5) {
        old_res = selected_res;
        if (selected_res > 0)
            selected_res--;
        if (old_res != selected_res) {
            if (!DXSwitchVideoMode(selected_res, old_res, 0))
                selected_res = old_res;
            for (i = 0; i < 8; i++) {
                T_RemovePrint(dtext[i]);
                dtext[i] = NULL;
            }
            for (i = 8; i < 14; i++)
                T_RemovePrint(dtext[i]);
        }
    }
    if ((inputDB & IN_RIGHT) && selection == 5) {
        old_res = selected_res;
        if (selected_res < App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].nDisplayMode - 1)
            selected_res++;
        if (old_res != selected_res) {
            if (!DXSwitchVideoMode(selected_res, old_res, 0))
                selected_res = old_res;
            for (i = 0; i < 8; i++) {
                T_RemovePrint(dtext[i]);
                dtext[i] = NULL;
            }
            for (i = 8; i < 14; i++)
                T_RemovePrint(dtext[i]);
        }
    }
    if ((inputDB & IN_RIGHT) || (inputDB & IN_LEFT)) {
        switch (selection) {
        case 4:
            if (App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].bHardware) {
                if (App.lpZBuffer) {
                    App.DXConfigPtr->bZBuffer = 0;
                    DXSwitchVideoMode(selected_res, selected_res, 0);
                    T_ChangeText(dtext[selection + 8], GF_PCStrings[16]);
                    App.lpZBuffer = NULL;
                } else {
                    App.DXConfigPtr->bZBuffer = 1;
                    if (DXSwitchVideoMode(selected_res, selected_res, 1))
                        T_ChangeText(dtext[selection + 8], GF_PCStrings[17]);
                    else {
                        App.DXConfigPtr->bZBuffer = 0;
                        App.lpZBuffer = NULL;
                    }
                }
                HWR_InitState();
            }
            break;
        case 3:
            if (HWConfig.nFilter == D3DFILTER_LINEAR) {
                HWConfig.nFilter = D3DFILTER_NEAREST;
                T_ChangeText(dtext[selection + 8], GF_PCStrings[16]);
            } else {
                HWConfig.nFilter = D3DFILTER_LINEAR;
                T_ChangeText(dtext[selection + 8], GF_PCStrings[17]);
            }
            HWR_InitState();
            break;
        case 2:
            if (HWConfig.Dither) {
                T_ChangeText(dtext[selection + 8], GF_PCStrings[16]);
                HWConfig.Dither = 0;
            } else {
                T_ChangeText(dtext[selection + 8], GF_PCStrings[17]);
                HWConfig.Dither = 1;
            }
            HWR_InitState();
            break;
        case 1:
            if (HWConfig.TrueAlpha) {
                T_ChangeText(dtext[selection + 8], GF_PCStrings[17]);
                HWConfig.TrueAlpha = 0;
            } else {
                T_ChangeText(dtext[selection + 8], GF_PCStrings[16]);
                HWConfig.TrueAlpha = 1;
            }
            CloseDrawPrimitive();
            InitDrawPrimitive(App.lpD3DDevice, App.lpBackBuffer, App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].bHardware);
            HWR_InitState();
            break;
        case 0:
            if (inputDB & IN_RIGHT)
                GammaOption++;
            if (inputDB & IN_LEFT)
                GammaOption--;
            if (GammaOption > 10)
                GammaOption = 10;
            if (GammaOption < 1)
                GammaOption = 1;
            HWR_InitState();
            T_RemovePrint(dtext[8]);
            sprintf(gtxt, "%d", (long)GammaOption);
            dtext[8] = T_Print(w + 130, 55, 0, gtxt);
            T_CentreV(dtext[8], 1);
            break;
        }
        T_RemoveOutline(dtext[selection + 8]);
        T_RemoveBackground(dtext[selection + 8]);
        T_AddOutline(dtext[selection + 8], 1, 4, 0, 0);
        T_AddBackground(dtext[selection + 8], (sint16)T_GetTextWidth(dtext[selection + 8]), 0, 0, 0, 48, 0, 0, 0);
    }
    if ((inputDB & IN_BACK) && selection > 0) {
        T_RemoveOutline(dtext[selection + 8]);
        T_RemoveBackground(dtext[selection + 8]);
        selection--;
        i = selection;
        while (!available[selection]) {
            selection--;
            if (selection < 0) {
                selection = i;
                break;
            }
        }
        T_AddOutline(dtext[selection + 8], 1, 4, 0, 0);
        T_AddBackground(dtext[selection + 8], (sint16)T_GetTextWidth(dtext[selection + 8]), 0, 0, 0, 48, 0, 0, 0);
    }
    if ((inputDB & IN_FORWARD) && selection <= 4) {
        T_RemoveOutline(dtext[selection + 8]);
        T_RemoveBackground(dtext[selection + 8]);
        selection++;
        i = selection;
        while (!available[selection]) {
            selection++;
            if (selection > 5) {
                selection = i;
                break;
            }
        }
        T_AddOutline(dtext[selection + 8], 1, 4, 0, 0);
        T_AddBackground(dtext[selection + 8], (sint16)T_GetTextWidth(dtext[selection + 8]), 0, 0, 0, 48, 0, 0, 0);
    }
    if (inputDB & (IN_SELECT | IN_DESELECT)) {
        for (i = 0; i < 8; i++) {
            T_RemovePrint(dtext[i]);
            dtext[i] = NULL;
        }
        for (i = 8; i < 14; i++)
            T_RemovePrint(dtext[i]);
    }
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
        if (Option_SoundFX_Volume > 10)
            Option_SoundFX_Volume = 10;
        wsprintfA(string, "} %2d", Option_SoundFX_Volume);
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
        if ((input & IN_LEFT) && Option_SoundFX_Volume > 0) {
            idelay = 1;
            idcount = 10;
            Option_SoundFX_Volume--;
            wsprintfA(string, "} %2d", Option_SoundFX_Volume);
            T_ChangeText(sa_text[1], string);
            if (Option_SoundFX_Volume)
                S_SoundSetMasterVolume(Option_SoundFX_Volume * 6 + 4);
            else
                S_SoundSetMasterVolume(0);
            SoundEffect(115, 0, 2);
        } else if ((input & IN_RIGHT) && Option_SoundFX_Volume < 10) {
            idelay = 1;
            idcount = 10;
            Option_SoundFX_Volume++;
            wsprintfA(string, "} %2d", Option_SoundFX_Volume);
            T_ChangeText(sa_text[1], string);
            if (Option_SoundFX_Volume)
                S_SoundSetMasterVolume(Option_SoundFX_Volume * 6 + 4);
            else
                S_SoundSetMasterVolume(0);
            SoundEffect(115, 0, 2);
        }
    }
    is_sound_sfxlevel[6].param1 = Option_SoundFX_Volume;
    is_sound_sfxlevellow[6].param1 = Option_SoundFX_Volume;
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

    if (!ControlTextInfo[0]) {
        if (iconfig)
            ControlTextInfo[0] = T_Print(0, -50, 0, GF_PCStrings[6]);
        else
            ControlTextInfo[0] = T_Print(0, -50, 0, GF_PCStrings[19]);
        ControlTextInfo[0]->zpos = 16;
        T_CentreH(ControlTextInfo[0], 1);
        T_CentreV(ControlTextInfo[0], 1);
        S_ShowControls();
        keychange = -1;
        T_AddBackground(ControlTextInfo[0], 0, 0, 0, 0, 48, 0, NULL, 0);
        T_AddOutline(ControlTextInfo[0], 1, 15, NULL, 0);
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
            T_RemovePrint(ControlTextInfo[0]);
            ControlTextInfo[0] = NULL;
            T_RemovePrint(ControlTextInfo[1]);
            ControlTextInfo[1] = NULL;
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
                    T_RemoveBackground(ControlTextInfo[0]);
                    T_RemoveOutline(ControlTextInfo[0]);
                } else {
                    ctext[keychange]->zpos = 16;
                    T_RemoveBackground(ctext[keychange]);
                    T_RemoveOutline(ctext[keychange]);
                }
                keychange--;
                if (keychange < -1)
                    keychange = 13;
                if (keychange == -1) {
                    T_AddBackground(ControlTextInfo[0], 0, 0, 0, 0, 0, 0, NULL, 0);
                    T_AddOutline(ControlTextInfo[0], 1, 15, NULL, 0);
                } else {
                    ctext[keychange]->zpos = 16;
                    T_AddBackground(ctext[keychange], 0, 0, 0, 0, 0, 0, NULL, 0);
                    T_AddOutline(ctext[keychange], 1, 15, NULL, 0);
                }
            } else if (inputDB & IN_BACK) {
                if (keychange == -1) {
                    T_RemoveBackground(ControlTextInfo[0]);
                    T_RemoveOutline(ControlTextInfo[0]);
                } else {
                    ctext[keychange]->zpos = 16;
                    T_RemoveBackground(ctext[keychange]);
                    T_RemoveOutline(ctext[keychange]);
                }
                keychange++;
                if (keychange > 13)
                    keychange = -1;
                if (keychange == -1) {
                    T_AddBackground(ControlTextInfo[0], 0, 0, 0, 0, 0, 0, NULL, 0);
                    T_AddOutline(ControlTextInfo[0], 1, 15, NULL, 0);
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

void S_ShowControls(void)
{
    int mid;
    int i;
    sint16 x;
    sint16 *key;

    mid = GetRenderWidth() / 2;
    if (!btext[0]) {
        key = &layout[iconfig][0];
        if (mid < 320)
            x = mid - 150;
        else
            x = mid - 200;
        btext[0] = T_Print(x, -25, 0, kname[key[0]]);
        btext[1] = T_Print(x, -9, 0, kname[key[1]]);
        btext[2] = T_Print(x, 7, 0, kname[key[2]]);
        btext[3] = T_Print(x, 23, 0, kname[key[3]]);
        btext[4] = T_Print(x, 39, 0, kname[key[4]]);
        btext[5] = T_Print(x, 55, 0, kname[key[5]]);
        btext[6] = T_Print(x, 71, 0, kname[key[6]]);
        if (mid < 320)
            x = mid - 20;
        else
            x = mid + 10;
        btext[7] = T_Print(x, -25, 0, kname[key[7]]);
        btext[8] = T_Print(x, -9, 0, kname[key[8]]);
        btext[9] = T_Print(x, 7, 0, kname[key[9]]);
        btext[10] = T_Print(x, 23, 0, kname[key[10]]);
        btext[11] = T_Print(x, 39, 0, kname[key[11]]);
        btext[12] = T_Print(x, 55, 0, kname[key[12]]);
        btext[13] = T_Print(x, 71, 0, kname[key[13]]);
        for (i = 0; i < 14; i++)
            T_CentreV(btext[i], 1);
        keychange = 0;
    }
    if (!ctext[0]) {
        if (mid < 320)
            x = mid - 78;
        else
            x = mid - 120;
        ctext[0] = T_Print(x, -25, 0, GF_GameStrings[19]);
        ctext[1] = T_Print(x, -9, 0, GF_GameStrings[22]);
        ctext[2] = T_Print(x, 7, 0, GF_GameStrings[20]);
        ctext[3] = T_Print(x, 23, 0, GF_GameStrings[21]);
        ctext[4] = T_Print(x, 39, 0, GF_GameStrings[23]);
        ctext[5] = T_Print(x, 55, 0, GF_GameStrings[25]);
        ctext[6] = T_Print(x, 71, 0, GF_GameStrings[17]);
        if (mid < 320)
            x = mid + 55;
        else
            x = mid + 90;
        ctext[7] = T_Print(x, -25, 0, GF_GameStrings[28]);
        ctext[8] = T_Print(x, -9, 0, GF_GameStrings[29]);
        ctext[9] = T_Print(x, 7, 0, GF_GameStrings[30]);
        ctext[10] = T_Print(x, 23, 0, GF_GameStrings[33]);
        ctext[11] = T_Print(x, 39, 0, GF_GameStrings[27]);
        ctext[12] = T_Print(x, 55, 0, GF_GameStrings[18]);
        ctext[13] = T_Print(x, 71, 0, GF_GameStrings[32]);
        for (i = 0; i < 14; i++)
            T_CentreV(ctext[i], 1);
    }
    ControlTextInfo[1] = T_Print(0, -55, 0, " ");
    T_CentreV(ControlTextInfo[1], 1);
    T_CentreH(ControlTextInfo[1], 1);
    T_AddOutline(ControlTextInfo[1], 1, 15, NULL, 0);
    if (mid < 320)
        T_AddBackground(ControlTextInfo[1], 308, 145, 0, 0, 48, 0, NULL, 0);
    else
        T_AddBackground(ControlTextInfo[1], 420, 150, 0, 0, 48, 0, NULL, 0);
}

void S_ChangeCtrlText(void)
{
    int i;
    char str[40];

    if (iconfig) {
        wsprintfA(str, GF_PCStrings[6], iconfig);
        T_ChangeText(ControlTextInfo[0], str);
    } else {
        T_ChangeText(ControlTextInfo[0], GF_PCStrings[19]);
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

#ifdef TR3_EXE
extern "C" {

int selection = 5;

}
#endif
