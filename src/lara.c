#include "game.h"
#include <stdlib.h>

#define DAMAGE_START 140
#define DAMAGE_LENGTH 14
#define FASTFALL_FRICTION item->speed = (item->speed * 95) / 100
#define STEPUP_HEIGHT ((STEP_L * 3) / 2)
#define CAM_A_HANG 0
#define CAM_E_HANG (-60 * ONE_DEGREE)
