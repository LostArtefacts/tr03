#include "standard.h"
#include <mmreg.h>
#include <msacm.h>
#include "winmain.h"

extern "C" {

typedef struct track_info {
    char name[260];
    unsigned long size;
    unsigned long offset;
} TRACK_INFO;

extern HANDLE acm_file;
extern bool acm_ready;
extern bool acm_wait;
extern bool acm_done;
static bool acm_loop_track;
extern bool acm_eof;
extern volatile bool acm_locked;
extern LPDIRECTSOUNDBUFFER DSBuffer;
extern LPDIRECTSOUNDNOTIFY DSNotify;
extern LPDIRECTSOUND lpDirectSound;
extern ACMSTREAMHEADER StreamHeaders[4];
extern HACMDRIVER hACMDriver;
extern HACMDRIVERID hACMDriverID;
extern HACMSTREAM hACMStream;
extern HANDLE NotifyEventHandles[2];
extern HANDLE ACMNotifyThread;
extern TRACK_INFO TrackInfos[130];
extern WAVEFORMATEX wav_format;
extern char source_wav_format[50];
extern unsigned char *AllocBuffer;
extern unsigned char *ADPCMBuffer;
extern unsigned char *pAudioWrite;
extern DWORD AudioBytes;
extern DWORD acm_read;
extern DWORD acm_total_read;
extern DWORD acm_start_time;
extern DWORD StreamSize;
extern int audio_buffer_size;
extern int CurrentNotify;
extern int NotifySize;
extern DWORD NextWriteOffset;
extern int XATrack;
extern int acm_volume;

BOOL WINAPI acmThunkInitialize(void);
BOOL WINAPI acmThunkTerminate(void);
const char *GetFullPath(const char *name);
void ACMCloseFile(void);
bool ACMOpenFile(const char *name);
int ACMSetupNotifications(void);
void ACMEmulateCDStop(void);
extern int ACMSilentCount;
extern unsigned char CurrentAtmosphere;

BOOL CALLBACK ACMEnumCallBack(HACMDRIVERID hadid, DWORD dwInstance, DWORD fdwSupport)
{
    ACMDRIVERDETAILS driver;

    memset(&driver, 0, sizeof(driver));
    driver.cbStruct = sizeof(ACMDRIVERDETAILS);
    acmDriverDetails(hadid, &driver, 0);
    if (strcmp(driver.szShortName, "MS-ADPCM"))
        return TRUE;
    hACMDriverID = hadid;
    return FALSE;
}

void ACMClose(void)
{
    if (!acm_ready)
        return;

    acm_done = true;
    if (DSBuffer) {
        DSBuffer->Play(0, 0, DSBPLAY_LOOPING);
        DSBuffer->Stop();
    }

    if (StreamHeaders[0].fdwStatus & ACMSTREAMHEADER_STATUSF_PREPARED) {
        StreamHeaders[0].cbSrcLength = 0x5800;
        StreamHeaders[0].cbDstLength = StreamSize;
        acmStreamUnprepareHeader(hACMStream, &StreamHeaders[0], 0);
    }
    if (StreamHeaders[1].fdwStatus & ACMSTREAMHEADER_STATUSF_PREPARED) {
        StreamHeaders[0].cbSrcLength = 0x5800;
        StreamHeaders[0].cbDstLength = StreamSize;
        acmStreamUnprepareHeader(hACMStream, &StreamHeaders[1], 0);
    }
    if (StreamHeaders[2].fdwStatus & ACMSTREAMHEADER_STATUSF_PREPARED) {
        StreamHeaders[0].cbSrcLength = 0x5800;
        StreamHeaders[0].cbDstLength = StreamSize;
        acmStreamUnprepareHeader(hACMStream, &StreamHeaders[2], 0);
    }
    if (StreamHeaders[3].fdwStatus & ACMSTREAMHEADER_STATUSF_PREPARED) {
        StreamHeaders[0].cbSrcLength = 0x5800;
        StreamHeaders[0].cbDstLength = StreamSize;
        acmStreamUnprepareHeader(hACMStream, &StreamHeaders[3], 0);
    }

    acmStreamClose(hACMStream, 0);
    acmDriverClose(hACMDriver, 0);
    GlobalFree(AllocBuffer);
    if (DSNotify)
        DSNotify->Release();
    if (DSBuffer)
        DSBuffer->Release();
    ACMCloseFile();
    acmThunkTerminate();
}

bool ACMOpenFile(const char *name)
{
    ACMCloseFile();
    acm_file = CreateFile(name, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);
    if (acm_file == INVALID_HANDLE_VALUE)
        return false;
    return true;
}

void ACMCloseFile(void)
{
    if (acm_file) {
        CloseHandle(acm_file);
        acm_file = 0;
    }
}

void ACMEmulateCDStop(void)
{
    if (acm_ready && DSBuffer)
        DSBuffer->Stop();
}

int ACMGetTrackLocation(void)
{
    return (int)(60.0f * ((float)(timeGetTime() - acm_start_time) * 0.001f));
}

void ACMSetVolume(int volume)
{
    acm_volume = (int)(((float)volume * 1.5625f - 400.0f) * 6.0f);
    if (DSBuffer)
        DSBuffer->SetVolume(acm_volume);
}

}
