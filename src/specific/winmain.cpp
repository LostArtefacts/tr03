#include "standard.h"
#include "specific.h"
#include "winmain.h"
#include "texture.h"

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
int WinRegisterWindow(HINSTANCE hInstance);
HWND WinCreateWindow(HINSTANCE hInstance, int nCmdShow);
void DXGetDeviceInfo(DEVICEINFO *device, HWND hWnd, HINSTANCE hInstance);
bool DXUserDialog(DEVICEINFO *DeviceInfo, DXCONFIG *DXConfig, HINSTANCE hInstance);
void DXFreeDeviceInfo(DEVICEINFO *dd);
bool S_LoadSettings(void);
bool CD_Init(void);
char *UT_FindArg(char *pArg);
bool WinDXInit(DEVICEINFO *device, DXCONFIG *config, bool createNew);
void WinFreeDX(bool free_dd);
void UT_InitAccurateTimer(void);
bool DXResetPalette(DXTEXTURE *tex);
void DS_Init(void);
bool TIME_Init(void);
bool HWR_Init(void);
void DS_Start(HWND hwnd);
void DI_Start(void);
bool ACMInit(void);
void setup_screen_size(void);
sint32 GameMain(void);
extern char *G_lpCmdLine;
extern sint32 framedump;
extern int farz;
extern int distanceFogValue;
extern int game_closedown;
extern bool GtWindowClosed;
extern bool GtFullScreenClearNeeded;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    bool hw;

    G_lpCmdLine = lpCmdLine;
    memset(&App, 0, sizeof(WINAPP));
    App.hInstance = hInstance;
    if (!hPrevInstance && !WinRegisterWindow(hInstance)) {
        MessageBox(0, "Unable To Register Window Class", "", MB_OK);
        return 0;
    }
    App.WindowHandle = WinCreateWindow(hInstance, nShowCmd);
    if (!App.WindowHandle) {
        MessageBox(0, "Unable To Create Window", "", MB_OK);
        return 0;
    }
    if (!CD_Init())
        return 0;
    DXGetDeviceInfo(&App.DeviceInfo, App.WindowHandle, App.hInstance);
    App.DXConfigPtr = &App.DXConfig;
    App.DeviceInfoPtr = &App.DeviceInfo;
    if ((!S_LoadSettings() || UT_FindArg("setup")) && !DXUserDialog(&App.DeviceInfo, &App.DXConfig, App.hInstance)) {
        DXFreeDeviceInfo(&App.DeviceInfo);
        return 0;
    }
    if (!WinDXInit(&App.DeviceInfo, &App.DXConfig, 1)) {
        WinFreeDX(1);
        DXFreeDeviceInfo(&App.DeviceInfo);
        ShowWindow(App.WindowHandle, 0);
        MessageBox(App.WindowHandle, "Tomb Raider 3 Failed To Initialise, Please Run Setup", "Error", MB_ICONEXCLAMATION);
        return 0;
    }
    if (!(App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DeviceDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_ALPHA))
        App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].Texture[App.DXConfigPtr->D3DTF].bAlpha = 0;
    HWConfig.Perspective = 1;
    HWConfig.Dither = App.DXConfig.Dither;
    HWConfig.nFilter = D3DFILTER_NEAREST + (App.DXConfig.Filter != 0);
    HWConfig.nShadeMode = D3DSHADE_GOURAUD;
    HWConfig.nFillMode = D3DFILL_SOLID;
    HWConfig.TrueAlpha = 0;
    hw = App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].bHardware;
    framedump = 0;
    App.nUVAdd = hw ? 256 : 128;
    UT_InitAccurateTimer();
    DXResetPalette(PictureTextures);
    InitDrawPrimitive(App.lpD3DDevice, App.lpBackBuffer, App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].bHardware);
    farz = 0x5000;
    distanceFogValue = 0x3000;
    DS_Init();
    TIME_Init();
    HWR_Init();
    DS_Start(0);
    DI_Start();
    ACMInit();
    setup_screen_size();
    game_closedown = 0;
    GtWindowClosed = 0;
    GtFullScreenClearNeeded = 0;
    GameMain();
    return 0;
}

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

bool WinDXInit(DEVICEINFO *device, DXCONFIG *config, bool createNew)
{
    DDSURFACEDESC desc;
    D3DMATERIAL m;
    DDSCAPS caps;
    D3DMATERIALHANDLE handle;

    App.nRenderMode = 1;
    if (createNew) {
        if (!DXCreateDirectDraw(device, config, &App.lpDD))
            return false;
        if (!DXCreateDirect3D(App.lpDD, &App.lpD3D))
            return false;
    }
    if (!DXSetCooperativeLevel(App.lpDD, App.WindowHandle, DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE))
        return false;
    if (!DXSetVideoMode(App.lpDD, device->DDInfo[config->nDD].D3DInfo[config->nD3D].DisplayMode[config->nVMode].w,
            device->DDInfo[config->nDD].D3DInfo[config->nD3D].DisplayMode[config->nVMode].h,
            device->DDInfo[config->nDD].D3DInfo[config->nD3D].DisplayMode[config->nVMode].bpp))
        return false;
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    if (device->DDInfo[config->nDD].D3DInfo[config->nD3D].bHardware) {
        desc.dwBackBufferCount = 1;
        desc.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
        desc.ddsCaps.dwCaps = DDSCAPS_COMPLEX | DDSCAPS_FLIP | DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE;
    } else {
        desc.dwBackBufferCount = 1;
        desc.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
        desc.ddsCaps.dwCaps = DDSCAPS_COMPLEX | DDSCAPS_FLIP | DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE;
    }
    if (!DXCreateSurface(App.lpDD, &desc, &App.lpFrontBuffer))
        return false;
    App.lpFrontBuffer->GetSurfaceDesc(&desc);
    DXBitMask2ShiftCnt(desc.ddpfPixelFormat.dwRBitMask,
        &device->DDInfo[config->nDD].D3DInfo[config->nD3D].DisplayMode[config->nVMode].rshift,
        &device->DDInfo[config->nDD].D3DInfo[config->nD3D].DisplayMode[config->nVMode].rbpp);
    DXBitMask2ShiftCnt(desc.ddpfPixelFormat.dwGBitMask,
        &device->DDInfo[config->nDD].D3DInfo[config->nD3D].DisplayMode[config->nVMode].gshift,
        &device->DDInfo[config->nDD].D3DInfo[config->nD3D].DisplayMode[config->nVMode].gbpp);
    DXBitMask2ShiftCnt(desc.ddpfPixelFormat.dwBBitMask,
        &device->DDInfo[config->nDD].D3DInfo[config->nD3D].DisplayMode[config->nVMode].bshift,
        &device->DDInfo[config->nDD].D3DInfo[config->nD3D].DisplayMode[config->nVMode].bbpp);
    if (device->DDInfo[config->nDD].D3DInfo[config->nD3D].bHardware) {
        caps.dwCaps = DDSCAPS_BACKBUFFER;
        if (!DXGetAttachedSurface(App.lpFrontBuffer, &caps, &App.lpBackBuffer))
            return false;
    } else {
        App.unk = (unsigned long *)malloc(4 * device->DDInfo[config->nDD].D3DInfo[config->nD3D].DisplayMode[config->nVMode].w
            * device->DDInfo[config->nDD].D3DInfo[config->nD3D].DisplayMode[config->nVMode].h);
        caps.dwCaps = DDSCAPS_BACKBUFFER;
        DXGetAttachedSurface(App.lpFrontBuffer, &caps, &App.lpBackBuffer);
    }
    if (!DXCreateZBuffer(device, config))
        return false;
    if (!DXCreateDirect3DDevice(App.lpD3D, device->DDInfo[config->nDD].D3DInfo[config->nD3D].Guid, App.lpBackBuffer, &App.lpD3DDevice))
        return false;
    if (!DXCreateViewPort(App.lpD3D, App.lpD3DDevice, device->DDInfo[config->nDD].D3DInfo[config->nD3D].DisplayMode[config->nVMode].w,
            device->DDInfo[config->nDD].D3DInfo[config->nD3D].DisplayMode[config->nVMode].h, &App.lpViewPort))
        return false;
    memset(&m, 0, sizeof(m));
    m.dwSize = sizeof(m);
    App.lpD3D->CreateMaterial(&App.lpViewPortMaterial, NULL);
    App.lpViewPortMaterial->SetMaterial(&m);
    App.lpViewPortMaterial->GetHandle(App.lpD3DDevice, &handle);
    App.lpViewPort->SetBackground(handle);
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwWidth = 640;
    desc.dwHeight = 480;
    desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    DXCreateSurface(App.lpDD, &desc, &App.lpPictureBuffer);
    DXClearBuffers(11, 0);
    InitDrawPrimitive(App.lpD3DDevice, App.lpBackBuffer, App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].bHardware);
    HWR_InitState();
    if (device->DDInfo[config->nDD].D3DInfo[config->nD3D].bHardware) {
        DXCreateMaxTPages(1);
        if (!nTPages)
            return false;
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

bool DXSurfBlt(LPDIRECTDRAWSURFACE3 lpSurface, LPRECT lpRect, int FillColor);
void CloseDrawPrimitive(void);
void HWR_GetAllTextureHandles(void);
void HWR_SetCurrentTexture(DWORD handle);
void setup_screen_size(void);
extern DWORD TPages[];

bool DXSwitchVideoMode(int needed, int current, bool disableZBuffer)
{
    unsigned long bpp;
    bool change;
    int nModes;
    int i;

    change = true;
    bpp = App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].bpp;
    App.DXConfig.nVMode = needed;
    nModes = App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].nDisplayMode;
    if (needed != current && App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].bpp != bpp) {
        if (needed > current) {
            while (1) {
                if (++needed > nModes)
                    break;
                App.DXConfig.nVMode = needed;
                if (App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].bpp == bpp)
                    goto ok;
            }
            App.DXConfig.nVMode = current;
            return false;
        } else {
            needed--;
            while (1) {
                if (needed < 0)
                    break;
                App.DXConfig.nVMode = needed;
                if (App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].bpp == bpp)
                    goto ok;
                needed--;
            }
            App.DXConfig.nVMode = current;
            return false;
        }
    }
ok:
    DXSurfBlt(App.lpFrontBuffer, NULL, 0);
    for (i = 0; i < 32; i++) {
        if (PictureTextures[i].tex)
            PictureTextures[i].tex = NULL;
    }
    if (!App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].bHardware)
        CloseDrawPrimitive();
    WinFreeDX(false);
    if (!WinDXInit(App.DeviceInfoPtr, App.DXConfigPtr, false)) {
        WinFreeDX(false);
        App.DXConfig.nVMode = current;
        if (disableZBuffer)
            App.DXConfigPtr->bZBuffer = 0;
        WinDXInit(App.DeviceInfoPtr, App.DXConfigPtr, false);
        change = false;
    }
    HWR_GetAllTextureHandles();
    for (i = 0; i < nTPages; i++)
        HWR_SetCurrentTexture(TPages[i]);
    HWR_InitState();
    setup_screen_size();
    return change;
}

void S_ExitSystem(char *message)
{
    DXSetCooperativeLevel(App.lpDD, App.WindowHandle, DDSCL_NORMAL);
    MessageBox(App.WindowHandle, message, 0, MB_OK);
    ShutdownGame();
    strcpy(exit_message, message);
    exit(1);
}


LRESULT CALLBACK WinAppProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_CREATE:
        ShowCursor(FALSE);
        break;
    case WM_CLOSE:
        WinAppExit();
        PostQuitMessage(0);
        break;
    case WM_CHAR:
        break;
    case WM_KEYDOWN:
        switch (wParam) {
        case VK_F1:
            break;
        }
        break;
    case WM_KEYUP:
        break;
    case WM_ACTIVATE:
        switch (LOWORD(wParam)) {
        case WA_ACTIVE:
        case WA_CLICKACTIVE:
            App.bFocus = TRUE;
            break;
        case WA_INACTIVE:
            App.bFocus = FALSE;
            break;
        }
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

}
