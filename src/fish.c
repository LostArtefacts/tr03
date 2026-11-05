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

extern uint32 CurrentLevel;
extern uint8 jungle_fish_ranges[][3];
extern uint8 temple_fish_ranges[][3];
extern uint8 quadchase_fish_ranges[][3];
extern uint8 house_fish_ranges[][3];
extern uint8 shore_fish_ranges[][3];
extern uint8 crash_fish_ranges[][3];
extern uint8 rapids_fish_ranges[][3];

void SetupShoal(sint32 shoal_number)
{
    if (CurrentLevel == 1) {
        lead_info[shoal_number].Xrange = (jungle_fish_ranges[shoal_number][0] + 2) << 8;
        lead_info[shoal_number].Zrange = (jungle_fish_ranges[shoal_number][1] + 2) << 8;
        lead_info[shoal_number].Yrange = jungle_fish_ranges[shoal_number][2] << 8;
    } else if (CurrentLevel == 2) {
        lead_info[shoal_number].Xrange = (temple_fish_ranges[shoal_number][0] + 2) << 8;
        lead_info[shoal_number].Zrange = (temple_fish_ranges[shoal_number][1] + 2) << 8;
        lead_info[shoal_number].Yrange = temple_fish_ranges[shoal_number][2] << 8;
    } else if (CurrentLevel == 3) {
        lead_info[shoal_number].Xrange = (quadchase_fish_ranges[shoal_number][0] + 2) << 8;
        lead_info[shoal_number].Zrange = (quadchase_fish_ranges[shoal_number][1] + 2) << 8;
        lead_info[shoal_number].Yrange = quadchase_fish_ranges[shoal_number][2] << 8;
    } else if (CurrentLevel == 0) {
        lead_info[shoal_number].Xrange = (house_fish_ranges[shoal_number][0] + 2) << 8;
        lead_info[shoal_number].Zrange = (house_fish_ranges[shoal_number][1] + 2) << 8;
        lead_info[shoal_number].Yrange = house_fish_ranges[shoal_number][2] << 8;
    } else if (CurrentLevel == 5) {
        lead_info[shoal_number].Xrange = (shore_fish_ranges[shoal_number][0] + 2) << 8;
        lead_info[shoal_number].Zrange = (shore_fish_ranges[shoal_number][1] + 2) << 8;
        lead_info[shoal_number].Yrange = shore_fish_ranges[shoal_number][2] << 8;
    } else if (CurrentLevel == 6) {
        lead_info[shoal_number].Xrange = (crash_fish_ranges[shoal_number][0] + 2) << 8;
        lead_info[shoal_number].Zrange = (crash_fish_ranges[shoal_number][1] + 2) << 8;
        lead_info[shoal_number].Yrange = crash_fish_ranges[shoal_number][2] << 8;
    } else if (CurrentLevel == 7) {
        lead_info[shoal_number].Xrange = (rapids_fish_ranges[shoal_number][0] + 2) << 8;
        lead_info[shoal_number].Zrange = (rapids_fish_ranges[shoal_number][1] + 2) << 8;
        lead_info[shoal_number].Yrange = rapids_fish_ranges[shoal_number][2] << 8;
    } else {
        lead_info[shoal_number].Xrange = 1 << 8;
        lead_info[shoal_number].Zrange = 1 << 8;
        lead_info[shoal_number].Yrange = 1 << 8;
    }
}

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
