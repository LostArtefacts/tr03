#ifndef WINMAIN_H
#define WINMAIN_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct deviceinfo {
    long nDDInfo;
    struct directdrawinfo *DDInfo;
    long nDSInfo;
    struct dxdirectsoundinfo *DSInfo;
    long nDIInfo;
    struct dxdirectinputinfo *DIInfo;
} DEVICEINFO;

typedef struct dxconfig {
    long nDD;
    long nD3D;
    long DS;
    long nVMode;
    long DI;
    long D3DTF;
    long bZBuffer;
    long Dither;
    long Filter;
    long AGP;
    long sound;
    long Joystick;
    long MMX;
} DXCONFIG;

typedef struct winapp {
    WNDCLASS WindowClass;
    HWND WindowHandle;
    HINSTANCE hInstance;
    DEVICEINFO DeviceInfo;
    DXCONFIG DXConfig;
    DEVICEINFO *DeviceInfoPtr;
    DXCONFIG *DXConfigPtr;
    LPDIRECTDRAW2 lpDD;
    LPDIRECT3D2 lpD3D;
    LPDIRECT3DDEVICE2 lpD3DDevice;
    LPDIRECTDRAWSURFACE3 lpFrontBuffer;
    LPDIRECTDRAWSURFACE3 lpBackBuffer;
    LPDIRECTDRAWSURFACE3 lpZBuffer;
    LPDIRECTDRAWSURFACE3 lpPictureBuffer;
    unsigned long *unk;
    LPDIRECT3DVIEWPORT2 lpViewPort;
    LPDIRECT3DMATERIAL2 lpViewPortMaterial;
    LPDIRECTDRAWPALETTE Palette;
    PALETTEENTRY PaletteEntries[257];
    char bFocus;
    long nRenderMode;
    long nUVAdd;
    unsigned long nFrames;
    float fps;
} WINAPP;

extern WINAPP App;

#ifdef __cplusplus
}
#endif

#endif
