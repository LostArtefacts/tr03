#include "game.h"

void finish_level_effect(ITEM_INFO *item)
{
    level_complete = 1;
}

void turn180_effect(ITEM_INFO *item)
{
    item->pos.y_rot += -0x8000;
    item->pos.x_rot = -item->pos.x_rot;
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
