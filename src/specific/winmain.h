#ifndef WINMAIN_H
#define WINMAIN_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct displaymode {
    long w;
    long h;
    unsigned long bpp;
    bool bPalette;
    unsigned char rbpp;
    unsigned char gbpp;
    unsigned char bbpp;
    unsigned char abpp;
    DDSURFACEDESC ddsd;
    unsigned char rshift;
    unsigned char gshift;
    unsigned char bshift;
    unsigned char ashift;
} DISPLAYMODE;

typedef struct d3dtextureinfo {
    unsigned long bpp;
    bool bPalette;
    bool bAlpha;
    unsigned char rbpp;
    unsigned char gbpp;
    unsigned char bbpp;
    unsigned char abpp;
    DDSURFACEDESC ddsd;
    DDPIXELFORMAT ddpf;
    unsigned char rshift;
    unsigned char gshift;
    unsigned char bshift;
    unsigned char ashift;
} D3DTEXTUREINFO;

typedef struct direct3dinfo {
    char Name[30];
    char About[80];
    LPGUID lpGuid;
    GUID Guid;
    D3DDEVICEDESC DeviceDesc;
    bool bAlpha;
    bool bHardware;
    bool bAGP;
    long nDisplayMode;
    DISPLAYMODE *DisplayMode;
    long nTexture;
    D3DTEXTUREINFO *Texture;
} DIRECT3DINFO;

typedef struct directdrawinfo {
    char Name[30];
    char About[80];
    LPGUID lpGuid;
    GUID Guid;
    DDCAPS DDCaps;
    long nDisplayMode;
    DISPLAYMODE *DisplayMode;
    long nD3DInfo;
    DIRECT3DINFO *D3DInfo;
} DIRECTDRAWINFO;

typedef struct dxdirectsoundinfo {
    char Name[30];
    char About[80];
    LPGUID lpGuid;
    GUID Guid;
} DXDIRECTSOUNDINFO;

typedef struct dxdirectinputinfo {
    char Name[30];
    char About[80];
    LPGUID lpGuid;
    GUID Guid;
} DXDIRECTINPUTINFO;

typedef struct deviceinfo {
    long nDDInfo;
    DIRECTDRAWINFO *DDInfo;
    long nDSInfo;
    DXDIRECTSOUNDINFO *DSInfo;
    long nDIInfo;
    DXDIRECTINPUTINFO *DIInfo;
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
