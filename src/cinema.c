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

int GetCinematicRoom(sint32 x, sint32 y, sint32 z)
{
    int i;

    for (i = 0; i < number_rooms; i++)
    {
        if (x >= room[i].x + WALL_L && x < room[i].x + (room[i].y_size - 1) * WALL_L && y >= room[i].maxceiling && y <= room[i].minfloor && z >= room[i].z + WALL_L && z < room[i].z + (room[i].x_size - 1) * WALL_L)
            return i;
    }
    return -1;
}
