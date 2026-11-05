#include "standard.h"
#include "specific.h"
#include "winmain.h"

extern "C" {

extern char *G_lpCmdLine;

extern double period;
extern double start_us;
extern DWORD start;

double UT_GetAccurateTimer(void);

void UT_InitAccurateTimer(void)
{
    LARGE_INTEGER fq;

    if (QueryPerformanceFrequency(&fq)) {
        period = 1.0 / (double)fq.QuadPart;
        start_us = 0;
        start_us = UT_GetAccurateTimer();
    } else {
        period = 0;
        start = timeGetTime();
    }
}

double UT_GetAccurateTimer(void)
{
    LARGE_INTEGER counter;

    if (QueryPerformanceCounter(&counter))
        return ((double)counter.QuadPart - start_us) * period;
    return (double)(timeGetTime() - start) / 1000.0;
}

void UT_CenterWindow(HWND hWnd)
{
    RECT rArea;
    RECT rWindow;

    SystemParametersInfo(SPI_GETWORKAREA, 0, &rArea, 0);
    GetWindowRect(hWnd, &rWindow);
    SetWindowPos(hWnd, 0, ((rArea.right + rArea.left) >> 1) - ((rWindow.right - rWindow.left) >> 1), ((rArea.bottom + rArea.top) >> 1) - ((rWindow.bottom - rWindow.top) >> 1), -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

char *UT_FindArg(char *pArg)
{
    char *p;

    p = strstr(G_lpCmdLine, pArg);
    if (!p)
        return 0;
    return p + strlen(pArg);
}

BOOL CALLBACK UT_OKCB_DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool UT_OKCancelBox(char *szDlgResource, HWND hWndOwner)
{
    bool ret;

    ShowCursor(1);
    ret = DialogBox(App.hInstance, szDlgResource, hWndOwner, UT_OKCB_DlgProc) != 0;
    ShowCursor(0);
    return ret;
}

BOOL CALLBACK UT_OKCB_DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_INITDIALOG:
        UT_CenterWindow(hWnd);
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            EndDialog(hWnd, TRUE);
            return TRUE;
        case IDCANCEL:
            EndDialog(hWnd, FALSE);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

}
