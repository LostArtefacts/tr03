#include "standard.h"
#include "specific.h"
#include "winmain.h"

extern "C" {

void ShutdownGame(void);

void WinAppExit(void)
{
    ShutdownGame();
    exit(0);
}

}
