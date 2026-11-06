#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <msacm.h>

#define ACMTHUNK_MAX_FUNCTIONS 41

extern HMODULE ghinstAcm;
extern char gszAcmModuleName[];
extern LPSTR gapszAcmFunctions[];
extern FARPROC *gpafnAcmFunctions;

BOOL WINAPI acmThunkInitialize(void)
{
    DWORD (ACMAPI *pfnAcmGetVersion)(void);
    UINT fuErrorMode;
    WORD wVersion;
    UINT u;

    if (gpafnAcmFunctions != NULL)
        return TRUE;

    if (ghinstAcm != (HMODULE)-1)
        return FALSE;

    fuErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX);
    ghinstAcm = LoadLibrary(gszAcmModuleName);
    SetErrorMode(fuErrorMode);
    if (ghinstAcm < (HMODULE)HINSTANCE_ERROR) {
        ghinstAcm = NULL;
        return FALSE;
    }

    pfnAcmGetVersion = (DWORD (ACMAPI *)(void))GetProcAddress(ghinstAcm, gapszAcmFunctions[0]);
    if (pfnAcmGetVersion == NULL) {
        FreeLibrary(ghinstAcm);
        ghinstAcm = NULL;
        return FALSE;
    }

    gpafnAcmFunctions = (FARPROC *)LocalAlloc(LPTR, sizeof(FARPROC) * ACMTHUNK_MAX_FUNCTIONS);
    if (gpafnAcmFunctions == NULL) {
        FreeLibrary(ghinstAcm);
        ghinstAcm = NULL;
        return FALSE;
    }

    gpafnAcmFunctions[0] = (FARPROC)pfnAcmGetVersion;
    wVersion = HIWORD(pfnAcmGetVersion());
    if (wVersion >= 0x0200) {
        for (u = 1; u < ACMTHUNK_MAX_FUNCTIONS; u++)
            gpafnAcmFunctions[u] = GetProcAddress(ghinstAcm, gapszAcmFunctions[u]);
    }

    return TRUE;
}

BOOL WINAPI acmThunkTerminate(void)
{
    if (gpafnAcmFunctions != NULL) {
        LocalFree((HLOCAL)gpafnAcmFunctions);
        gpafnAcmFunctions = NULL;
        FreeLibrary(ghinstAcm);
        ghinstAcm = (HMODULE)-1;
    }
    return TRUE;
}

DWORD ACMAPI acmGetVersion(void)
{
    DWORD (ACMAPI *pfnAcmGetVersion)(void);

    if (gpafnAcmFunctions == NULL)
        return 0;

    pfnAcmGetVersion = (DWORD (ACMAPI *)(void))gpafnAcmFunctions[0];
    if (pfnAcmGetVersion == NULL)
        return 0;

    return pfnAcmGetVersion();
}

MMRESULT ACMAPI acmMetrics(HACMOBJ hao, UINT uMetric, LPVOID pMetric)
{
    MMRESULT (ACMAPI *pfn)(HACMOBJ, UINT, LPVOID);

    if (gpafnAcmFunctions == NULL)
        return MMSYSERR_ERROR;

    pfn = (MMRESULT (ACMAPI *)(HACMOBJ, UINT, LPVOID))gpafnAcmFunctions[1];
    if (pfn == NULL)
        return MMSYSERR_ERROR;

    return pfn(hao, uMetric, pMetric);
}

MMRESULT ACMAPI acmDriverEnum(ACMDRIVERENUMCB fnCallback, DWORD dwInstance, DWORD fdwEnum)
{
    MMRESULT (ACMAPI *pfn)(ACMDRIVERENUMCB, DWORD, DWORD);

    if (gpafnAcmFunctions == NULL)
        return MMSYSERR_ERROR;

    pfn = (MMRESULT (ACMAPI *)(ACMDRIVERENUMCB, DWORD, DWORD))gpafnAcmFunctions[2];
    if (pfn == NULL)
        return MMSYSERR_ERROR;

    return pfn(fnCallback, dwInstance, fdwEnum);
}

MMRESULT ACMAPI acmDriverDetailsA(HACMDRIVERID hadid, LPACMDRIVERDETAILSA padd, DWORD fdwDetails)
{
    MMRESULT (ACMAPI *pfn)(HACMDRIVERID, LPACMDRIVERDETAILSA, DWORD);

    if (gpafnAcmFunctions == NULL)
        return MMSYSERR_ERROR;

    pfn = (MMRESULT (ACMAPI *)(HACMDRIVERID, LPACMDRIVERDETAILSA, DWORD))gpafnAcmFunctions[4];
    if (pfn == NULL)
        return MMSYSERR_ERROR;

    return pfn(hadid, padd, fdwDetails);
}

MMRESULT ACMAPI acmDriverOpen(LPHACMDRIVER phad, HACMDRIVERID hadid, DWORD fdwOpen)
{
    MMRESULT (ACMAPI *pfn)(LPHACMDRIVER, HACMDRIVERID, DWORD);

    if (gpafnAcmFunctions == NULL)
        return MMSYSERR_ERROR;

    pfn = (MMRESULT (ACMAPI *)(LPHACMDRIVER, HACMDRIVERID, DWORD))gpafnAcmFunctions[8];
    if (pfn == NULL)
        return MMSYSERR_ERROR;

    return pfn(phad, hadid, fdwOpen);
}

MMRESULT ACMAPI acmDriverClose(HACMDRIVER had, DWORD fdwClose)
{
    MMRESULT (ACMAPI *pfn)(HACMDRIVER, DWORD);

    if (gpafnAcmFunctions == NULL)
        return MMSYSERR_ERROR;

    pfn = (MMRESULT (ACMAPI *)(HACMDRIVER, DWORD))gpafnAcmFunctions[9];
    if (pfn == NULL)
        return MMSYSERR_ERROR;

    return pfn(had, fdwClose);
}

MMRESULT ACMAPI acmFormatSuggest(HACMDRIVER had, LPWAVEFORMATEX pwfxSrc, LPWAVEFORMATEX pwfxDst, DWORD cbwfxDst, DWORD fdwSuggest)
{
    MMRESULT (ACMAPI *pfn)(HACMDRIVER, LPWAVEFORMATEX, LPWAVEFORMATEX, DWORD, DWORD);

    if (gpafnAcmFunctions == NULL)
        return MMSYSERR_ERROR;

    pfn = (MMRESULT (ACMAPI *)(HACMDRIVER, LPWAVEFORMATEX, LPWAVEFORMATEX, DWORD, DWORD))gpafnAcmFunctions[23];
    if (pfn == NULL)
        return MMSYSERR_ERROR;

    return pfn(had, pwfxSrc, pwfxDst, cbwfxDst, fdwSuggest);
}
