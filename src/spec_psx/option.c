#include "../game.h"

__declspec(dllimport) int __cdecl wsprintfA(char *, const char *, ...);
int sprintf(char *buf, const char *fmt, ...);

extern sint16 layout[2][14];
extern char *kname[];
extern int iconfig;
extern int conflict[14];
extern TEXTSTRING *ctext[14];
extern TEXTSTRING *btext[14];
extern TEXTSTRING *ctrltext[2];
extern sint32 inputDB;
extern int CurrentLevel;
extern char **GF_PCStrings;

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
