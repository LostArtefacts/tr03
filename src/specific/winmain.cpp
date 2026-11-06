#include "standard.h"
#include "specific.h"
#include "winmain.h"

extern "C" {

void ShutdownGame(void);
void DXFreeTPages(void);
bool DXCreateSurface(LPDIRECTDRAW2 lpDD2, DDSURFACEDESC *ddsd, LPDIRECTDRAWSURFACE3 *lpSurface);
bool DXAddAttachedSurface(LPDIRECTDRAWSURFACE3 lpSurface, LPDIRECTDRAWSURFACE3 lpAddSurface);
bool DXSetCooperativeLevel(LPDIRECTDRAW2 lpDD2, HWND WindowHandle, int Flags);
extern char exit_message[128];
bool DXCreateDirectDraw(DEVICEINFO *dd, DXCONFIG *DXConfig, LPDIRECTDRAW2 *lpDD2);
bool DXCreateDirect3D(LPDIRECTDRAW2 lpDD2, LPDIRECT3D2 *lpD3D2);
bool DXSetVideoMode(LPDIRECTDRAW2 lpDD2, int w, int h, int bpp);
void DXBitMask2ShiftCnt(unsigned long mask, unsigned char *shift, unsigned char *cnt);
bool DXGetAttachedSurface(LPDIRECTDRAWSURFACE3 lpPrimary, DDSCAPS *ddscaps, LPDIRECTDRAWSURFACE3 *lpAttached);
bool DXCreateDirect3DDevice(LPDIRECT3D2 lpD3D2, GUID Guid, LPDIRECTDRAWSURFACE3 lpSurface, LPDIRECT3DDEVICE2 *lpD3DDevice2);
bool DXCreateViewPort(LPDIRECT3D2 lpD3D, LPDIRECT3DDEVICE2 lpD3DDevice, long w, long h, LPDIRECT3DVIEWPORT2 *lpViewport);
void DXClearBuffers(DWORD dwClrFlags, DWORD dwColour);
void InitDrawPrimitive(LPDIRECT3DDEVICE2 lpD3DDevice, LPDIRECTDRAWSURFACE3 lpSurface, bool hw);
void HWR_InitState(void);
void DXCreateMaxTPages(int create);
extern int nTPages;
LRESULT CALLBACK WinAppProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void WinAppExit(void)
{
    ShutdownGame();
    exit(0);
}

void WinFreeDX(bool free_dd)
{
    DXFreeTPages();
    if (App.Palette) {
        App.Palette->Release();
        App.Palette = NULL;
    }
    if (App.lpViewPort) {
        App.lpViewPort->Release();
        App.lpViewPort = NULL;
    }
    if (App.lpD3DDevice) {
        App.lpD3DDevice->Release();
        App.lpD3DDevice = NULL;
    }
    if (App.lpZBuffer) {
        App.lpZBuffer->Release();
        App.lpZBuffer = NULL;
    }
    if (App.lpBackBuffer) {
        App.lpBackBuffer->Release();
        App.lpBackBuffer = NULL;
    }
    if (App.lpFrontBuffer) {
        App.lpFrontBuffer->Release();
        App.lpFrontBuffer = NULL;
    }
    if (App.lpPictureBuffer) {
        App.lpPictureBuffer->Release();
        App.lpPictureBuffer = NULL;
    }
    if (App.unk)
        free(App.unk);
    if (free_dd) {
        if (App.lpD3D) {
            App.lpD3D->Release();
            App.lpD3D = NULL;
        }
        if (App.lpDD) {
            App.lpDD->Release();
            App.lpDD = NULL;
        }
    }
}

int WinRegisterWindow(HINSTANCE hInstance)
{
    App.WindowClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(115));
    App.WindowClass.lpszMenuName = 0;
    App.WindowClass.lpszClassName = "Window Class";
    App.WindowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    App.WindowClass.hInstance = hInstance;
    App.WindowClass.style = CS_VREDRAW | CS_HREDRAW;
    App.WindowClass.lpfnWndProc = WinAppProc;
    App.WindowClass.cbClsExtra = 0;
    App.WindowClass.cbWndExtra = 0;
    return RegisterClass(&App.WindowClass);
}

HWND WinCreateWindow(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd;

    hWnd = CreateWindowEx(WS_EX_APPWINDOW, "Window Class", "Tomb Raider III", WS_POPUP, 0, 0, 0, 0, 0, 0, hInstance, 0);
    if (hWnd) {
        ShowWindow(hWnd, nCmdShow);
        UpdateWindow(hWnd);
    }
    return hWnd;
}

bool DXCreateZBuffer(DEVICEINFO *device, DXCONFIG *config)
{
    DIRECT3DINFO **d3d;
    DDSURFACEDESC desc;

    if (config->bZBuffer) {
        memset(&desc, 0, sizeof(desc));
        desc.dwSize = sizeof(desc);
        desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_ZBUFFERBITDEPTH;
        d3d = &device->DDInfo[config->nDD].D3DInfo;
        desc.ddsCaps.dwCaps = (*d3d)[config->nD3D].bHardware ? DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY : DDSCAPS_ZBUFFER | DDSCAPS_SYSTEMMEMORY;
        desc.dwWidth = (*d3d)[config->nD3D].DisplayMode[config->nVMode].w;
        desc.dwHeight = (*d3d)[config->nD3D].DisplayMode[config->nVMode].h;
        desc.dwZBufferBitDepth = 16;
        if (!DXCreateSurface(App.lpDD, &desc, &App.lpZBuffer))
            return false;
        if (!DXAddAttachedSurface(App.lpBackBuffer, App.lpZBuffer))
            return false;
    } else {
        App.lpZBuffer = NULL;
    }
    return true;
}

float WinFrameRate(void)
{
    static int time = clock();
    static int counter;
    static float fps;
    double t;
    int now;

    counter++;
    if (counter == 10) {
        now = clock();
        t = (double)(now - time) * 0.001;
        time = now;
        fps = (float)(counter / t);
        counter = 0;
    }
    App.fps = fps;
    return fps;
}

void S_ExitSystem(char *message)
{
    DXSetCooperativeLevel(App.lpDD, App.WindowHandle, DDSCL_NORMAL);
    MessageBox(App.WindowHandle, message, 0, MB_OK);
    ShutdownGame();
    strcpy(exit_message, message);
    exit(1);
}

}
