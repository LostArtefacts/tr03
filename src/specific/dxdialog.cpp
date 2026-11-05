#include "standard.h"
#include "specific.h"
#include "winmain.h"

extern "C" {

extern DEVICEINFO *G_DeviceInfo;
extern DXCONFIG *G_DXConfig;

BOOL CALLBACK DXSetupDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

bool DXUserDialog(DEVICEINFO *DeviceInfo, DXCONFIG *DXConfig, HINSTANCE hInstance)
{
    int ret;

    G_DeviceInfo = DeviceInfo;
    G_DXConfig = DXConfig;
    ShowCursor(1);
    ret = DialogBox(hInstance, MAKEINTRESOURCE(101), 0, DXSetupDlgProc);
    ShowCursor(0);
    if (ret == -1) {
        MessageBox(0, "Unable To Initialise Dialog", "", MB_OK);
        return false;
    }
    return ret ? true : false;
}

}
