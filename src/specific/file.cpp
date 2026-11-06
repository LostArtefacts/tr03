#include "standard.h"
#include "specific.h"
#include "winmain.h"

extern "C" {

#include "../types.h"
#include "../camera.h"
#include "../anim.h"
#include "../room.h"
#include "../gameflow.h"

extern char LastLoadedLevelPath[256];
extern unsigned char *texture_page_ptrs[32];
extern int nTInfos;
extern DWORD nBytesRead;

extern int number_cameras;
extern int number_sound_effects;
extern OBJECT_VECTOR *sound_effects;
extern sint16 num_cine_frames;
extern sint16 *cine;
extern sint16 cine_loaded;
extern int democount;
extern unsigned long *demoptr;
extern int demo_loaded;
extern unsigned char TexturesUVFlag[];
extern unsigned char game_palette[768];
extern unsigned char G_GouraudPalette[1024];
extern int BlackGouraudIndex;
extern bool bFixSkyColour;

typedef struct phdtexturestruct {
    unsigned short drawtype;
    unsigned short tpage;
    unsigned short u1;
    unsigned short v1;
    unsigned short u2;
    unsigned short v2;
    unsigned short u3;
    unsigned short v3;
    unsigned short u4;
    unsigned short v4;
} PHDTEXTURESTRUCT;

extern PHDTEXTURESTRUCT phdtextinfo[];

typedef struct phdspritestruct {
    unsigned short tpage;
    unsigned short offset;
    unsigned short width;
    unsigned short height;
    sint16 x1;
    sint16 y1;
    sint16 x2;
    sint16 y2;
} PHDSPRITESTRUCT;

extern PHDSPRITESTRUCT phdspriteinfo[];

typedef struct {
    sint16 mesh_number;
    sint16 flags;
    sint16 x_minp;
    sint16 x_maxp;
    sint16 y_minp;
    sint16 y_maxp;
    sint16 z_minp;
    sint16 z_maxp;
    sint16 x_minc;
    sint16 x_maxc;
    sint16 y_minc;
    sint16 y_maxc;
    sint16 z_minc;
    sint16 z_maxc;
} STATIC_INFO;

extern STATIC_INFO static_objects[];
extern ITEM_INFO *items;
extern int level_items;
extern char exit_message[128];
void InitialiseItemArray(int numitems);
void InitialiseItem(sint16 item_num);
extern char depthq_table[33][256];
extern char gouraud_table[256][32];
extern unsigned char water_palette[768];
extern int wet;
extern BOX_INFO *boxes;
extern int number_boxes;
extern uint16 *overlap;
extern sint16 *ground_zone[4][2];
extern sint16 *fly_zone[2];
extern sint16 *aranges;

typedef struct {
    sint16 number;
    uint8 volume;
    sint8 radius;
    sint8 randomness;
    sint8 pitch;
    sint16 flags;
} SAMPLE_INFO;

extern int sound_active;
extern sint16 sample_lut[370];
extern int num_sample_infos;
extern SAMPLE_INFO *sample_infos;
bool DS_IsSoundEnabled(void);
void DS_FreeAllSamples(void);
bool DS_MakeSample(int nSample, LPWAVEFORMATEX pWF, LPVOID pData, DWORD dwLen);
void game_free(int size, int type);
const char *GetFullPath(const char *name);
extern int GnTexturePages;
void HWR_LoadTexturePages(int nPages, unsigned char *src, unsigned char *palette);
extern sint16 number_rooms;
extern ROOM_INFO *room;
extern sint16 *floor_data;
extern sint16 phd_winxmax;
extern sint16 phd_winymax;
extern sint32 wibble;
extern uint8 CurrentAtmosphere;
void init_water_table(void);
void CalculateWibbleTable(void);
void BuildOutsideTable(void);

typedef struct {
    sint16 goal_anim_state;
    sint16 number_ranges;
    sint16 range_index;
} CHANGE_STRUCT;

typedef struct {
    sint16 start_frame;
    sint16 end_frame;
    sint16 link_anim_num;
    sint16 link_frame_num;
} RANGE_STRUCT;

extern sint16 *mesh_base;
extern CHANGE_STRUCT *changes;
extern RANGE_STRUCT *ranges;
extern sint16 *commands;
extern sint32 *bones;
extern sint16 *frames;
void InitialiseObjects(void);
extern sint16 GF_Offsets[200];
extern DWORD LdwPaletteOffset;
extern DWORD LdwTextureOffset;
extern DWORD LdwDepthQOffset;
void init_game_malloc(void);

extern char cd_drive;
extern char **GF_picfilenames;
extern sint16 GF_LoadingPic;
extern char GF_Playing_Story;

void S_UnloadLevelFile(void);
void build_ext(char *fname, char *ext);
void S_CDStop(void);
bool LoadPicture(const char *name, LPDIRECTDRAWSURFACE3 surf, int a);
void FadePictureUp(int steps);
void FadePictureDown(int steps);
int LoadLevel(char *name, int number);
bool DD_SpinMessageLoop(bool tWait);

void HWR_FreeTexturePages(void);

BOOL MyReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
    nBytesRead += nNumberOfBytesToRead;
    if (nBytesRead > 0x4000) {
        nBytesRead = 0;
        DD_SpinMessageLoop(false);
    }
    return ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
}

int LoadTexturePages(HANDLE file)
{
    DWORD read;
    char *base;
    int i;
    int num_pages;
    bool t16BitTextures;
    int nBytesPerPage;

    MyReadFile(file, &num_pages, sizeof(int), &read, NULL);
    if (App.nRenderMode == 0) {
        for (i = 0; i < num_pages; i++) {
            if (texture_page_ptrs[i] == NULL)
                texture_page_ptrs[i] = (unsigned char *)game_malloc(65536, 1);
            MyReadFile(file, texture_page_ptrs[i], 65536, &read, NULL);
        }
        SetFilePointer(file, num_pages * 131072, NULL, FILE_CURRENT);
        return 1;
    }
    if (App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].Texture[App.DXConfigPtr->D3DTF].bPalette == false)
        t16BitTextures = true;
    else
        t16BitTextures = false;
    nBytesPerPage = t16BitTextures ? 0x20000 : 0x10000;
    base = (char *)GlobalAlloc(GMEM_FIXED, num_pages * nBytesPerPage);
    if (base == NULL)
        return 0;
    if (t16BitTextures) {
        SetFilePointer(file, num_pages * 65536, NULL, FILE_CURRENT);
        for (i = 0; i < num_pages; i++)
            MyReadFile(file, base + i * nBytesPerPage, nBytesPerPage, &read, NULL);
        HWR_LoadTexturePages(num_pages, (unsigned char *)base, NULL);
    } else {
        for (i = 0; i < num_pages; i++)
            MyReadFile(file, base + i * nBytesPerPage, nBytesPerPage, &read, NULL);
        SetFilePointer(file, num_pages * 131072, NULL, FILE_CURRENT);
        HWR_LoadTexturePages(num_pages, (unsigned char *)base, game_palette);
    }
    GnTexturePages = num_pages;
    GlobalFree((HGLOBAL)base);
    return 1;
}

int LoadRooms(HANDLE file)
{
    int i;
    sint16 ndoors;
    sint32 size;
    ROOM_INFO *r;
    DWORD read;

    SeedRandomDraw(0xd371f947);
    SeedRandomControl(0xd371f947);
    wibble = 0;
    CurrentAtmosphere = 0;
    init_water_table();
    CalculateWibbleTable();
    MyReadFile(file, &number_rooms, sizeof(number_rooms), &read, NULL);
    if (number_rooms < 0 || number_rooms > 1024) {
        lstrcpy(exit_message, "LoadRoom(): Too many rooms");
        return 0;
    }
    room = (ROOM_INFO *)game_malloc(number_rooms * sizeof(ROOM_INFO), 11);
    if (room == NULL) {
        lstrcpy(exit_message, "LoadRoom(): Could not allocate memory for rooms");
        return 0;
    }
    for (i = 0, r = room; i < number_rooms; i++, r++) {
        MyReadFile(file, &r->x, sizeof(sint32), &read, NULL);
        r->y = 0;
        MyReadFile(file, &r->z, sizeof(sint32), &read, NULL);
        MyReadFile(file, &r->minfloor, sizeof(sint32), &read, NULL);
        MyReadFile(file, &r->maxceiling, sizeof(sint32), &read, NULL);
        MyReadFile(file, &size, sizeof(sint32), &read, NULL);
        r->data = (sint16 *)game_malloc(size * sizeof(sint16), 12);
        MyReadFile(file, r->data, sizeof(sint16) * size, &read, NULL);
        MyReadFile(file, &ndoors, sizeof(sint16), &read, NULL);
        if (ndoors) {
            r->door = (sint16 *)game_malloc((ndoors * 16 + 1) * sizeof(sint16), 13);
            r->door[0] = ndoors;
            MyReadFile(file, &r->door[1], sizeof(sint16) * ndoors * 16, &read, NULL);
        } else {
            r->door = NULL;
        }
        MyReadFile(file, &r->x_size, sizeof(sint16), &read, NULL);
        MyReadFile(file, &r->y_size, sizeof(sint16), &read, NULL);
        size = (int)r->x_size * r->y_size;
        r->floor = (FLOOR_INFO *)game_malloc(size * sizeof(FLOOR_INFO), 14);
        MyReadFile(file, r->floor, sizeof(FLOOR_INFO) * size, &read, NULL);
        MyReadFile(file, &r->ambient, sizeof(sint16), &read, NULL);
        MyReadFile(file, &r->lighting, sizeof(sint16), &read, NULL);
        MyReadFile(file, &r->num_lights, sizeof(sint16), &read, NULL);
        if (r->num_lights) {
            r->light = (LIGHT_INFO *)game_malloc(r->num_lights * sizeof(LIGHT_INFO), 15);
            MyReadFile(file, r->light, sizeof(LIGHT_INFO) * r->num_lights, &read, NULL);
        } else {
            r->light = NULL;
        }
        MyReadFile(file, &r->num_meshes, sizeof(sint16), &read, NULL);
        if (r->num_meshes) {
            r->mesh = (MESH_INFO *)game_malloc(r->num_meshes * sizeof(MESH_INFO), 16);
            MyReadFile(file, r->mesh, sizeof(MESH_INFO) * r->num_meshes, &read, NULL);
        } else {
            r->mesh = NULL;
        }
        MyReadFile(file, &r->flipped_room, sizeof(sint16), &read, NULL);
        MyReadFile(file, &r->flags, sizeof(uint16), &read, NULL);
        MyReadFile(file, &r->MeshEffect, sizeof(char), &read, NULL);
        MyReadFile(file, &r->ReverbType, sizeof(sint16), &read, NULL);
        r->bound_active = 0;
        r->left = phd_winxmax;
        r->top = phd_winymax;
        r->right = r->bottom = 0;
        r->item_number = -1;
        r->fx_number = -1;
    }
    BuildOutsideTable();
    MyReadFile(file, &size, sizeof(sint32), &read, NULL);
    floor_data = (sint16 *)game_malloc(size * sizeof(sint16), 17);
    MyReadFile(file, floor_data, sizeof(sint16) * size, &read, NULL);
    return 1;
}

void AdjustTextureUVs(bool reset)
{
    sint16 *uv;
    int num;
    unsigned char flag;
    unsigned int i;
    unsigned char *pFlag;
    int j;

    if (reset)
        App.nUVAdd = 0;
    num = 256 - App.nUVAdd;
    pFlag = TexturesUVFlag;
    for (i = 0; i != nTInfos; i++) {
        uv = (sint16 *)&phdtextinfo[i].u1;
        flag = *pFlag++;
        for (j = 0; j < 8; j++) {
            if (flag & 1)
                uv[j] -= (sint16)num;
            else
                uv[j] += (sint16)num;
            flag >>= 1;
        }
    }
    App.nUVAdd += num;
}

int LoadObjects(HANDLE file)
{
    int i;
    int j;
    sint32 number;
    sint32 size;
    sint32 number_anims;
    DWORD read;

    MyReadFile(file, &size, sizeof(sint32), &read, NULL);
    mesh_base = (sint16 *)game_malloc(size * sizeof(sint16), 3);
    MyReadFile(file, mesh_base, sizeof(sint16) * size, &read, NULL);
    MyReadFile(file, &number, sizeof(sint32), &read, NULL);
    meshes = (sint16 **)game_malloc(number * sizeof(sint16 *), 2);
    MyReadFile(file, meshes, sizeof(sint16 *) * number, &read, NULL);
    for (i = 0; i < number; i++)
        meshes[i] = mesh_base + (int)meshes[i] / 2;
    MyReadFile(file, &number_anims, sizeof(sint32), &read, NULL);
    anims = (ANIM_STRUCT *)game_malloc(number_anims * sizeof(ANIM_STRUCT), 4);
    MyReadFile(file, anims, sizeof(ANIM_STRUCT) * number_anims, &read, NULL);
    MyReadFile(file, &number, sizeof(sint32), &read, NULL);
    changes = (CHANGE_STRUCT *)game_malloc(number * sizeof(CHANGE_STRUCT), 5);
    MyReadFile(file, changes, sizeof(CHANGE_STRUCT) * number, &read, NULL);
    MyReadFile(file, &number, sizeof(sint32), &read, NULL);
    ranges = (RANGE_STRUCT *)game_malloc(number * sizeof(RANGE_STRUCT), 6);
    MyReadFile(file, ranges, sizeof(RANGE_STRUCT) * number, &read, NULL);
    MyReadFile(file, &size, sizeof(sint32), &read, NULL);
    commands = (sint16 *)game_malloc(size * sizeof(sint16), 7);
    MyReadFile(file, commands, sizeof(sint16) * size, &read, NULL);
    MyReadFile(file, &size, sizeof(sint32), &read, NULL);
    bones = (sint32 *)game_malloc(size * sizeof(sint32), 8);
    MyReadFile(file, bones, sizeof(sint32) * size, &read, NULL);
    MyReadFile(file, &size, sizeof(sint32), &read, NULL);
    frames = (sint16 *)game_malloc(size * sizeof(sint16), 9);
    MyReadFile(file, frames, sizeof(sint16) * size, &read, NULL);
    for (i = 0; i < number_anims; i++)
        anims[i].frame_ptr = (sint16 *)((long)anims[i].frame_ptr + (long)frames);
    MyReadFile(file, &number, sizeof(sint32), &read, NULL);
    for (i = 0; i < number; i++) {
        MyReadFile(file, &j, sizeof(sint32), &read, NULL);
        MyReadFile(file, &objects[j].nmeshes, sizeof(sint16), &read, NULL);
        MyReadFile(file, &objects[j].mesh_index, sizeof(sint16), &read, NULL);
        MyReadFile(file, &objects[j].bone_index, sizeof(sint32), &read, NULL);
        MyReadFile(file, &size, sizeof(sint32), &read, NULL);
        objects[j].frame_base = (sint16 *)((long)size + (long)frames);
        MyReadFile(file, &objects[j].anim_index, sizeof(sint16), &read, NULL);
        objects[j].loaded = 1;
    }
    InitialiseObjects();
    MyReadFile(file, &number, sizeof(sint32), &read, NULL);
    for (i = 0; i < number; i++) {
        MyReadFile(file, &j, sizeof(sint32), &read, NULL);
        MyReadFile(file, &static_objects[j].mesh_number, sizeof(sint16), &read, NULL);
        MyReadFile(file, &static_objects[j].x_minp, sizeof(sint16) * 6, &read, NULL);
        MyReadFile(file, &static_objects[j].x_minc, sizeof(sint16) * 6, &read, NULL);
        MyReadFile(file, &static_objects[j].flags, sizeof(sint16), &read, NULL);
    }
    return 1;
}

int LoadSprites(HANDLE file)
{
    int i;
    int j;
    int number;
    DWORD read;

    MyReadFile(file, &number, sizeof(sint32), &read, NULL);
    MyReadFile(file, phdspriteinfo, sizeof(PHDSPRITESTRUCT) * number, &read, NULL);
    MyReadFile(file, &number, sizeof(sint32), &read, NULL);
    for (i = 0; i < number; i++) {
        MyReadFile(file, &j, sizeof(sint32), &read, NULL);
        if (j < 376) {
            MyReadFile(file, &objects[j].nmeshes, sizeof(sint16), &read, NULL);
            MyReadFile(file, &objects[j].mesh_index, sizeof(sint16), &read, NULL);
            objects[j].loaded = 1;
        } else {
            j -= 376;
            SetFilePointer(file, sizeof(sint16), NULL, FILE_CURRENT);
            MyReadFile(file, &static_objects[j].mesh_number, sizeof(sint16), &read, NULL);
        }
    }
    return 1;
}

int LoadItems(HANDLE file)
{
    sint16 i;
    sint32 number_items;
    ITEM_INFO *item;
    DWORD read;

    MyReadFile(file, &number_items, sizeof(sint32), &read, NULL);
    if (number_items == 0)
        return 1;
    if (number_items > 256) {
        lstrcpy(exit_message, "LoadItems(): Too Many Items being Loaded!!");
        return 0;
    }
    items = (ITEM_INFO *)game_malloc(sizeof(ITEM_INFO) * 256, 18);
    if (items == NULL) {
        lstrcpy(exit_message, "LoadItems(): Unable to allocate memory for 'items'");
        return 0;
    }
    level_items = number_items;
    InitialiseItemArray(256);
    for (i = 0; i < number_items; i++) {
        item = &items[i];
        MyReadFile(file, &item->object_number, sizeof(sint16), &read, NULL);
        MyReadFile(file, &item->room_number, sizeof(sint16), &read, NULL);
        MyReadFile(file, &item->pos.x_pos, sizeof(sint32), &read, NULL);
        MyReadFile(file, &item->pos.y_pos, sizeof(sint32), &read, NULL);
        MyReadFile(file, &item->pos.z_pos, sizeof(sint32), &read, NULL);
        MyReadFile(file, &item->pos.y_rot, sizeof(sint16), &read, NULL);
        MyReadFile(file, &item->shade, sizeof(sint16), &read, NULL);
        MyReadFile(file, &item->shadeB, sizeof(sint16), &read, NULL);
        MyReadFile(file, &item->flags, sizeof(sint16), &read, NULL);
        if (item->object_number < 0 || item->object_number >= 376) {
            wsprintf(exit_message, "LoadItems(): Bad Object number (%d) on Item %d", item->object_number, i);
            return 0;
        }
    }
    for (i = 0; i < number_items; i++)
        InitialiseItem(i);
    return 1;
}

int LoadDepthQ(HANDLE file)
{
    int i;
    int j;
    DWORD read;

    MyReadFile(file, depthq_table, 32 * 256, &read, NULL);
    for (i = 0; i < 33; i++)
        depthq_table[i][0] = 0;
    memcpy(depthq_table[32], depthq_table[24], 256);
    for (i = 0; i < 32; i++) {
        for (j = 0; j < 256; j++)
            gouraud_table[j][i] = depthq_table[i][j];
    }
    wet = 0;
    for (i = 0; i < 256 * 3; i += 3) {
        water_palette[i] = (int)(game_palette[i] * 2) / 3;
        water_palette[i + 1] = (int)(game_palette[i + 1] * 2) / 3;
        water_palette[i + 2] = game_palette[i + 2];
    }
    return 1;
}

int LoadPalette(HANDLE file)
{
    DWORD read;
    int i;

    MyReadFile(file, game_palette, sizeof(game_palette), &read, 0);
    game_palette[2] = 0;
    game_palette[1] = 0;
    game_palette[0] = 0;
    for (i = 3; i < 768; i++)
        game_palette[i] <<= 2;
    MyReadFile(file, G_GouraudPalette, sizeof(G_GouraudPalette), &read, 0);
    for (i = 0; i < 256; i++) {
        if (!G_GouraudPalette[i * 4] && !G_GouraudPalette[i * 4 + 1] && !G_GouraudPalette[i * 4 + 2]) {
            BlackGouraudIndex = i;
            break;
        }
    }
    bFixSkyColour = true;
    return 1;
}

int LoadCameras(HANDLE file)
{
    DWORD read;

    MyReadFile(file, &number_cameras, sizeof(int), &read, 0);
    if (number_cameras) {
        camera.fixed = (OBJECT_VECTOR *)game_malloc(sizeof(OBJECT_VECTOR) * number_cameras, 19);
        if (!camera.fixed)
            return 0;
        MyReadFile(file, camera.fixed, sizeof(OBJECT_VECTOR) * number_cameras, &read, 0);
    }
    return 1;
}

int LoadSoundEffects(HANDLE file)
{
    DWORD read;

    MyReadFile(file, &number_sound_effects, sizeof(int), &read, 0);
    if (number_sound_effects) {
        sound_effects = (OBJECT_VECTOR *)game_malloc(sizeof(OBJECT_VECTOR) * number_sound_effects, 20);
        if (!sound_effects)
            return 0;
        MyReadFile(file, sound_effects, sizeof(OBJECT_VECTOR) * number_sound_effects, &read, 0);
    }
    return 1;
}

int LoadBoxes(HANDLE file)
{
    sint32 i;
    sint32 j;
    sint32 number_overlaps;
    DWORD read;

    MyReadFile(file, &number_boxes, sizeof(sint32), &read, NULL);
    boxes = (BOX_INFO *)game_malloc(sizeof(BOX_INFO) * number_boxes, 21);
    MyReadFile(file, boxes, sizeof(BOX_INFO) * number_boxes, &read, NULL);
    if (read != sizeof(BOX_INFO) * number_boxes) {
        lstrcpy(exit_message, "LoadBoxes(): Unable to load boxes");
        return 0;
    }
    MyReadFile(file, &number_overlaps, sizeof(sint32), &read, NULL);
    overlap = (uint16 *)game_malloc(sizeof(uint16) * number_overlaps, 22);
    MyReadFile(file, overlap, sizeof(uint16) * number_overlaps, &read, NULL);
    if (read != sizeof(uint16) * number_overlaps) {
        lstrcpy(exit_message, "LoadBoxes(): Unable to load box overlaps");
        return 0;
    }
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 4; j++) {
            ground_zone[j][i] = (sint16 *)game_malloc(sizeof(sint16) * number_boxes, 23);
            MyReadFile(file, ground_zone[j][i], sizeof(sint16) * number_boxes, &read, NULL);
            if (read != sizeof(sint16) * number_boxes) {
                lstrcpy(exit_message, "LoadBoxes(): Unable to load 'ground_zone'");
                return 0;
            }
        }
        fly_zone[i] = (sint16 *)game_malloc(sizeof(sint16) * number_boxes, 24);
        MyReadFile(file, fly_zone[i], sizeof(sint16) * number_boxes, &read, NULL);
        if (read != sizeof(sint16) * number_boxes) {
            lstrcpy(exit_message, "LoadBoxes(): Unable to load 'fly_zone'");
            return 0;
        }
    }
    for (i = 0; i < number_boxes; i++) {
        if (boxes[i].overlap_index & 0x8000)
            boxes[i].overlap_index |= 0x4000;
    }
    return 1;
}

int LoadAnimatedTextures(HANDLE file)
{
    int size;
    DWORD read;
    sint32 number;
    uint16 *pUV;
    uint8 bByte;
    int nBit;

    MyReadFile(file, &size, sizeof(sint32), &read, NULL);
    aranges = (sint16 *)game_malloc(size * sizeof(sint16), 25);
    MyReadFile(file, aranges, sizeof(sint16) * size, &read, NULL);
    MyReadFile(file, &number, sizeof(sint32), &read, NULL);
    if (number > 4096) {
        lstrcpy(exit_message, "Too many Textures in level");
        return 0;
    }
    MyReadFile(file, phdtextinfo, sizeof(PHDTEXTURESTRUCT) * number, &read, NULL);
    nTInfos = number;
    for (number = 0; number < nTInfos; number++) {
        pUV = &phdtextinfo[number].u1;
        bByte = 0;
        for (nBit = 0; nBit < 8; nBit++) {
            if (*pUV & 0x80) {
                *pUV |= 0xff;
                bByte |= 1 << nBit;
            } else {
                *pUV &= 0xff00;
            }
            pUV++;
        }
        TexturesUVFlag[number] = bByte;
    }
    AdjustTextureUVs(true);
    return 1;
}

int LoadCinematic(HANDLE file)
{
    DWORD read;

    MyReadFile(file, &num_cine_frames, sizeof(sint16), &read, 0);
    if (num_cine_frames) {
        cine = (sint16 *)game_malloc(16 * num_cine_frames, 26);
        MyReadFile(file, cine, 16 * num_cine_frames, &read, 0);
        cine_loaded = 1;
    } else {
        cine_loaded = 0;
    }
    return 1;
}

int LoadDemo(HANDLE file)
{
    DWORD read;
    sint16 size;

    democount = 0;
    demoptr = (unsigned long *)game_malloc(3608 * sizeof(unsigned long), 27);
    MyReadFile(file, &size, sizeof(sint16), &read, 0);
    if (size) {
        MyReadFile(file, demoptr, size, &read, 0);
        demo_loaded = 1;
    } else {
        demo_loaded = 0;
    }
    return 1;
}

void LoadDemFile(char *name)
{
    HANDLE file;
    DWORD read;
    char buf[80];

    strcpy(buf, name);
    build_ext(buf, "DEM");
    file = CreateFile(buf, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (file != INVALID_HANDLE_VALUE) {
        MyReadFile(file, demoptr, 3608 * sizeof(unsigned long), &read, 0);
        demo_loaded = read > 0;
        CloseHandle(file);
    }
}

int LoadSamples(HANDLE file)
{
    int i;
    int n;
    int num_samples;
    int size;
    int fsize;
    HANDLE sfx;
    char *data;
    LPWAVEFORMATEX wf;
    DWORD read;
    int header[11];
    int sample_offsets[500];

    sound_active = 0;
    if (!DS_IsSoundEnabled())
        return 1;
    DS_FreeAllSamples();
    MyReadFile(file, sample_lut, sizeof(sint16) * 370, &read, NULL);
    MyReadFile(file, &num_sample_infos, sizeof(sint32), &read, NULL);
    if (!num_sample_infos)
        return 0;
    sample_infos = (SAMPLE_INFO *)game_malloc(sizeof(SAMPLE_INFO) * num_sample_infos, 35);
    MyReadFile(file, sample_infos, sizeof(SAMPLE_INFO) * num_sample_infos, &read, NULL);
    MyReadFile(file, &num_samples, sizeof(sint32), &read, NULL);
    if (!num_samples)
        return 0;
    MyReadFile(file, sample_offsets, sizeof(sint32) * num_samples, &read, NULL);
    sfx = CreateFile(GetFullPath("data\\main.sfx"), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (sfx == INVALID_HANDLE_VALUE) {
        wsprintf(exit_message, "Could not open MAIN.SFX file");
        return 0;
    }
    for (n = 0, i = 0; n < num_samples; i++) {
        MyReadFile(sfx, header, sizeof(header), &read, NULL);
        if (header[0] != 0x46464952 || header[2] != 0x45564157 || header[9] != 0x61746164)
            return 0;
        wf = (LPWAVEFORMATEX)&header[5];
        size = header[10];
        fsize = (size + 1) & ~1;
        wf->cbSize = 0;
        if (sample_offsets[n] != i) {
            SetFilePointer(sfx, fsize, NULL, FILE_CURRENT);
        } else {
            data = (char *)game_malloc(fsize, 36);
            MyReadFile(sfx, data, fsize, &read, NULL);
            if (!DS_MakeSample(n, wf, data, size))
                return 0;
            game_free(fsize, 36);
            n++;
        }
    }
    CloseHandle(sfx);
    sound_active = 1;
    return 1;
}

void build_ext(char *fname, char *ext)
{
    while (*fname != '.' && *fname != 0)
        fname++;
    *(fname++) = '.';
    *(fname++) = *(ext++);
    *(fname++) = *(ext++);
    *(fname++) = *(ext++);
    *(fname++) = 0;
}

const char *GetFullPath(const char *name)
{
    static char path[128];

    wsprintf(path, "%c:\\%s", cd_drive, name);
    return path;
}

int FindCDDrive(void)
{
    HANDLE file;
    DWORD drives;
    char path[] = "D:\\data\\tombpc.dat";
    char root[8];

    drives = GetLogicalDrives();
    cd_drive = 'A';
    lstrcpy(root, "A:\\");
    while (drives) {
        if (drives & 1) {
            root[0] = cd_drive;
            if (GetDriveType(root) == DRIVE_CDROM) {
                path[0] = cd_drive;
                file = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                if (file != INVALID_HANDLE_VALUE) {
                    CloseHandle(file);
                    return 1;
                }
            }
        }
        cd_drive++;
        drives >>= 1;
    }
    return 0;
}

int LoadLevel(char *name, int number)
{
    const char *path;
    DWORD read;
    HANDLE file;
    int version;
    int level;
    char dem[80];

    path = GetFullPath(name);
    strcpy(LastLoadedLevelPath, path);
    init_game_malloc();
    file = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (file == INVALID_HANDLE_VALUE) {
        wsprintf(exit_message, "LoadLevel(): Could not open %s (level %d)", path, number);
        return 0;
    }
    MyReadFile(file, &version, sizeof(int), &read, NULL);
    LdwPaletteOffset = SetFilePointer(file, 0, NULL, FILE_CURRENT);
    if (!LoadPalette(file))
        return 0;
    LdwTextureOffset = SetFilePointer(file, 0, NULL, FILE_CURRENT);
    if (!LoadTexturePages(file))
        return 0;
    MyReadFile(file, &level, sizeof(int), &read, NULL);
    if (!LoadRooms(file))
        return 0;
    if (!LoadObjects(file))
        return 0;
    if (!LoadSprites(file))
        return 0;
    if (!LoadCameras(file))
        return 0;
    if (!LoadSoundEffects(file))
        return 0;
    if (!LoadBoxes(file))
        return 0;
    if (!LoadAnimatedTextures(file))
        return 0;
    if (!LoadItems(file))
        return 0;
    LdwDepthQOffset = SetFilePointer(file, 0, NULL, FILE_CURRENT);
    if (!LoadDepthQ(file))
        return 0;
    if (!LoadCinematic(file))
        return 0;
    if (!LoadDemo(file))
        return 0;
    strcpy(dem, path);
    if (!LoadSamples(file))
        return 0;
    LoadDemFile(dem);
    CloseHandle(file);
    return 1;
}

int S_LoadLevelFile(char *name, int number, int type)
{
    int loaded;
    bool fade;

    S_UnloadLevelFile();
    S_CDStop();
    fade = false;
    if (type && type != 6 && (type != 4 || GF_Playing_Story)) {
        LoadPicture(GF_picfilenames[GF_LoadingPic], App.lpPictureBuffer, 1);
        FadePictureUp(32);
        fade = true;
    }
    loaded = LoadLevel(name, number);
    if (fade)
        FadePictureDown(32);
    return loaded;
}

void S_UnloadLevelFile(void)
{
    if (App.nRenderMode == 1)
        HWR_FreeTexturePages();
    for (int i = 0; i < 32; ++i)
        texture_page_ptrs[i] = 0;
    LastLoadedLevelPath[0] = 0;
    nTInfos = 0;
}

int Read_Strings(int num, char **strings, char **buffer, DWORD *read, HANDLE file)
{
    sint16 size;
    int i;
    char *p;

    MyReadFile(file, GF_Offsets, sizeof(sint16) * num, read, NULL);
    MyReadFile(file, &size, sizeof(sint16), read, NULL);
    *buffer = (char *)GlobalAlloc(GMEM_FIXED, size);
    if (buffer == NULL)
        return 0;
    MyReadFile(file, *buffer, size, read, NULL);
    if (gameflow.cyphered_strings) {
        p = *buffer;
        for (i = 0; i < size; i++)
            p[i] ^= gameflow.cypher_code;
    }
    for (i = 0; i < num; i++)
        strings[i] = *buffer + GF_Offsets[i];
    return 1;
}

}
