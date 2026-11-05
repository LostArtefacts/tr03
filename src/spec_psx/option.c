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
