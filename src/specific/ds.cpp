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

bool DS_IsChannelPlaying(int nChannel)
{
    LPDIRECTSOUNDBUFFER pBuffer;
    DWORD dwStatus;

    pBuffer = DS_Samples[nChannel];
    if (!pBuffer)
        return false;
    if (DX_TRY(pBuffer->GetStatus(&dwStatus)))
        return false;
    if (dwStatus & DSBSTATUS_PLAYING)
        return true;
    pBuffer->Release();
    DS_Samples[nChannel] = 0;
    return false;
}

int DS_GetFreeChannel(void)
{
    int nChannel;

    for (nChannel = 0; nChannel < ArraySize(DS_Samples); nChannel++) {
        if (DS_Samples[nChannel] == 0)
            return nChannel;
    }
    for (nChannel = 0; nChannel < ArraySize(DS_Samples); nChannel++) {
        if (!DS_IsChannelPlaying(nChannel))
            return nChannel;
    }
    return -1;
}

void DS_AdjustVolumeAndPan(int nHandle, int nVolume, int nPan)
{
    LPDIRECTSOUNDBUFFER pBuffer;

    if (nHandle < 0)
        return;
    pBuffer = DS_Samples[nHandle];
    if (pBuffer) {
        DX_TRY(pBuffer->SetVolume(nVolume));
        DX_TRY(pBuffer->SetPan(nPan));
    }
}

void DS_AdjustPitch(int nHandle, int nPitch)
{
    LPDIRECTSOUNDBUFFER pBuffer;
    int nFrequency;

    if (nHandle < 0)
        return;
    pBuffer = DS_Samples[nHandle];
    if (pBuffer) {
        nFrequency = (nPitch * DS_SampleFrequencies[DS_SamplesPlaying[nHandle]]) >> 16;
        DX_TRY(pBuffer->SetFrequency(nFrequency));
    }
}

void DS_StopSample(int nHandle)
{
    LPDIRECTSOUNDBUFFER pBuffer;

    if (nHandle < 0)
        return;
    pBuffer = DS_Samples[nHandle];
    if (pBuffer) {
        DX_TRY(pBuffer->Stop());
        pBuffer->Release();
        DS_Samples[nHandle] = 0;
    }
}

void DS_StopAllSamples(void)
{
    int nChannel;
    LPDIRECTSOUNDBUFFER pBuffer;

    for (nChannel = 0; nChannel < ArraySize(DS_Samples); nChannel++) {
        pBuffer = DS_Samples[nChannel];
        if (pBuffer) {
            DX_TRY(pBuffer->Stop());
            pBuffer->Release();
            DS_Samples[nChannel] = 0;
        }
    }
}

bool DS_MakeAdapterList(void);

void DS_Init(void)
{
    DS_MakeAdapterList();
}

BOOL CALLBACK DS_EnumCallback(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext);

bool DS_EnumerateDevices(LPVOID lpContext)
{
    if (DX_TRY(DirectSoundEnumerate(DS_EnumCallback, lpContext)))
        return false;
    return true;
}

bool DS_Create(LPGUID lpGuid)
{
    if (DX_TRY(DirectSoundCreate(lpGuid, &lpDirectSound, 0)))
        return false;
    return true;
}

bool DS_SetOutputFormat(void)
{
    DSBUFFERDESC dsbd;
    LPDIRECTSOUNDBUFFER pPrimary;
    WAVEFORMATEX wf;

    memset(&dsbd, 0, sizeof(dsbd));
    dsbd.dwSize = sizeof(dsbd);
    dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER;
    if (DX_TRY(lpDirectSound->CreateSoundBuffer(&dsbd, &pPrimary, 0)))
        return false;
    memset(&wf, 0, sizeof(wf));
    wf.wFormatTag = WAVE_FORMAT_PCM;
    wf.nChannels = 2;
    wf.nSamplesPerSec = 22050;
    wf.nAvgBytesPerSec = 22050 * 2 * 2;
    wf.nBlockAlign = 4;
    wf.wBitsPerSample = 16;
    if (DX_TRY(pPrimary->SetFormat(&wf))) {
        if (pPrimary) {
            pPrimary->Release();
            pPrimary = 0;
        }
        return false;
    }
    if (pPrimary) {
        pPrimary->Release();
        pPrimary = 0;
    }
    return true;
}

void DS_Release(void)
{
    if (lpDirectSound) {
        lpDirectSound->Release();
        lpDirectSound = 0;
    }
}

void DS_Finish(void)
{
    DS_FreeAllSamples();
    DS_Release();
}

bool DS_IsSoundEnabled(void)
{
    return App.DXConfig.sound;
}

}
