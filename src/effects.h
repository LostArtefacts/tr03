#ifndef EFFECTS_H
#define EFFECTS_H

#include "items.h"

void finish_level_effect(ITEM_INFO *item);
void turn180_effect(ITEM_INFO *item);
void BoilerFX(ITEM_INFO *item);
void RubbleFX(ITEM_INFO *item);
void lara_hands_free(ITEM_INFO *item);
void flip_map_effect(ITEM_INFO *item);
void shoot_right_gun(ITEM_INFO *item);
void shoot_left_gun(ITEM_INFO *item);
void invisibility_on(ITEM_INFO *item);
void invisibility_off(ITEM_INFO *item);
void dynamic_light_on(ITEM_INFO *item);
void dynamic_light_off(ITEM_INFO *item);
void reset_hair(ITEM_INFO *item);

#endif
