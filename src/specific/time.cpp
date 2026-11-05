#include "standard.h"
#include "specific.h"

extern "C" {

extern __int64 frequency;
extern __int64 counter;

void TIME_Reset(void)
{
    QueryPerformanceCounter((LARGE_INTEGER *)&counter);
}

}
