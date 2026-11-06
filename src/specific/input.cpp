#include "standard.h"
#include "specific.h"
#include "winmain.h"

extern "C" {

#include "../game.h"

extern sint16 layout[2][14];
extern int conflict[15];
extern uint8 keymap[256];
extern int joy_fire;
extern int joy_x;
extern int joy_y;
extern int farz;
extern int distanceFogValue;
extern bool pictureFading;
extern int GnGameMode;
extern int nLoadedPictures;
extern int FinishLevelCheat;
extern bool GtWindowClosed;
extern int med_debounce;

bool DD_SpinMessageLoop(bool tWait);
void DI_ReadKeyboard(unsigned char *KeyMap);
int DI_ReadJoystick(int *x, int *y);
void IncreaseScreenSize(void);
void DecreaseScreenSize(void);
int Inv_RequestItem(int item_num);
void UseItem(sint16 object_num);
void DXSaveScreen(LPDIRECTDRAWSURFACE3 lpSurface);

int Key(int number)
{
    sint16 key;

    key = layout[1][number];

    if (key >= 256) {
        if ((1 << (key & 0xff)) & joy_fire)
            return 1;
    } else {
        if (keymap[key] & 0x80)
            return 1;

        if (key == DIK_RCONTROL)
            return keymap[DIK_LCONTROL] & 0x80;
        if (key == DIK_LCONTROL)
            return keymap[DIK_RCONTROL] & 0x80;
        if (key == DIK_RSHIFT)
            return keymap[DIK_LSHIFT] & 0x80;
        if (key == DIK_LSHIFT)
            return keymap[DIK_RSHIFT] & 0x80;
        if (key == DIK_RMENU)
            return keymap[DIK_LMENU] & 0x80;
        if (key == DIK_LMENU)
            return keymap[DIK_RMENU] & 0x80;
    }

    if (conflict[number])
        return 0;

    key = layout[0][number];

    if (keymap[key] & 0x80)
        return 1;

    if (key == DIK_RCONTROL)
        return keymap[DIK_LCONTROL] & 0x80;
    if (key == DIK_LCONTROL)
        return keymap[DIK_RCONTROL] & 0x80;
    if (key == DIK_RSHIFT)
        return keymap[DIK_LSHIFT] & 0x80;
    if (key == DIK_LSHIFT)
        return keymap[DIK_RSHIFT] & 0x80;
    if (key == DIK_RMENU)
        return keymap[DIK_LMENU] & 0x80;
    if (key == DIK_LMENU)
        return keymap[DIK_RMENU] & 0x80;

    return 0;
}


}
