#include "standard.h"
#include "specific.h"
#include "winmain.h"
#include "template.h"
#include "init_result.h"

struct JOYSTICK : GENERIC_ADAPTER {
};

extern "C" {

extern LPDIRECTINPUT lpDirectInput;
extern LPDIRECTINPUTDEVICE Keyboard;
static LIST<JOYSTICK> JoystickList;
static JOYSTICK CurrentJoystick;
static int no_joy = 1;
static JOYCAPS joystick;

inline bool DX_TRY(HRESULT hr)
{
    if (SUCCEEDED(hr))
        return false;
    return true;
}

bool DI_Create(void)
{
    if (DX_TRY(DirectInputCreate(App.hInstance, DIRECTINPUT_VERSION, &lpDirectInput, 0)))
        return false;
    return true;
}

void DI_Release(void)
{
    if (lpDirectInput) {
        lpDirectInput->Release();
        lpDirectInput = 0;
    }
}

void DI_ReadKeyboard(unsigned char *KeyMap)
{
    HRESULT hr;

    hr = Keyboard->GetDeviceState(256, KeyMap);
    if (FAILED(hr) && hr == DIERR_INPUTLOST) {
        if (DX_TRY(Keyboard->Acquire()) || DX_TRY(Keyboard->GetDeviceState(256, KeyMap)))
            memset(KeyMap, 0, 256);
    }
}

int DI_ReadJoystick(int &x, int &y)
{
    JOYINFOEX jinfo;

    if (!App.DXConfig.Joystick) {
        y = 0;
        x = 0;
        return 0;
    }
    jinfo.dwSize = sizeof(JOYINFOEX);
    jinfo.dwFlags = JOY_RETURNBUTTONS | JOY_RETURNX | JOY_RETURNY;
    if (joyGetPosEx(JOYSTICKID1, &jinfo) != JOYERR_NOERROR) {
        no_joy = 1;
        y = 0;
        x = 0;
        return 0;
    }
    if (no_joy) {
        if (joyGetDevCaps(JOYSTICKID1, &joystick, sizeof(joystick)) == JOYERR_NOERROR) {
            no_joy = 0;
        } else {
            y = 0;
            x = 0;
            return 0;
        }
    }
    x = (jinfo.dwXpos << 5) / (joystick.wXmax - joystick.wXmin) - 16;
    y = (jinfo.dwYpos << 5) / (joystick.wYmax - joystick.wYmin) - 16;
    return jinfo.dwButtons;
}

void DI_StartKeyboard(void)
{
    if (DX_TRY(lpDirectInput->CreateDevice(GUID_SysKeyboard, &Keyboard, 0)))
        throw INIT_ERR_CantCreateKeyboardDevice;
    if (DX_TRY(Keyboard->SetCooperativeLevel(App.WindowHandle, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND)))
        throw INIT_ERR_CantSetKBCooperativeLevel;
    if (DX_TRY(Keyboard->SetDataFormat(&c_dfDIKeyboard)))
        throw INIT_ERR_CantSetKBDataFormat;
    if (DX_TRY(Keyboard->Acquire()))
        throw INIT_ERR_CantAcquireKeyboard;
}

void DI_FinishKeyboard(void)
{
    if (Keyboard)
        Keyboard->Unacquire();
    if (Keyboard) {
        Keyboard->Release();
        Keyboard = 0;
    }
}

bool DI_StartJoystick(void)
{
    return true;
}

void DI_Start(void)
{
    if (!DI_Create())
        throw INIT_ERR_CantCreateDirectInput;
    DI_StartKeyboard();
    DI_StartJoystick();
}

void DI_FinishJoystick(void);

void DI_Finish(void)
{
    DI_FinishJoystick();
    DI_FinishKeyboard();
    DI_Release();
}

}

#ifdef TR3_EXE
extern "C" void DI_FinishJoystick(void)
{
}
#endif
