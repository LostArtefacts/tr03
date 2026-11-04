#ifndef SPECIFIC_H
#define SPECIFIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../types.h"

void S_CDPlay(sint16 track, int mode);
void S_ExitSystem(char *message);
sint32 GetRandomControl(void);
void SeedRandomControl(sint32 seed);
sint32 GetRandomDraw(void);
void SeedRandomDraw(sint32 seed);
void *game_malloc(int size, int type);
int GetRenderWidth(void);
int GetRenderHeight(void);

#ifdef __cplusplus
}
#endif

#endif
