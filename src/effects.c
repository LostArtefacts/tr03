#include "game.h"

#define FLIPFLAG 64
#define UNFLIPFLAG 128

typedef struct {
    sint32 x;
    sint32 y;
    sint32 z;
    sint32 mesh_num;
} BITE_INFO;

extern OBJECT_VECTOR *sound_effects;
extern sint32 number_sound_effects;
extern void (*effect_routines[])(ITEM_INFO *item);

void SOUND_EndScene(void);
void GetJointAbsPosition(ITEM_INFO *item, PHD_VECTOR *pos, int joint);
sint16 DoBloodSplat(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num);

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

void finish_level_effect(ITEM_INFO *item)
{
    level_complete = 1;
}

void turn180_effect(ITEM_INFO *item)
{
    item->pos.y_rot += -0x8000;
    item->pos.x_rot = -item->pos.x_rot;
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

void lara_hands_free(ITEM_INFO *item)
{
    lara.gun_status = LG_ARMLESS;
}

void flip_map_effect(ITEM_INFO *item)
{
    FlipMap();
}

void shoot_right_gun(ITEM_INFO *item)
{
    lara.right_arm.flash_gun = 3;
}

void shoot_left_gun(ITEM_INFO *item)
{
    lara.left_arm.flash_gun = 3;
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
