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

}
