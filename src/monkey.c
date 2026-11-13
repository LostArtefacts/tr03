#include "game.h"

#define MONKEY_SIT_ANIM 2
#define MONKEY_SIT 6

void InitialiseMonkey(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    InitialiseCreature(item_number);
    item->anim_number = objects[MONKEY].anim_index + MONKEY_SIT_ANIM;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = item->goal_anim_state = MONKEY_SIT;
}

extern sint32 *bones;

int GetFrames(ITEM_INFO *item, sint16 *frmptr[], int *rate);
void S_PrintShadow(int size, sint16 *bptr, ITEM_INFO *item, int underwater);
void phd_PushMatrix(void);
void phd_TranslateAbs(sint32 x, sint32 y, sint32 z);
void phd_TranslateRel(sint32 x, sint32 y, sint32 z);
void phd_RotX(sint16 rx);
void phd_RotY(sint16 ry);
void phd_RotZ(sint16 rz);
void phd_RotYXZ(sint16 ry, sint16 rx, sint16 rz);
int S_GetObjectBounds(sint16 *bptr);
void CalculateObjectLighting(ITEM_INFO *item, sint16 *frame);
void gar_RotYXZsuperpack(sint16 **pprot, int skip);
void phd_PutPolygons(sint16 *objptr, int clip);
void InitInterpolate(int frac, int rate);
void phd_TranslateRel_ID(sint32 x, sint32 y, sint32 z, sint32 x2, sint32 y2, sint32 z2);
void gar_RotYXZsuperpack_I(sint16 **pprot1, sint16 **pprot2, int skip);
void phd_PutPolygons_I(sint16 *ptr, int clip);
void phd_PopMatrix_I(void);
void phd_PushMatrix_I(void);
void phd_TranslateRel_I(sint32 x, sint32 y, sint32 z);
void phd_RotX_I(sint16 ang);
void phd_RotY_I(sint16 ang);
void phd_RotZ_I(sint16 ang);

void DrawMonkey(ITEM_INFO *item)
{
    int clip, i, poppush;
    OBJECT_INFO *object;
    sint32 *bone;
    sint16 *rotation1, *rotation2;
    sint16 *extra_rotation;
    sint16 **meshpp, **swappp;
    uint32 bit;
    sint16 *frmptr[2];
    int rate, frac;

    frac = GetFrames(item, frmptr, &rate);
    object = &objects[item->object_number];
    if (object->shadow_size)
        S_PrintShadow(object->shadow_size, frmptr[0], item, 0);

    phd_PushMatrix();
    phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);
    if ((clip = S_GetObjectBounds(frmptr[0]))) {
        CalculateObjectLighting(item, frmptr[0]);
        extra_rotation = (sint16 *)item->data;
        meshpp = &meshes[objects[71].mesh_index];
        if (item->ai_bits != 8)
            swappp = &meshes[objects[123].mesh_index];
        else
            swappp = &meshes[objects[124].mesh_index];
        bone = bones + object->bone_index;
        bit = 1;
        if (!frac) {
            phd_TranslateRel((sint32)*(frmptr[0] + 6), (sint32)*(frmptr[0] + 7), (sint32)*(frmptr[0] + 8));
            rotation1 = frmptr[0] + 9;
            gar_RotYXZsuperpack(&rotation1, 0);
            if (bit & item->mesh_bits)
                phd_PutPolygons(*meshpp, clip);
            else
                phd_PutPolygons(*swappp, clip);
            meshpp++;
            swappp++;
            for (i = object->nmeshes - 1; i > 0; i--, bone += 4, meshpp++, swappp++) {
                poppush = *bone;
                if (poppush & 1)
                    phd_PopMatrix();
                if (poppush & 2)
                    phd_PushMatrix();
                phd_TranslateRel(*(bone + 1), *(bone + 2), *(bone + 3));
                gar_RotYXZsuperpack(&rotation1, 0);
                if (extra_rotation && (poppush & 0x1c)) {
                    if (poppush & 8)
                        phd_RotY(*(extra_rotation++));
                    if (poppush & 4)
                        phd_RotX(*(extra_rotation++));
                    if (poppush & 16)
                        phd_RotZ(*(extra_rotation++));
                }
                bit <<= 1;
                if (bit & item->mesh_bits)
                    phd_PutPolygons(*meshpp, clip);
                else
                    phd_PutPolygons(*swappp, clip);
            }
        } else {
            InitInterpolate(frac, rate);
            phd_TranslateRel_ID((sint32)*(frmptr[0] + 6), (sint32)*(frmptr[0] + 7), (sint32)*(frmptr[0] + 8),
                                (sint32)*(frmptr[1] + 6), (sint32)*(frmptr[1] + 7), (sint32)*(frmptr[1] + 8));
            rotation1 = frmptr[0] + 9;
            rotation2 = frmptr[1] + 9;
            gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
            if (bit & item->mesh_bits)
                phd_PutPolygons_I(*meshpp, clip);
            else
                phd_PutPolygons_I(*swappp, clip);
            meshpp++;
            swappp++;
            for (i = object->nmeshes - 1; i > 0; i--, bone += 4, meshpp++, swappp++) {
                poppush = *bone;
                if (poppush & 1)
                    phd_PopMatrix_I();
                if (poppush & 2)
                    phd_PushMatrix_I();
                phd_TranslateRel_I(*(bone + 1), *(bone + 2), *(bone + 3));
                gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
                if (extra_rotation && (poppush & 0x1c)) {
                    if (poppush & 8)
                        phd_RotY_I(*(extra_rotation++));
                    if (poppush & 4)
                        phd_RotX_I(*(extra_rotation++));
                    if (poppush & 16)
                        phd_RotZ_I(*(extra_rotation++));
                }
                bit <<= 1;
                if (bit & item->mesh_bits)
                    phd_PutPolygons_I(*meshpp, clip);
                else if (item->hit_points > 0 || item->status == ACTIVE || bit != 0x400 || item->carried_item == -1)
                    phd_PutPolygons_I(*swappp, clip);
            }
        }
    }
    phd_PopMatrix();
}

typedef struct {
    sint16 zone_number;
    sint16 enemy_zone;
    sint32 distance;
    sint32 ahead;
    sint32 bite;
    sint16 angle;
    sint16 x_angle;
    sint16 enemy_facing;
} AI_INFO;

#define MONKEY_WALK 2
#define MONKEY_STAND 3
#define MONKEY_RUN 4
#define MONKEY_PICKUP 5
#define MONKEY_EAT 7
#define MONKEY_SCRATCH 8
#define MONKEY_ROLL 9
#define MONKEY_ANGRY 10
#define MONKEY_DEATH 11
#define MONKEY_ATAK_LOW 12
#define MONKEY_ATAK_HIGH 13
#define MONKEY_ATAK_JUMP 14
#define MONKEY_CLIMB4 15
#define MONKEY_CLIMB3 16
#define MONKEY_CLIMB2 17
#define MONKEY_DOWN4 18
#define MONKEY_DOWN3 19
#define MONKEY_DOWN2 20

#define SQUARE(x) ((x) * (x))
#define MONKEY_HIT_DAMAGE 40
#define MONKEY_SWIPE_DAMAGE 50
#define MONKEY_WALK_TURN (ONE_DEGREE * 7)
#define MONKEY_RUN_TURN (ONE_DEGREE * 11)
#define MONKEY_ATTACK_RANGE SQUARE(WALL_L / 3)
#define MONKEY_JUMP_RANGE SQUARE(WALL_L * 2 / 3)
#define MONKEY_WALK_RANGE SQUARE(WALL_L * 2 / 3)
#define MONKEY_ROLL_RANGE SQUARE(WALL_L)
#define MONKEY_WAIT_CHANCE 0x100
#define MONKEY_DIE_ANIM 14
#define MONKEY_CLIMB2_ANIM 19
#define MONKEY_CLIMB3_ANIM 18
#define MONKEY_CLIMB4_ANIM 17
#define MONKEY_DOWN2_ANIM 22
#define MONKEY_DOWN3_ANIM 21
#define MONKEY_DOWN4_ANIM 20
#define MONKEY_TOUCH 0x2400
#define MONKEY_VAULT_SHIFT 128
#define MONKEY_HIT_RADIUS STEP_L
#define GUARD 1
#define AMBUSH 2
#define PATROL1 4
#define MODIFY 8
#define FOLLOW 16
#define MEDI_ITEM 176
#define KEY_ITEM4 227
#define AI_AMBUSH 75
#define AI_PATROL1 76
#define AI_PATROL2 79
#define TIGER 28
#define KILLED_ITEM (-0x8000)

int CreatureActive(sint16 item_number);
void CreatureAIInfo(ITEM_INFO *item, AI_INFO *info);
void GetCreatureMood(ITEM_INFO *item, AI_INFO *info, int violent);
void CreatureMood(ITEM_INFO *item, AI_INFO *info, int violent);
sint16 CreatureTurn(ITEM_INFO *item, sint16 maximum_turn);
int CreatureAnimation(sint16 item_number, sint16 angle, sint16 tilt);
void CreatureJoint(ITEM_INFO *item, sint16 joint, sint16 required);
int CreatureVault(sint16 item_number, sint16 angle, int vault, int shift);
sint16 CreatureEffect(ITEM_INFO *item, BITE_INFO *bite, sint16 (*generate)(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 yrot, sint16 room_number));
sint16 DoBloodSplat(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num);
void GetAITarget(CREATURE_INFO *creature);
void AlertAllGuards(sint16 item_number);
sint16 AIGuard(CREATURE_INFO *creature);
void RemoveDrawnItem(sint16 item_num);

extern BITE_INFO monkey_hit;

int abs(int x);
#pragma intrinsic(abs)

void MonkeyControl(sint16 item_number)
{
    ITEM_INFO *item, *enemy, *target, *pickup;
    CREATURE_INFO *monkey, *cinfo;
    sint16 angle, torso_y, torso_x, head, tilt, slot, i;
    sint32 lara_dx, lara_dz;
    AI_INFO info, lara_info;

    if (!CreatureActive(item_number))
        return;
    item = &items[item_number];
    monkey = (CREATURE_INFO *)item->data;
    torso_y = torso_x = head = angle = tilt = 0;
    if (item->hit_points <= 0) {
        if (item->current_anim_state != MONKEY_DEATH) {
            item->mesh_bits = 0xffffffff;
            item->anim_number = objects[MONKEY].anim_index + MONKEY_DIE_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = MONKEY_DEATH;
        }
    } else {
        GetAITarget(monkey);
        if (monkey->hurt_by_lara)
            monkey->enemy = lara_item;
        if (item->ai_bits != MODIFY) {
            if (item->carried_item != NO_ITEM)
                item->mesh_bits = 0xfffffeff;
            else
                item->mesh_bits = 0xffffffff;
        } else {
            if (item->carried_item != NO_ITEM)
                item->mesh_bits = 0xffff6e6f;
            else
                item->mesh_bits = 0xffff6f6f;
        }
        CreatureAIInfo(item, &info);
        if (!monkey->hurt_by_lara && monkey->enemy == lara_item && objects[TIGER].loaded)
            monkey->enemy = NULL;
        if (monkey->enemy == lara_item) {
            lara_info.angle = info.angle;
            lara_info.distance = info.distance;
        } else {
            lara_dz = lara_item->pos.z_pos - item->pos.z_pos;
            lara_dx = lara_item->pos.x_pos - item->pos.x_pos;
            lara_info.angle = phd_atan(lara_dz, lara_dx) - item->pos.y_rot;
            lara_info.distance = lara_dz * lara_dz + lara_dx * lara_dx;
        }
        GetCreatureMood(item, &info, 1);
        if (lara.skidoo != NO_ITEM)
            monkey->mood = ESCAPE_MOOD;
        CreatureMood(item, &info, 1);
        angle = CreatureTurn(item, monkey->maximum_turn);
        enemy = monkey->enemy;
        monkey->enemy = lara_item;
        if (item->hit_status)
            AlertAllGuards(item_number);
        monkey->enemy = enemy;
        enemy = monkey->enemy;
        switch (item->current_anim_state) {
        case MONKEY_SIT:
            monkey->flags = 0;
            monkey->maximum_turn = 0;
            head = lara_info.angle;
            if (item->ai_bits & GUARD) {
                head = AIGuard(monkey);
                if (!(GetRandomControl() & 0xf)) {
                    if (GetRandomControl() & 1)
                        item->goal_anim_state = MONKEY_SCRATCH;
                    else
                        item->goal_anim_state = MONKEY_EAT;
                }
                break;
            } else if (item->ai_bits & PATROL1)
                item->goal_anim_state = MONKEY_WALK;
            else if (monkey->mood == ESCAPE_MOOD)
                item->goal_anim_state = MONKEY_STAND;
            else if (monkey->mood == BORED_MOOD) {
                if (item->required_anim_state)
                    item->goal_anim_state = item->required_anim_state;
                else if (!(GetRandomControl() & 0xf))
                    item->goal_anim_state = MONKEY_WALK;
                else if (!(GetRandomControl() & 0xf)) {
                    if (GetRandomControl() & 1)
                        item->goal_anim_state = MONKEY_SCRATCH;
                    else
                        item->goal_anim_state = MONKEY_EAT;
                }
            } else if ((item->ai_bits & FOLLOW) && (monkey->reached_goal || lara_info.distance > SQUARE(WALL_L * 2))) {
                if (item->required_anim_state)
                    item->goal_anim_state = item->required_anim_state;
                else if (info.ahead)
                    item->goal_anim_state = MONKEY_SIT;
                else
                    item->goal_anim_state = MONKEY_STAND;
            } else if (info.bite && info.distance < MONKEY_JUMP_RANGE)
                item->goal_anim_state = MONKEY_STAND;
            else if (info.bite && info.distance < MONKEY_WALK_RANGE)
                item->goal_anim_state = MONKEY_WALK;
            else
                item->goal_anim_state = MONKEY_STAND;
            break;
        case MONKEY_STAND:
            monkey->flags = 0;
            monkey->maximum_turn = 0;
            head = lara_info.angle;
            if (item->ai_bits & GUARD) {
                head = AIGuard(monkey);
                if (!(GetRandomControl() & 0xf)) {
                    if (GetRandomControl() & 1)
                        item->goal_anim_state = MONKEY_ANGRY;
                    else
                        item->goal_anim_state = MONKEY_SIT;
                }
                break;
            } else if (item->ai_bits & PATROL1)
                item->goal_anim_state = MONKEY_WALK;
            else if (monkey->mood == ESCAPE_MOOD) {
                if (lara.target != item && info.ahead)
                    item->goal_anim_state = MONKEY_STAND;
                else
                    item->goal_anim_state = MONKEY_RUN;
            } else if (monkey->mood == BORED_MOOD) {
                if (item->required_anim_state)
                    item->goal_anim_state = item->required_anim_state;
                else if (!(GetRandomControl() & 0xf))
                    item->goal_anim_state = MONKEY_WALK;
                else if (!(GetRandomControl() & 0xf)) {
                    if (GetRandomControl() & 1)
                        item->goal_anim_state = MONKEY_ANGRY;
                    else
                        item->goal_anim_state = MONKEY_SIT;
                }
            } else if ((item->ai_bits & FOLLOW) && (monkey->reached_goal || lara_info.distance > SQUARE(WALL_L * 2))) {
                if (item->required_anim_state)
                    item->goal_anim_state = item->required_anim_state;
                else if (info.ahead)
                    item->goal_anim_state = MONKEY_SIT;
                else
                    item->goal_anim_state = MONKEY_RUN;
            } else if (info.bite && info.distance < MONKEY_ATTACK_RANGE) {
                if (lara_item->pos.y_pos < item->pos.y_pos)
                    item->goal_anim_state = MONKEY_ATAK_HIGH;
                else
                    item->goal_anim_state = MONKEY_ATAK_LOW;
            } else if (info.bite && info.distance < MONKEY_JUMP_RANGE)
                item->goal_anim_state = MONKEY_ATAK_JUMP;
            else if (info.bite && info.distance < MONKEY_WALK_RANGE)
                item->goal_anim_state = MONKEY_WALK;
            else if (info.distance < MONKEY_WALK_RANGE && monkey->enemy != lara_item && monkey->enemy != NULL
                     && monkey->enemy->object_number != AI_PATROL1 && monkey->enemy->object_number != AI_PATROL2
                     && abs(item->pos.y_pos - monkey->enemy->pos.y_pos) < STEP_L)
                item->goal_anim_state = MONKEY_PICKUP;
            else if (info.bite && info.distance < MONKEY_ROLL_RANGE)
                item->goal_anim_state = MONKEY_ROLL;
            else
                item->goal_anim_state = MONKEY_RUN;
            break;
        case MONKEY_PICKUP:
            monkey->reached_goal = 1;
            if (monkey->enemy == NULL)
                break;
            else if ((monkey->enemy->object_number == MEDI_ITEM || monkey->enemy->object_number == KEY_ITEM4)
                     && item->frame_number == anims[item->anim_number].frame_base + 12) {
                if (monkey->enemy->room_number == NO_ROOM || monkey->enemy->status == INVISIBLE
                    || monkey->enemy->flags & KILLED_ITEM)
                    monkey->enemy = NULL;
                else {
                    i = monkey->enemy - items;
                    item->carried_item = i;
                    RemoveDrawnItem(i);
                    monkey->enemy->room_number = NO_ROOM;
                    monkey->enemy->carried_item = NO_ITEM;
                    cinfo = baddie_slots;
                    for (slot = 0; slot < NUM_SLOTS; slot++, cinfo++) {
                        if (cinfo->item_num == NO_ITEM || cinfo->item_num == item_number)
                            continue;
                        target = &items[cinfo->item_num];
                        if (cinfo->enemy == monkey->enemy)
                            cinfo->enemy = NULL;
                    }
                    monkey->enemy = NULL;
                    if (item->ai_bits != MODIFY) {
                        item->ai_bits |= AMBUSH;
                        item->ai_bits |= MODIFY;
                    }
                }
            } else if (monkey->enemy->object_number == AI_AMBUSH
                       && item->frame_number == anims[item->anim_number].frame_base + 12) {
                item->ai_bits = 0;
                pickup = &items[item->carried_item];
                pickup->pos.x_pos = item->pos.x_pos;
                pickup->pos.y_pos = item->pos.y_pos;
                pickup->pos.z_pos = item->pos.z_pos;
                ItemNewRoom(item->carried_item, item->room_number);
                item->carried_item = NO_ITEM;
                pickup->ai_bits = 1;
                monkey->enemy = NULL;
            } else {
                monkey->maximum_turn = 0;
                if (abs(info.angle) < MONKEY_WALK_TURN)
                    item->pos.y_rot += info.angle;
                else if (info.angle < 0)
                    item->pos.y_rot -= MONKEY_WALK_TURN;
                else
                    item->pos.y_rot += MONKEY_WALK_TURN;
            }
            break;
        case MONKEY_WALK:
            head = lara_info.angle;
            monkey->maximum_turn = MONKEY_WALK_TURN;
            if (item->ai_bits & PATROL1) {
                item->goal_anim_state = MONKEY_WALK;
                head = 0;
            } else if (monkey->mood == ESCAPE_MOOD)
                item->goal_anim_state = MONKEY_RUN;
            else if (monkey->mood == BORED_MOOD) {
                if (GetRandomControl() < MONKEY_WAIT_CHANCE)
                    item->goal_anim_state = MONKEY_SIT;
            } else if (info.bite && info.distance < MONKEY_JUMP_RANGE)
                item->goal_anim_state = MONKEY_STAND;
            else
                item->goal_anim_state = MONKEY_STAND;
            break;
        case MONKEY_RUN:
            if (info.ahead)
                head = info.angle;
            monkey->maximum_turn = MONKEY_RUN_TURN;
            tilt = angle / 2;
            if (item->ai_bits & GUARD)
                item->goal_anim_state = MONKEY_STAND;
            else if (monkey->mood == ESCAPE_MOOD) {
                if (lara.target != item && info.ahead)
                    item->goal_anim_state = MONKEY_STAND;
                break;
            } else if ((item->ai_bits & FOLLOW) && (monkey->reached_goal || lara_info.distance > SQUARE(WALL_L * 2)))
                item->goal_anim_state = MONKEY_STAND;
            else if (monkey->mood == BORED_MOOD)
                item->goal_anim_state = MONKEY_ROLL;
            else if (info.distance < MONKEY_WALK_RANGE)
                item->goal_anim_state = MONKEY_STAND;
            else if (info.bite && info.distance < MONKEY_ROLL_RANGE)
                item->goal_anim_state = MONKEY_ROLL;
            break;
        case MONKEY_ATAK_LOW:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            monkey->maximum_turn = 0;
            if (abs(info.angle) < MONKEY_WALK_TURN)
                item->pos.y_rot += info.angle;
            else if (info.angle < 0)
                item->pos.y_rot -= MONKEY_WALK_TURN;
            else
                item->pos.y_rot += MONKEY_WALK_TURN;
            if (enemy == lara_item) {
                if (!monkey->flags && (item->touch_bits & MONKEY_TOUCH)) {
                    lara_item->hit_points -= MONKEY_HIT_DAMAGE;
                    lara_item->hit_status = 1;
                    CreatureEffect(item, &monkey_hit, DoBloodSplat);
                    monkey->flags = 1;
                }
            } else if (!monkey->flags && enemy) {
                if (ABS(enemy->pos.x_pos - item->pos.x_pos) < MONKEY_HIT_RADIUS
                    && ABS(enemy->pos.y_pos - item->pos.y_pos) <= MONKEY_HIT_RADIUS
                    && ABS(enemy->pos.z_pos - item->pos.z_pos) < MONKEY_HIT_RADIUS) {
                    enemy->hit_points -= MONKEY_HIT_DAMAGE >> 1;
                    enemy->hit_status = 1;
                    monkey->flags = 1;
                    CreatureEffect(item, &monkey_hit, DoBloodSplat);
                }
            }
            break;
        case MONKEY_ATAK_HIGH:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            monkey->maximum_turn = 0;
            if (abs(info.angle) < MONKEY_WALK_TURN)
                item->pos.y_rot += info.angle;
            else if (info.angle < 0)
                item->pos.y_rot -= MONKEY_WALK_TURN;
            else
                item->pos.y_rot += MONKEY_WALK_TURN;
            if (enemy == lara_item) {
                if (!monkey->flags && (item->touch_bits & MONKEY_TOUCH)) {
                    lara_item->hit_points -= MONKEY_HIT_DAMAGE;
                    lara_item->hit_status = 1;
                    CreatureEffect(item, &monkey_hit, DoBloodSplat);
                    monkey->flags = 1;
                }
            } else if (!monkey->flags && enemy) {
                if (ABS(enemy->pos.x_pos - item->pos.x_pos) < MONKEY_HIT_RADIUS
                    && ABS(enemy->pos.y_pos - item->pos.y_pos) <= MONKEY_HIT_RADIUS
                    && ABS(enemy->pos.z_pos - item->pos.z_pos) < MONKEY_HIT_RADIUS) {
                    enemy->hit_points -= MONKEY_HIT_DAMAGE >> 1;
                    enemy->hit_status = 1;
                    monkey->flags = 1;
                    CreatureEffect(item, &monkey_hit, DoBloodSplat);
                }
            }
        case MONKEY_ATAK_JUMP:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            monkey->maximum_turn = 0;
            if (abs(info.angle) < MONKEY_WALK_TURN)
                item->pos.y_rot += info.angle;
            else if (info.angle < 0)
                item->pos.y_rot -= MONKEY_WALK_TURN;
            else
                item->pos.y_rot += MONKEY_WALK_TURN;
            if (enemy == lara_item) {
                if (monkey->flags != 1 && (item->touch_bits & MONKEY_TOUCH)) {
                    lara_item->hit_points -= MONKEY_SWIPE_DAMAGE;
                    lara_item->hit_status = 1;
                    CreatureEffect(item, &monkey_hit, DoBloodSplat);
                    monkey->flags = 1;
                }
            } else if (monkey->flags != 1 && enemy) {
                if (ABS(enemy->pos.x_pos - item->pos.x_pos) < MONKEY_HIT_RADIUS
                    && ABS(enemy->pos.y_pos - item->pos.y_pos) <= MONKEY_HIT_RADIUS
                    && ABS(enemy->pos.z_pos - item->pos.z_pos) < MONKEY_HIT_RADIUS) {
                    enemy->hit_points -= MONKEY_SWIPE_DAMAGE >> 1;
                    enemy->hit_status = 1;
                    monkey->flags = 1;
                    CreatureEffect(item, &monkey_hit, DoBloodSplat);
                }
            }
            break;
        }
    }
    CreatureTilt(item, tilt);
    CreatureJoint(item, 0, torso_y);
    if (item->current_anim_state < MONKEY_CLIMB4) {
        switch (CreatureVault(item_number, angle, 2, MONKEY_VAULT_SHIFT)) {
        case 2:
            monkey->maximum_turn = 0;
            item->anim_number = objects[MONKEY].anim_index + MONKEY_CLIMB2_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = MONKEY_CLIMB2;
            break;
        case 3:
            monkey->maximum_turn = 0;
            item->anim_number = objects[MONKEY].anim_index + MONKEY_CLIMB3_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = MONKEY_CLIMB3;
            break;
        case 4:
            monkey->maximum_turn = 0;
            item->anim_number = objects[MONKEY].anim_index + MONKEY_CLIMB4_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = MONKEY_CLIMB4;
            break;
        case -2:
            monkey->maximum_turn = 0;
            item->anim_number = objects[MONKEY].anim_index + MONKEY_DOWN2_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = MONKEY_DOWN2;
            break;
        case -3:
            monkey->maximum_turn = 0;
            item->anim_number = objects[MONKEY].anim_index + MONKEY_DOWN3_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = MONKEY_DOWN3;
            break;
        case -4:
            monkey->maximum_turn = 0;
            item->anim_number = objects[MONKEY].anim_index + MONKEY_DOWN4_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = MONKEY_DOWN4;
            break;
        }
    } else {
        monkey->maximum_turn = 0;
        CreatureAnimation(item_number, angle, 0);
    }
}

#ifdef TR3_EXE
BITE_INFO monkey_hit = { 10, 10, 11, 13 };
#endif
