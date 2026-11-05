#include "game.h"

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

#define DINO_STOP 1
#define DINO_WALK 2
#define DINO_RUN 3
#define DINO_DEATH 5
#define DINO_ROAR 6
#define DINO_ATTACK2 7
#define DINO_KILL 8
#define DINO_LONGROARSTART 9
#define DINO_LONGROAREND 11
#define DINO_SNIFFSTART 12
#define DINO_SNIFFMID 13
#define DINO_SNIFFEND 14

#define SQUARE(x) ((x) * (x))
#define DINO_TOUCH_DAMAGE 1
#define DINO_TRAMPLE_DAMAGE 10
#define DINO_BITE_DAMAGE 10000
#define DINO_TOUCH 0x3000
#define DINO_KILL_ANIM 11
#define DINO_ROAR_CHANCE 0x100
#define DINO_RUN_TURN (ONE_DEGREE * 4)
#define DINO_WALK_TURN (ONE_DEGREE * 2)
#define DINO_RUN_RANGE SQUARE(WALL_L * 5)
#define DINO_ATTACK_RANGE SQUARE(WALL_L * 4)
#define DINO_BITE_RANGE SQUARE(1500)
#define DINO_HIT_RADIUS SQUARE(WALL_L * 2 / 3)
#define FRONT_ARC 0x4000

int CreatureActive(sint16 item_number);
void CreatureAIInfo(ITEM_INFO *item, AI_INFO *info);
void GetCreatureMood(ITEM_INFO *item, AI_INFO *info, int violent);
void CreatureMood(ITEM_INFO *item, AI_INFO *info, int violent);
sint16 CreatureTurn(ITEM_INFO *item, sint16 maximum_turn);
int CreatureAnimation(sint16 item_number, sint16 angle, sint16 tilt);
void CreatureKill(ITEM_INFO *item, int kill_anim, int kill_state, int lara_kill_state);
sint16 CreatureEffect(ITEM_INFO *item, BITE_INFO *bite, sint16 (*generate)(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 yrot, sint16 room_number));
sint16 DoBloodSplat(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num);
void GetNearByRooms(sint32 x, sint32 y, sint32 z, sint32 r, sint32 h, sint16 room_number);

extern sint16 draw_rooms[];
extern int number_draw_rooms;
extern BITE_INFO trex_hit;

void DinoControl(sint16 item_number)
{
    ITEM_INFO *item, *best, *target;
    CREATURE_INFO *dino;
    sint16 head, neck, angle;
    sint32 x, y, z, best_distance, distance;
    long lp, target_number;
    AI_INFO info;

    if (!CreatureActive(item_number))
        return;
    item = &items[item_number];
    dino = (CREATURE_INFO *)item->data;
    head = neck = angle = 0;
    if (item->hit_points <= 0) {
        if (item->current_anim_state == DINO_STOP)
            item->goal_anim_state = DINO_DEATH;
        else
            item->goal_anim_state = DINO_STOP;
    } else {
        best = NULL;
        best_distance = 0x7fffffff;
        GetNearByRooms(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 4096, 0, item->room_number);
        for (lp = 0; lp < number_draw_rooms; lp++) {
            for (target_number = room[draw_rooms[lp]].item_number; target_number != NO_ITEM;
                 target_number = target->next_item) {
                target = &items[target_number];
                if ((target->object_number != 179 && target->object_number != 288) || !target->hit_points
                    || target->status != ACTIVE)
                    continue;
                x = (target->pos.x_pos - item->pos.x_pos) >> 6;
                y = (target->pos.y_pos - item->pos.y_pos) >> 6;
                z = (target->pos.z_pos - item->pos.z_pos) >> 6;
                distance = x * x + y * y + z * z;
                if (distance < best_distance) {
                    best = target;
                    best_distance = distance;
                }
            }
        }
        dino->enemy = best;
        if (dino->hurt_by_lara)
            dino->enemy = lara_item;
        CreatureAIInfo(item, &info);
        if (info.ahead)
            head = info.angle;
        GetCreatureMood(item, &info, 1);
        if (!item->item_flags[0] && !item->item_flags[1] && dino->enemy == lara_item)
            dino->mood = BORED_MOOD;
        CreatureMood(item, &info, 1);
        if (dino->mood == BORED_MOOD)
            dino->maximum_turn >>= 1;
        angle = CreatureTurn(item, dino->maximum_turn);
        neck = -(angle * 10);
        if (item->touch_bits)
            lara_item->hit_points -= (item->current_anim_state == DINO_RUN) ? DINO_TRAMPLE_DAMAGE : DINO_TOUCH_DAMAGE;
        dino->flags = (dino->mood != ESCAPE_MOOD && !info.ahead && info.enemy_facing > -FRONT_ARC
                       && info.enemy_facing < FRONT_ARC);
        if (!dino->flags && info.distance > DINO_BITE_RANGE && info.distance < DINO_ATTACK_RANGE && info.bite)
            dino->flags = 1;
        if (lara.gun_type != LG_FLARE
            && (lara_item->current_anim_state == AS_STOP || lara_item->current_anim_state == AS_DUCK)
            && lara_item->goal_anim_state == lara_item->current_anim_state && !item->hit_status) {
            if (item->item_flags[0] > 0)
                item->item_flags[0]--;
        } else {
            item->item_flags[0] = 120;
            item->item_flags[1] = 3;
        }
        switch (item->current_anim_state) {
        case DINO_STOP:
            if (item->required_anim_state)
                item->goal_anim_state = item->required_anim_state;
            else if (dino->mood == BORED_MOOD || dino->flags)
                item->goal_anim_state = DINO_WALK;
            else if (dino->mood == ESCAPE_MOOD) {
                if (lara.target != item && info.ahead && !item->hit_status) {
                    if (GetRandomControl() < DINO_ROAR_CHANCE)
                        item->goal_anim_state = DINO_ROAR;
                    else
                        item->goal_anim_state = DINO_STOP;
                } else
                    item->goal_anim_state = DINO_RUN;
            } else if (info.distance < DINO_BITE_RANGE && info.bite) {
                if (item->item_flags[0])
                    item->goal_anim_state = DINO_ATTACK2;
                else if ((GetRandomControl() & 1) && item->item_flags[1])
                    item->goal_anim_state = DINO_LONGROARSTART;
                else if (item->item_flags[1])
                    item->goal_anim_state = DINO_SNIFFSTART;
            } else
                item->goal_anim_state = DINO_RUN;
            break;
        case DINO_WALK:
            dino->maximum_turn = DINO_WALK_TURN;
            if (dino->mood != BORED_MOOD || !dino->flags)
                item->goal_anim_state = DINO_STOP;
            else if (info.ahead && GetRandomControl() < DINO_ROAR_CHANCE) {
                item->required_anim_state = DINO_ROAR;
                item->goal_anim_state = DINO_STOP;
            }
            break;
        case DINO_RUN:
            dino->maximum_turn = DINO_RUN_TURN;
            if (info.distance < DINO_RUN_RANGE && info.bite)
                item->goal_anim_state = DINO_STOP;
            else if (dino->flags)
                item->goal_anim_state = DINO_STOP;
            else if (dino->mood != ESCAPE_MOOD && info.ahead && GetRandomControl() < DINO_ROAR_CHANCE) {
                item->required_anim_state = DINO_ROAR;
                item->goal_anim_state = DINO_STOP;
            } else if (dino->mood == BORED_MOOD)
                item->goal_anim_state = DINO_STOP;
            else if (dino->mood == ESCAPE_MOOD && lara.target != item && info.ahead)
                item->goal_anim_state = DINO_STOP;
            break;
        case DINO_ATTACK2:
            dino->maximum_turn = DINO_WALK_TURN;
            if (dino->enemy == lara_item) {
                if (item->touch_bits & DINO_TOUCH) {
                    CreatureEffect(item, &trex_hit, DoBloodSplat);
                    lara_item->hit_points -= DINO_BITE_DAMAGE;
                    lara_item->hit_status = 1;
                    item->goal_anim_state = DINO_KILL;
                    dino->maximum_turn = 0;
                    CreatureKill(item, DINO_KILL_ANIM, DINO_KILL, 6);
                }
            } else if (dino->enemy && item->frame_number == anims[item->anim_number].frame_base + 20) {
                if (ABS(dino->enemy->pos.z_pos
                        - (item->pos.z_pos + (objects[287].pivot_length * phd_cos(item->pos.y_rot) >> W2V_SHIFT)))
                        < DINO_HIT_RADIUS
                    && ABS(dino->enemy->pos.y_pos - item->pos.y_pos) <= DINO_HIT_RADIUS
                    && ABS(dino->enemy->pos.x_pos
                           - (item->pos.x_pos + (objects[287].pivot_length * phd_sin(item->pos.y_rot) >> W2V_SHIFT)))
                        < DINO_HIT_RADIUS) {
                    if (dino->enemy->object_number == 179)
                        dino->enemy->hit_points = 0;
                    else {
                        dino->enemy->hit_points -= DINO_TRAMPLE_DAMAGE * 5;
                        dino->enemy->hit_status = 1;
                        CreatureEffect(item, &trex_hit, DoBloodSplat);
                    }
                }
            }
            if (!(GetRandomControl() & 3))
                item->required_anim_state = DINO_WALK;
            break;
        case DINO_SNIFFSTART:
            dino->maximum_turn = 0;
            if (item->item_flags[1] > 0 && item->frame_number == anims[item->anim_number].frame_base) {
                item->item_flags[1]--;
                if (dino->enemy && dino->enemy->object_number == 179) {
                    dino->enemy->hit_points = 0;
                    item->item_flags[1] = 0;
                }
            }
            break;
        case DINO_SNIFFMID:
            dino->maximum_turn = 0;
            if (item->frame_number == anims[item->anim_number].frame_base) {
                if ((GetRandomControl() & 1) && item->item_flags[1] && !item->item_flags[0]) {
                    item->goal_anim_state = DINO_SNIFFMID;
                    if (item->item_flags[1] > 0)
                        item->item_flags[1]--;
                } else
                    item->goal_anim_state = DINO_SNIFFEND;
            }
            break;
        case DINO_LONGROARSTART:
            dino->maximum_turn = 0;
            if (item->item_flags[1] > 0 && item->frame_number == anims[item->anim_number].frame_base) {
                item->item_flags[1]--;
                if (dino->enemy && dino->enemy->object_number == 179) {
                    dino->enemy->hit_points = 0;
                    item->item_flags[1]--;
                }
            }
            item->goal_anim_state = DINO_LONGROAREND;
            break;
        case DINO_ROAR:
            dino->maximum_turn = 0;
            break;
        case DINO_KILL:
            dino->maximum_turn = 0;
            CreatureEffect(item, &trex_hit, DoBloodSplat);
            break;
        }
    }
    CreatureAnimation(item_number, angle, 0);
    if (item->hit_points <= 0)
        item->collidable = 0;
    else
        item->collidable = 1;
}
