#include "standard.h"
#include "specific.h"

extern "C" {

extern HRESULT (*EndScene)(void);
extern void *VertexBuffer;

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

void HWR_EndScene(void)
{
    EndScene();
}

}
