#include "game.h"

#define MAX_SLOTS 32

#define ArraySize(a) (sizeof(a) / sizeof((a)[0]))

typedef struct {
    sint32 nVolume;
    sint32 nPan;
    sint32 nSampleInfo;
    sint32 nPitch;
} SOUND_SLOT;

extern SOUND_SLOT LaSlot[MAX_SLOTS];
extern int sound_active;

void S_SoundStopAllSamples(void);
void S_SoundStopSample(int nHandle);

typedef struct {
    sint16 number;
    uint8 volume;
    sint8 radius;
    sint8 randomness;
    sint8 pitch;
    sint16 flags;
} SAMPLE_INFO;

extern sint16 sample_lut[];
extern SAMPLE_INFO *sample_infos;

void StopSoundEffect(int nSFX)
{
    int nSlot;
    int nStartSampleInfo, nEndSampleInfo;

    if (!sound_active)
        return;
    nStartSampleInfo = sample_lut[nSFX];
    nEndSampleInfo = nStartSampleInfo + ((sample_infos[nStartSampleInfo].flags >> 2) & 15);
    for (nSlot = 0; nSlot < ArraySize(LaSlot); nSlot++)
    {
        if (LaSlot[nSlot].nSampleInfo >= nStartSampleInfo && LaSlot[nSlot].nSampleInfo < nEndSampleInfo)
        {
            S_SoundStopSample(nSlot);
            LaSlot[nSlot].nSampleInfo = -1;
        }
    }
}

void S_SoundSetPanAndVolume(int nHandle, sint16 nPan, uint16 nVolume);
void S_SoundSetPitch(int nHandle, int nPitch);
int S_SoundSampleIsPlaying(int nHandle);

void SOUND_EndScene(void)
{
    int nSlot;

    if (!sound_active)
        return;
    for (nSlot = 0; nSlot < ArraySize(LaSlot); nSlot++)
    {
        if (LaSlot[nSlot].nSampleInfo >= 0)
        {
            if ((sample_infos[LaSlot[nSlot].nSampleInfo].flags & 3) == 3)
            {
                if (LaSlot[nSlot].nVolume)
                {
                    S_SoundSetPanAndVolume(nSlot, (sint16)LaSlot[nSlot].nPan, (uint16)LaSlot[nSlot].nVolume);
                    S_SoundSetPitch(nSlot, LaSlot[nSlot].nPitch);
                    LaSlot[nSlot].nVolume = 0;
                }
                else
                {
                    S_SoundStopSample(nSlot);
                    LaSlot[nSlot].nSampleInfo = -1;
                }
            }
            else if (!S_SoundSampleIsPlaying(nSlot))
                LaSlot[nSlot].nSampleInfo = -1;
        }
    }
}

void SOUND_Stop(void)
{
    int nSlot;

    if (!sound_active)
        return;

    S_SoundStopAllSamples();
    for (nSlot = 0; nSlot < ArraySize(LaSlot); ++nSlot)
        LaSlot[nSlot].nSampleInfo = -1;
}

void SOUND_Init(void)
{
    int nSlot;

    for (nSlot = 0; nSlot < ArraySize(LaSlot); ++nSlot)
        LaSlot[nSlot].nSampleInfo = -1;
    sound_active = 1;
}
