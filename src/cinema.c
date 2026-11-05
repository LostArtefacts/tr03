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

void InitialiseGenPlayer(sint16 item_num)
{
    AddActiveItem(item_num);
    items[item_num].pos.y_rot = 0;
    items[item_num].dynamic_light = 0;
}

extern int title_loaded;
extern int sound_active;
extern int GnGameMode;
extern sint16 Option_Music_Volume;
extern sint16 cinematic_level;
extern uint16 GF_Cutscene_Orientation;
extern sint16 cine_frame;
extern sint32 cutscene_track;
extern sint32 actual_current_frame;
extern sint32 framedump;

void InitCinematicRooms(void);
void InitialisePlayer1(sint16 item_num);
void S_ClearScreen(void);
sint32 S_StartSyncedAudio(sint32 track);
void S_CDStop(void);
void S_CDVolume(int volume);
void S_SoundStopAllSamples(void);
sint32 DoCinematic(sint32 nframes);

sint32 StartCinematic(sint32 level_number)
{
    sint32 old_sound, ret, nframes;

    title_loaded = 0;
    cinematic_level = (sint16)level_number;
    InitCinematicRooms();
    InitialisePlayer1(lara.item_number);
    old_sound = sound_active;
    camera.target_angle = GF_Cutscene_Orientation;
    sound_active = 0;
    cine_frame = 0;
    S_ClearScreen();
    if (!S_StartSyncedAudio(cutscene_track))
        return 1;
    S_CDVolume(255);
    actual_current_frame = 0;
    GnGameMode = 3;
    do {
        DrawPhaseCinematic();
        nframes = actual_current_frame + 2 * (4 - cine_frame);
        if (nframes < 2)
            nframes = 2;
        if (framedump)
            nframes = 2;
        ret = DoCinematic(nframes);
    } while (!ret);
    GnGameMode = 0;
    if (Option_Music_Volume)
        S_CDVolume(25 * Option_Music_Volume + 5);
    else
        S_CDVolume(0);
    S_CDStop();
    sound_active = old_sound;
    S_SoundStopAllSamples();
    level_complete = 1;
    return ret;
}

extern sint16 draw_rooms[];
extern int number_draw_rooms;

void InitCinematicRooms(void)
{
    int i;

    for (i = 0; i < number_rooms; i++) {
        if (room[i].flipped_room >= 0)
            room[room[i].flipped_room].bound_active = 1;
        room[i].flags |= 8;
    }
    number_draw_rooms = 0;
    for (i = 0; i < number_rooms; i++) {
        if (!room[i].bound_active)
            draw_rooms[number_draw_rooms++] = i;
    }
}

extern sint32 original_room;

void DrawDummyItem(ITEM_INFO *item);
void LaraControlCinematic(sint16 item_num);

void InitialisePlayer1(sint16 item_num)
{
    ITEM_INFO *item;
    OBJECT_INFO *obj;

    obj = &objects[LARA];
    obj->draw_routine = DrawDummyItem;
    obj->control = LaraControlCinematic;
    obj->shadow_size = 0;
    AddActiveItem(item_num);
    lara_item = item = &items[item_num];
    camera.pos.x = item->pos.x_pos;
    camera.pos.y = item->pos.y_pos;
    camera.pos.z = item->pos.z_pos;
    camera.target_angle = item->pos.y_rot = 0;
    original_room = camera.pos.room_number = item->room_number;
    item->goal_anim_state = 0;
    item->current_anim_state = 0;
    item->frame_number = 0;
    item->anim_number = 0;
    item->dynamic_light = 0;
    lara.hit_direction = -1;
}

int GetCinematicRoom(sint32 x, sint32 y, sint32 z);
void AnimateLara(ITEM_INFO *item);

void LaraControlCinematic(sint16 item_num)
{
    PHD_VECTOR pos;
    ITEM_INFO *item;
    sint16 room_number;

    item = &items[item_num];
    item->pos.y_rot = camera.target_angle;
    item->pos.x_pos = camera.pos.x;
    item->pos.y_pos = camera.pos.y;
    item->pos.z_pos = camera.pos.z;
    pos.x = 0;
    pos.y = 0;
    pos.z = 0;
    GetJointAbsPosition(item, &pos, 0);
    room_number = GetCinematicRoom(pos.x, pos.y, pos.z);
    if (room_number != -1 && item->room_number != room_number)
        ItemNewRoom(item_num, room_number);
    AnimateLara(item);
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

void ControlCinematicPlayer(sint16 item_num)
{
    ITEM_INFO *item;
    PHD_VECTOR pos;
    sint16 room_number;

    item = &items[item_num];
    item->pos.y_rot = camera.target_angle;
    item->pos.x_pos = camera.pos.x;
    item->pos.y_pos = camera.pos.y;
    item->pos.z_pos = camera.pos.z;
    pos.x = 0;
    pos.y = 0;
    pos.z = 0;
    GetJointAbsPosition(item, &pos, 0);
    room_number = GetCinematicRoom(pos.x, pos.y, pos.z);
    if (room_number != -1 && item->room_number != room_number)
        ItemNewRoom(item_num, room_number);
    if (item->dynamic_light && item->status != INVISIBLE) {
        pos.x = 0;
        pos.y = 0;
        pos.z = 0;
        GetJointAbsPosition(item, &pos, 0);
    }
    AnimateItem(item);
}
