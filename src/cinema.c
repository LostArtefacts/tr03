#include "game.h"

extern int camera_underwater;

void DrawRooms(sint16 current_room);
void S_DrawSparks(void);
void S_FadePicture(void);
void S_OutputPolyList(void);
int S_DumpScreen(void);
int S_DumpCine(void);
void S_AnimateTextures(int nframes);

sint32 DrawPhaseCinematic(void)
{
    camera_underwater = 0;
    DrawRooms(camera.pos.room_number);
    S_DrawSparks();
    S_FadePicture();
    S_OutputPolyList();
    camera.number_frames = S_DumpScreen();
    if (S_DumpCine() == 1)
        camera.number_frames = 2;
    S_AnimateTextures(camera.number_frames);
    return camera.number_frames;
}
