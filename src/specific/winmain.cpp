#include "standard.h"
#include "specific.h"
#include "winmain.h"

extern "C" {

void ShutdownGame(void);
bool DXSetCooperativeLevel(LPDIRECTDRAW2 lpDD2, HWND WindowHandle, int Flags);
extern char exit_message[128];
LRESULT CALLBACK WinAppProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void WinAppExit(void)
{
    ShutdownGame();
    exit(0);
}

int WinRegisterWindow(HINSTANCE hInstance)
{
    App.WindowClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(115));
    App.WindowClass.lpszMenuName = 0;
    App.WindowClass.lpszClassName = "Window Class";
    App.WindowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    App.WindowClass.hInstance = hInstance;
    App.WindowClass.style = CS_VREDRAW | CS_HREDRAW;
    App.WindowClass.lpfnWndProc = WinAppProc;
    App.WindowClass.cbClsExtra = 0;
    App.WindowClass.cbWndExtra = 0;
    return RegisterClass(&App.WindowClass);
}

HWND WinCreateWindow(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd;

    hWnd = CreateWindowEx(WS_EX_APPWINDOW, "Window Class", "Tomb Raider III", WS_POPUP, 0, 0, 0, 0, 0, 0, hInstance, 0);
    if (hWnd) {
        ShowWindow(hWnd, nCmdShow);
        UpdateWindow(hWnd);
    }
    return hWnd;
}

void S_ExitSystem(char *message)
{
    DXSetCooperativeLevel(App.lpDD, App.WindowHandle, DDSCL_NORMAL);
    MessageBox(App.WindowHandle, message, 0, MB_OK);
    ShutdownGame();
    strcpy(exit_message, message);
    exit(1);
}

}
