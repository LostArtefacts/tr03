#include "standard.h"
#include "specific.h"
#include "winmain.h"

extern "C" {

extern DEVICEINFO *G_DeviceInfo;
extern DXCONFIG *G_DXConfig;

extern bool bSoftwareDefault;

void DXCheckMMXTechnology(HWND hwnd);
void DXInitD3DDrivers(HWND hwnd, int nDrivers);
void DXInitVideoModes(HWND hwnd, int nDD, int nD3D);
void DXInitTextures(HWND hwnd, int nDD, int nD3D);
void DXInitDSAdapters(HWND hwnd);
void DXInitJoystickAdapter(HWND hwnd);
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

void DXInitDialogBox(HWND hwnd)
{
    HWND gfx;
    int nDD;
    int i;
    char abt[80];

    gfx = GetDlgItem(hwnd, 1000);
    for (i = 0; i < G_DeviceInfo->nDDInfo; i++) {
        sprintf(abt, "%s (%s)", G_DeviceInfo->DDInfo[i].About, G_DeviceInfo->DDInfo[i].Name);
        SendMessage(gfx, CB_ADDSTRING, 0, (LPARAM)abt);
    }
    nDD = G_DeviceInfo->nDDInfo - 1;
    if (bSoftwareDefault)
        nDD = 0;
    SendMessage(gfx, CB_SETCURSEL, nDD, 0);
    DXCheckMMXTechnology(hwnd);
    DXInitD3DDrivers(hwnd, nDD);
    DXInitVideoModes(hwnd, nDD, SendMessage(GetDlgItem(hwnd, 1001), CB_GETCURSEL, 0, 0));
    DXInitTextures(hwnd, nDD, SendMessage(GetDlgItem(hwnd, 1001), CB_GETCURSEL, 0, 0));
    DXInitDSAdapters(hwnd);
    DXInitJoystickAdapter(hwnd);
}

void DXInitDSAdapters(HWND hwnd)
{
    HWND list;
    HWND disable;
    int i;

    list = GetDlgItem(hwnd, 1010);
    disable = GetDlgItem(hwnd, 1015);
    if (!G_DeviceInfo->DSInfo) {
        EnableWindow(disable, 0);
        SendMessage(disable, BM_SETCHECK, 1, 0);
        SendMessage(list, CB_ADDSTRING, 0, (LPARAM)"No Sound Adapter Available");
        SendMessage(list, CB_SETCURSEL, 0, 0);
        EnableWindow(list, 0);
    } else {
        SendMessage(list, CB_RESETCONTENT, 0, 0);
        for (i = 0; i < G_DeviceInfo->nDSInfo; i++)
            SendMessage(list, CB_ADDSTRING, 0, (LPARAM)G_DeviceInfo->DSInfo[i].About);
        SendMessage(list, CB_SETCURSEL, 0, 0);
    }
}

void DXInitJoystickAdapter(HWND hwnd)
{
    HWND list;
    HWND disable;
    int i;

    list = GetDlgItem(hwnd, 1011);
    disable = GetDlgItem(hwnd, 1014);
    if (!G_DeviceInfo->DIInfo) {
        EnableWindow(disable, 0);
        SendMessage(disable, BM_SETCHECK, 1, 0);
        SendMessage(list, CB_ADDSTRING, 0, (LPARAM)"No Joystick Adapter Available");
        SendMessage(list, CB_SETCURSEL, 0, 0);
        EnableWindow(list, 0);
    } else {
        SendMessage(list, CB_RESETCONTENT, 0, 0);
        for (i = 0; i < G_DeviceInfo->nDIInfo; i++)
            SendMessage(list, CB_ADDSTRING, 0, (LPARAM)G_DeviceInfo->DIInfo[i].About);
        SendMessage(list, CB_SETCURSEL, 0, 0);
    }
}

}
