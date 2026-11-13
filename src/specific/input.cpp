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


int S_UpdateInput(void)
{
    int linput;

    DD_SpinMessageLoop(false);
    DI_ReadKeyboard(keymap);
    joy_fire = DI_ReadJoystick(&joy_x, &joy_y);

    linput = 0;
    if (joy_x < -8)
        linput = IN_LEFT;
    else if (joy_x > 8)
        linput = IN_RIGHT;

    if (joy_y > 8)
        linput |= IN_BACK;
    else if (joy_y < -8)
        linput |= IN_FORWARD;

    if (Key(0))
        linput |= IN_FORWARD;
    if (Key(1))
        linput |= IN_BACK;
    if (Key(2))
        linput |= IN_LEFT;
    if (Key(3))
        linput |= IN_RIGHT;
    if (Key(4))
        linput |= IN_DUCK;
    if (Key(5))
        linput |= IN_DASH;
    if (Key(6))
        linput |= IN_SLOW;
    if (Key(7))
        linput |= IN_JUMP;
    if (Key(8))
        linput |= IN_ACTION;
    if (Key(9))
        linput |= IN_DRAW;
    if (Key(10))
        linput |= IN_F;
    if (Key(11))
        linput |= IN_LOOK;
    if (Key(12))
        linput |= IN_ROLL;

    if ((linput & IN_SLOW) && !(linput & (IN_FORWARD | IN_BACK))) {
        if (linput & IN_LEFT)
            linput = (linput & ~IN_LEFT) | IN_STEPL;
        else if (linput & IN_RIGHT)
            linput = (linput & ~IN_RIGHT) | IN_STEPR;
    }

    if (keymap[DIK_MULTIPLY] & 0x80) {
        farz -= 50;
        if (farz - 0x2000 < 0)
            farz = 0x2000;
        distanceFogValue = farz - 0x2000;
    }

    if (keymap[DIK_DIVIDE] & 0x80) {
        farz += 50;
        if (farz > 0x5000)
            farz = 0x5000;
        distanceFogValue = farz - 0x2000;
    }

    if (Key(13) && camera.type != CINEMATIC_CAMERA && !pictureFading)
        linput |= IN_OPTION;

    if ((linput & IN_FORWARD) && (linput & IN_BACK))
        linput |= IN_ROLL;

    if ((keymap[DIK_RETURN] & 0x80) || (linput & IN_ACTION))
        linput |= IN_SELECT;

    if (keymap[DIK_ESCAPE] & 0x80)
        linput |= IN_DESELECT;

    if ((linput & (IN_LEFT | IN_RIGHT)) == (IN_LEFT | IN_RIGHT))
        linput -= IN_LEFT | IN_RIGHT;

    if (GnGameMode == 1 && !nLoadedPictures) {
        if (keymap[DIK_ADD] & 0x80)
            IncreaseScreenSize();
        if (keymap[DIK_SUBTRACT] & 0x80)
            DecreaseScreenSize();
    }

    if ((keymap[DIK_1] & 0x80) && Inv_RequestItem(185))
        lara.request_gun_type = LG_PISTOLS;
    else if ((keymap[DIK_2] & 0x80) && Inv_RequestItem(186))
        lara.request_gun_type = LG_SHOTGUN;
    else if ((keymap[DIK_3] & 0x80) && Inv_RequestItem(187))
        lara.request_gun_type = LG_MAGNUMS;
    else if ((keymap[DIK_4] & 0x80) && Inv_RequestItem(188))
        lara.request_gun_type = LG_UZIS;
    else if ((keymap[DIK_5] & 0x80) && Inv_RequestItem(189))
        lara.request_gun_type = LG_HARPOON;
    else if ((keymap[DIK_6] & 0x80) && Inv_RequestItem(190))
        lara.request_gun_type = LG_M16;
    else if ((keymap[DIK_7] & 0x80) && Inv_RequestItem(191))
        lara.request_gun_type = LG_ROCKET;
    else if ((keymap[DIK_8] & 0x80) && Inv_RequestItem(192))
        lara.request_gun_type = LG_GRENADE;

    if ((keymap[DIK_0] & 0x80) && Inv_RequestItem(201)) {
        if (!med_debounce) {
            UseItem(201);
            med_debounce = 15;
        }
    } else if ((keymap[DIK_9] & 0x80) && Inv_RequestItem(202)) {
        if (!med_debounce) {
            UseItem(202);
            med_debounce = 15;
        }
    } else if (med_debounce) {
        med_debounce--;
    }

    if (keymap[DIK_APOSTROPHE] & 0x80)
        DXSaveScreen(App.lpFrontBuffer);

    if (FinishLevelCheat) {
        level_complete = 1;
        FinishLevelCheat = 0;
    }

    if (!gameflow.loadsave_disabled) {
        if (keymap[DIK_F5] & 0x80)
            linput |= IN_SAVE;
        else if (keymap[DIK_F6] & 0x80)
            linput |= IN_LOAD;
    }

    input = linput;
    return GtWindowClosed;
}

}

#ifdef TR3_EXE
extern "C" {

int bLaraOn = 1;
int bRoomOn = 1;
int bObjectOn = 1;
int bAObjectOn = 1;
int bEffectOn = 1;
int bDraw = 1;
sint16 layout[2][14] = {
    {
        DIK_UP, DIK_DOWN, DIK_LEFT, DIK_RIGHT, DIK_PERIOD, DIK_SLASH, DIK_RSHIFT, DIK_RMENU,
        DIK_RCONTROL, DIK_SPACE, DIK_COMMA, DIK_NUMPAD0, DIK_END, DIK_ESCAPE,
    },
    {
        DIK_NUMPAD8, DIK_NUMPAD2, DIK_NUMPAD4, DIK_NUMPAD6, DIK_DECIMAL, DIK_NUMPAD1,
        DIK_RSHIFT, DIK_RMENU, DIK_RCONTROL, DIK_SPACE, DIK_NUMPAD5, DIK_NUMPAD0, DIK_END,
        DIK_ESCAPE,
    },
};
char *kname[272] = {
    0, "ESC", "1", "2", "3", "4", "5", "6",
    "7", "8", "9", "0", "-", "+", "BKSP", "TAB",
    "Q", "W", "E", "R", "T", "Y", "U", "I",
    "O", "P", "<", ">", "RET", "CTRL", "A", "S",
    "D", "F", "G", "H", "J", "K", "L", ";",
    "'", "`", "SHIFT", "#", "Z", "X", "C", "V",
    "B", "N", "M", ",", ".", "/", "SHIFT", "PADx",
    "ALT", "SPACE", "CAPS", 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, "NMLK", 0, "PAD7",
    "PAD8", "PAD9", "PAD-", "PAD4", "PAD5", "PAD6", "PAD+", "PAD1",
    "PAD2", "PAD3", "PAD0", "PAD.", 0, 0, "\\", 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, "ENTER", "CTRL", 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, "SHIFT", 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, "PAD/", 0, 0,
    "ALT", 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, "HOME",
    "UP", "PGUP", 0, "LEFT", 0, "RIGHT", 0, "END",
    "DOWN", "PGDN", "INS", "DEL", 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    "JOY1", "JOY2", "JOY3", "JOY4", "JOY5", "JOY6", "JOY7", "JOY8",
    "JOY9", "JOY10", "JOY11", "JOY12", "JOY13", "JOY14", "JOY15", "JOY16",
};

}
#endif
