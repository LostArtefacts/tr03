#ifndef GAMEFLOW_H
#define GAMEFLOW_H

#include "types.h"

enum level_types {
    IL_TITLE,
    IL_NORMAL,
    IL_SAVED,
    IL_DEMO,
    IL_CUTSCENE,
    IL_STORY_SO_FAR,
    IL_QUIET,
    IL_STORY_SO_FAR_MIDLEVEL
};

extern sint16 *GF_frontendSequence;

sint32 GF_InterpretSequence(sint16 *ptr, int type, int seq_type);

#endif
