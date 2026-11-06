#include "standard.h"
#include "specific.h"
#include "winmain.h"

extern "C" {

#include "../types.h"
#include "../camera.h"
#include "../anim.h"
#include "../room.h"

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

}
