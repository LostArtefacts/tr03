#include "standard.h"

extern "C" {

extern HANDLE acm_file;
extern bool acm_ready;
extern LPDIRECTSOUNDBUFFER DSBuffer;
extern DWORD acm_start_time;
extern int acm_volume;

void ACMCloseFile(void);

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

}
