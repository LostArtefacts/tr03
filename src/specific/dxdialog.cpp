#include "standard.h"
#include "specific.h"
#include "winmain.h"

extern "C" {

extern DEVICEINFO *G_DeviceInfo;
extern DXCONFIG *G_DXConfig;

extern bool bSoftwareDefault;

void DXCheckMMXTechnology(HWND hwnd);
void DXInitD3DDrivers(HWND hwnd, int nDrivers);
void DXInitD3DDrivers(HWND hwnd, int nDD)
{
    HWND hD3D;
    HWND hZBuffer;
    HWND hDither;
    HWND hFilter;
    HWND h8Bit;
    HWND hAGP;
    HWND hHardware;
    HWND hSoftware;
    int n;
    char Buffer[80];
    char Buffer1[80];
    static int CurSel = -1;
    int nHW;

    hD3D = GetDlgItem(hwnd, 1001);
    hZBuffer = GetDlgItem(hwnd, 1004);
    hDither = GetDlgItem(hwnd, 1005);
    hFilter = GetDlgItem(hwnd, 1006);
    h8Bit = GetDlgItem(hwnd, 1009);
    hAGP = GetDlgItem(hwnd, 1012);
    hHardware = GetDlgItem(hwnd, 1007);
    hSoftware = GetDlgItem(hwnd, 1008);
    nHW = -1;
    if (CurSel != -1)
        SendMessage(hD3D, CB_GETLBTEXT, SendMessage(hD3D, CB_GETCURSEL, 0, 0), (LPARAM)Buffer1);
    SendMessage(hD3D, CB_RESETCONTENT, 0, 0);
    for (n = 0; n < G_DeviceInfo->DDInfo[nDD].nD3DInfo; n++) {
        sprintf(Buffer, "%s", G_DeviceInfo->DDInfo[nDD].D3DInfo[n].About);
        SendMessage(hD3D, CB_ADDSTRING, 0, (LPARAM)Buffer);
        if (CurSel == -1 && !bSoftwareDefault)
            if (G_DeviceInfo->DDInfo[nDD].D3DInfo[n].bHardware)
                nHW = n;
        if (!strcmp(Buffer1, Buffer))
            nHW = n;
    }
    if (nHW == -1)
        nHW = 0;
    EnableWindow(hAGP, 0);
    if (CurSel == -1) {
        SendMessage(hZBuffer, BM_SETCHECK, 1, 0);
        SendMessage(hFilter, BM_SETCHECK, 1, 0);
        SendMessage(hDither, BM_SETCHECK, 1, 0);
        if (G_DeviceInfo->DDInfo[nDD].D3DInfo[nHW].bAGP)
            SendMessage(hAGP, BM_SETCHECK, 1, 0);
    }
    SendMessage(hD3D, CB_SETCURSEL, nHW, 0);
    CurSel = nHW;
    if (G_DeviceInfo->DDInfo[nDD].D3DInfo[nHW].bHardware) {
        EnableWindow(hZBuffer, 1);
        EnableWindow(hDither, 1);
        EnableWindow(hFilter, 1);
        EnableWindow(h8Bit, 1);
        EnableWindow(hAGP, 0);
        SendMessage(hAGP, BM_SETCHECK, 0, 0);
        if (G_DeviceInfo->DDInfo[nDD].D3DInfo[nHW].bAGP) {
            EnableWindow(hAGP, 1);
            SendMessage(hAGP, BM_SETCHECK, 1, 0);
        }
        SendMessage(hHardware, BM_SETCHECK, 1, 0);
        SendMessage(hSoftware, BM_SETCHECK, 0, 0);
        SendMessage(hZBuffer, BM_SETCHECK, 1, 0);
        SendMessage(hFilter, BM_SETCHECK, 1, 0);
        SendMessage(hDither, BM_SETCHECK, 1, 0);
        SendMessage(h8Bit, BM_SETCHECK, 0, 0);
    } else {
        SendMessage(hZBuffer, BM_SETCHECK, 0, 0);
        SendMessage(hFilter, BM_SETCHECK, 0, 0);
        SendMessage(hDither, BM_SETCHECK, 0, 0);
        SendMessage(hSoftware, BM_SETCHECK, 1, 0);
        SendMessage(hHardware, BM_SETCHECK, 0, 0);
        EnableWindow(hZBuffer, 0);
        EnableWindow(hDither, 0);
        if (G_DXConfig->MMX) {
            EnableWindow(hFilter, 1);
            SendMessage(hFilter, BM_SETCHECK, 1, 0);
        } else {
            EnableWindow(hFilter, 0);
        }
        if (!G_DXConfig->MMX)
            EnableWindow(h8Bit, 0);
        EnableWindow(hAGP, 0);
        if (!G_DXConfig->MMX)
            SendMessage(h8Bit, BM_SETCHECK, 0, 0);
        SendMessage(hAGP, BM_SETCHECK, 0, 0);
    }
}

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

void DXInitVideoModes(HWND hwnd, int nDD, int nD3D)
{
    HWND res;
    int n;
    char Buffer[40];
    char Default[] = "640x480 High Colour (16 Bit)";
    static int CurSel = -1;
    int nDef;
    int nListed;

    if (bSoftwareDefault)
        strcpy(Default, "320x200 High Colour (16 Bit)");
    nDef = -1;
    res = GetDlgItem(hwnd, 1002);
    if (CurSel != -1)
        SendMessage(res, CB_GETLBTEXT, SendMessage(res, CB_GETCURSEL, 0, 0), (LPARAM)Default);
    SendMessage(res, CB_RESETCONTENT, 0, 0);
    nListed = 0;
    for (n = 0; n < G_DeviceInfo->DDInfo[nDD].D3DInfo[nD3D].nDisplayMode; n++) {
        if (G_DeviceInfo->DDInfo[nDD].D3DInfo[nD3D].DisplayMode[n].bpp == 16) {
            sprintf(Buffer, "%dx%d High Colour (%d Bit)", G_DeviceInfo->DDInfo[nDD].D3DInfo[nD3D].DisplayMode[n].w,
                G_DeviceInfo->DDInfo[nDD].D3DInfo[nD3D].DisplayMode[n].h, 16);
            SendMessage(res, CB_ADDSTRING, 0, (LPARAM)Buffer);
            SendMessage(res, CB_SETITEMDATA, nListed, n);
            if (!strcmp(Default, Buffer))
                nDef = nListed;
            nListed++;
        }
    }
    for (n = 0; n < G_DeviceInfo->DDInfo[nDD].D3DInfo[nD3D].nDisplayMode; n++) {
        if (G_DeviceInfo->DDInfo[nDD].D3DInfo[nD3D].DisplayMode[n].bpp == 24) {
            sprintf(Buffer, "%dx%d True Colour (%d Bit)", G_DeviceInfo->DDInfo[nDD].D3DInfo[nD3D].DisplayMode[n].w,
                G_DeviceInfo->DDInfo[nDD].D3DInfo[nD3D].DisplayMode[n].h, 24);
            SendMessage(res, CB_ADDSTRING, 0, (LPARAM)Buffer);
            SendMessage(res, CB_SETITEMDATA, nListed, n);
            if (!strcmp(Default, Buffer))
                nDef = nListed;
            nListed++;
        }
    }
    for (n = 0; n < G_DeviceInfo->DDInfo[nDD].D3DInfo[nD3D].nDisplayMode; n++) {
        if (G_DeviceInfo->DDInfo[nDD].D3DInfo[nD3D].DisplayMode[n].bpp == 32) {
            sprintf(Buffer, "%dx%d True Colour (%d Bit)", G_DeviceInfo->DDInfo[nDD].D3DInfo[nD3D].DisplayMode[n].w,
                G_DeviceInfo->DDInfo[nDD].D3DInfo[nD3D].DisplayMode[n].h, 32);
            SendMessage(res, CB_ADDSTRING, 0, (LPARAM)Buffer);
            SendMessage(res, CB_SETITEMDATA, nListed, n);
            if (!strcmp(Default, Buffer))
                nDef = nListed;
            nListed++;
        }
    }
    if (nDef != -1) {
        CurSel = nDef;
        SendMessage(res, CB_SETCURSEL, nDef, 0);
    } else {
        CurSel = 0;
        SendMessage(res, CB_SETCURSEL, 0, 0);
    }
}

void DXInitTextures(HWND hwnd, int nDD, int nD3D)
{
    HWND hTMode;
    int n;
    char Buffer[20];
    char Buffer1[20];
    char Default[12];
    static int CurSel = -1;
    int nDef;
    bool palette;
    bool found;
    HWND h;

    nDef = -1;
    hTMode = GetDlgItem(hwnd, 1003);
    if (CurSel != -1)
        SendMessage(hTMode, CB_GETLBTEXT, SendMessage(hTMode, CB_GETCURSEL, 0, 0), (LPARAM)Buffer1);
    SendMessage(hTMode, CB_RESETCONTENT, 0, 0);
    palette = 0;
    for (n = 0; n < G_DeviceInfo->DDInfo[nDD].D3DInfo[nD3D].nTexture; n++) {
        if (G_DeviceInfo->DDInfo[nDD].D3DInfo[nD3D].Texture[n].bPalette == false) {
            if (G_DeviceInfo->DDInfo[nDD].D3DInfo[nD3D].Texture[n].abpp != 0) {
                sprintf(Buffer, "%d%d%d%d", G_DeviceInfo->DDInfo[nDD].D3DInfo[nD3D].Texture[n].rbpp,
                    G_DeviceInfo->DDInfo[nDD].D3DInfo[nD3D].Texture[n].gbpp,
                    G_DeviceInfo->DDInfo[nDD].D3DInfo[nD3D].Texture[n].bbpp,
                    G_DeviceInfo->DDInfo[nDD].D3DInfo[nD3D].Texture[n].abpp);
            } else {
                sprintf(Buffer, "%d%d%d", G_DeviceInfo->DDInfo[nDD].D3DInfo[nD3D].Texture[n].rbpp,
                    G_DeviceInfo->DDInfo[nDD].D3DInfo[nD3D].Texture[n].gbpp,
                    G_DeviceInfo->DDInfo[nDD].D3DInfo[nD3D].Texture[n].bbpp);
            }
        } else {
            palette = 1;
            sprintf(Buffer, "%d Bit", G_DeviceInfo->DDInfo[nDD].D3DInfo[nD3D].Texture[n].bpp);
        }
        SendMessage(hTMode, CB_ADDSTRING, 0, (LPARAM)Buffer);
        if (CurSel == -1) {
            if (!strcmp(Default, Buffer))
                nDef = n;
        } else {
            if (!strcmp(Buffer1, Buffer))
                nDef = n;
        }
    }
    found = 0;
    if (palette) {
        h = GetDlgItem(hwnd, 1009);
        if (SendMessage(h, BM_GETCHECK, 0, 0)) {
            strcpy(Default, "8 Bit");
            for (n = 0; n < G_DeviceInfo->DDInfo[nDD].D3DInfo[nD3D].nTexture; n++) {
                SendMessage(hTMode, CB_GETLBTEXT, n, (LPARAM)Buffer);
                if (!strcmp(Default, Buffer)) {
                    nDef = n;
                    found = 1;
                    break;
                }
            }
        }
    } else {
        EnableWindow(GetDlgItem(hwnd, 1009), 0);
    }
    if (G_DXConfig->MMX && SendMessage(GetDlgItem(hwnd, 1008), BM_GETCHECK, 0, 0)) {
        if (!found) {
            strcpy(Default, "8888");
            for (n = 0; n < G_DeviceInfo->DDInfo[nDD].D3DInfo[nD3D].nTexture; n++) {
                SendMessage(hTMode, CB_GETLBTEXT, n, (LPARAM)Buffer);
                if (!strcmp(Default, Buffer)) {
                    nDef = n;
                    found = 1;
                    break;
                }
            }
        }
    }
    if (!found) {
        strcpy(Default, "5551");
        for (n = 0; n < G_DeviceInfo->DDInfo[nDD].D3DInfo[nD3D].nTexture; n++) {
            SendMessage(hTMode, CB_GETLBTEXT, n, (LPARAM)Buffer);
            if (!strcmp(Default, Buffer)) {
                nDef = n;
                found = 1;
                break;
            }
        }
    }
    if (!found) {
        strcpy(Default, "4444");
        for (n = 0; n < G_DeviceInfo->DDInfo[nDD].D3DInfo[nD3D].nTexture; n++) {
            SendMessage(hTMode, CB_GETLBTEXT, n, (LPARAM)Buffer);
            if (!strcmp(Default, Buffer)) {
                nDef = n;
                found = 1;
                break;
            }
        }
    }
    if (!found) {
        strcpy(Default, "555");
        for (n = 0; n < G_DeviceInfo->DDInfo[nDD].D3DInfo[nD3D].nTexture; n++) {
            SendMessage(hTMode, CB_GETLBTEXT, n, (LPARAM)Buffer);
            if (!strcmp(Default, Buffer)) {
                nDef = n;
                found = 1;
                break;
            }
        }
    }
    if (!found) {
        strcpy(Default, "565");
        for (n = 0; n < G_DeviceInfo->DDInfo[nDD].D3DInfo[nD3D].nTexture; n++) {
            SendMessage(hTMode, CB_GETLBTEXT, n, (LPARAM)Buffer);
            if (!strcmp(Default, Buffer)) {
                nDef = n;
                found = 1;
                break;
            }
        }
    }
    if (nDef != -1) {
        CurSel = nDef;
        SendMessage(hTMode, CB_SETCURSEL, nDef, 0);
    } else {
        CurSel = 0;
        SendMessage(hTMode, CB_SETCURSEL, 0, 0);
    }
}

}
