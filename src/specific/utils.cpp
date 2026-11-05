#include "standard.h"
#include "specific.h"

extern "C" {

extern char *G_lpCmdLine;

char *UT_FindArg(char *pArg)
{
    char *p;

    p = strstr(G_lpCmdLine, pArg);
    if (!p)
        return 0;
    return p + strlen(pArg);
}

}
