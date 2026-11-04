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
