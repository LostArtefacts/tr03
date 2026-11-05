#include "standard.h"
#include "specific.h"
#include "winmain.h"

extern "C" {

extern HRESULT (*EndScene)(void);
extern void *VertexBuffer;
extern HRESULT (*SetRenderState)(D3DRENDERSTATETYPE dwRenderStateType, DWORD dwRenderState);
extern bool GtZBufferCompareEnabled;
extern bool GtZBufferWriteEnabled;
extern bool GtColorAdditionEnabled;
extern bool GtAlphaBlendEnabled;
extern bool GtColorKeyEnabled;

void HWR_InitState(void);
void HWR_SetCurrentTexture(DWORD handle);
void HWR_EnableAlphaBlend(bool enable);
void HWR_EnableColorKey(bool enable);
void HWR_EnableColorAddition(bool enable);

void HWR_ResetCurrentTexture(void)
{
    HWR_SetCurrentTexture(0);
}

void HWR_ResetColorKey(void)
{
    HWR_EnableAlphaBlend(false);
    HWR_EnableColorKey(false);
    HWR_EnableColorAddition(false);
}

void HWR_ResetZBuffer(void)
{
    GtZBufferWriteEnabled = false;
    GtZBufferCompareEnabled = false;
    if (App.lpZBuffer) {
        SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS);
    } else {
        SetRenderState(D3DRENDERSTATE_ZENABLE, false);
        SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, false);
    }
}

void HWR_EnableColorAddition(bool enable)
{
    if (enable) {
        if (!GtColorAdditionEnabled) {
            GtColorAdditionEnabled = true;
            SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
            SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
        }
    } else if (GtColorAdditionEnabled) {
        GtColorAdditionEnabled = false;
        SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
        SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
    }
}

void HWR_EnableAlphaBlend(bool enable)
{
    if (enable) {
        if (!GtAlphaBlendEnabled) {
            SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, true);
            GtAlphaBlendEnabled = true;
        }
    } else if (GtAlphaBlendEnabled) {
        SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, false);
        GtAlphaBlendEnabled = false;
    }
}

void HWR_EndScene(void)
{
    EndScene();
}

bool HWR_Init(void)
{
    VertexBuffer = 0;
    HWR_InitState();
    return true;
}

}
