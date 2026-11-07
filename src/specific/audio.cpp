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
extern DWORD AudioBytes;
extern DWORD acm_read;
extern DWORD acm_total_read;
extern DWORD acm_start_time;
extern DWORD StreamSize;
extern int audio_buffer_size;
extern int CurrentNotify;
extern int NotifySize;
extern unsigned char *pAudioWrite;
extern DWORD NextWriteOffset;
extern int XATrack;
extern int acm_volume;

BOOL WINAPI Func48e5e0(void);
BOOL WINAPI Func48e6d0(void);
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

bool ACMInit(void)
{
    DSBUFFERDESC desc;
    DWORD pMetric;
    int nEmpty;

    acm_ready = false;
    if (!App.DXConfig.sound)
        return false;
    if (!lpDirectSound)
        return false;

    Func48e5e0();
    acmGetVersion();
    acmDriverEnum(ACMEnumCallBack, 0, 0);
    if (!hACMDriverID)
        return false;
    if (acmDriverOpen(&hACMDriver, hACMDriverID, 0))
        return false;

    char wadname[80] = "audio\\cdaudio.wad";
    if (!ACMOpenFile(wadname))
        ACMOpenFile(GetFullPath(wadname));

    AllocBuffer = (unsigned char *)GlobalAlloc(GMEM_FIXED, 0x16040);
    ADPCMBuffer = (unsigned char *)(((long)AllocBuffer + 32) & ~31);
    ReadFile(acm_file, TrackInfos, sizeof(TrackInfos), &acm_read, 0);
    nEmpty = 0;
    while (!TrackInfos[nEmpty].size)
        nEmpty++;
    SetFilePointer(acm_file, TrackInfos[nEmpty].offset, 0, FILE_BEGIN);
    SetFilePointer(acm_file, 20, 0, FILE_CURRENT);
    ReadFile(acm_file, source_wav_format, 50, &acm_read, 0);

    wav_format.wFormatTag = WAVE_FORMAT_PCM;
    acmMetrics(0, ACM_METRIC_MAX_SIZE_FORMAT, &pMetric);
    acmFormatSuggest(hACMDriver, (LPWAVEFORMATEX)source_wav_format, &wav_format, pMetric, ACM_FORMATSUGGESTF_WFORMATTAG);

    memset(&desc, 0, sizeof(desc));
    audio_buffer_size = 0x577C0;
    NotifySize = 0x15DF0;
    desc.dwBufferBytes = 0x577C0;
    desc.dwSize = sizeof(DSBUFFERDESC);
    desc.dwFlags = DSBCAPS_LOCSOFTWARE | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2;
    desc.lpwfxFormat = &wav_format;
    lpDirectSound->CreateSoundBuffer(&desc, &DSBuffer, 0);
    DSBuffer->QueryInterface(IID_IDirectSoundNotify, (LPVOID *)&DSNotify);
    ACMSetupNotifications();
    acmStreamOpen(&hACMStream, hACMDriver, (LPWAVEFORMATEX)source_wav_format, &wav_format, 0, 0, 0, 0);
    acm_ready = true;

    while (DSBuffer->Lock(0, audio_buffer_size, (LPVOID *)&pAudioWrite, &AudioBytes, 0, 0, 0) != DS_OK);
    acmStreamSize(hACMStream, 0x5800, &StreamSize, 0);
    memset(pAudioWrite, 0, audio_buffer_size);

    memset(&StreamHeaders[0], 0, sizeof(ACMSTREAMHEADER));
    memset(&StreamHeaders[1], 0, sizeof(ACMSTREAMHEADER));
    memset(&StreamHeaders[2], 0, sizeof(ACMSTREAMHEADER));
    memset(&StreamHeaders[3], 0, sizeof(ACMSTREAMHEADER));
    StreamHeaders[0].cbStruct = sizeof(ACMSTREAMHEADER);
    StreamHeaders[0].pbSrc = ADPCMBuffer;
    StreamHeaders[0].cbSrcLength = 0x5800;
    StreamHeaders[0].cbDstLength = StreamSize;
    StreamHeaders[0].pbDst = &pAudioWrite[NotifySize * 0];
    StreamHeaders[1].cbStruct = sizeof(ACMSTREAMHEADER);
    StreamHeaders[1].pbSrc = ADPCMBuffer;
    StreamHeaders[1].cbSrcLength = 0x5800;
    StreamHeaders[1].cbDstLength = StreamSize;
    StreamHeaders[1].pbDst = &pAudioWrite[NotifySize * 1];
    StreamHeaders[2].cbStruct = sizeof(ACMSTREAMHEADER);
    StreamHeaders[2].pbSrc = ADPCMBuffer;
    StreamHeaders[2].cbSrcLength = 0x5800;
    StreamHeaders[2].cbDstLength = StreamSize;
    StreamHeaders[2].pbDst = &pAudioWrite[NotifySize * 2];
    StreamHeaders[3].cbStruct = sizeof(ACMSTREAMHEADER);
    StreamHeaders[3].pbSrc = ADPCMBuffer;
    StreamHeaders[3].cbSrcLength = 0x5800;
    StreamHeaders[3].cbDstLength = StreamSize;
    StreamHeaders[3].pbDst = &pAudioWrite[NotifySize * 3];
    acmStreamPrepareHeader(hACMStream, &StreamHeaders[0], 0);
    acmStreamPrepareHeader(hACMStream, &StreamHeaders[1], 0);
    acmStreamPrepareHeader(hACMStream, &StreamHeaders[2], 0);
    acmStreamPrepareHeader(hACMStream, &StreamHeaders[3], 0);

    DSBuffer->Unlock(pAudioWrite, audio_buffer_size, 0, 0);
    return true;
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
    Func48e6d0();
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

void ACMEmulateCDPlay(int track, int mode)
{
    if (!acm_ready)
        return;

    acm_wait = true;
    if (acm_locked)
        while (acm_locked);
    ACMEmulateCDStop();

    if (!DSBuffer || track > 130 || !TrackInfos[track].size)
        return;

    SetFilePointer(acm_file, TrackInfos[track].offset + 90, 0, FILE_BEGIN);
    XATrack = track;
    ReadFile(acm_file, ADPCMBuffer, 0x5800, &acm_read, 0);
    acm_total_read = acm_read;
    DSBuffer->SetVolume(acm_volume);
    DSBuffer->Lock(0, audio_buffer_size, (LPVOID *)&pAudioWrite, &AudioBytes, 0, 0, 0);
    acmStreamConvert(hACMStream, &StreamHeaders[0], ACM_STREAMCONVERTF_BLOCKALIGN | ACM_STREAMCONVERTF_START);
    while (!(StreamHeaders[0].fdwStatus & ACMSTREAMHEADER_STATUSF_DONE));

    ReadFile(acm_file, ADPCMBuffer, 0x5800, &acm_read, 0);
    acm_total_read += acm_read;
    acmStreamConvert(hACMStream, &StreamHeaders[1], ACM_STREAMCONVERTF_BLOCKALIGN);
    while (!(StreamHeaders[1].fdwStatus & ACMSTREAMHEADER_STATUSF_DONE));

    DSBuffer->Unlock(pAudioWrite, audio_buffer_size, 0, 0);
    if (DSBuffer) {
        acm_start_time = timeGetTime();
        DSBuffer->Stop();
        DSBuffer->SetCurrentPosition(0);
        DSBuffer->SetVolume(acm_volume);
        DSBuffer->Play(0, 0, DSBPLAY_LOOPING);
    }

    CurrentNotify = 2;
    NextWriteOffset = 2 * NotifySize;
    acm_loop_track = mode != 0;
    if (acm_total_read > TrackInfos[track].size) {
        acm_total_read = TrackInfos[track].size;
        acm_eof = true;
    } else {
        acm_eof = false;
    }
    acm_wait = false;
}

void ThreadACMEmulateCDPlay(int track, int mode)
{
    if (!acm_ready)
        return;

    acm_wait = true;
    ACMEmulateCDStop();

    if (!DSBuffer || track > 130 || !TrackInfos[track].size)
        return;

    SetFilePointer(acm_file, TrackInfos[track].offset + 90, 0, FILE_BEGIN);
    XATrack = track;
    ReadFile(acm_file, ADPCMBuffer, 0x5800, &acm_read, 0);
    acm_total_read = acm_read;
    DSBuffer->SetVolume(acm_volume);
    DSBuffer->Lock(0, audio_buffer_size, (LPVOID *)&pAudioWrite, &AudioBytes, 0, 0, 0);
    acmStreamConvert(hACMStream, &StreamHeaders[0], ACM_STREAMCONVERTF_BLOCKALIGN | ACM_STREAMCONVERTF_START);
    while (!(StreamHeaders[0].fdwStatus & ACMSTREAMHEADER_STATUSF_DONE));

    ReadFile(acm_file, ADPCMBuffer, 0x5800, &acm_read, 0);
    acm_total_read += acm_read;
    acmStreamConvert(hACMStream, &StreamHeaders[1], ACM_STREAMCONVERTF_BLOCKALIGN);
    while (!(StreamHeaders[1].fdwStatus & ACMSTREAMHEADER_STATUSF_DONE));

    DSBuffer->Unlock(pAudioWrite, audio_buffer_size, 0, 0);
    if (DSBuffer) {
        acm_start_time = timeGetTime();
        DSBuffer->Stop();
        DSBuffer->SetCurrentPosition(0);
        DSBuffer->SetVolume(acm_volume);
        DSBuffer->Play(0, 0, DSBPLAY_LOOPING);
    }

    CurrentNotify = 2;
    NextWriteOffset = 2 * NotifySize;
    acm_loop_track = mode != 0;
    if (acm_total_read > TrackInfos[track].size) {
        acm_total_read = TrackInfos[track].size;
        acm_eof = true;
    } else {
        acm_eof = false;
    }
    acm_wait = false;
}

void ACMEmulateCDStop(void)
{
    if (acm_ready && DSBuffer)
        DSBuffer->Stop();
}

int ACMHandleNotifications(void)
{
    unsigned char *write;
    DWORD wait;
    DWORD bytes;
    DWORD flag;
    DWORD abytes;
    volatile bool lostFocus;
    bool eot;

    lostFocus = false;
    while ((wait = WaitForMultipleObjects(2, NotifyEventHandles, FALSE, INFINITE)) != WAIT_FAILED) {
        if (acm_wait)
            continue;
        if (acm_done == true)
            break;

        while (!App.bFocus)
            lostFocus = true;

        if (lostFocus) {
            ThreadACMEmulateCDPlay(XATrack, acm_loop_track);
            lostFocus = false;
            continue;
        }

        if (wait)
            continue;

        if (!acm_eof) {
            eot = false;
            bytes = 0x5800;
            flag = ACM_STREAMCONVERTF_BLOCKALIGN;
            if (acm_total_read + bytes > TrackInfos[XATrack].size) {
                bytes = TrackInfos[XATrack].size - acm_total_read;
                flag = 0;
                eot = true;
            }

            ReadFile(acm_file, ADPCMBuffer, bytes, &acm_read, 0);
            acm_total_read += bytes;
            if (eot) {
                if (acm_loop_track) {
                    SetFilePointer(acm_file, -(long)acm_total_read, 0, FILE_CURRENT);
                    acm_total_read = 0;
                } else {
                    acm_eof = true;
                }
            }

            acm_locked = true;
            DSBuffer->Lock(NextWriteOffset, NotifySize, (LPVOID *)&write, &abytes, 0, 0, 0);
            if (eot) {
                memset(write, 0, NotifySize);
                StreamHeaders[CurrentNotify].cbSrcLength = bytes;
            }
            acmStreamConvert(hACMStream, &StreamHeaders[CurrentNotify], flag);
            while (!(StreamHeaders[CurrentNotify].fdwStatus & ACMSTREAMHEADER_STATUSF_DONE));
            DSBuffer->Unlock(write, abytes, 0, 0);
            acm_locked = false;
            if (eot)
                StreamHeaders[CurrentNotify].cbSrcLength = 0x5800;

            NextWriteOffset += abytes;
            if (NextWriteOffset >= audio_buffer_size)
                NextWriteOffset -= audio_buffer_size;
        } else {
            ACMSilentCount++;
            acm_locked = true;
            DSBuffer->Lock(NextWriteOffset, NotifySize, (LPVOID *)&write, &abytes, 0, 0, 0);
            memset(write, 0, NotifySize);
            NextWriteOffset += abytes;
            if (NextWriteOffset >= audio_buffer_size)
                NextWriteOffset -= audio_buffer_size;
            DSBuffer->Unlock(write, abytes, 0, 0);
            acm_locked = false;

            if (ACMSilentCount > 4) {
                ACMSilentCount = 0;
                DSBuffer->Stop();
                if (CurrentAtmosphere) {
                    ThreadACMEmulateCDPlay(CurrentAtmosphere, 1);
                    continue;
                }
            } else {
                acm_locked = true;
                DSBuffer->Lock(NextWriteOffset, NotifySize, (LPVOID *)&write, &abytes, 0, 0, 0);
                memset(write, 0, NotifySize);
                DSBuffer->Unlock(write, abytes, 0, 0);
                acm_locked = false;
            }
        }

        CurrentNotify++;
        if (CurrentNotify > 3)
            CurrentNotify = 0;
    }

    CloseHandle(NotifyEventHandles[0]);
    CloseHandle(NotifyEventHandles[1]);
    NotifyEventHandles[1] = 0;
    NotifyEventHandles[0] = 0;
    ExitThread(1);
    return 1;
}

int ACMSetupNotifications(void)
{
    DSBPOSITIONNOTIFY posNotif[5];
    DWORD ThreadId;
    int result;
    int size;
    int i;

    size = NotifySize;
    NotifyEventHandles[0] = CreateEvent(0, FALSE, FALSE, 0);
    NotifyEventHandles[1] = CreateEvent(0, FALSE, FALSE, 0);
    posNotif[0].dwOffset = size;
    posNotif[0].hEventNotify = NotifyEventHandles[0];
    for (i = 1; i < 4; i++) {
        posNotif[i].dwOffset = size + posNotif[i - 1].dwOffset;
        posNotif[i].hEventNotify = NotifyEventHandles[0];
    }
    posNotif[i - 1].dwOffset--;
    posNotif[4].dwOffset = DSBPN_OFFSETSTOP;
    posNotif[4].hEventNotify = NotifyEventHandles[1];
    ACMNotifyThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ACMHandleNotifications, 0, 0, &ThreadId);
    result = DSNotify->SetNotificationPositions(5, posNotif);
    if (result != DS_OK) {
        CloseHandle(NotifyEventHandles[0]);
        CloseHandle(NotifyEventHandles[1]);
        NotifyEventHandles[1] = 0;
        NotifyEventHandles[0] = 0;
    }
    return result;
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
