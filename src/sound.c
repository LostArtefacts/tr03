#include "specific/standard.h"
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
    uint8 radius;
    sint8 randomness;
    sint8 pitch;
    sint16 flags;
} SAMPLE_INFO;

extern sint16 sample_lut[];
extern SAMPLE_INFO *sample_infos;

int S_SoundPlaySample(int nSample, uint16 wVol, int nPitch, sint16 wPan);
int S_SoundPlaySampleLooped(int nSample, uint16 wVol, int nPitch, sint16 wPan);
int S_SoundSampleIsPlaying(int nHandle);

int SoundEffect(int nSFX, PHD_3DPOS *pPos, int flags)
{
    SAMPLE_INFO *pSI;
    uint32 distance;
    uint16 pan;
    sint32 x, y, z;
    int pitch;
    int nVolume;
    int nSampleInfo, nSample, nSfxSamples, nSlot, nMode, nHandle;
    int nMinVolume, nMinSlot, nAttenuation;
    int SOUND_RADIUS;
    int SOUND_RADIUS_SQRD;

    if (!sound_active || (!(flags & 2) && (flags & UNDERWATER) != (room[camera.pos.room_number].flags & UNDERWATER)))
        return 0;
    nSampleInfo = sample_lut[nSFX];
    if (nSampleInfo == -1) {
        sample_lut[nSFX] = -2;
        return 0;
    }
    if (nSampleInfo == -2)
        return 0;
    pSI = &sample_infos[nSampleInfo];
    if (pSI->randomness && (GetRandomControl() & 255) > pSI->randomness)
        return 0;
    pan = 0;
    SOUND_RADIUS = pSI->radius << 10;
    SOUND_RADIUS_SQRD = SOUND_RADIUS * SOUND_RADIUS;
    if (pPos) {
        x = pPos->x_pos - camera.mike_pos.x;
        y = pPos->y_pos - camera.mike_pos.y;
        z = pPos->z_pos - camera.mike_pos.z;
        if (x < -SOUND_RADIUS || x > SOUND_RADIUS || y < -SOUND_RADIUS || y > SOUND_RADIUS || z < -SOUND_RADIUS || z > SOUND_RADIUS)
            return 0;
        distance = x * x + y * y + z * z;
        if (distance > (uint32)SOUND_RADIUS_SQRD)
            return 0;
        else if (distance < 0x100000)
            distance = 0;
        else
            distance = phd_sqrt(distance) - 1024;
        if (!(pSI->flags & 0x1000))
            pan = phd_atan(z, x) - camera.actual_angle;
    } else {
        distance = 0;
    }
    nVolume = pSI->volume << 7;
    if (pSI->flags & 0x4000)
        nVolume -= (GetRandomControl() * 0x1000) >> 15;
    nAttenuation = distance * distance / (SOUND_RADIUS_SQRD / 0x10000);
    nVolume = ((0x10000 - nAttenuation) * nVolume) >> 16;
    if (nVolume <= 0)
        return 0;
    else if (nVolume > 0x7fff)
        nVolume = 0x7fff;
    if (flags & 4)
        pitch = (flags >> 8) & 0xffffff;
    else
        pitch = 0x10000;
    pitch += pSI->pitch << 9;
    if (pSI->flags & 0x2000)
        pitch += ((GetRandomControl() * 6000) >> 14) - 6000;
    if (pSI->number < 0)
        return 0;
    nSfxSamples = (pSI->flags >> 2) & 15;
    if (nSfxSamples == 1)
        nSample = pSI->number;
    else
        nSample = pSI->number + ((GetRandomControl() * nSfxSamples) >> 15);
    nMode = pSI->flags & 3;
    switch (nMode) {
    case 1:
        for (nSlot = 0; nSlot < ArraySize(LaSlot); nSlot++) {
            if (LaSlot[nSlot].nSampleInfo == nSampleInfo) {
                if (S_SoundSampleIsPlaying(nSlot))
                    return 0;
                LaSlot[nSlot].nSampleInfo = -1;
            }
        }
        break;
    case 2:
        for (nSlot = 0; nSlot < ArraySize(LaSlot); nSlot++) {
            if (LaSlot[nSlot].nSampleInfo == nSampleInfo) {
                S_SoundStopSample(nSlot);
                LaSlot[nSlot].nSampleInfo = -1;
                break;
            }
        }
        break;
    case 3:
        for (nSlot = 0; nSlot < ArraySize(LaSlot); nSlot++) {
            if (LaSlot[nSlot].nSampleInfo == nSampleInfo) {
                if (nVolume > LaSlot[nSlot].nVolume) {
                    LaSlot[nSlot].nVolume = nVolume;
                    LaSlot[nSlot].nPan = pan;
                    LaSlot[nSlot].nPitch = pitch;
                    return 1;
                }
                return 0;
            }
        }
        break;
    case 0:
        break;
    }
    if (nMode == 3)
        nHandle = S_SoundPlaySampleLooped(nSample, (uint16)nVolume, pitch, pan);
    else
        nHandle = S_SoundPlaySample(nSample, (uint16)nVolume, pitch, pan);
    if (nHandle >= 0) {
        LaSlot[nHandle].nVolume = nVolume;
        LaSlot[nHandle].nPan = pan;
        LaSlot[nHandle].nPitch = pitch;
        LaSlot[nHandle].nSampleInfo = nSampleInfo;
        return 1;
    }
    if (nHandle != -1) {
        pSI->number = -1;
        return 0;
    }
    nMinVolume = 0x8000;
    nMinSlot = -1;
    for (nSlot = 1; nSlot < ArraySize(LaSlot); nSlot++) {
        if (LaSlot[nSlot].nSampleInfo >= 0 && LaSlot[nSlot].nVolume < nMinVolume) {
            nMinVolume = LaSlot[nSlot].nVolume;
            nMinSlot = nSlot;
        }
    }
    if (nMinSlot < 0)
        return 0;
    if (nVolume < nMinVolume)
        return 0;
    S_SoundStopSample(nMinSlot);
    LaSlot[nMinSlot].nSampleInfo = -1;
    if (nMode == 3)
        nHandle = S_SoundPlaySampleLooped(nSample, (uint16)nVolume, pitch, pan);
    else
        nHandle = S_SoundPlaySample(nSample, (uint16)nVolume, pitch, pan);
    if (nHandle >= 0) {
        LaSlot[nHandle].nVolume = nVolume;
        LaSlot[nHandle].nPan = pan;
        LaSlot[nHandle].nPitch = pitch;
        LaSlot[nHandle].nSampleInfo = nSampleInfo;
        return 1;
    }
    return 0;
}

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
