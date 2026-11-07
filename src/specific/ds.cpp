#include "standard.h"
#include "specific.h"
#include "winmain.h"
#include "template.h"

struct SOUND_ADAPTER : GENERIC_ADAPTER {
};

extern "C" {

static LIST<SOUND_ADAPTER> SoundAdapterList;
static SOUND_ADAPTER CurrentSoundAdapter;
static LIST<SOUND_ADAPTER>::NODE *PrimarySoundAdapter;
extern LPDIRECTSOUND lpDirectSound;
extern LPDIRECTSOUNDBUFFER DS_Buffers[256];
extern int DS_SampleFrequencies[256];
extern int DS_SamplesPlaying[32];
extern LPDIRECTSOUNDBUFFER DS_Samples[32];
int DS_GetFreeChannel(void);

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

bool DS_MakeSample(int nSample, LPWAVEFORMATEX pWF, LPVOID pData, DWORD dwLen)
{
    DSBUFFERDESC desc;
    LPVOID pBlock;
    DWORD dwBlockLength;

    if (!App.DXConfig.sound)
        return false;
    if (nSample >= ArraySize(DS_Buffers))
        return false;
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DSBCAPS_STATIC | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME;
    desc.dwBufferBytes = dwLen;
    desc.lpwfxFormat = pWF;
    desc.dwReserved = 0;
    if (DX_TRY(lpDirectSound->CreateSoundBuffer(&desc, DS_Buffers + nSample, NULL)))
        return false;
    if (DX_TRY(DS_Buffers[nSample]->Lock(0, dwLen, &pBlock, &dwBlockLength, NULL, NULL, 0)))
        return false;
    memcpy(pBlock, pData, dwBlockLength);
    if (DX_TRY(DS_Buffers[nSample]->Unlock(pBlock, dwBlockLength, NULL, 0)))
        return false;
    DS_SampleFrequencies[nSample] = pWF->nSamplesPerSec;
    return true;
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

int DS_StartSample(int nSample, int nVolume, int nPitch, int nPan, DWORD dwFlags)
{
    int nChannel;
    LPDIRECTSOUNDBUFFER pOrigBuffer;
    LPDIRECTSOUNDBUFFER pBuffer;
    DWORD dwFrequency;

    nChannel = DS_GetFreeChannel();
    if (nChannel < 0)
        return -1;
    pOrigBuffer = DS_Buffers[nSample];
    if (DX_TRY(lpDirectSound->DuplicateSoundBuffer(pOrigBuffer, &pBuffer)))
        return -2;
    dwFrequency = (DWORD)(float)(DWORD)(nPitch * DS_SampleFrequencies[nSample]) >> 16;
    if (dwFrequency < 100)
        dwFrequency = 100;
    if (dwFrequency > 100000)
        dwFrequency = 100000;
    if (DX_TRY(pBuffer->SetVolume(nVolume)) || DX_TRY(pBuffer->SetFrequency(dwFrequency)) || DX_TRY(pBuffer->SetPan(nPan))
        || DX_TRY(pBuffer->SetCurrentPosition(0)) || DX_TRY(pBuffer->Play(0, 0, dwFlags)))
        return -2;
    DS_Samples[nChannel] = pBuffer;
    DS_SamplesPlaying[nChannel] = nSample;
    return nChannel;
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

bool DS_EnumerateDevices(LPVOID lpContext);

bool DS_MakeAdapterList(void)
{
    LIST<SOUND_ADAPTER>::NODE *node;

    SoundAdapterList.RemoveAll();
    if (!DS_EnumerateDevices(&SoundAdapterList))
        return false;
    PrimarySoundAdapter = 0;
    for (node = SoundAdapterList.GetHead(); node; node = SoundAdapterList.GetNext(node)) {
        if (!SoundAdapterList[node].lpGuid) {
            PrimarySoundAdapter = node;
            return true;
        }
    }
    return true;
}

BOOL CALLBACK DS_EnumCallback(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext)
{
    LIST<SOUND_ADAPTER> *list = (LIST<SOUND_ADAPTER> *)lpContext;
    SOUND_ADAPTER &adapter = list->NewTail();

    adapter.SetGUID(lpGuid);
    strcpy(adapter.description, lpcstrDescription);
    strcpy(adapter.name, lpcstrModule);
    return TRUE;
}

bool DS_EnumerateDevices(LPVOID lpContext)
{
    if (DX_TRY(DirectSoundEnumerate(DS_EnumCallback, lpContext)))
        return false;
    return true;
}

#include "../game.h"
#include "init_result.h"

extern int UnusedSoundStartupState;
bool DS_Create(LPGUID lpGuid);
bool DS_SetOutputFormat(void);

void DS_Start(HWND hwnd)
{
    memset(DS_Buffers, 0, sizeof(DS_Buffers));
    memset(DS_Samples, 0, sizeof(DS_Samples));
    UnusedSoundStartupState = 0;
    camera.mike_at_lara = 0;
    if (DS_Create(App.DeviceInfoPtr->DSInfo[App.DXConfigPtr->DS].lpGuid)) {
        if (!hwnd)
            hwnd = App.WindowHandle;
        if (DX_TRY(lpDirectSound->SetCooperativeLevel(hwnd, DSSCL_EXCLUSIVE)))
            throw INIT_ERR_CantSetDSCooperativeLevel;
        DS_SetOutputFormat();
    }
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
