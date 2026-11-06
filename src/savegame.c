#include "specific/standard.h"
#include <mmreg.h>
#include "game.h"

extern int SGcount;
extern char *SGpoint;

void ModifyStartInfo(int level_number);

void InitialiseStartInfo(void)
{
    int i;
    FILE *file;

    if (savegame.bonus_flag)
        return;

    for (i = 0; i < 24; i++) {
        ModifyStartInfo(i);
        savegame.start[i].available = 0;
        savegame.start[i].timer = 0;
        savegame.start[i].ammo_used = 0;
        savegame.start[i].ammo_hit = 0;
        savegame.start[i].distance_travelled = 0;
        savegame.start[i].kills = 0;
        savegame.start[i].secrets_found = 0;
        savegame.start[i].health_used = 0;
    }

    for (i = 0; i < 10; i++) {
        savegame.best_assault_times[i] = 0;
        savegame.best_quadbike_times[i] = 0;
    }

    savegame.AfterAdventureSave = 0;
    savegame.WorldRequired = 0;
    savegame.IndiaComplete = 0;
    savegame.SPacificComplete = 0;
    savegame.NevadaComplete = 0;
    savegame.LondonComplete = 0;
    savegame.AntarcticaComplete = 0;
    savegame.PeruComplete = 0;
    savegame.AfterIndia = 0;
    savegame.AfterSPacific = 0;
    savegame.AfterLondon = 0;
    savegame.AfterNevada = 0;
    savegame.QuadbikeKeyFlag = 0;
    savegame.start[0].available = 1;
    savegame.start[1].available = 1;
    savegame.bonus_flag = 0;
    savegame.GameComplete = 0;

    file = fopen("data.bin", "rb");
    if (file) {
        fread(savegame.best_assault_times, 4, 10, file);
        fread(savegame.best_quadbike_times, 4, 10, file);
        fread(&savegame.QuadbikeKeyFlag, 4, 1, file);
    }
}

void ModifyStartInfo(int level_number)
{
    START_INFO *start;

    start = &savegame.start[level_number];
    start->got_pistols = 1;
    start->gun_type = LG_PISTOLS;
    start->pistol_ammo = 1000;

    switch (level_number) {
    case 0:
        start->available = 1;
        start->pistol_ammo = start->magnum_ammo = start->uzi_ammo = start->shotgun_ammo = start->harpoon_ammo
            = start->rocket_ammo = start->grenade_ammo = start->m16_ammo = start->harpoon_ammo = start->rocket_ammo = 0;
        start->got_pistols = start->got_magnums = start->got_uzis = start->got_shotgun = start->got_harpoon
            = start->got_rocket = start->got_grenade = start->got_m16 = start->got_harpoon = 0;
        start->num_medis = start->num_big_medis = start->num_flares = 0;
        start->num_sgcrystals = 1;
        start->gun_type = LG_UNARMED;
        start->gun_status = LG_ARMLESS;
        break;

    case 1:
        start->available = 1;
        start->num_flares = 2;
        start->num_medis = 1;
        start->num_big_medis = 1;
        start->num_sgcrystals = 1;
        start->got_magnums = start->got_uzis = start->got_m16 = start->got_harpoon = start->got_rocket
            = start->got_grenade = 0;
        start->magnum_ammo = start->uzi_ammo = start->m16_ammo = start->harpoon_ammo = start->rocket_ammo
            = start->grenade_ammo = 0;
        start->gun_status = LG_ARMLESS;
        break;
    }

    if (savegame.bonus_flag && level_number != 0) {
        start->available = 1;
        start->got_pistols = start->got_magnums = start->got_uzis = start->got_shotgun = start->got_rocket
            = start->got_grenade = start->got_m16 = start->got_harpoon = 1;
        start->magnum_ammo = start->uzi_ammo = start->shotgun_ammo = start->rocket_ammo = start->grenade_ammo
            = start->m16_ammo = start->harpoon_ammo = 10000;
        start->gun_type = LG_ROCKET;
        start->num_flares = 255;
        start->num_sgcrystals = 50;
    }
}

int Inv_RequestItem(int item_num);

void CreateStartInfo(int level_number)
{
    START_INFO *start;

    start = &savegame.start[level_number];
    start->available = 1;

    if (Inv_RequestItem(160)) {
        start->pistol_ammo = 1000;
        start->got_pistols = 1;
    } else {
        start->pistol_ammo = 1000;
        start->got_pistols = 0;
    }

    if (Inv_RequestItem(162)) {
        start->magnum_ammo = lara.magnums.ammo;
        start->got_magnums = 1;
    } else {
        start->magnum_ammo = Inv_RequestItem(170) * 10;
        start->got_magnums = 0;
    }

    if (Inv_RequestItem(163)) {
        start->uzi_ammo = lara.uzis.ammo;
        start->got_uzis = 1;
    } else {
        start->uzi_ammo = Inv_RequestItem(171) * 40;
        start->got_uzis = 0;
    }

    if (Inv_RequestItem(161)) {
        start->shotgun_ammo = lara.shotgun.ammo;
        start->got_shotgun = 1;
    } else {
        start->shotgun_ammo = Inv_RequestItem(169) * 12;
        start->got_shotgun = 0;
    }

    if (Inv_RequestItem(164)) {
        start->harpoon_ammo = lara.harpoon.ammo;
        start->got_harpoon = 1;
    } else {
        start->harpoon_ammo = lara.harpoon.ammo;
        start->got_harpoon = 0;
    }

    if (Inv_RequestItem(165)) {
        start->m16_ammo = lara.m16.ammo;
        start->got_m16 = 1;
    } else {
        start->m16_ammo = Inv_RequestItem(173) * 60;
        start->got_m16 = 0;
    }

    if (Inv_RequestItem(166)) {
        start->rocket_ammo = lara.rocket.ammo;
        start->got_rocket = 1;
    } else {
        start->rocket_ammo = Inv_RequestItem(174) * 1;
        start->got_rocket = 0;
    }

    if (Inv_RequestItem(167)) {
        start->grenade_ammo = lara.grenade.ammo;
        start->got_grenade = 1;
    } else {
        start->grenade_ammo = Inv_RequestItem(175) * 2;
        start->got_grenade = 0;
    }

    start->num_flares = Inv_RequestItem(179);
    start->num_medis = Inv_RequestItem(176);
    start->num_big_medis = Inv_RequestItem(177);
    start->num_sgcrystals = Inv_RequestItem(180);
    start->num_icon1 = Inv_RequestItem(240);
    start->num_icon2 = Inv_RequestItem(241);
    start->num_icon3 = Inv_RequestItem(242);
    start->num_icon4 = Inv_RequestItem(243);
    if (lara.gun_type == LG_FLARE)
        start->gun_type = (char)lara.last_gun_type;
    else
        start->gun_type = (char)lara.gun_type;
    start->gun_status = LG_ARMLESS;
}

extern uint32 CurrentLevel;
extern int flipmap[];
extern char cd_flags[];
extern int number_cameras;
extern sint32 level_items;
extern sint16 *mesh_base;
extern sint16 *frames;
extern uint8 CurrentAtmosphere;
extern sint32 compy_scared_timer;
extern sint32 compys_attack_lara;
extern sint16 CarcassItem;
extern char TribeBossShieldOn;
extern ITEM_INFO *TribeBossItem;
extern char lizard_man_active;

#define SAVE_CREATURE (1 << 31)

void ResetSG(void);
void WriteSG(void *pointer, int size);

void CreateSaveGameInfo(void)
{
    int i, texplode;
    char tflag;
    ITEM_INFO *item;
    OBJECT_INFO *obj;
    char noresetexplode = 0;
    int number_flares, flare_age;

    savegame.current_level = (sint16)CurrentLevel;

    CreateStartInfo(CurrentLevel);

    savegame.num_pickup1 = Inv_RequestItem(236);
    savegame.num_pickup2 = Inv_RequestItem(237);
    savegame.num_puzzle1 = Inv_RequestItem(205);
    savegame.num_puzzle2 = Inv_RequestItem(206);
    savegame.num_puzzle3 = Inv_RequestItem(207);
    savegame.num_puzzle4 = Inv_RequestItem(208);
    savegame.num_key1 = Inv_RequestItem(224);
    savegame.num_key2 = Inv_RequestItem(225);
    savegame.num_key3 = Inv_RequestItem(226);
    savegame.num_key4 = Inv_RequestItem(227);

    ResetSG();

    for (i = 0; i < MAX_SAVEGAME_BUFFER; i++)
        savegame.buffer[i] = 0;

    WriteSG(&flip_status, sizeof(int));

    for (i = 0; i < 10; i++) {
        tflag = (char)(flipmap[i] >> 8);
        WriteSG(&tflag, sizeof(char));
    }

    WriteSG(cd_flags, sizeof(char) * 128);

    for (i = 0; i < number_cameras; i++)
        WriteSG(&camera.fixed[i].flags, sizeof(sint16));

    for (i = 0, item = items; i < level_items; i++, item++) {
        obj = &objects[item->object_number];

        if ((item->object_number == 73 && item->hit_points > 0 && bossdata.explode_count) || bossdata.dead)
            noresetexplode = 1;

        if (obj->save_position) {
            WriteSG(&item->pos, sizeof(PHD_3DPOS));
            WriteSG(&item->room_number, sizeof(sint16));
            WriteSG(&item->speed, sizeof(sint16));
            WriteSG(&item->fallspeed, sizeof(sint16));
        }

        if (obj->save_anim) {
            WriteSG(&item->current_anim_state, sizeof(sint16));
            WriteSG(&item->goal_anim_state, sizeof(sint16));
            WriteSG(&item->required_anim_state, sizeof(sint16));
            WriteSG(&item->anim_number, sizeof(sint16));
            WriteSG(&item->frame_number, sizeof(sint16));
        }

        if (obj->save_hitpoints)
            WriteSG(&item->hit_points, sizeof(sint16));

        if (obj->save_flags) {
            uint32 flags = 0;

            flags = ((uint16)item->flags) | (item->active << 16)
                | (item->status << 17) | (item->gravity_status << 19)
                | (item->hit_status << 20) | (item->collidable << 21)
                | (item->looked_at << 22) | (item->dynamic_light << 23)
                | (item->clear_body << 24) | (item->ai_bits << 25)
                | (item->really_active << 30);

            if (obj->intelligent && item->data)
                flags |= SAVE_CREATURE;

            WriteSG(&flags, sizeof(uint32));

            if (obj->intelligent)
                WriteSG(&item->carried_item, sizeof(sint16));

            WriteSG(&item->timer, sizeof(sint16));

            if (flags & SAVE_CREATURE)
                WriteSG((CREATURE_INFO *)item->data, 18);

            WriteSG(&item->item_flags[0], sizeof(sint16) * 4);
        }

        if (item->object_number == 18)
            WriteSG(item->data, 8);

        if (item->object_number == 15)
            WriteSG(item->data, 26);

        if (item->object_number == 14)
            WriteSG(item->data, 46);

        if (item->object_number == 17)
            WriteSG(item->data, 30);

        if (item->object_number == 16)
            WriteSG(item->data, 39);

        if (item->object_number == 19)
            WriteSG(item->data, 16);

        if (item->object_number == 291)
            WriteSG(&item->shadeB, sizeof(sint16));
    }

    texplode = bossdata.explode_count;
    if (noresetexplode == 0)
        bossdata.explode_count = 0;
    WriteSG(&bossdata, sizeof(BOSS_STRUCT));
    bossdata.explode_count = texplode;

    for (i = 0; i < 15; i++)
        lara.mesh_ptrs[i] = (sint16 *)((long)lara.mesh_ptrs[i] - (long)mesh_base);
    lara.left_arm.frame_base = (sint16 *)((long)lara.left_arm.frame_base - (long)frames);
    lara.right_arm.frame_base = (sint16 *)((long)lara.right_arm.frame_base - (long)frames);

    WriteSG(&lara, sizeof(LARA_INFO));

    for (i = 0; i < 15; i++)
        lara.mesh_ptrs[i] = (sint16 *)((long)lara.mesh_ptrs[i] + (long)mesh_base);
    lara.left_arm.frame_base = (sint16 *)((long)lara.left_arm.frame_base + (long)frames);
    lara.right_arm.frame_base = (sint16 *)((long)lara.right_arm.frame_base + (long)frames);

    if (lara.weapon_item != NO_ITEM) {
        item = &items[lara.weapon_item];
        WriteSG(&item->object_number, sizeof(sint16));
        WriteSG(&item->anim_number, sizeof(sint16));
        WriteSG(&item->frame_number, sizeof(sint16));
        WriteSG(&item->current_anim_state, sizeof(sint16));
        WriteSG(&item->goal_anim_state, sizeof(sint16));
    }

    WriteSG(&flipeffect, sizeof(int));
    WriteSG(&fliptimer, sizeof(int));

    WriteSG(&CurrentAtmosphere, sizeof(char));
    WriteSG(&compy_scared_timer, sizeof(int));
    WriteSG(&compys_attack_lara, sizeof(int));
    WriteSG(&CarcassItem, sizeof(sint16));
    WriteSG(&TribeBossShieldOn, sizeof(char));
    WriteSG(&TribeBossItem, sizeof(sint32));
    WriteSG(&lizard_man_active, sizeof(char));

    number_flares = 0;
    for (i = level_items, item = &items[level_items]; i < 256; i++, item++)
        if (item->active && item->object_number == 179)
            number_flares++;

    WriteSG(&number_flares, sizeof(int));
    for (i = level_items, item = &items[level_items]; i < 256; i++, item++)
        if (item->active && item->object_number == 179) {
            WriteSG(&item->pos, sizeof(PHD_3DPOS));
            WriteSG(&item->room_number, sizeof(sint16));
            WriteSG(&item->speed, sizeof(sint16));
            WriteSG(&item->fallspeed, sizeof(sint16));
            flare_age = (int)item->data;
            WriteSG(&flare_age, sizeof(int));
        }
}

#define KILLED_ITEM (-0x8000)

extern int flipeffect;
extern int fliptimer;
extern sint16 body_bag;

void InitialiseLaraInventory(int level_number);
int Inv_AddItem(int item_num);
void FlipMap(void);
void AlterFloorHeight(ITEM_INFO *item, int height);
void ItemNewRoom(sint16 item_number, sint16 room_number);
FLOOR_INFO *GetFloor(sint32 x, sint32 y, sint32 z, sint16 *room_number);
sint32 GetHeight(FLOOR_INFO *floor, sint32 x, sint32 y, sint32 z);
void KillItem(sint16 item_number);
void AddActiveItem(sint16 item_number);
int EnableBaddieAI(sint16 item_number, int always);
void RemoveDrawnItem(sint16 item_number);
void SetupCleanerFromSavegame(ITEM_INFO *item, int block);
void SetupShoal(sint32 shoal_number);
sint16 CreateItem(void);
void LaraBurn(void);
void InitialiseItem(sint16 item_number);
void MovableBlock(sint16 item_number);
void PuzzleHoleCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void PickUpCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void ReadSG(void *pointer, int size);

void ExtractSaveGameInfo(void)
{
    sint16 i;
    OBJECT_INFO *obj;
    ITEM_INFO *item;
    FLOOR_INFO *floor;
    sint16 room_number;
    char tflag;
    sint16 item_number;
    CREATURE_INFO *creature;
    int number_flares, flare_age;

    InitialiseLaraInventory(CurrentLevel);
    for (i = savegame.num_pickup1; i > 0; i--)
        Inv_AddItem(236);
    for (i = savegame.num_pickup2; i > 0; i--)
        Inv_AddItem(237);
    for (i = savegame.num_puzzle1; i > 0; i--)
        Inv_AddItem(205);
    for (i = savegame.num_puzzle2; i > 0; i--)
        Inv_AddItem(206);
    for (i = savegame.num_puzzle3; i > 0; i--)
        Inv_AddItem(207);
    for (i = savegame.num_puzzle4; i > 0; i--)
        Inv_AddItem(208);
    for (i = savegame.num_key1; i > 0; i--)
        Inv_AddItem(224);
    for (i = savegame.num_key2; i > 0; i--)
        Inv_AddItem(225);
    for (i = savegame.num_key3; i > 0; i--)
        Inv_AddItem(226);
    for (i = savegame.num_key4; i > 0; i--)
        Inv_AddItem(227);
    ResetSG();
    ReadSG(&i, sizeof(int));
    if (i)
        FlipMap();
    for (i = 0; i < 10; i++) {
        ReadSG(&tflag, sizeof(char));
        flipmap[i] = (sint16)tflag << 8;
    }
    ReadSG(cd_flags, sizeof(char) * 128);
    for (i = 0; i < number_cameras; i++)
        ReadSG(&camera.fixed[i].flags, sizeof(sint16));
    for (i = 0, item = items; i < level_items; i++, item++) {
        obj = &objects[item->object_number];
        if (obj->control == MovableBlock)
            AlterFloorHeight(item, 1024);
        if (obj->save_position) {
            ReadSG(&item->pos, sizeof(PHD_3DPOS));
            ReadSG(&room_number, sizeof(sint16));
            ReadSG(&item->speed, sizeof(sint16));
            ReadSG(&item->fallspeed, sizeof(sint16));
            if (item->room_number != room_number)
                ItemNewRoom(i, room_number);
            if (obj->shadow_size) {
                floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
                item->floor = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
            }
        }
        if (obj->save_anim) {
            ReadSG(&item->current_anim_state, sizeof(sint16));
            ReadSG(&item->goal_anim_state, sizeof(sint16));
            ReadSG(&item->required_anim_state, sizeof(sint16));
            ReadSG(&item->anim_number, sizeof(sint16));
            ReadSG(&item->frame_number, sizeof(sint16));
        }
        if (obj->save_hitpoints)
            ReadSG(&item->hit_points, sizeof(sint16));
        if (obj->save_flags) {
            uint32 flags;

            ReadSG(&flags, sizeof(uint32));
            item->flags = flags & 0xffff;
            if (obj->intelligent)
                ReadSG(&item->carried_item, sizeof(sint16));
            ReadSG(&item->timer, sizeof(sint16));
            if (item->flags & KILLED_ITEM) {
                KillItem(i);
                item->status = DEACTIVATED;
            } else {
                if ((flags & 0x10000) && !item->active)
                    AddActiveItem(i);
                item->active = (flags >> 16) & 1;
                item->status = (flags >> 17) & 3;
                item->gravity_status = (flags >> 19) & 1;
                item->hit_status = (flags >> 20) & 1;
                item->collidable = (flags >> 21) & 1;
                item->looked_at = (flags >> 22) & 1;
                item->dynamic_light = (flags >> 23) & 1;
                item->clear_body = (flags >> 24) & 1;
                item->ai_bits = (flags >> 25) & 31;
                item->really_active = (flags >> 30) & 1;
            }
            if (flags & 0x80000000) {
                EnableBaddieAI(i, 1);
                creature = (CREATURE_INFO *)item->data;
                if (creature)
                    ReadSG(creature, 18);
                else
                    SGpoint += 18;
            } else if (obj->intelligent) {
                item->data = NULL;
                if (item->clear_body && item->hit_points <= 0 && !(item->flags & KILLED_ITEM)) {
                    item->next_active = body_bag;
                    body_bag = i;
                }
            }
            if (obj->collision == PuzzleHoleCollision && (item->status == DEACTIVATED || item->status == ACTIVE))
                item->object_number += 4;
            if (obj->collision == PickUpCollision && item->status == DEACTIVATED)
                RemoveDrawnItem(i);
            if ((item->object_number == 101 || item->object_number == 102 || item->object_number == 103 || item->object_number == 104) && (item->flags & ONESHOT))
                item->mesh_bits = 0x100;
            ReadSG(&item->item_flags[0], sizeof(sint16) * 4);
        }
        if (obj->control == MovableBlock && item->status == NOT_ACTIVE)
            AlterFloorHeight(item, -1024);
        else if (obj->control == MovableBlock && item->status != NOT_ACTIVE)
            SetupCleanerFromSavegame(item, 1);
        if (item->object_number == 58)
            SetupCleanerFromSavegame(item, 0);
        if ((item->object_number == 339 || item->object_number == 338) && item->hit_points != -1)
            SetupShoal(item->hit_points);
        if (item->object_number == 18)
            ReadSG(item->data, 8);
        if (item->object_number == 15)
            ReadSG(item->data, 26);
        if (item->object_number == 14)
            ReadSG(item->data, 46);
        if (item->object_number == 17)
            ReadSG(item->data, 30);
        if (item->object_number == 16)
            ReadSG(item->data, 39);
        if (item->object_number == 19)
            ReadSG(item->data, 16);
        if (item->object_number == 291)
            ReadSG(&item->shadeB, sizeof(sint16));
        if (item->object_number == 71 && item->carried_item != NO_ITEM) {
            RemoveDrawnItem(item->carried_item);
            items[item->carried_item].room_number = NO_ROOM;
            items[item->carried_item].carried_item = NO_ITEM;
        }
    }
    ReadSG(&bossdata, sizeof(BOSS_STRUCT));
    ReadSG(&lara, sizeof(LARA_INFO));
    lara.creature = NULL;
    lara.hit_effect = NULL;
    for (i = 0; i < 15; i++)
        lara.mesh_ptrs[i] = (sint16 *)((long)lara.mesh_ptrs[i] + (long)mesh_base);
    lara.target = NULL;
    lara.left_arm.frame_base = (sint16 *)((long)lara.left_arm.frame_base + (long)frames);
    lara.right_arm.frame_base = (sint16 *)((long)lara.right_arm.frame_base + (long)frames);
    if (lara.skidoo != NO_ITEM) {
        if (items[lara.skidoo].object_number == 17) {
            if (((char *)items[lara.skidoo].data)[0x1c] & 1)
                meshes[objects[VEHICLE_ANIM].mesh_index + HAND_R] = meshes[objects[315].mesh_index + HAND_R];
        } else if (items[lara.skidoo].object_number == 14) {
            if (((char *)items[lara.skidoo].data)[0x2d] & 0x80)
                meshes[objects[VEHICLE_ANIM].mesh_index + HAND_R] = meshes[objects[315].mesh_index + HAND_R];
        }
    }
    if (lara.weapon_item != NO_ITEM) {
        lara.weapon_item = CreateItem();
        item = &items[lara.weapon_item];
        ReadSG(&item->object_number, sizeof(sint16));
        ReadSG(&item->anim_number, sizeof(sint16));
        ReadSG(&item->frame_number, sizeof(sint16));
        ReadSG(&item->current_anim_state, sizeof(sint16));
        ReadSG(&item->goal_anim_state, sizeof(sint16));
        item->status = ACTIVE;
        item->room_number = NO_ROOM;
    }
    if (lara.burn) {
        lara.burn = 0;
        LaraBurn();
    }
    ReadSG(&flipeffect, sizeof(int));
    ReadSG(&fliptimer, sizeof(int));
    ReadSG(&CurrentAtmosphere, sizeof(char));
    ReadSG(&compy_scared_timer, sizeof(int));
    ReadSG(&compys_attack_lara, sizeof(int));
    ReadSG(&CarcassItem, sizeof(sint16));
    ReadSG(&TribeBossShieldOn, sizeof(char));
    ReadSG(&TribeBossItem, sizeof(sint32));
    ReadSG(&lizard_man_active, sizeof(char));
    ReadSG(&number_flares, sizeof(int));
    for (i = 0; i < number_flares; i++) {
        item_number = CreateItem();
        item = &items[item_number];
        item->object_number = 179;
        ReadSG(&item->pos, sizeof(PHD_3DPOS));
        ReadSG(&item->room_number, sizeof(sint16));
        ReadSG(&item->speed, sizeof(sint16));
        ReadSG(&item->fallspeed, sizeof(sint16));
        InitialiseItem(item_number);
        AddActiveItem(item_number);
        ReadSG(&flare_age, sizeof(int));
        item->data = (void *)flare_age;
    }
}

void ResetSG(void)
{
    SGcount = 0;
    SGpoint = savegame.buffer;
}

void WriteSG(void *pointer, int size)
{
    char *data;

    SGcount += size;
    if (SGcount >= MAX_SAVEGAME_BUFFER)
        S_ExitSystem("FATAL: Savegame is too big to fit in buffer");

    data = pointer;
    for (; size > 0; size--)
        *(SGpoint++) = *(data++);
}

void ReadSG(void *pointer, int size)
{
    char *data;

    SGcount += size;
    data = pointer;
    for (; size > 0; size--)
        *(data++) = *(SGpoint++);
}
