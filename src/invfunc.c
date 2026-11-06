#include "specific/standard.h"
#include "game.h"

#define CAMERA_YOFFSET -96
#define NUM_ASS_TIMES 10

void phd_GetVectorAngles(sint32 x, sint32 y, sint32 z, PHD_ANGLE *dest);

extern TEXTSTRING *Inv_ringText;
extern TEXTSTRING *Inv_tagText;
extern TEXTSTRING *Inv_upArrow1;
extern TEXTSTRING *Inv_upArrow2;
extern TEXTSTRING *Inv_downArrow1;
extern TEXTSTRING *Inv_downArrow2;

TEXTSTRING *T_Print(sint32 xpos, sint32 ypos, sint32 zpos, char *string);

extern sint32 Inventory_Mode;
extern char **GF_GameStrings;
extern sint16 inv_keys_objects;

extern uint16 inv_colours[];

uint16 S_COLOUR(sint32 r, sint32 g, sint32 b);

void InitColours(void)
{
    inv_colours[0] = S_COLOUR(0, 0, 0);
    inv_colours[1] = S_COLOUR(64, 64, 64);
    inv_colours[2] = S_COLOUR(255, 255, 255);
    inv_colours[3] = S_COLOUR(255, 0, 0);
    inv_colours[4] = S_COLOUR(255, 128, 0);
    inv_colours[5] = S_COLOUR(255, 255, 0);
    inv_colours[12] = S_COLOUR(0, 128, 0);
    inv_colours[13] = S_COLOUR(0, 255, 0);
    inv_colours[14] = S_COLOUR(0, 255, 255);
    inv_colours[15] = S_COLOUR(0, 0, 255);
    inv_colours[16] = S_COLOUR(255, 0, 255);
}

void RingIsOpen(RING_INFO *ring)
{
    if (Inventory_Mode == 1)
        return;

    if (!Inv_ringText) {
        switch (ring->type) {
        case 0:
            Inv_ringText = T_Print(0, 26, 5, GF_GameStrings[0]);
            break;
        case 1:
            if (Inventory_Mode == 5)
                Inv_ringText = T_Print(0, 26, 5, GF_GameStrings[3]);
            else
                Inv_ringText = T_Print(0, 26, 5, GF_GameStrings[1]);
            break;
        case 2:
            Inv_ringText = T_Print(0, 26, 5, GF_GameStrings[2]);
            break;
        case 3:
            Inv_ringText = T_Print(0, 26, 5, GF_GameStrings[90]);
            break;
        }
        T_CentreH(Inv_ringText, 1);
    }

    if (Inventory_Mode == 2 || Inventory_Mode == 5)
        return;

    if (!Inv_upArrow1) {
        if (ring->type == 1 || (ring->type == 0 && inv_keys_objects)) {
            Inv_upArrow1 = T_Print(20, 28, 2, "[");
            Inv_upArrow2 = T_Print(-20, 28, 2, "[");
            T_RightAlign(Inv_upArrow2, 1);
        }
    }

    if (!Inv_downArrow1) {
        if ((ring->type == 0 && !gameflow.lockout_optionring) || ring->type == 2) {
            Inv_downArrow1 = T_Print(20, -15, 2, "]");
            T_BottomAlign(Inv_downArrow1, 1);
            Inv_downArrow2 = T_Print(-20, -15, 2, "]");
            T_BottomAlign(Inv_downArrow2, 1);
            T_RightAlign(Inv_downArrow2, 1);
        }
    }
}

void RingIsNotOpen(RING_INFO *ring)
{
    T_RemovePrint(Inv_tagText);
    Inv_tagText = NULL;
    T_RemovePrint(Inv_ringText);
    Inv_ringText = NULL;
    T_RemovePrint(Inv_upArrow1);
    Inv_upArrow1 = NULL;
    T_RemovePrint(Inv_upArrow2);
    Inv_upArrow2 = NULL;
    T_RemovePrint(Inv_downArrow1);
    Inv_downArrow1 = NULL;
    T_RemovePrint(Inv_downArrow2);
    Inv_downArrow2 = NULL;
}

extern char **GF_Puzzle1Strings;
extern char **GF_Puzzle2Strings;
extern char **GF_Puzzle3Strings;
extern char **GF_Puzzle4Strings;
extern char **GF_Pickup1Strings;
extern char **GF_Pickup2Strings;
extern char **GF_Key1Strings;
extern char **GF_Key2Strings;
extern char **GF_Key3Strings;
extern char **GF_Key4Strings;
extern uint32 CurrentLevel;
extern int health_bar_timer;
int FlashIt(void);
void DrawHealthBar(int flash_state);
int Inv_RequestItem(int item_num);
__declspec(dllimport) int __cdecl wsprintfA(char *, const char *, ...);

void RingNotActive(INVENTORY_ITEM *inv_item)
{
    int qty;
    char txt[64];

    if (!Inv_itemText[IT_NAME]) {
        switch (inv_item->object_number) {
        case 209:
            Inv_itemText[IT_NAME] = T_Print(0, -16, 0, GF_Puzzle1Strings[CurrentLevel]);
            break;
        case 210:
            Inv_itemText[IT_NAME] = T_Print(0, -16, 0, GF_Puzzle2Strings[CurrentLevel]);
            break;
        case 211:
            Inv_itemText[IT_NAME] = T_Print(0, -16, 0, GF_Puzzle3Strings[CurrentLevel]);
            break;
        case 212:
            Inv_itemText[IT_NAME] = T_Print(0, -16, 0, GF_Puzzle4Strings[CurrentLevel]);
            break;
        case 238:
            Inv_itemText[IT_NAME] = T_Print(0, -16, 0, GF_Pickup1Strings[CurrentLevel]);
            break;
        case 239:
            Inv_itemText[IT_NAME] = T_Print(0, -16, 0, GF_Pickup2Strings[CurrentLevel]);
            break;
        case 228:
            Inv_itemText[IT_NAME] = T_Print(0, -16, 0, GF_Key1Strings[CurrentLevel]);
            break;
        case 229:
            Inv_itemText[IT_NAME] = T_Print(0, -16, 0, GF_Key2Strings[CurrentLevel]);
            break;
        case 230:
            Inv_itemText[IT_NAME] = T_Print(0, -16, 0, GF_Key3Strings[CurrentLevel]);
            break;
        case 231:
            Inv_itemText[IT_NAME] = T_Print(0, -16, 0, GF_Key4Strings[CurrentLevel]);
            break;
        case 145:
        case 184:
            break;
        default:
            Inv_itemText[IT_NAME] = T_Print(0, -16, 5, inv_item->itemText);
            break;
        }
        if (Inv_itemText[IT_NAME]) {
            T_BottomAlign(Inv_itemText[IT_NAME], 1);
            T_CentreH(Inv_itemText[IT_NAME], 1);
        }
    }
    qty = Inv_RequestItem(inv_item->object_number);
    switch (inv_item->object_number) {
    case 186:
        if (Inv_itemText[IT_QTY] || savegame.bonus_flag)
            return;
        wsprintfA(txt, "%5d", lara.shotgun.ammo / 6);
        break;
    case 187:
        if (Inv_itemText[IT_QTY] || savegame.bonus_flag)
            return;
        wsprintfA(txt, "%5d", lara.magnums.ammo);
        break;
    case 188:
        if (Inv_itemText[IT_QTY] || savegame.bonus_flag)
            return;
        wsprintfA(txt, "%5d", lara.uzis.ammo);
        break;
    case 189:
        if (Inv_itemText[IT_QTY] || savegame.bonus_flag)
            return;
        wsprintfA(txt, "%5d", lara.harpoon.ammo);
        break;
    case 190:
        if (Inv_itemText[IT_QTY] || savegame.bonus_flag)
            return;
        wsprintfA(txt, "%5d", lara.m16.ammo);
        break;
    case 191:
        if (Inv_itemText[IT_QTY] || savegame.bonus_flag)
            return;
        wsprintfA(txt, "%5d", lara.rocket.ammo);
        break;
    case 192:
        if (Inv_itemText[IT_QTY] || savegame.bonus_flag)
            return;
        wsprintfA(txt, "%5d", lara.grenade.ammo);
        break;
    case 194:
        if (Inv_itemText[IT_QTY])
            return;
        wsprintfA(txt, "%d", qty * 2);
        break;
    case 195:
    case 196:
    case 198:
        if (Inv_itemText[IT_QTY] || savegame.bonus_flag)
            return;
        wsprintfA(txt, "%d", qty * 2);
        break;
    case 197:
        if (Inv_itemText[IT_QTY] || savegame.bonus_flag)
            return;
        wsprintfA(txt, "%d", lara.harpoon.ammo);
        break;
    case 199:
    case 200:
    case 203:
        if (Inv_itemText[IT_QTY] || savegame.bonus_flag)
            return;
        wsprintfA(txt, "%d", qty);
        break;
    case 201:
    case 202:
        health_bar_timer = 40;
        DrawHealthBar(FlashIt());
    case 204:
    case 209:
    case 210:
    case 211:
    case 212:
    case 228:
    case 229:
    case 230:
    case 231:
    case 238:
    case 239:
    case 244:
    case 245:
    case 246:
    case 247:
        if (Inv_itemText[IT_QTY] || qty <= 1)
            return;
        wsprintfA(txt, "%d", qty);
        break;
    default:
        return;
    }
    if (!Inv_itemText[IT_QTY]) {
        Inv_itemText[IT_QTY] = T_Print(64, -56, 3, txt);
        T_BottomAlign(Inv_itemText[IT_QTY], 1);
        T_CentreH(Inv_itemText[IT_QTY], 1);
    }
}

void RingActive(void)
{
    T_RemovePrint(Inv_itemText[IT_NAME]);
    Inv_itemText[IT_NAME] = NULL;
    T_RemovePrint(Inv_itemText[IT_QTY]);
    Inv_itemText[IT_QTY] = NULL;
}

extern INVENTORY_ITEM *inv_main_list[];
extern sint16 inv_main_qtys[];
extern INVENTORY_ITEM *inv_keys_list[];
extern sint16 inv_keys_qtys[];

int Inv_GetItemOption(int item_num);

void Inv_InsertItem(INVENTORY_ITEM *inv_item)
{
    int n, m;
    INVENTORY_ITEM *inv_place;

    if (inv_item->inv_pos < 100) {
        for (n = 0; n < inv_main_objects; n++) {
            inv_place = inv_main_list[n];
            if (inv_place->inv_pos > inv_item->inv_pos)
                break;
        }
        if (n == inv_main_objects) {
            inv_main_list[inv_main_objects] = inv_item;
            inv_main_qtys[inv_main_objects] = 1;
            inv_main_objects++;
        } else {
            for (m = inv_main_objects; m > n - 1; m--) {
                inv_main_list[m + 1] = inv_main_list[m];
                inv_main_qtys[m + 1] = inv_main_qtys[m];
            }
            inv_main_list[n] = inv_item;
            inv_main_qtys[n] = 1;
            inv_main_objects++;
        }
    } else {
        for (n = 0; n < inv_keys_objects; n++) {
            inv_place = inv_keys_list[n];
            if (inv_place->inv_pos > inv_item->inv_pos)
                break;
        }
        if (n == inv_keys_objects) {
            inv_keys_list[inv_keys_objects] = inv_item;
            inv_keys_qtys[inv_keys_objects] = 1;
            inv_keys_objects++;
        } else {
            for (m = inv_keys_objects; m > n - 1; m--) {
                inv_keys_list[m + 1] = inv_keys_list[m];
                inv_keys_qtys[m + 1] = inv_keys_qtys[m];
            }
            inv_keys_list[n] = inv_item;
            inv_keys_qtys[n] = 1;
            inv_keys_objects++;
        }
    }
}

int Inv_RequestItem(int item_num)
{
    int n;
    int option;

    option = Inv_GetItemOption(item_num);
    for (n = 0; n < inv_main_objects; n++) {
        if (inv_main_list[n]->object_number == option)
            return inv_main_qtys[n];
    }
    for (n = 0; n < inv_keys_objects; n++) {
        if (inv_keys_list[n]->object_number == option)
            return inv_keys_qtys[n];
    }
    return 0;
}

void Inv_RemoveAllItems(void)
{
    inv_main_objects = 0;
    inv_main_current = 0;
    inv_keys_objects = 0;
    inv_keys_current = 0;
}

int Inv_RemoveItem(int itemNum)
{
    int n, m;
    int itemNumOption;
    INVENTORY_ITEM *inv_item;

    itemNumOption = Inv_GetItemOption(itemNum);
    for (n = 0; n < inv_main_objects; n++) {
        inv_item = inv_main_list[n];
        if (inv_item->object_number == itemNumOption) {
            inv_main_qtys[n]--;
            if (inv_main_qtys[n] <= 0) {
                inv_main_objects--;
                for (m = n; m < inv_main_objects; m++) {
                    inv_main_list[m] = inv_main_list[m + 1];
                    inv_main_qtys[m] = inv_main_qtys[m + 1];
                }
            }
            return 1;
        }
    }
    for (n = 0; n < inv_keys_objects; n++) {
        inv_item = inv_keys_list[n];
        if (inv_item->object_number == itemNumOption) {
            inv_keys_qtys[n]--;
            if (inv_keys_qtys[n] <= 0) {
                inv_keys_objects--;
                for (m = n; m < inv_keys_objects; m++) {
                    inv_keys_list[m] = inv_keys_list[m + 1];
                    inv_keys_qtys[m] = inv_keys_qtys[m + 1];
                }
            }
            return 1;
        }
    }
    return 0;
}

int Inv_GetItemOption(int item_num)
{
    switch (item_num) {
    case 160:
    case 185:
        return 185;
    case 178:
    case 179:
    case 203:
        return 203;
    case 161:
    case 186:
        return 186;
    case 162:
    case 187:
        return 187;
    case 163:
    case 188:
        return 188;
    case 165:
    case 190:
        return 190;
    case 166:
    case 191:
        return 191;
    case 167:
    case 192:
        return 192;
    case 164:
    case 189:
        return 189;
    case 169:
    case 194:
        return 194;
    case 170:
    case 195:
        return 195;
    case 172:
    case 197:
        return 197;
    case 173:
    case 198:
        return 198;
    case 174:
    case 199:
        return 199;
    case 175:
    case 200:
        return 200;
    case 171:
    case 196:
        return 196;
    case 176:
    case 201:
        return 201;
    case 177:
    case 202:
        return 202;
    case 205:
    case 209:
        return 209;
    case 206:
    case 210:
        return 210;
    case 207:
    case 211:
        return 211;
    case 208:
    case 212:
        return 212;
    case 236:
    case 238:
        return 238;
    case 237:
    case 239:
        return 239;
    case 224:
    case 228:
        return 228;
    case 225:
    case 229:
        return 229;
    case 226:
    case 230:
        return 230;
    case 227:
    case 231:
        return 231;
    case 240:
    case 244:
        return 244;
    case 241:
    case 245:
        return 245;
    case 242:
    case 246:
        return 246;
    case 243:
    case 247:
        return 247;
    }
    return -1;
}

void RemoveInventoryText(void)
{
    int n;

    for (n = 0; n < IT_NUMBEROF; n++)
    {
        T_RemovePrint(Inv_itemText[n]);
        Inv_itemText[n] = NULL;
    }
}

extern sint32 Inventory_Mode;

void Inv_RingMotionInit(RING_INFO *ring, sint16 frames, sint16 status, sint16 status_target);
void Inv_RingMotionRadius(RING_INFO *ring, sint16 target);
void Inv_RingMotionCameraPos(RING_INFO *ring, sint16 target);
void Inv_RingMotionRotation(RING_INFO *ring, sint16 rotation, sint16 target);

void Inv_RingInit(RING_INFO *ring, sint16 type, INVENTORY_ITEM **list, sint16 qty, sint16 current, IMOTION_INFO *imo)
{
    ring->type = type;
    ring->radius = 0;
    ring->list = list;
    ring->number_of_objects = qty;
    ring->current_object = current;
    ring->angle_adder = 0x10000 / qty;
    if (Inventory_Mode != 1)
        ring->camera_pitch = 0;
    else
        ring->camera_pitch = 0x400;
    ring->rotating = 0;
    ring->rot_count = 0;
    ring->target_object = 0;
    ring->rot_adder = 0;
    ring->rot_adderL = 0;
    ring->rot_adderR = 0;
    ring->imo = imo;
    ring->camerapos.x_pos = 0;
    ring->camerapos.y_pos = -0x600;
    ring->camerapos.z_pos = 0x380;
    ring->camerapos.x_rot = 0;
    ring->camerapos.y_rot = 0;
    ring->camerapos.z_rot = 0;

    Inv_RingMotionInit(ring, 32, 0, 1);
    Inv_RingMotionRadius(ring, 0x2b0);
    Inv_RingMotionCameraPos(ring, -0x100);
    Inv_RingMotionRotation(ring, -0x8000, (sint16)(0xc000 - ring->current_object * ring->angle_adder));

    ring->ringpos.x_pos = 0;
    ring->ringpos.y_pos = 0;
    ring->ringpos.z_pos = 0;
    ring->ringpos.x_rot = 0;
    ring->ringpos.y_rot = imo->rotate_target + 0x8000;
    ring->ringpos.z_rot = 0;
    ring->light.x = -0x600;
    ring->light.y = 0x100;
    ring->light.z = 0x400;
}

void Inv_RingGetView(RING_INFO *ring, PHD_3DPOS *viewer)
{
    PHD_ANGLE angles[2];

    phd_GetVectorAngles(0 - ring->camerapos.x_pos, CAMERA_YOFFSET - ring->camerapos.y_pos, ring->radius - ring->camerapos.z_pos, angles);
    viewer->x_pos = ring->camerapos.x_pos;
    viewer->y_pos = ring->camerapos.y_pos;
    viewer->z_pos = ring->camerapos.z_pos;
    viewer->x_rot = angles[1] + ring->camera_pitch;
    viewer->y_rot = angles[0];
    viewer->z_rot = 0;
}

void Inv_RingCalcAdders(RING_INFO *ring, sint16 rotation_duration)
{
    ring->angle_adder = 0x10000 / ring->number_of_objects;
    ring->rot_adderL = ring->angle_adder / rotation_duration;
    ring->rot_adderR = -ring->rot_adderL;
}

void Inv_RingDoMotions(RING_INFO *ring)
{
    INVENTORY_ITEM *inv_item;
    IMOTION_INFO *imo;

    imo = ring->imo;
    if (imo->count) {
        ring->radius += imo->radius_rate;
        ring->camerapos.y_pos += imo->camera_yrate;
        ring->ringpos.y_rot += imo->rotate_rate;
        ring->camera_pitch += imo->camera_pitch_rate;
        inv_item = *(ring->list + ring->current_object);
        inv_item->pt_xrot += imo->item_ptxrot_rate;
        inv_item->x_rot += imo->item_xrot_rate;
        inv_item->ytrans += imo->item_ytrans_rate;
        inv_item->ztrans += imo->item_ztrans_rate;

        imo->count--;
        if (!imo->count) {
            imo->status = imo->status_target;
            if (imo->radius_rate) {
                imo->radius_rate = 0;
                ring->radius = imo->radius_target;
            }
            if (imo->camera_yrate) {
                imo->camera_yrate = 0;
                ring->camerapos.y_pos = imo->camera_ytarget;
            }
            if (imo->rotate_rate) {
                imo->rotate_rate = 0;
                ring->ringpos.y_rot = imo->rotate_target;
            }
            if (imo->item_ptxrot_rate) {
                imo->item_ptxrot_rate = 0;
                inv_item->pt_xrot = imo->item_ptxrot_target;
            }
            if (imo->item_xrot_rate) {
                imo->item_xrot_rate = 0;
                inv_item->x_rot = imo->item_xrot_target;
            }
            if (imo->item_ytrans_rate) {
                imo->item_ytrans_rate = 0;
                inv_item->ytrans = imo->item_ytrans_target;
            }
            if (imo->item_ztrans_rate) {
                imo->item_ztrans_rate = 0;
                inv_item->ztrans = imo->item_ztrans_target;
            }
            if (imo->camera_pitch_rate) {
                imo->camera_pitch_rate = 0;
                ring->camera_pitch = imo->camera_pitch_target;
            }
        }
    }

    if (ring->rotating) {
        ring->ringpos.y_rot += ring->rot_adder;
        ring->rot_count--;
        if (ring->rot_count == 0) {
            ring->current_object = ring->target_object;
            ring->ringpos.y_rot = 0xc000 - ring->current_object * ring->angle_adder;
            ring->rotating = 0;
        }
    }
}

void Inv_RingRotateLeft(RING_INFO *ring)
{
    ring->rotating = 1;
    ring->target_object = ring->current_object - 1;
    if (ring->target_object < 0)
        ring->target_object = ring->number_of_objects - 1;
    ring->rot_count = ROTATE_DURATION;
    ring->rot_adder = ring->rot_adderL;
}

void Inv_RingRotateRight(RING_INFO *ring)
{
    ring->rotating = 1;
    ring->target_object = ring->current_object + 1;
    if (ring->target_object >= ring->number_of_objects)
        ring->target_object = 0;
    ring->rot_count = ROTATE_DURATION;
    ring->rot_adder = ring->rot_adderR;
}

void Inv_RingMotionInit(RING_INFO *ring, sint16 frames, sint16 status, sint16 status_target)
{
    IMOTION_INFO *imo;

    imo = ring->imo;
    imo->count = frames;
    imo->status = status;
    imo->status_target = status_target;
    imo->radius_target = 0;
    imo->radius_rate = 0;
    imo->camera_ytarget = 0;
    imo->camera_yrate = 0;
    imo->camera_pitch_target = 0;
    imo->camera_pitch_rate = 0;
    imo->rotate_target = 0;
    imo->rotate_rate = 0;
    imo->item_ptxrot_target = 0;
    imo->item_ptxrot_rate = 0;
    imo->item_xrot_target = 0;
    imo->item_xrot_rate = 0;
    imo->item_ytrans_target = 0;
    imo->item_ytrans_rate = 0;
    imo->item_ztrans_target = 0;
    imo->item_ztrans_rate = 0;
    imo->misc = 0;
}

void Inv_RingMotionSetup(RING_INFO *ring, sint16 status, sint16 status_target, sint16 frames)
{
    IMOTION_INFO *imo;

    imo = ring->imo;
    imo->count = frames;
    imo->status = status;
    imo->status_target = status_target;
    imo->radius_rate = 0;
    imo->camera_yrate = 0;
}

void Inv_RingMotionRadius(RING_INFO *ring, sint16 target)
{
    IMOTION_INFO *imo;

    imo = ring->imo;
    imo->radius_target = target;
    imo->radius_rate = (target - ring->radius) / imo->count;
}

void Inv_RingMotionRotation(RING_INFO *ring, sint16 rotation, sint16 target)
{
    IMOTION_INFO *imo;

    imo = ring->imo;
    imo->rotate_rate = rotation / imo->count;
    imo->rotate_target = target;
}

void Inv_RingMotionCameraPos(RING_INFO *ring, sint16 target)
{
    IMOTION_INFO *imo;

    imo = ring->imo;
    imo->camera_ytarget = target;
    imo->camera_yrate = (target - ring->camerapos.y_pos) / imo->count;
}

void Inv_RingMotionCameraPitch(RING_INFO *ring, sint16 target)
{
    IMOTION_INFO *imo;

    imo = ring->imo;
    imo->camera_pitch_target = target;
    imo->camera_pitch_rate = target / imo->count;
}

void Inv_RingMotionItemSelect(RING_INFO *ring, INVENTORY_ITEM *inv_item)
{
    IMOTION_INFO *imo;

    imo = ring->imo;
    imo->item_ptxrot_target = inv_item->pt_xrot_sel;
    imo->item_ptxrot_rate = inv_item->pt_xrot_sel / imo->count;
    imo->item_xrot_target = inv_item->x_rot_sel;
    imo->item_xrot_rate = (inv_item->x_rot_sel - inv_item->x_rot_nosel) / imo->count;
    imo->item_ytrans_target = inv_item->ytrans_sel;
    imo->item_ytrans_rate = inv_item->ytrans_sel / imo->count;
    imo->item_ztrans_target = inv_item->ztrans_sel;
    imo->item_ztrans_rate = inv_item->ztrans_sel / imo->count;
}

void Inv_RingMotionItemDeselect(RING_INFO *ring, INVENTORY_ITEM *inv_item)
{
    IMOTION_INFO *imo;

    imo = ring->imo;
    imo->item_ptxrot_target = 0;
    imo->item_ptxrot_rate = 0 - inv_item->pt_xrot_sel / imo->count;
    imo->item_xrot_target = inv_item->x_rot_nosel;
    imo->item_xrot_rate = (inv_item->x_rot_nosel - inv_item->x_rot_sel) / imo->count;
    imo->item_ytrans_target = 0;
    imo->item_ytrans_rate = 0 - inv_item->ytrans_sel / imo->count;
    imo->item_ztrans_target = 0;
    imo->item_ztrans_rate = 0 - inv_item->ztrans_sel / imo->count;
}

extern uint32 RequesterFlags1[];
extern uint32 RequesterFlags2[];

void Init_Requester(REQUEST_INFO *req)
{
    int n;

    req->items = 0;
    req->heading1text = NULL;
    req->heading2text = NULL;
    req->heading1_flags = 0;
    req->heading2_flags = 0;
    req->backgroundtext = NULL;
    req->background_flags = 1;
    req->moreuptext = NULL;
    req->moreup_flags = 1;
    req->moredowntext = NULL;
    req->moredown_flags = 1;
    for (n = 0; n < MAX_REQLINES; n++) {
        req->texts1[n] = NULL;
        req->texts1_flags[n] = 0;
        req->texts2[n] = NULL;
        req->texts2_flags[n] = 0;
    }
    req->itemtexts1_flags = RequesterFlags1;
    req->itemtexts2_flags = RequesterFlags2;
    req->original_render_width = GetRenderWidth();
    req->original_render_height = GetRenderHeight();
}

void Remove_Requester(REQUEST_INFO *req)
{
    int n;

    T_RemovePrint(req->heading1text);
    req->heading1text = NULL;
    T_RemovePrint(req->heading2text);
    req->heading2text = NULL;
    T_RemovePrint(req->backgroundtext);
    req->backgroundtext = NULL;
    T_RemovePrint(req->moreuptext);
    req->moreuptext = NULL;
    T_RemovePrint(req->moredowntext);
    req->moredowntext = NULL;
    for (n = 0; n < MAX_REQLINES; n++) {
        T_RemovePrint(req->texts1[n]);
        req->texts1[n] = NULL;
        T_RemovePrint(req->texts2[n]);
        req->texts2[n] = NULL;
    }
}

void ReqItemCentreAlign(REQUEST_INFO *req, TEXTSTRING *txt)
{
    if (txt)
    {
        txt->xpos = req->xpos;
        txt->bgndOffX = 0;
    }
}

void ReqItemLeftalign(REQUEST_INFO *req, TEXTSTRING *txt)
{
    int x;
    uint32 scaleH = GetTextScaleH(txt->scaleH);

    if (txt)
    {
        x = (((req->pixwidth * scaleH) >> 16) / 2 - T_GetTextWidth(txt) / 2) - ((8 * scaleH) >> 16);
        txt->xpos = req->xpos - x;
        txt->bgndOffX = x;
    }
}

void ReqItemRightalign(REQUEST_INFO *req, TEXTSTRING *txt)
{
    int x;
    uint32 scaleH = GetTextScaleH(txt->scaleH);

    if (txt)
    {
        x = (((req->pixwidth * scaleH) >> 16) / 2 - T_GetTextWidth(txt) / 2) - ((8 * scaleH) >> 16);
        txt->xpos = req->xpos + x;
        txt->bgndOffX = -x;
    }
}

void SetRequesterHeading(REQUEST_INFO *req, char *text1, uint32 flags1, char *text2, uint32 flags2)
{
    T_RemovePrint(req->heading1text);
    req->heading1text = NULL;
    T_RemovePrint(req->heading2text);
    req->heading2text = NULL;
    if (text1) {
        sprintf(req->heading1_str, text1);
        req->heading1_flags = flags1 | 1;
    } else {
        sprintf(req->heading1_str, "u");
        req->heading1_flags = 0;
    }
    if (text2) {
        sprintf(req->heading2_str, text2);
        req->heading2_flags = flags2 | 1;
    } else {
        sprintf(req->heading2_str, "u");
        req->heading2_flags = 0;
    }
}

void RemoveAllReqItems(REQUEST_INFO *req)
{
    req->items = 0;
    req->line_offset = 0;
    req->selected = 0;
}

void ChangeRequesterItem(REQUEST_INFO *req, int item, char *text1, uint32 flags1, char *text2, uint32 flags2)
{
    T_RemovePrint(req->texts1[item]);
    req->texts1[item] = NULL;
    T_RemovePrint(req->texts2[item]);
    req->texts2[item] = NULL;
    if (text1) {
        sprintf(req->itemtexts1 + item * req->itemtextlen, text1);
        req->itemtexts1_flags[item] = flags1 | 1;
    } else {
        req->itemtexts1_flags[item] = 0;
    }
    if (text2) {
        sprintf(req->itemtexts2 + item * req->itemtextlen, text2);
        req->itemtexts2_flags[item] = flags2 | 1;
    } else {
        req->itemtexts2_flags[item] = 0;
    }
}

void AddRequesterItem(REQUEST_INFO *req, char *text1, uint32 flags1, char *text2, uint32 flags2)
{
    req->itemtexts1_flags = RequesterFlags1;
    req->itemtexts2_flags = RequesterFlags2;
    if (text1) {
        sprintf(req->itemtexts1 + req->items * req->itemtextlen, text1);
        req->itemtexts1_flags[req->items] = flags1 | 1;
    } else {
        req->itemtexts1_flags[req->items] = 0;
    }
    if (text2) {
        sprintf(req->itemtexts2 + req->items * req->itemtextlen, text2);
        req->itemtexts2_flags[req->items] = flags2 | 1;
    } else {
        req->itemtexts2_flags[req->items] = 0;
    }
    req->items++;
}

void SetPCRequesterSize(REQUEST_INFO *req, int maxlines, int ypos)
{
    int height = GetRenderHeight();
    int lines = (height / 2) / (TEXT_HEIGHT + 7);

    if (lines > maxlines)
        lines = maxlines;
    req->ypos = ypos;
    req->vis_lines = lines;
}

int AddAssaultTime(uint32 time)
{
    int i, j, insert = 0;

    for (i = 0; i < NUM_ASS_TIMES; i++)
    {
        if (savegame.best_assault_times[i] == 0 || time < savegame.best_assault_times[i])
        {
            insert = 1;
            break;
        }
    }
    if (insert)
    {
        for (j = NUM_ASS_TIMES - 1; j > i; j--)
            savegame.best_assault_times[j] = savegame.best_assault_times[j - 1];
        savegame.best_assault_times[i] = time;
        return 1;
    }
    return 0;
}

int AddQuadbikeTime(uint32 time)
{
    int i, j, insert = 0;

    for (i = 0; i < NUM_ASS_TIMES; i++)
    {
        if (savegame.best_quadbike_times[i] == 0 || time < savegame.best_quadbike_times[i])
        {
            insert = 1;
            break;
        }
    }
    if (insert)
    {
        for (j = NUM_ASS_TIMES - 1; j > i; j--)
            savegame.best_quadbike_times[j] = savegame.best_quadbike_times[j - 1];
        savegame.best_quadbike_times[i] = time;
        return 1;
    }
    return 0;
}

extern REQUEST_INFO Stats_Requester;
extern char Valid_Level_Strings[24][50];
extern char Valid_Level_Strings2[24][50];
extern char **GF_Level_Names;
extern uint32 CurrentLevel;
extern sint16 LevelSecrets[];
extern int inputDB;
sint32 Display_Requester(REQUEST_INFO *req, int des, int backgrounds);

static int gym_mode;

void ShowGymStatsText(char *time_str, int type)
{
    int select, ms, s, hs, i;
    char str[32];

    if (gym_mode == 1) {
        if ((select = Display_Requester(&Stats_Requester, 1, 1)))
            gym_mode = 0;
        else
            input = inputDB = 0;
        return;
    }
    Stats_Requester.noselector = 1;
    SetPCRequesterSize(&Stats_Requester, 7, -32);
    Stats_Requester.line_height = 18;
    Stats_Requester.items = 0;
    Stats_Requester.selected = 0;
    Stats_Requester.line_offset = 0;
    Stats_Requester.line_oldoffset = 0;
    Stats_Requester.pixwidth = 296;
    Stats_Requester.xpos = 0;
    Stats_Requester.zpos = 0;
    Stats_Requester.itemtexts1 = Valid_Level_Strings[0];
    Stats_Requester.itemtexts2 = Valid_Level_Strings2[0];
    Stats_Requester.itemtextlen = 50;
    Init_Requester(&Stats_Requester);
    SetRequesterHeading(&Stats_Requester, GF_GameStrings[73], 0, NULL, 0);
    for (i = -1; i < 10; i++) {
        if (i == -1) {
            if (gameflow.language == 1)
                AddRequesterItem(&Stats_Requester, "Entra(inement", 8, NULL, 0);
            else if (gameflow.language == 2)
                AddRequesterItem(&Stats_Requester, "Hinderniskurs", 8, NULL, 0);
            else if (gameflow.language == 5)
                AddRequesterItem(&Stats_Requester, "Circuito d'addestramento", 8, NULL, 0);
            else if (gameflow.language == 6)
                AddRequesterItem(&Stats_Requester, "Asaltar Carrera", 8, NULL, 0);
            else
                AddRequesterItem(&Stats_Requester, "Assault Course", 8, NULL, 0);
            continue;
        }
        if (!savegame.best_assault_times[i])
            break;
        s = savegame.best_assault_times[i] / 30;
        hs = savegame.best_assault_times[i] % 30 * 334 / 100;
        ms = s / 60;
        s = s % 60;
        sprintf(str, "%d:%2.2d.%02d", ms, s, hs);
        if (i == 0)
            AddRequesterItem(&Stats_Requester, str, 16, NULL, 0);
        else
            AddRequesterItem(&Stats_Requester, str, 32, NULL, 0);
    }
    if (i == 0)
        AddRequesterItem(&Stats_Requester, GF_GameStrings[74], 64, NULL, 0);
    if (savegame.QuadbikeKeyFlag) {
        AddRequesterItem(&Stats_Requester, " ", 0, NULL, 0);
        for (i = -1; i < 10; i++) {
            if (i == -1) {
                if (gameflow.language == 1)
                    AddRequesterItem(&Stats_Requester, "Course de moto", 8, NULL, 0);
                else if (gameflow.language == 2)
                    AddRequesterItem(&Stats_Requester, "Quadbike Teststrecke", 8, NULL, 0);
                else if (gameflow.language == 5)
                    AddRequesterItem(&Stats_Requester, "Circuito per moto", 8, NULL, 0);
                else if (gameflow.language == 6)
                    AddRequesterItem(&Stats_Requester, "Circuito Moto Quad", 8, NULL, 0);
                else
                    AddRequesterItem(&Stats_Requester, "Quadbike Track", 8, NULL, 0);
                continue;
            }
            if (!savegame.best_quadbike_times[i])
                break;
            s = savegame.best_quadbike_times[i] / 30;
            hs = savegame.best_quadbike_times[i] % 30 * 334 / 100;
            ms = s / 60;
            s = s % 60;
            sprintf(str, "%d:%2.2d.%02d", ms, s, hs);
            if (i == 0)
                AddRequesterItem(&Stats_Requester, str, 16, NULL, 0);
            else
                AddRequesterItem(&Stats_Requester, str, 32, NULL, 0);
        }
        if (i == 0)
            AddRequesterItem(&Stats_Requester, GF_GameStrings[74], 64, NULL, 0);
    }
    gym_mode = 1;
}

static int stats_mode;

void ShowStatsText(char *time_str, int type)
{
    int select, numsecrets, distance;
    char str[32];

    if (stats_mode == 1) {
        ChangeRequesterItem(&Stats_Requester, 0, GF_GameStrings[61], 2, time_str, 4);
        if ((select = Display_Requester(&Stats_Requester, type, 1)))
            stats_mode = 0;
        else
            input = inputDB = 0;
        return;
    }
    Stats_Requester.noselector = 1;
    SetPCRequesterSize(&Stats_Requester, 7, -32);
    Stats_Requester.line_height = 18;
    Stats_Requester.items = 0;
    Stats_Requester.selected = 0;
    Stats_Requester.line_offset = 0;
    Stats_Requester.line_oldoffset = 0;
    Stats_Requester.pixwidth = 304;
    Stats_Requester.xpos = 0;
    Stats_Requester.zpos = 0;
    Stats_Requester.itemtexts1 = Valid_Level_Strings[0];
    Stats_Requester.itemtexts2 = Valid_Level_Strings2[0];
    Stats_Requester.itemtextlen = 50;
    Init_Requester(&Stats_Requester);
    SetRequesterHeading(&Stats_Requester, GF_Level_Names[CurrentLevel], 0, NULL, 0);
    AddRequesterItem(&Stats_Requester, GF_GameStrings[61], 2, time_str, 4);
    numsecrets = (savegame.secrets & 1) + ((savegame.secrets & 2) >> 1) + ((savegame.secrets & 4) >> 2) + ((savegame.secrets & 8) >> 3)
        + ((savegame.secrets & 16) >> 4) + ((savegame.secrets & 32) >> 5) + ((savegame.secrets & 64) >> 6) + ((savegame.secrets & 128) >> 7);
    if (LevelSecrets[CurrentLevel])
        sprintf(str, "%d %s %d", numsecrets, GF_GameStrings[78], LevelSecrets[CurrentLevel]);
    else
        sprintf(str, "%s", GF_GameStrings[71]);
    AddRequesterItem(&Stats_Requester, GF_GameStrings[62], 2, str, 4);
    sprintf(str, "%d", savegame.kills);
    AddRequesterItem(&Stats_Requester, GF_GameStrings[64], 2, str, 4);
    sprintf(str, "%d", savegame.ammo_used);
    AddRequesterItem(&Stats_Requester, GF_GameStrings[65], 2, str, 4);
    sprintf(str, "%d", savegame.ammo_hit);
    AddRequesterItem(&Stats_Requester, GF_GameStrings[66], 2, str, 4);
    if (savegame.health_used & 1)
        sprintf(str, "%d.5", savegame.health_used >> 1);
    else
        sprintf(str, "%d.0", savegame.health_used >> 1);
    AddRequesterItem(&Stats_Requester, GF_GameStrings[69], 2, str, 4);
    distance = savegame.distance_travelled / 445;
    if (distance >= 1000)
        sprintf(str, "%d.%02dkm", distance / 1000, distance % 100);
    else
        sprintf(str, "%dm", distance);
    AddRequesterItem(&Stats_Requester, GF_GameStrings[68], 2, str, 4);
    stats_mode = 1;
}

static int end_stats_mode;

void ShowEndStatsText(void)
{
    int select, numsecrets, distance, i, num;
    char str[32];
    int h = 0, m = 0, s = 0;
    int totallevels = gameflow.num_levels - gameflow.num_demos;

    if (end_stats_mode == 1) {
        if ((select = Display_Requester(&Stats_Requester, 0, 1)))
            end_stats_mode = 0;
        else
            input = inputDB = 0;
        return;
    }
    Stats_Requester.noselector = 1;
    SetPCRequesterSize(&Stats_Requester, 7, -32);
    Stats_Requester.line_height = 18;
    Stats_Requester.items = 0;
    Stats_Requester.selected = 0;
    Stats_Requester.line_offset = 0;
    Stats_Requester.line_oldoffset = 0;
    Stats_Requester.pixwidth = 304;
    Stats_Requester.xpos = 0;
    Stats_Requester.zpos = 0;
    Stats_Requester.itemtexts1 = Valid_Level_Strings[0];
    Stats_Requester.itemtexts2 = Valid_Level_Strings2[0];
    Stats_Requester.itemtextlen = 50;
    Init_Requester(&Stats_Requester);
    SetRequesterHeading(&Stats_Requester, GF_GameStrings[77], 0, NULL, 0);
    s = 0;
    for (i = 1; i < totallevels; i++)
        s += savegame.start[i].timer;
    s /= 30;
    h = s / 3600;
    m = s / 60 % 60;
    s %= 60;
    sprintf(str, "%02d:%02d:%02d", h, m, s);
    AddRequesterItem(&Stats_Requester, GF_GameStrings[61], 2, str, 4);
    numsecrets = num = 0;
    for (i = 0; i < totallevels; i++) {
        numsecrets += (savegame.start[i].secrets_found & 1) + ((savegame.start[i].secrets_found & 2) >> 1)
            + ((savegame.start[i].secrets_found & 4) >> 2) + ((savegame.start[i].secrets_found & 8) >> 3)
            + ((savegame.start[i].secrets_found & 16) >> 4) + ((savegame.start[i].secrets_found & 32) >> 5)
            + ((savegame.start[i].secrets_found & 64) >> 6) + ((savegame.start[i].secrets_found & 128) >> 7);
        num += LevelSecrets[i];
    }
    sprintf(str, "%d %s %d", numsecrets, GF_GameStrings[78], num);
    AddRequesterItem(&Stats_Requester, GF_GameStrings[62], 2, str, 4);
    num = 0;
    for (i = 1; i < totallevels; i++)
        num += savegame.start[i].kills;
    sprintf(str, "%d", num);
    AddRequesterItem(&Stats_Requester, GF_GameStrings[64], 2, str, 4);
    num = 0;
    for (i = 1; i < totallevels; i++)
        num += savegame.start[i].ammo_used;
    sprintf(str, "%d", num);
    AddRequesterItem(&Stats_Requester, GF_GameStrings[65], 2, str, 4);
    num = 0;
    for (i = 1; i < totallevels; i++)
        num += savegame.start[i].ammo_hit;
    sprintf(str, "%d", num);
    AddRequesterItem(&Stats_Requester, GF_GameStrings[66], 2, str, 4);
    num = 0;
    for (i = 1; i < totallevels; i++)
        num += savegame.start[i].health_used;
    if (num & 1)
        sprintf(str, "%d.5", num >> 1);
    else
        sprintf(str, "%d.0", num >> 1);
    AddRequesterItem(&Stats_Requester, GF_GameStrings[69], 2, str, 4);
    num = 0;
    for (i = 1; i < totallevels; i++)
        num += savegame.start[i].distance_travelled;
    distance = num / 445;
    if (distance >= 1000)
        sprintf(str, "%d.%02dkm", distance / 1000, distance % 100);
    else
        sprintf(str, "%dm", distance);
    AddRequesterItem(&Stats_Requester, GF_GameStrings[68], 2, str, 4);
    end_stats_mode = 1;
}
