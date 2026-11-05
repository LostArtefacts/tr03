#include "standard.h"
#include "specific.h"
#include "winmain.h"

extern "C" {

extern LPDIRECTSOUND lpDirectSound;
extern LPDIRECTSOUNDBUFFER DS_Buffers[256];
extern int DS_SampleFrequencies[256];
extern int DS_SamplesPlaying[32];
extern LPDIRECTSOUNDBUFFER DS_Samples[32];

#define ArraySize(x) (sizeof(x) / sizeof((x)[0]))

inline bool DX_TRY(HRESULT hr)
{
    if (SUCCEEDED(hr))
        return false;
    return true;
}

void DS_FreeAllSamples(void)
{
    int i;

    if (!App.DXConfig.sound)
        return;
    for (i = 0; i < ArraySize(DS_Buffers); i++) {
        if (DS_Buffers[i]) {
            DS_Buffers[i]->Release();
            DS_Buffers[i] = 0;
        }
    }
}

bool DS_MakeAdapterList(void);

}
