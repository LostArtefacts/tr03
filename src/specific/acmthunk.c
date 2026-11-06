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
