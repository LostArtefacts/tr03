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
int FindCDDrive(void);
bool UT_OKCancelBox(char *szDlgResource, HWND hWndOwner);

extern char cd_drive;
extern char VFAW[];
extern char NEIR[];
extern char OKET[];
extern char AWCS[];

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

extern UINT wDeviceID;
extern int cd_looping;

void S_CDLoop(void)
{
    MCI_PLAY_PARMS pp;
    MCI_STATUS_PARMS stat;
    int ok;

    if (!cd_looping)
        return;
    stat.dwItem = MCI_STATUS_MODE;
    ok = mciSendCommand(wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)&stat) == 0;
    if (ok && stat.dwReturn == MCI_MODE_STOP) {
        pp.dwFrom = MCI_MAKE_TMSF(cd_looping, 0, 0, 0);
        pp.dwTo = MCI_MAKE_TMSF(cd_looping + 1, 0, 0, 0);
        mciSendCommand(wDeviceID, MCI_PLAY, MCI_FROM | MCI_TO, (DWORD)&pp);
    }
}

bool CD_Init(void)
{
    FILE *file;
    int found;

    while (!FindCDDrive()) {
        if (!UT_OKCancelBox(MAKEINTRESOURCE(102), App.WindowHandle))
            return false;
    }

    VFAW[0] = cd_drive;
    NEIR[0] = cd_drive;
    OKET[0] = cd_drive;
    AWCS[0] = cd_drive;

    while (1) {
        file = fopen(VFAW, "rb");
        if (!file) {
            found = 0;
        } else {
            fseek(file, 0x29845000, SEEK_SET);
            found = fgetc(file) == '{';
            fclose(file);
        }
        if (found)
            break;

        file = fopen(NEIR, "rb");
        if (!file) {
            found = 0;
        } else {
            fseek(file, 0x29825800, SEEK_SET);
            found = fgetc(file) == '3';
            fclose(file);
        }
        if (found)
            break;

        file = fopen(OKET, "rb");
        if (!file) {
            found = 0;
        } else {
            fseek(file, 0x29842800, SEEK_SET);
            found = fgetc(file) == 'u';
            fclose(file);
        }
        if (found)
            break;

        file = fopen(AWCS, "rb");
        if (!file) {
            found = 0;
        } else {
            fseek(file, 0x2981E000, SEEK_SET);
            found = fgetc(file) == '{';
            fclose(file);
        }
        if (found)
            break;

        if (!UT_OKCancelBox(MAKEINTRESOURCE(102), App.WindowHandle))
            return false;
    }

    return true;
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

void S_CDVolume(int volume)
{
    ACMSetVolume(volume);
}

}

#ifdef TR3_EXE
extern "C" {

char VFAW[] = "d:\\VFAW.AFP";
char NEIR[] = "d:\\NEIR.AFP";
char OKET[] = "d:\\OKET.AFP";
char AWCS[] = "d:\\AWCS.AFP";

}
#endif
