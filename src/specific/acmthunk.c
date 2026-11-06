#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <msacm.h>

#define ACMTHUNK_MAX_FUNCTIONS 41

extern HMODULE Var4ce708;
extern char Var4ce710[];
extern LPSTR Var4ce720[];
extern FARPROC *Var6cf7f0;

BOOL WINAPI Func48e5e0(void)
{
    DWORD (ACMAPI *pfnAcmGetVersion)(void);
    UINT fuErrorMode;
    WORD wVersion;
    UINT u;

    if (Var6cf7f0 != NULL)
        return TRUE;

    if (Var4ce708 != (HMODULE)-1)
        return FALSE;

    fuErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX);
    Var4ce708 = LoadLibrary(Var4ce710);
    SetErrorMode(fuErrorMode);
    if (Var4ce708 < (HMODULE)HINSTANCE_ERROR) {
        Var4ce708 = NULL;
        return FALSE;
    }

    pfnAcmGetVersion = (DWORD (ACMAPI *)(void))GetProcAddress(Var4ce708, Var4ce720[0]);
    if (pfnAcmGetVersion == NULL) {
        FreeLibrary(Var4ce708);
        Var4ce708 = NULL;
        return FALSE;
    }

    Var6cf7f0 = (FARPROC *)LocalAlloc(LPTR, sizeof(FARPROC) * ACMTHUNK_MAX_FUNCTIONS);
    if (Var6cf7f0 == NULL) {
        FreeLibrary(Var4ce708);
        Var4ce708 = NULL;
        return FALSE;
    }

    Var6cf7f0[0] = (FARPROC)pfnAcmGetVersion;
    wVersion = HIWORD(pfnAcmGetVersion());
    if (wVersion >= 0x0200) {
        for (u = 1; u < ACMTHUNK_MAX_FUNCTIONS; u++)
            Var6cf7f0[u] = GetProcAddress(Var4ce708, Var4ce720[u]);
    }

    return TRUE;
}

BOOL WINAPI Func48e6d0(void)
{
    if (Var6cf7f0 != NULL) {
        LocalFree((HLOCAL)Var6cf7f0);
        Var6cf7f0 = NULL;
        FreeLibrary(Var4ce708);
        Var4ce708 = (HMODULE)-1;
    }
    return TRUE;
}

DWORD ACMAPI acmGetVersion(void)
{
    DWORD (ACMAPI *pfnAcmGetVersion)(void);

    if (Var6cf7f0 == NULL)
        return 0;

    pfnAcmGetVersion = (DWORD (ACMAPI *)(void))Var6cf7f0[0];
    if (pfnAcmGetVersion == NULL)
        return 0;

    return pfnAcmGetVersion();
}

MMRESULT ACMAPI acmMetrics(HACMOBJ hao, UINT uMetric, LPVOID pMetric)
{
    MMRESULT (ACMAPI *pfn)(HACMOBJ, UINT, LPVOID);

    if (Var6cf7f0 == NULL)
        return MMSYSERR_ERROR;

    pfn = (MMRESULT (ACMAPI *)(HACMOBJ, UINT, LPVOID))Var6cf7f0[1];
    if (pfn == NULL)
        return MMSYSERR_ERROR;

    return pfn(hao, uMetric, pMetric);
}

MMRESULT ACMAPI acmDriverEnum(ACMDRIVERENUMCB fnCallback, DWORD dwInstance, DWORD fdwEnum)
{
    MMRESULT (ACMAPI *pfn)(ACMDRIVERENUMCB, DWORD, DWORD);

    if (Var6cf7f0 == NULL)
        return MMSYSERR_ERROR;

    pfn = (MMRESULT (ACMAPI *)(ACMDRIVERENUMCB, DWORD, DWORD))Var6cf7f0[2];
    if (pfn == NULL)
        return MMSYSERR_ERROR;

    return pfn(fnCallback, dwInstance, fdwEnum);
}

MMRESULT ACMAPI acmDriverDetailsA(HACMDRIVERID hadid, LPACMDRIVERDETAILSA padd, DWORD fdwDetails)
{
    MMRESULT (ACMAPI *pfn)(HACMDRIVERID, LPACMDRIVERDETAILSA, DWORD);

    if (Var6cf7f0 == NULL)
        return MMSYSERR_ERROR;

    pfn = (MMRESULT (ACMAPI *)(HACMDRIVERID, LPACMDRIVERDETAILSA, DWORD))Var6cf7f0[4];
    if (pfn == NULL)
        return MMSYSERR_ERROR;

    return pfn(hadid, padd, fdwDetails);
}

MMRESULT ACMAPI acmDriverOpen(LPHACMDRIVER phad, HACMDRIVERID hadid, DWORD fdwOpen)
{
    MMRESULT (ACMAPI *pfn)(LPHACMDRIVER, HACMDRIVERID, DWORD);

    if (Var6cf7f0 == NULL)
        return MMSYSERR_ERROR;

    pfn = (MMRESULT (ACMAPI *)(LPHACMDRIVER, HACMDRIVERID, DWORD))Var6cf7f0[8];
    if (pfn == NULL)
        return MMSYSERR_ERROR;

    return pfn(phad, hadid, fdwOpen);
}

MMRESULT ACMAPI acmDriverClose(HACMDRIVER had, DWORD fdwClose)
{
    MMRESULT (ACMAPI *pfn)(HACMDRIVER, DWORD);

    if (Var6cf7f0 == NULL)
        return MMSYSERR_ERROR;

    pfn = (MMRESULT (ACMAPI *)(HACMDRIVER, DWORD))Var6cf7f0[9];
    if (pfn == NULL)
        return MMSYSERR_ERROR;

    return pfn(had, fdwClose);
}

MMRESULT ACMAPI acmFormatSuggest(HACMDRIVER had, LPWAVEFORMATEX pwfxSrc, LPWAVEFORMATEX pwfxDst, DWORD cbwfxDst, DWORD fdwSuggest)
{
    MMRESULT (ACMAPI *pfn)(HACMDRIVER, LPWAVEFORMATEX, LPWAVEFORMATEX, DWORD, DWORD);

    if (Var6cf7f0 == NULL)
        return MMSYSERR_ERROR;

    pfn = (MMRESULT (ACMAPI *)(HACMDRIVER, LPWAVEFORMATEX, LPWAVEFORMATEX, DWORD, DWORD))Var6cf7f0[23];
    if (pfn == NULL)
        return MMSYSERR_ERROR;

    return pfn(had, pwfxSrc, pwfxDst, cbwfxDst, fdwSuggest);
}

MMRESULT ACMAPI acmStreamOpen(LPHACMSTREAM phas, HACMDRIVER had, LPWAVEFORMATEX pwfxSrc, LPWAVEFORMATEX pwfxDst, LPWAVEFILTER pwfltr, DWORD dwCallback, DWORD dwInstance, DWORD fdwOpen)
{
    MMRESULT (ACMAPI *pfn)(LPHACMSTREAM, HACMDRIVER, LPWAVEFORMATEX, LPWAVEFORMATEX, LPWAVEFILTER, DWORD, DWORD, DWORD);

    if (Var6cf7f0 == NULL)
        return MMSYSERR_ERROR;

    pfn = (MMRESULT (ACMAPI *)(LPHACMSTREAM, HACMDRIVER, LPWAVEFORMATEX, LPWAVEFORMATEX, LPWAVEFILTER, DWORD, DWORD, DWORD))Var6cf7f0[34];
    if (pfn == NULL)
        return MMSYSERR_ERROR;

    return pfn(phas, had, pwfxSrc, pwfxDst, pwfltr, dwCallback, dwInstance, fdwOpen);
}

MMRESULT ACMAPI acmStreamClose(HACMSTREAM has, DWORD fdwClose)
{
    MMRESULT (ACMAPI *pfn)(HACMSTREAM, DWORD);

    if (Var6cf7f0 == NULL)
        return MMSYSERR_ERROR;

    pfn = (MMRESULT (ACMAPI *)(HACMSTREAM, DWORD))Var6cf7f0[35];
    if (pfn == NULL)
        return MMSYSERR_ERROR;

    return pfn(has, fdwClose);
}

MMRESULT ACMAPI acmStreamSize(HACMSTREAM has, DWORD cbInput, LPDWORD pdwOutputBytes, DWORD fdwSize)
{
    MMRESULT (ACMAPI *pfn)(HACMSTREAM, DWORD, LPDWORD, DWORD);

    if (Var6cf7f0 == NULL)
        return MMSYSERR_ERROR;

    pfn = (MMRESULT (ACMAPI *)(HACMSTREAM, DWORD, LPDWORD, DWORD))Var6cf7f0[36];
    if (pfn == NULL)
        return MMSYSERR_ERROR;

    return pfn(has, cbInput, pdwOutputBytes, fdwSize);
}

MMRESULT ACMAPI acmStreamConvert(HACMSTREAM has, LPACMSTREAMHEADER pash, DWORD fdwConvert)
{
    MMRESULT (ACMAPI *pfn)(HACMSTREAM, LPACMSTREAMHEADER, DWORD);

    if (Var6cf7f0 == NULL)
        return MMSYSERR_ERROR;

    pfn = (MMRESULT (ACMAPI *)(HACMSTREAM, LPACMSTREAMHEADER, DWORD))Var6cf7f0[37];
    if (pfn == NULL)
        return MMSYSERR_ERROR;

    return pfn(has, pash, fdwConvert);
}

MMRESULT ACMAPI acmStreamPrepareHeader(HACMSTREAM has, LPACMSTREAMHEADER pash, DWORD fdwPrepare)
{
    MMRESULT (ACMAPI *pfn)(HACMSTREAM, LPACMSTREAMHEADER, DWORD);

    if (Var6cf7f0 == NULL)
        return MMSYSERR_ERROR;

    pfn = (MMRESULT (ACMAPI *)(HACMSTREAM, LPACMSTREAMHEADER, DWORD))Var6cf7f0[39];
    if (pfn == NULL)
        return MMSYSERR_ERROR;

    return pfn(has, pash, fdwPrepare);
}

MMRESULT ACMAPI acmStreamUnprepareHeader(HACMSTREAM has, LPACMSTREAMHEADER pash, DWORD fdwUnprepare)
{
    MMRESULT (ACMAPI *pfn)(HACMSTREAM, LPACMSTREAMHEADER, DWORD);

    if (Var6cf7f0 == NULL)
        return MMSYSERR_ERROR;

    pfn = (MMRESULT (ACMAPI *)(HACMSTREAM, LPACMSTREAMHEADER, DWORD))Var6cf7f0[40];
    if (pfn == NULL)
        return MMSYSERR_ERROR;

    return pfn(has, pash, fdwUnprepare);
}
