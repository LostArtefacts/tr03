#include "standard.h"
#include "specific.h"

extern "C" {

extern __int64 frequency;
extern __int64 counter;

void TIME_Reset(void)
{
    QueryPerformanceCounter((LARGE_INTEGER *)&counter);
}

bool TIME_Init(void)
{
    __int64 pfq;

    if (!QueryPerformanceFrequency((LARGE_INTEGER *)&pfq))
        return false;
    frequency = pfq / 60;
    TIME_Reset();
    return true;
}

}
