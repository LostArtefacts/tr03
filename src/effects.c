#include "game.h"
#include <stdlib.h>

#define FLIPFLAG 64
#define UNFLIPFLAG 128
#define LARA_HEAD 4
#define SQUARE(A) ((A) * (A))
#define BLOOD_RADIUS 256
#define BUBBLES1 297
#define MAX_BOUNCE 100
#define ABS(x) (((x) < 0) ? (-(x)) : (x))

void S_CDPlay(sint32 track, int mode);


extern OBJECT_VECTOR *sound_effects;
extern sint32 number_sound_effects;
extern void (*effect_routines[])(ITEM_INFO *item);

void SOUND_EndScene(void);
sint32 mGetAngle(sint32 x1, sint32 y1, sint32 x2, sint32 y2);
void TriggerRicochetSpark(GAME_VECTOR *pos, sint32 angle, int size);
sint16 DoBloodSplat(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num);
sint16 DoBloodSplatD(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num);
void TriggerBlood(sint32 x, sint32 y, sint32 z, sint32 angle, sint32 num);
void TriggerBloodD(sint32 x, sint32 y, sint32 z, sint32 angle, sint32 num);
void TriggerUnderwaterBlood(sint32 x, sint32 y, sint32 z, sint32 size);
void TriggerUnderwaterBloodD(sint32 x, sint32 y, sint32 z, sint32 size);
void CreateBubble(PHD_3DPOS *pos, sint16 room_number, sint32 size, sint32 sizerange);
void TriggerBubble(sint32 x, sint32 y, sint32 z, sint32 size, sint32 sizerange, sint16 fxNum);
void GetLaraHandAbsPosition(PHD_VECTOR *vec, sint32 lr);

int ItemNearLara(PHD_3DPOS *pos, sint32 distance)
{
    sint32 x, y, z;
    sint16 *bounds;

    x = pos->x_pos - lara_item->pos.x_pos;
    y = pos->y_pos - lara_item->pos.y_pos;
    z = pos->z_pos - lara_item->pos.z_pos;
    if (x < -distance || x > distance || z < -distance || z > distance || y < -WALL_L * 3 || y > WALL_L * 3)
        return 0;

    if (x * x + z * z > distance * distance)
        return 0;

    bounds = GetBoundsAccurate(lara_item);
    if (y < bounds[2] || y > bounds[3] + 100)
        return 0;

    return 1;
}

void SoundEffects(void)
{
    int i;
    OBJECT_VECTOR *sound;

    sound = sound_effects;
    for (i = number_sound_effects; i > 0; i--, sound++)
    {
        if (!flip_status && (sound->flags & UNFLIPFLAG) == 0)
            continue;
        else if (flip_status && (sound->flags & FLIPFLAG) == 0)
            continue;
        SoundEffect(sound->data, (PHD_3DPOS *)&sound->x, 0);
    }

    if (flipeffect != -1)
        (*effect_routines[flipeffect])(NULL);

    SOUND_EndScene();
}

void Richochet(GAME_VECTOR *pos)
{
    sint32 angle;

    angle = (mGetAngle(pos->z, pos->x, lara_item->pos.z_pos, lara_item->pos.x_pos) >> 4) & 4095;
    TriggerRicochetSpark(pos, angle, 16);
    SoundEffect(10, (PHD_3DPOS *)pos, 0);
}

sint16 DoBloodSplat(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num)
{
    if (room[room_num].flags & UNDERWATER)
        TriggerUnderwaterBlood(x, y, z, GetRandomControl() & 7);
    else
        TriggerBlood(x, y, z, direction >> 4, (GetRandomControl() & 7) + 6);
    return NO_ITEM;
}

sint16 DoBloodSplatD(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num)
{
    if (room[room_num].flags & UNDERWATER)
        TriggerUnderwaterBloodD(x, y + 64, z, GetRandomDraw() & 7);
    else
        TriggerBloodD(x, y, z, direction >> 4, (GetRandomDraw() & 7) + 6);
    return NO_ITEM;
}

void DoLotsOfBlood(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num, int number)
{
    for (; number > 0; number--)
        DoBloodSplat(x + BLOOD_RADIUS - (GetRandomControl() * BLOOD_RADIUS * 2 / 0x8000), y + BLOOD_RADIUS - (GetRandomControl() * BLOOD_RADIUS * 2 / 0x8000), z + BLOOD_RADIUS - (GetRandomControl() * BLOOD_RADIUS * 2 / 0x8000), speed, direction, room_num);
}

void DoLotsOfBloodD(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num, int number)
{
    for (; number > 0; number--)
        DoBloodSplatD(x + BLOOD_RADIUS - (GetRandomDraw() * BLOOD_RADIUS * 2 / 0x8000), y, z + BLOOD_RADIUS - (GetRandomDraw() * BLOOD_RADIUS * 2 / 0x8000), speed, direction, room_num);
}

void CreateBubble(PHD_3DPOS *pos, sint16 room_number, sint32 size, sint32 sizerange)
{
    sint16 fx_number, roomnum;
    FLOOR_INFO *floor;
    FX_INFO *fx;

    roomnum = room_number;
    floor = GetFloor(pos->x_pos, pos->y_pos, pos->z_pos, &roomnum);
    if (!(room[roomnum].flags & UNDERWATER))
        return;
    fx_number = CreateEffect(room_number);
    if (fx_number != NO_ITEM)
    {
        fx = &effects[fx_number];
        fx->pos.x_pos = pos->x_pos;
        fx->pos.y_pos = pos->y_pos;
        fx->pos.z_pos = pos->z_pos;
        fx->speed = 64 + (GetRandomControl() & 255);
        fx->flag1 = (GetRandomControl() & 31) + 32;
        fx->flag2 = 0;
        fx->frame_number = 0;
        fx->object_number = BUBBLES1;
        TriggerBubble(pos->x_pos, pos->y_pos, pos->z_pos, size, sizerange, fx_number);
    }
}

void LaraBubbles(ITEM_INFO *item)
{
    PHD_VECTOR offset;
    int i;

    SoundEffect(37, &item->pos, UNDERWATER);
    offset.x = 0;
    offset.y = -4;
    offset.z = 64;
    GetLaraHandAbsPosition(&offset, LARA_HEAD);
    for (i = (GetRandomControl() & 3) + 2; i > 0; i--)
        CreateBubble((PHD_3DPOS *)&offset, item->room_number, 8, 8);
}

#define DUCKBREATHE_A 222
#define DUCKBREATHE_F GF(DUCKBREATHE_A, 0)

extern sint32 wibble;
void TriggerBreath(sint32 x, sint32 y, sint32 z, sint32 xv, sint32 yv, sint32 zv);

void LaraBreath(ITEM_INFO *item)
{
    PHD_VECTOR offset, offsetv;

    if (lara.water_status == LARA_UNDERWATER || lara_item->hit_points <= 0)
        return;

    if (lara_item->current_anim_state == AS_STOP) {
        if (lara_item->frame_number < BREATH_F + 30)
            return;
    } else if (lara_item->current_anim_state == AS_DUCK) {
        if (lara_item->frame_number < DUCKBREATHE_F + 30)
            return;
    } else if (wibble < 0x80 || wibble > 0xc0) {
        return;
    }

    offset.x = 0;
    offset.y = -4;
    offset.z = 64;
    GetLaraHandAbsPosition(&offset, LARA_HEAD);
    offsetv.x = (GetRandomControl() & 7) - 4;
    offsetv.y = -8 + (GetRandomControl() & 7);
    offsetv.z = 64 + (GetRandomControl() & 127);
    GetLaraHandAbsPosition(&offsetv, LARA_HEAD);
    TriggerBreath(offset.x, offset.y, offset.z, offsetv.x - offset.x, offsetv.y - offset.y, offsetv.z - offset.z);
}

void KillEffect(sint16 fx_num);
void EffectNewRoom(sint16 fx_num, sint16 room_number);
RIPPLE_STRUCT *SetupRipple(sint32 x, sint32 y, sint32 z, sint32 size, sint32 moretrans);

void ControlBubble1(sint16 fx_num)
{
    FX_INFO *fx;
    OBJECT_INFO *object;
    FLOOR_INFO *floor;
    sint32 x, y, z, h;
    sint16 roomy;

    fx = &effects[fx_num];
    object = &objects[fx->object_number];
    fx->pos.y_rot += 9 * ONE_DEGREE;
    fx->pos.x_rot += 13 * ONE_DEGREE;
    fx->speed += fx->flag1;
    x = fx->pos.x_pos + ((phd_sin(fx->pos.y_rot) * 3) >> W2V_SHIFT);
    y = fx->pos.y_pos - (fx->speed >> 8);
    z = fx->pos.z_pos + ((phd_cos(fx->pos.x_rot) * 1) >> W2V_SHIFT);
    roomy = fx->room_number;
    floor = GetFloor(x, y, z, &roomy);
    h = GetHeight(floor, x, y, z);
    if (y > h) {
        KillEffect(fx_num);
        return;
    }

    if (!floor || !(room[roomy].flags & UNDERWATER)) {
        if (floor)
            SetupRipple(fx->pos.x_pos, room[fx->room_number].maxceiling, fx->pos.z_pos, -2 - (GetRandomControl() & 1), 1);
        KillEffect(fx_num);
        return;
    }

    h = GetCeiling(floor, x, y, z);
    if (h == NO_HEIGHT || y <= h) {
        KillEffect(fx_num);
        return;
    }

    if (fx->room_number != roomy)
        EffectNewRoom(fx_num, roomy);
    fx->pos.x_pos = x;
    fx->pos.y_pos = y;
    fx->pos.z_pos = z;
}

typedef struct {
    sint32 x;
    sint32 y;
    sint32 z;
    sint16 InnerXZoff;
    sint16 InnerXZsize;
    sint16 InnerYsize;
    sint16 InnerXZvel;
    sint16 InnerYvel;
    sint16 InnerGravity;
    sint16 InnerFriction;
    sint16 MiddleXZoff;
    sint16 MiddleXZsize;
    sint16 MiddleYsize;
    sint16 MiddleXZvel;
    sint16 MiddleYvel;
    sint16 MiddleGravity;
    sint16 MiddleFriction;
    sint16 OuterXZoff;
    sint16 OuterXZsize;
    sint16 OuterXZvel;
    sint16 OuterFriction;
} SPLASH_SETUP;

extern SPLASH_SETUP splash_setup;
sint32 GetWaterHeight(sint32 x, sint32 y, sint32 z, sint16 room_number);
void SetupSplash(SPLASH_SETUP *spl);

void Splash(ITEM_INFO *item)
{
    sint32 height;
    sint16 room_number;

    room_number = item->room_number;
    GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    if (!(room[room_number].flags & UNDERWATER))
        return;

    height = GetWaterHeight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, room_number);
    splash_setup.x = item->pos.x_pos;
    splash_setup.y = height;
    splash_setup.z = item->pos.z_pos;
    splash_setup.InnerXZoff = 32;
    splash_setup.InnerXZsize = 8;
    splash_setup.InnerYsize = -128;
    splash_setup.InnerXZvel = 0x140;
    splash_setup.InnerYvel = -item->fallspeed * 40;
    splash_setup.InnerGravity = 0xa0;
    splash_setup.InnerFriction = 7;
    splash_setup.MiddleXZoff = 48;
    splash_setup.MiddleXZsize = 32;
    splash_setup.MiddleYsize = -64;
    splash_setup.MiddleXZvel = 0x1e0;
    splash_setup.MiddleYvel = -item->fallspeed * 20;
    splash_setup.MiddleGravity = 0x60;
    splash_setup.MiddleFriction = 8;
    splash_setup.OuterXZoff = 32;
    splash_setup.OuterXZsize = 128;
    splash_setup.OuterXZvel = 0x220;
    splash_setup.OuterFriction = 9;
    SetupSplash(&splash_setup);
}

extern sint32 SplashCount;
sint16 *GetBestFrame(ITEM_INFO *item);

void WadeSplash(ITEM_INFO *item, int water, int waterdepth)
{
    sint16 *bounds, room_number;

    room_number = item->room_number;
    GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    if (!(room[room_number].flags & UNDERWATER))
        return;

    bounds = GetBestFrame(item);
    if (item->pos.y_pos + bounds[2] > water || item->pos.y_pos + bounds[3] < water)
        return;

    if (item->fallspeed > 0 && waterdepth < 730 - STEP_L && SplashCount == 0) {
        splash_setup.x = item->pos.x_pos;
        splash_setup.y = water;
        splash_setup.z = item->pos.z_pos;
        splash_setup.InnerXZoff = 16;
        splash_setup.InnerXZsize = 12;
        splash_setup.InnerYsize = -96;
        splash_setup.InnerXZvel = 0xa0;
        splash_setup.InnerYvel = -item->fallspeed * 72;
        splash_setup.InnerGravity = 0x80;
        splash_setup.InnerFriction = 7;
        splash_setup.MiddleXZoff = 24;
        splash_setup.MiddleXZsize = 24;
        splash_setup.MiddleYsize = -64;
        splash_setup.MiddleXZvel = 0xe0;
        splash_setup.MiddleYvel = -item->fallspeed * 36;
        splash_setup.MiddleGravity = 0x48;
        splash_setup.MiddleFriction = 8;
        splash_setup.OuterXZoff = 32;
        splash_setup.OuterXZsize = 32;
        splash_setup.OuterXZvel = 0x110;
        splash_setup.OuterFriction = 9;
        SetupSplash(&splash_setup);
        SplashCount = 16;
    } else if ((wibble & 15) == 0 && ((GetRandomControl() & 15) == 0 || item->current_anim_state != AS_STOP)) {
        if (item->current_anim_state != AS_STOP)
            SetupRipple(item->pos.x_pos, water, item->pos.z_pos, -16 - (GetRandomControl() & 15), 0);
        else
            SetupRipple(item->pos.x_pos, water, item->pos.z_pos, -16 - (GetRandomControl() & 15), 1);
    }
}

extern sint16 rcossin_tbl[];
void S_CalculateLight(sint32 x, sint32 y, sint32 z, sint32 room_number, ITEM_LIGHT *il);
void TriggerWaterfallMist(sint32 x, sint32 y, sint32 z, sint32 angle);

void WaterFall(sint16 item_number)
{
    ITEM_INFO *item;
    sint32 x, y, z, ang;

    item = &items[item_number];
    x = item->pos.x_pos - lara_item->pos.x_pos;
    y = item->pos.y_pos - lara_item->pos.y_pos;
    z = item->pos.z_pos - lara_item->pos.z_pos;
    if (x < -0x4000 || x > 0x4000 || z < -0x4000 || z > 0x4000 || y < -0x4000 || y > 0x4000)
        return;

    S_CalculateLight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, &item->il);
    ang = ((item->pos.y_rot >> 4) & 4095) << 1;
    z = (rcossin_tbl[ang + 1] * 544) >> 12;
    x = (rcossin_tbl[ang] * 544) >> 12;
    if ((wibble & 12) == 0)
        TriggerWaterfallMist(item->pos.x_pos + x, item->pos.y_pos, item->pos.z_pos + z, item->pos.y_rot >> 4);
    SoundEffect(79, &item->pos, 0);
}

void finish_level_effect(ITEM_INFO *item)
{
    level_complete = 1;
}

void turn180_effect(ITEM_INFO *item)
{
    item->pos.y_rot += -0x8000;
    item->pos.x_rot = -item->pos.x_rot;
}

void floor_shake_effect(ITEM_INFO *item)
{
    sint32 x, y, z;

    x = item->pos.x_pos - camera.pos.x;
    y = item->pos.y_pos - camera.pos.y;
    z = item->pos.z_pos - camera.pos.z;
    if (ABS(x) < 16 * WALL_L && ABS(y) < 16 * WALL_L && ABS(z) < 16 * WALL_L)
        camera.bounce = MAX_BOUNCE * ((SQUARE(x) + SQUARE(y) + SQUARE(z)) / 256 - WALL_L * WALL_L) / (WALL_L * WALL_L);
}

void lara_normal_effect(ITEM_INFO *item)
{
    item->current_anim_state = AS_STOP;
    item->goal_anim_state = AS_STOP;
    item->frame_number = STOP_F;
    item->anim_number = STOP_A;
    camera.type = CHASE_CAMERA;
    AlterFOV(GAME_FOV * ONE_DEGREE);
}

void BoilerFX(ITEM_INFO *item)
{
    SoundEffect(338, NULL, 0);
    flipeffect = -1;
}

void FloodFX(ITEM_INFO *item)
{
    PHD_3DPOS pos;

    if (fliptimer > 30 * 4)
    {
        flipeffect = -1;
    }
    else
    {
        pos.x_pos = lara_item->pos.x_pos;
        if (fliptimer < 30)
            pos.y_pos = camera.target.y + (30 - fliptimer) * 100;
        else
            pos.y_pos = camera.target.y + (fliptimer - 30) * 100;
        pos.z_pos = lara_item->pos.z_pos;
        SoundEffect(163, &pos, 0);
    }
    fliptimer++;
}

void RubbleFX(ITEM_INFO *item)
{
    SoundEffect(24, NULL, 0);
    camera.bounce = -350;
    flipeffect = -1;
}

void ChandelierFX(ITEM_INFO *item)
{
    SoundEffect(278, NULL, 0);
    fliptimer++;
    if (fliptimer > 30)
        flipeffect = -1;
}

void ExplosionFX(ITEM_INFO *item)
{
    SoundEffect(105, NULL, 0);
    camera.bounce = -75;
    flipeffect = -1;
}

void PistonFX(ITEM_INFO *item)
{
    SoundEffect(190, NULL, 0);
    flipeffect = -1;
}

void CurtainFX(ITEM_INFO *item)
{
    SoundEffect(191, NULL, 0);
    flipeffect = -1;
}

void StatueFX(ITEM_INFO *item)
{
    SoundEffect(331, NULL, 0);
    flipeffect = -1;
}

void SetChangeFX(ITEM_INFO *item)
{
    SoundEffect(330, NULL, 0);
    flipeffect = -1;
}

void ControlDingDong(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    if ((item->flags & CODE_BITS) == CODE_BITS)
    {
        SoundEffect(334, &item->pos, 0);
        item->flags -= CODE_BITS;
    }
}

void ControlLaraAlarm(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    if ((item->flags & CODE_BITS) == CODE_BITS)
        SoundEffect(335, &item->pos, 0);
}

void ControlBirdTweeter(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    if (item->object_number == BIRD_TWEETER)
    {
        if (GetRandomControl() < 0x400)
            SoundEffect(316, &item->pos, 0);
    }
    else if (GetRandomControl() < 0x100)
        SoundEffect(329, &item->pos, 0);
}

void DoChimeSound(ITEM_INFO *item)
{
    PHD_3DPOS pos;

    pos.x_pos = lara_item->pos.x_pos + ((item->pos.x_pos - lara_item->pos.x_pos) >> 6);
    pos.y_pos = lara_item->pos.y_pos + ((item->pos.y_pos - lara_item->pos.y_pos) >> 6);
    pos.z_pos = lara_item->pos.z_pos + ((item->pos.z_pos - lara_item->pos.z_pos) >> 6);
    SoundEffect(208, &pos, 0);
}

void ControlClockChimes(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    if (item->timer == 0)
        return;

    if ((item->timer % 60) == 59)
        DoChimeSound(item);
    item->timer--;
    if (item->timer == 0)
    {
        DoChimeSound(item);
        item->timer = -1;
        RemoveActiveItem(item_number);
        item->status = NOT_ACTIVE;
        item->flags &= ~CODE_BITS;
    }
}

void lara_hands_free(ITEM_INFO *item)
{
    lara.gun_status = LG_ARMLESS;
}

void flip_map_effect(ITEM_INFO *item)
{
    FlipMap();
}

void draw_right_gun(ITEM_INFO *item)
{
    sint16 *temp;

    temp = lara.mesh_ptrs[THIGH_R];
    lara.mesh_ptrs[THIGH_R] = meshes[objects[PISTOLS].mesh_index + THIGH_R];
    meshes[objects[PISTOLS].mesh_index + THIGH_R] = temp;
    temp = lara.mesh_ptrs[HAND_R];
    lara.mesh_ptrs[HAND_R] = meshes[objects[PISTOLS].mesh_index + HAND_R];
    meshes[objects[PISTOLS].mesh_index + HAND_R] = temp;
}

void draw_left_gun(ITEM_INFO *item)
{
    sint16 *temp;

    temp = lara.mesh_ptrs[THIGH_L];
    lara.mesh_ptrs[THIGH_L] = meshes[objects[PISTOLS].mesh_index + THIGH_L];
    meshes[objects[PISTOLS].mesh_index + THIGH_L] = temp;
    temp = lara.mesh_ptrs[HAND_L];
    lara.mesh_ptrs[HAND_L] = meshes[objects[PISTOLS].mesh_index + HAND_L];
    meshes[objects[PISTOLS].mesh_index + HAND_L] = temp;
}

void shoot_right_gun(ITEM_INFO *item)
{
    lara.right_arm.flash_gun = 3;
}

void shoot_left_gun(ITEM_INFO *item)
{
    lara.left_arm.flash_gun = 3;
}

void swap_meshes_with_meshswap1(ITEM_INFO *item)
{
    int i;
    sint16 nmeshes;
    sint16 *temp;
    OBJECT_INFO *obj;

    obj = &objects[item->object_number];
    nmeshes = obj->nmeshes;
    for (i = 0; i < obj->nmeshes; i++)
    {
        temp = meshes[obj->mesh_index + i];
        meshes[obj->mesh_index + i] = meshes[objects[MESHSWAP1].mesh_index + i];
        meshes[objects[MESHSWAP1].mesh_index + i] = temp;
    }
}

void swap_meshes_with_meshswap2(ITEM_INFO *item)
{
    int i;
    sint16 nmeshes;
    sint16 *temp;
    OBJECT_INFO *obj;

    obj = &objects[item->object_number];
    nmeshes = obj->nmeshes;
    for (i = 0; i < obj->nmeshes; i++)
    {
        temp = meshes[obj->mesh_index + i];
        meshes[obj->mesh_index + i] = meshes[objects[MESHSWAP2].mesh_index + i];
        meshes[objects[MESHSWAP2].mesh_index + i] = temp;
    }
}

void swap_meshes_with_meshswap3(ITEM_INFO *item)
{
    int i;
    sint16 nmeshes;
    sint16 *temp;
    OBJECT_INFO *obj;

    obj = &objects[item->object_number];
    nmeshes = obj->nmeshes;
    for (i = 0; i < obj->nmeshes; i++)
    {
        temp = meshes[obj->mesh_index + i];
        meshes[obj->mesh_index + i] = meshes[objects[MESHSWAP3].mesh_index + i];
        if (item == lara_item)
            lara.mesh_ptrs[i] = meshes[objects[MESHSWAP3].mesh_index + i];
        meshes[objects[MESHSWAP3].mesh_index + i] = temp;
    }
}

void invisibility_on(ITEM_INFO *item)
{
    item->status = INVISIBLE;
}

void invisibility_off(ITEM_INFO *item)
{
    item->status = ACTIVE;
}

void dynamic_light_on(ITEM_INFO *item)
{
    item->dynamic_light = 1;
}

void dynamic_light_off(ITEM_INFO *item)
{
    item->dynamic_light = 0;
}

void reset_hair(ITEM_INFO *item)
{
    InitialiseHair();
}

void AssaultStart(ITEM_INFO *item)
{
    savegame.timer = 0;
    assault_timer_active = 1;
    assault_timer_display = 1;
    assault_penalties = 0;
    assault_penalty_display_timer = 0;
    assault_targets = 7;
    ResetTargets();
    flipeffect = -1;
}

void AssaultStop(ITEM_INFO *item)
{
    assault_timer_active = 0;
    assault_timer_display = 1;
    flipeffect = -1;
}

void AssaultReset(ITEM_INFO *item)
{
    assault_timer_active = 0;
    assault_timer_display = 0;
    assault_penalties = 0;
    assault_penalty_display_timer = 0;
    flipeffect = -1;
}

void AssaultPenalty4(ITEM_INFO *item)
{
    if (assault_timer_active)
    {
        assault_penalties += 60 * 4;
        assault_penalty_display_timer = 4 * 30;
    }
    flipeffect = -1;
}

void AssaultPenalty30(ITEM_INFO *item)
{
    if (assault_timer_active)
    {
        assault_penalties += 30 * 30;
        assault_penalty_display_timer = 4 * 30;
    }
    flipeffect = -1;
}

void AssaultFinished(ITEM_INFO *item)
{
    if (assault_timer_active)
    {
        assault_target_penalties = assault_targets * 10 * 30;
        assault_penalty_display_timer = 10 * 30;
        savegame.timer += assault_target_penalties + assault_penalties;
        if (savegame.timer > MAX_TIME_ALLOWED)
            savegame.timer = MAX_TIME_ALLOWED;
        AddAssaultTime(savegame.timer);
        assault_timer_active = 0;
        if (savegame.timer < 3 * 60 * 30)
            S_CDPlay(95, 0);
    }
    flipeffect = -1;
}

void QuadStart(ITEM_INFO *item)
{
    if (lara.skidoo != NO_ITEM)
    {
        savegame.timer = 0;
        assault_timer_active = 1;
        assault_timer_display = 1;
    }
    flipeffect = -1;
}

void QuadStop(ITEM_INFO *item)
{
    if (lara.skidoo != NO_ITEM)
    {
        assault_timer_active = 0;
        assault_timer_display = 1;
    }
    flipeffect = -1;
}

void QuadFinished(ITEM_INFO *item)
{
    if (lara.skidoo != NO_ITEM)
    {
        if (assault_timer_active)
        {
            if (savegame.timer > MAX_TIME_ALLOWED)
                savegame.timer = MAX_TIME_ALLOWED;
            QuadbikeLapTime = savegame.timer;
            QuadbikeLapTimeDisplayTimer = 10 * 30;
            AddQuadbikeTime(QuadbikeLapTime);
            assault_timer_active = 0;
        }
    }
    flipeffect = -1;
}

void Exercise1_105(ITEM_INFO *item)
{
    if (ExerciseNumber == 0)
    {
        S_CDPlay(105 + 3, 0);
        ExerciseNumber++;
    }
    flipeffect = -1;
}

void Exercise1_104(ITEM_INFO *item)
{
    if (ExerciseNumber == 1)
    {
        S_CDPlay(104 + 3, 0);
        ExerciseNumber++;
    }
    flipeffect = -1;
}

void Exercise2_112(ITEM_INFO *item)
{
    if (ExerciseNumber == 2)
    {
        S_CDPlay(112 + 3, 0);
        ExerciseNumber++;
    }
    flipeffect = -1;
}

void Exercise2_83(ITEM_INFO *item)
{
    if (ExerciseNumber == 3)
    {
        S_CDPlay(83 + 3, 0);
        ExerciseNumber++;
    }
    flipeffect = -1;
}

void Exercise3_115(ITEM_INFO *item)
{
    if (ExerciseNumber == 4)
    {
        S_CDPlay(115 + 3, 0);
        ExerciseNumber++;
    }
    flipeffect = -1;
}

void Exercise3_107(ITEM_INFO *item)
{
    if (ExerciseNumber == 5)
    {
        S_CDPlay(107 + 3, 0);
        ExerciseNumber++;
    }
    flipeffect = -1;
}

void Exercise3_79(ITEM_INFO *item)
{
    if (ExerciseNumber == 6)
    {
        S_CDPlay(79 + 3, 0);
        ExerciseNumber++;
    }
    flipeffect = -1;
}

void Exercise4_110(ITEM_INFO *item)
{
    if (ExerciseNumber == 7)
    {
        S_CDPlay(110 + 3, 0);
        ExerciseNumber++;
    }
    flipeffect = -1;
}

void Exercise4_111(ITEM_INFO *item)
{
    if (ExerciseNumber == 8)
    {
        S_CDPlay(111 + 3, 0);
        ExerciseNumber++;
    }
    flipeffect = -1;
}

void Exercise5_116(ITEM_INFO *item)
{
    if (ExerciseNumber == 9)
    {
        S_CDPlay(116 + 3, 0);
        ExerciseNumber++;
    }
    flipeffect = -1;
}

void Exercise5_95(ITEM_INFO *item)
{
    if (ExerciseNumber == 10)
    {
        S_CDPlay(95 + 3, 0);
        ExerciseNumber++;
    }
    flipeffect = -1;
}

void Exercise6_113(ITEM_INFO *item)
{
    if (ExerciseNumber == 11)
    {
        S_CDPlay(113 + 3, 0);
        ExerciseNumber++;
    }
    flipeffect = -1;
}

void Exercise6_114(ITEM_INFO *item)
{
    if (ExerciseNumber == 12)
    {
        S_CDPlay(114 + 3, 0);
        ExerciseNumber++;
    }
    flipeffect = -1;
}

void Exercise6_80(ITEM_INFO *item)
{
    if (ExerciseNumber == 13)
    {
        S_CDPlay(80 + 3, 0);
        ExerciseNumber++;
    }
    flipeffect = -1;
}

void Exercise7_106(ITEM_INFO *item)
{
    if (ExerciseNumber == 14)
    {
        S_CDPlay(106 + 3, 0);
        ExerciseNumber++;
    }
    flipeffect = -1;
}

void Exercise7_86(ITEM_INFO *item)
{
    if (ExerciseNumber == 15)
    {
        S_CDPlay(86 + 3, 0);
        ExerciseNumber++;
    }
    flipeffect = -1;
}

void Exercise8_93(ITEM_INFO *item)
{
    if (ExerciseNumber == 16)
    {
        S_CDPlay(93 + 3, 0);
        ExerciseNumber++;
    }
    flipeffect = -1;
}

void Exercise8_109Sp(ITEM_INFO *item)
{
    if (ExerciseNumber == 17)
    {
        if (lara.water_status == LARA_SURFACE)
        {
            S_CDPlay(109 + 3, 0);
            ExerciseNumber++;
        }
    }
    flipeffect = -1;
}

void Exercise8_87(ITEM_INFO *item)
{
    if (ExerciseNumber == 18)
    {
        S_CDPlay(87 + 3, 0);
        ExerciseNumber = 0;
    }
    flipeffect = -1;
}

void ResetExercises(ITEM_INFO *item)
{
    ExerciseNumber = 0;
    flipeffect = -1;
}

void TubeTrain(ITEM_INFO *item)
{
    camera.bounce = -350;
    flipeffect = -1;
}

void RumbleNoShake(ITEM_INFO *item)
{
    SoundEffect(149, NULL, 0);
    flipeffect = -1;
}

void BaddieBiteEffect(ITEM_INFO *item, BITE_INFO *bite)
{
    PHD_VECTOR pos;

    pos.x = bite->x;
    pos.y = bite->y;
    pos.z = bite->z;
    GetJointAbsPosition(item, &pos, bite->mesh_num);
    DoBloodSplat(pos.x, pos.y, pos.z, item->speed, item->pos.y_rot, item->room_number);
}

int FishNearLara(PHD_3DPOS *pos, sint32 distance, ITEM_INFO *item)
{
    sint32 x, y, z;

    x = pos->x_pos - item->pos.x_pos;
    y = abs(pos->y_pos - item->pos.y_pos);
    z = pos->z_pos - item->pos.z_pos;
    if (x < -distance || x > distance || z < -distance || z > distance || y < -WALL_L * 3 || y > WALL_L * 3)
        return 0;
    if (x * x + z * z > SQUARE(distance))
        return 0;
    if (y > distance)
        return 0;
    return 1;
}
