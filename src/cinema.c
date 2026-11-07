#include "specific/standard.h"
#include "game.h"

extern int camera_underwater;

void DrawRooms(sint16);
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

extern sint32 cine_framecount;
extern sint32 cine_delay;
extern sint16 next_item_active;
extern sint16 num_cine_frames;

int S_UpdateInput(void);
void UpdateLaraGuns(void);
void UpdateSparks(void);
void HairControl(int in_cutscene);
void CalculateCinematicCamera(void);
sint32 S_CDGetLoc(void);

sint32 DoCinematic(sint32 nframes)
{
    sint16 item_num, nex;

    for (cine_framecount += cine_delay * nframes; cine_framecount >= 0; cine_framecount -= 0x10000) {
        if (S_UpdateInput())
            return 3;
        if (input & IN_ACTION)
            return 1;
        if (input & IN_OPTION)
            return 2;
        ClearDynamics();
        item_num = next_item_active;
        while (item_num != NO_ITEM) {
            nex = items[item_num].next_active;
            if (objects[items[item_num].object_number].control)
                (*objects[items[item_num].object_number].control)(item_num);
            item_num = nex;
        }
        item_num = next_fx_active;
        while (item_num != NO_ITEM) {
            nex = effects[item_num].next_active;
            if (objects[effects[item_num].object_number].control)
                (*objects[effects[item_num].object_number].control)(item_num);
            item_num = nex;
        }
        UpdateLaraGuns();
        UpdateSparks();
        HairControl(1);
        CalculateCinematicCamera();
        cine_frame++;
        if (cine_frame >= num_cine_frames)
            return 1;
    }
    actual_current_frame = S_CDGetLoc();
    return 0;
}

extern sint32 SmokeCountL, SmokeCountR;

sint16 CreateEffect(sint16 room_num);
void TriggerDynamic(sint32 x, sint32 y, sint32 z, sint32 falloff, sint32 r, sint32 g, sint32 b);
void TriggerGunSmoke(sint32 x, sint32 y, sint32 z, sint32 xv, sint32 yv, sint32 zv, sint32 initial, sint32 weapon, sint32 count);

void UpdateLaraGuns(void)
{
    uint16 flag, lr = 0;
    PHD_VECTOR posl, posr, *pos = &posl;

    if (lara.left_arm.flash_gun || lara.right_arm.flash_gun || SmokeCountL || SmokeCountR) {
        posl.x = -12;
        posl.y = (128 >> 2) + 16;
        posl.z = 40;
        GetJointAbsPosition(lara_item, &posl, HAND_L);

        posr.x = 8;
        posr.y = (128 >> 2) + 16;
        posr.z = 40;
        GetJointAbsPosition(lara_item, &posr, HAND_R);
    } else {
        return;
    }

    if ((flag = lara.left_arm.flash_gun)) {
        lara.left_arm.flash_gun--;
    } else if ((flag = lara.right_arm.flash_gun)) {
        lr = 1;
        pos = &posr;
        lara.right_arm.flash_gun--;
    }

    if (flag) {
        if (flag == 3) {
            sint16 fx_number;
            sint16 room_number;

            room_number = GetCinematicRoom(pos->x, pos->y, pos->z);

            if ((fx_number = CreateEffect(room_number)) != NO_ITEM) {
                FX_INFO *fx;

                fx = &effects[fx_number];
                fx->pos.x_pos = pos->x;
                fx->pos.y_pos = pos->y;
                fx->pos.z_pos = pos->z;
                fx->room_number = room_number;
                fx->pos.y_rot = 0;
                fx->pos.x_rot = 0;
                fx->pos.z_rot = GetRandomControl();
                fx->speed = 16 + (GetRandomControl() & 31);
                fx->fallspeed = -48 - (GetRandomControl() & 7);
                fx->object_number = 366;
                fx->frame_number = objects[366].mesh_index;
                fx->shade = 0x4210;
                fx->counter = (GetRandomControl() & 1) + 1;
            }

            if (lr)
                SmokeCountR = 28;
            else
                SmokeCountL = 28;
        }

        TriggerDynamic(pos->x + ((GetRandomControl() & 255) - 128), pos->y - ((GetRandomControl() & 127) - 63),
                       pos->z + ((GetRandomControl() & 255) - 128), 10, (GetRandomControl() & 7) + 24,
                       (GetRandomControl() & 3) + 16, GetRandomControl() & 7);
    }

    if (SmokeCountL) {
        TriggerGunSmoke(posl.x, posl.y, posl.z, 0, 0, 0, 0, LG_PISTOLS, SmokeCountL);
        SmokeCountL--;
    }

    if (SmokeCountR) {
        TriggerGunSmoke(posr.x, posr.y, posr.z, 0, 0, 0, 0, LG_PISTOLS, SmokeCountR);
        SmokeCountR--;
    }
}

extern sint16 *cine;

void phd_LookAt(sint32 xsrc, sint32 ysrc, sint32 zsrc, sint32 xtar, sint32 ytar, sint32 ztar, PHD_ANGLE roll);

void CalculateCinematicCamera(void)
{
    sint32 tx, ty, tz;
    sint32 cx, cy, cz;
    sint32 c, s;
    sint16 roll, fov;
    sint16 *ptr;
    PHD_VECTOR campos, camtar;

    int room_number;

    ptr = cine + (int)(8 * cine_frame);
    tx = (sint32) * (ptr + 0);
    ty = (sint32) * (ptr + 1);
    tz = (sint32) * (ptr + 2);
    cx = (sint32) * (ptr + 3);
    cy = (sint32) * (ptr + 4);
    cz = (sint32) * (ptr + 5);
    fov = *(ptr + 6);
    roll = *(ptr + 7);

    c = phd_cos(camera.target_angle);
    s = phd_sin(camera.target_angle);

    camtar.x = lara_item->pos.x_pos + ((tx * c + tz * s) >> W2V_SHIFT);
    camtar.y = lara_item->pos.y_pos + ty;
    camtar.z = lara_item->pos.z_pos + ((tz * c - tx * s) >> W2V_SHIFT);
    campos.x = lara_item->pos.x_pos + ((cx * c + cz * s) >> W2V_SHIFT);
    campos.y = lara_item->pos.y_pos + cy;
    campos.z = lara_item->pos.z_pos + ((cz * c - cx * s) >> W2V_SHIFT);

    if ((room_number = GetCinematicRoom(campos.x, campos.y, campos.z)) >= 0)
        camera.pos.room_number = room_number;

    AlterFOV(fov);
    phd_LookAt(campos.x, campos.y, campos.z, camtar.x, camtar.y, camtar.z, roll);
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

extern sint16 *cine;
extern PHD_3DPOS cinematic_pos;
extern sint32 phd_persp;

void phd_LookAt(sint32 xsrc, sint32 ysrc, sint32 zsrc, sint32 xtar, sint32 ytar, sint32 ztar, PHD_ANGLE roll);

void InGameCinematicCamera(void)
{
    sint32 c, s;
    sint32 tx, ty, tz;
    sint32 cx, cy, cz;
    sint16 roll, fov;
    sint16 *ptr;

    cine_frame++;
    if (cine_frame >= num_cine_frames) {
        cine_frame = num_cine_frames - 1;
    }

    ptr = cine + (int)(8 * cine_frame);
    tx = (sint32)*(ptr + 0);
    ty = (sint32)*(ptr + 1);
    tz = (sint32)*(ptr + 2);
    cx = (sint32)*(ptr + 3);
    cy = (sint32)*(ptr + 4);
    cz = (sint32)*(ptr + 5);
    fov = *(ptr + 6);
    roll = *(ptr + 7);

    c = phd_cos(cinematic_pos.y_rot);
    s = phd_sin(cinematic_pos.y_rot);
    camera.target.x = cinematic_pos.x_pos + ((c * tx + s * tz) >> W2V_SHIFT);
    camera.target.y = cinematic_pos.y_pos + ty;
    camera.target.z = cinematic_pos.z_pos + ((c * tz - s * tx) >> W2V_SHIFT);
    camera.pos.x = cinematic_pos.x_pos + ((c * cx + s * cz) >> W2V_SHIFT);
    camera.pos.y = cinematic_pos.y_pos + cy;
    camera.pos.z = cinematic_pos.z_pos + ((c * cz - s * cx) >> W2V_SHIFT);

    AlterFOV(fov);
    phd_LookAt(camera.pos.x, camera.pos.y, camera.pos.z, camera.target.x, camera.target.y, camera.target.z, roll);
    GetFloor(camera.pos.x, camera.pos.y, camera.pos.z, &camera.pos.room_number);

    if (camera.mike_at_lara) {
        camera.actual_angle = lara_item->pos.y_rot + lara.head_y_rot + lara.torso_y_rot;
        camera.mike_pos.x = lara_item->pos.x_pos;
        camera.mike_pos.y = lara_item->pos.y_pos;
        camera.mike_pos.z = lara_item->pos.z_pos;
    } else {
        camera.actual_angle = phd_atan(camera.target.z - camera.pos.z, camera.target.x - camera.pos.x);
        camera.mike_pos.x = camera.pos.x + ((phd_persp * phd_sin(camera.actual_angle)) >> W2V_SHIFT);
        camera.mike_pos.z = camera.pos.z + ((phd_persp * phd_cos(camera.actual_angle)) >> W2V_SHIFT);
        camera.mike_pos.y = camera.pos.y;
    }
}

void SetCutsceneTrack(int track)
{
    cutscene_track = track;
}
