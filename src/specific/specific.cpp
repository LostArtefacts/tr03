#include "standard.h"
#include "specific.h"
#include "winmain.h"

extern "C" {

extern int sound_active;
extern int LnMasterVolume;

int DS_StartSample(int nSample, int nVolume, int nPitch, int nPan, unsigned long dwFlags);
void DS_AdjustVolumeAndPan(int nHandle, int nVolume, int nPan);
void DS_AdjustPitch(int nHandle, int nPitch);
void DS_StopSample(int nHandle);
void DS_StopAllSamples(void);
bool DS_IsChannelPlaying(int nHandle);
void ACMEmulateCDPlay(int track, int mode);
void ACMEmulateCDStop(void);
int ACMGetTrackLocation(void);
void ACMSetVolume(int volume);

int CalcPan(sint16 angle);

int CalcVolume(int nGameVolume)
{
    nGameVolume *= LnMasterVolume;
    return (int)(((float)nGameVolume * 0.00019074068f - 400.0f) * 6.0f);
}


int S_SoundPlaySample(int nSample, unsigned short wVol, int nPitch, sint16 wPan)
{
    if (!sound_active)
        return -3;
    return DS_StartSample(nSample, CalcVolume(wVol), nPitch, CalcPan(wPan), 0);
}

int CalcPan(sint16 angle)
{
    angle += 0x4000;
    if (angle < 0) {
        angle = -angle;
        if (angle < 0)
            angle = 0x7fff;
    }
    angle -= 0x4000;
    return angle >> 4;
}

int S_SoundPlaySampleLooped(int nSample, unsigned short wVol, int nPitch, sint16 wPan)
{
    if (!sound_active)
        return -3;
    return DS_StartSample(nSample, CalcVolume(wVol), nPitch, CalcPan(wPan), DSBPLAY_LOOPING);
}

void S_SoundSetPanAndVolume(int nHandle, sint16 wPan, unsigned short wVolume)
{
    if (!sound_active)
        return;
    DS_AdjustVolumeAndPan(nHandle, CalcVolume(wVolume), CalcPan(wPan));
}

void S_SoundSetPitch(int nHandle, int nPitch)
{
    if (!sound_active)
        return;
    DS_AdjustPitch(nHandle, nPitch);
}

void S_SoundSetMasterVolume(int nVolume)
{
    LnMasterVolume = nVolume;
}

void S_SoundStopSample(int nHandle)
{
    if (!sound_active)
        return;
    DS_StopSample(nHandle);
}

void S_SoundStopAllSamples(void)
{
    if (!sound_active)
        return;
    DS_StopAllSamples();
}

int S_SoundSampleIsPlaying(int nHandle)
{
    if (!sound_active)
        return 0;
    return DS_IsChannelPlaying(nHandle);
}

void S_CDPlay(sint16 nTrack, int tMode)
{
    ACMEmulateCDPlay(nTrack, tMode);
}

void S_CDStop(void)
{
    ACMEmulateCDStop();
}

int S_StartSyncedAudio(int nTrack)
{
    if (!App.DXConfig.sound)
        return 0;
    ACMEmulateCDPlay(nTrack, 0);
    return 1;
}

int S_CDGetLoc(void)
{
    return ACMGetTrackLocation();
}

}
