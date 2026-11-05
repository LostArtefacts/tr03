#include "standard.h"
#include "specific.h"

extern "C" sint32 rand_1;
extern "C" sint32 rand_2;

extern "C" {

extern int overlay_flag;
extern int noinput_count;
extern int GnGameMode;
extern bool GtWindowClosed;
extern sint16 Option_Music_Volume;

void InitialiseCamera(void);
sint32 ControlPhase(sint32 nframes, int demo_mode);
sint32 DrawPhaseGame(void);
void S_FadeToBlack(void);
void S_SoundStopAllSamples(void);
void S_CDStop(void);
void S_CDVolume(int volume);

int GameLoop(int demo_mode)
{
    int nframes;
    int lp;

    overlay_flag = 1;
    InitialiseCamera();
    noinput_count = 0;
    if (demo_mode)
        GnGameMode = 2;
    else
        GnGameMode = 1;
    lp = ControlPhase(1, demo_mode);
    while (!lp) {
        nframes = DrawPhaseGame();
        if (GtWindowClosed)
            lp = 0x700;
        else
            lp = ControlPhase(nframes, demo_mode);
    }
    if (lp != 1)
        S_FadeToBlack();
    GnGameMode = 0;
    S_SoundStopAllSamples();
    S_CDStop();
    if (Option_Music_Volume)
        S_CDVolume(25 * Option_Music_Volume + 5);
    return lp;
}

}

extern "C" sint32 LevelCompleteSequence(int level_number)
{
    return 0x500;
}

sint32 GetRandomControl(void)
{
    rand_1 = rand_1 * 0x41c64e6d + 0x3039;
    return (rand_1 >> 10) & 0x7fff;
}

void SeedRandomControl(sint32 seed)
{
    rand_1 = seed;
}

sint32 GetRandomDraw(void)
{
    rand_2 = rand_2 * 0x41c64e6d + 0x3039;
    return (rand_2 >> 10) & 0x7fff;
}

void SeedRandomDraw(sint32 seed)
{
    rand_2 = seed;
}

extern "C" int S_LoadGame(LPVOID data, int size, int slot)
{
    HANDLE file;
    DWORD bytes;
    int value;
    char buffer[80];

    wsprintf(buffer, "savegame.%d", slot);
    file = CreateFile(buffer, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (file != INVALID_HANDLE_VALUE) {
        ReadFile(file, buffer, 75, &bytes, 0);
        ReadFile(file, &value, sizeof(int), &bytes, 0);
        ReadFile(file, data, size, &bytes, 0);
        CloseHandle(file);
        return 1;
    }
    return 0;
}
