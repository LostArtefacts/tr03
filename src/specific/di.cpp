#include "standard.h"
#include "specific.h"
#include "winmain.h"

extern "C" {

extern LPDIRECTINPUT lpDirectInput;
extern LPDIRECTINPUTDEVICE Keyboard;

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

void DI_FinishKeyboard(void)
{
    if (Keyboard)
        Keyboard->Unacquire();
    if (Keyboard) {
        Keyboard->Release();
        Keyboard = 0;
    }
}

}
