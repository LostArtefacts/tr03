#include "standard.h"
#include "specific.h"

extern "C" {

extern int smcr;
extern int smcg;
extern int smcb;

void S_CalculateStaticLight(sint16 adder)
{
    smcr = (adder & 0x1f) << 3;
    smcg = (adder & 0x3e0) >> 2;
    smcb = (adder & 0x7c00) >> 7;
}

}
