#include "standard.h"
#include "specific.h"
#include "winmain.h"

extern "C" {

void ShutdownGame(void);
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

}
