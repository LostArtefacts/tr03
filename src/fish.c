#include "game.h"

#define MAX_FISH 8

typedef struct {
    sint16 x;
    sint16 y;
    sint16 z;
    uint16 angle;
    sint16 desty;
    sint8 angadd;
    uint8 speed;
    uint8 acc;
    uint8 swim;
} FISH_INFO;

typedef struct {
    sint16 angle;
    uint8 speed;
    uint8 on;
    sint16 angle_time;
    sint16 speed_time;
    sint16 Xrange;
    sint16 Zrange;
    sint16 Yrange;
} LEADER_INFO;

extern FISH_INFO fish[];
extern LEADER_INFO lead_info[];

void SetupFish(sint32 leader, ITEM_INFO *item)
{
    sint32 lp, fish_Xrange, fish_Zrange, fish_Yrange;

    fish_Xrange = lead_info[leader].Xrange;
    fish_Yrange = lead_info[leader].Yrange;
    fish_Zrange = lead_info[leader].Zrange;
    fish[leader].x = 0;
    fish[leader].y = 0;
    fish[leader].z = 0;
    fish[leader].angle = 0;
    fish[leader].speed = (GetRandomControl() & 63) + 8;
    fish[leader].swim = GetRandomControl() & 63;
    for (lp = 0; lp < 24; lp++) {
        fish[MAX_FISH + leader * 24 + lp].x = (GetRandomControl() % (fish_Xrange << 1)) - fish_Xrange;
        fish[MAX_FISH + leader * 24 + lp].y = GetRandomControl() % fish_Yrange;
        fish[MAX_FISH + leader * 24 + lp].desty = GetRandomControl() % fish_Yrange;
        fish[MAX_FISH + leader * 24 + lp].z = (GetRandomControl() % (fish_Zrange << 1)) - fish_Zrange;
        fish[MAX_FISH + leader * 24 + lp].angle = GetRandomControl() & 4095;
        fish[MAX_FISH + leader * 24 + lp].speed = (GetRandomControl() & 31) + 32;
        fish[MAX_FISH + leader * 24 + lp].swim = GetRandomControl() & 63;
    }
    lead_info[leader].on = 1;
    lead_info[leader].angle = 0;
    lead_info[leader].speed = (GetRandomControl() & 127) + 32;
    lead_info[leader].angle_time = 0;
    lead_info[leader].speed_time = 0;
}
