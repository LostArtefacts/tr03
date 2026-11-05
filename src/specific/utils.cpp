#include "standard.h"
#include "specific.h"

extern "C" {

extern char *G_lpCmdLine;

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

}
