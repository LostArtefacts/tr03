#ifndef SAVEGAME_H
#define SAVEGAME_H

#include "types.h"

#define MAX_SAVEGAME_BUFFER 12416
#define MAX_TIME_ALLOWED ((59 * 60 * 30) + (59 * 30) + 27)

typedef struct start_info {
    uint16 pistol_ammo, magnum_ammo, uzi_ammo, shotgun_ammo, m16_ammo, rocket_ammo, harpoon_ammo, grenade_ammo;
    unsigned char num_medis, num_big_medis, num_scions, num_flares, num_sgcrystals;
    char gun_status, gun_type;
    uint16 available:1;
    uint16 got_pistols:1;
    uint16 got_magnums:1;
    uint16 got_uzis:1;
    uint16 got_shotgun:1;
    uint16 got_m16:1;
    uint16 got_rocket:1;
    uint16 got_grenade:1;
    uint16 got_harpoon:1;
    uint16 secrets;
    unsigned char num_icon1, num_icon2, num_icon3, num_icon4;
    uint32 timer;
    uint32 ammo_used, ammo_hit;
    uint32 distance_travelled;
    uint16 kills;
    unsigned char secrets_found;
    unsigned char health_used;
} START_INFO;

typedef struct savegame_info {
    uint32 best_assault_times[10];
    uint32 best_quadbike_times[10];
    uint32 QuadbikeKeyFlag;
    START_INFO start[25];
    uint32 timer;
    uint32 ammo_used, ammo_hit;
    uint32 distance_travelled;
    uint16 kills;
    unsigned char secrets;
    unsigned char health_used;
    uint32 cp_timer;
    uint32 cp_ammo_used, cp_ammo_hit;
    uint32 cp_distance_travelled;
    uint16 cp_kills;
    unsigned char cp_secrets;
    unsigned char cp_health_used;
    sint16 current_level;
    unsigned char bonus_flag;
    unsigned char num_pickup1, num_pickup2;
    unsigned char num_puzzle1, num_puzzle2, num_puzzle3, num_puzzle4;
    unsigned char num_key1, num_key2, num_key3, num_key4;
    signed char checksum;
    char GameComplete;
    uint32 mid_level_save:1;
    uint32 AfterAdventureSave:1;
    uint32 WorldRequired:3;
    uint32 IndiaComplete:1;
    uint32 SPacificComplete:1;
    uint32 LondonComplete:1;
    uint32 NevadaComplete:1;
    uint32 AntarcticaComplete:1;
    uint32 PeruComplete:1;
    uint32 AfterIndia:3;
    uint32 AfterSPacific:3;
    uint32 AfterLondon:3;
    uint32 AfterNevada:3;
    char buffer[MAX_SAVEGAME_BUFFER];
} SAVEGAME_INFO;

extern SAVEGAME_INFO savegame;

void ResetSG(void);
void WriteSG(void *pointer, int size);
void ReadSG(void *pointer, int size);

sint32 AddAssaultTime(uint32 time);
sint32 AddQuadbikeTime(uint32 time);

#endif
