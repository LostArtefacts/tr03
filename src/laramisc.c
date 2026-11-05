#include "game.h"


void InitialiseLaraLoad(sint16 item_num)
{
    lara.item_number = item_num;
    lara_item = &items[item_num];
}

extern sint16 cheat_hit_points;

int Inv_AddItem(int item_num);

void LaraCheatGetStuff(void)
{
    int i;

    Inv_AddItem(165);
    Inv_AddItem(161);
    Inv_AddItem(163);
    Inv_AddItem(162);
    Inv_AddItem(160);
    Inv_AddItem(166);
    Inv_AddItem(167);
    for (i = 0; i < 1; i++) {
        Inv_AddItem(178);
        Inv_AddItem(176);
        Inv_AddItem(177);
    }
    lara.magnums.ammo = 250;
    lara.uzis.ammo = 1000;
    lara.shotgun.ammo = 300;
    lara.m16.ammo = 300;
    lara.grenade.ammo = 1000;
}

void LaraUnderWater(ITEM_INFO *item, COLL_INFO *coll);
void LaraInitialiseMeshes(int level_number);

void LaraCheat(ITEM_INFO *item, COLL_INFO *coll)
{
    lara_item->hit_points = 1000;
    LaraUnderWater(item, coll);
    if ((input & IN_SLOW) && !(input & IN_LOOK)) {
        lara.water_status = LARA_ABOVEWATER;
        item->frame_number = STOP_F;
        item->anim_number = STOP_A;
        item->pos.x_rot = item->pos.z_rot = 0;
        lara.torso_x_rot = lara.torso_y_rot = 0;
        lara.head_x_rot = lara.head_y_rot = 0;
        lara.gun_status = LG_ARMLESS;
        LaraInitialiseMeshes(1);
        lara.mesh_effects = 0;
        lara_item->hit_points = cheat_hit_points;
    }
}

#define LARA_SKIN 315

void LaraInitialiseMeshes(int level_number)
{
    int i;
    START_INFO *start;

    start = &savegame.start[level_number];
    for (i = 0; i < 15; i++)
        lara.mesh_ptrs[i] = meshes[objects[LARA].mesh_index + i] = meshes[objects[LARA_SKIN].mesh_index + i];
    switch (start->gun_type) {
    case LG_UNARMED:
        break;
    case LG_MAGNUMS:
        lara.mesh_ptrs[THIGH_L] = meshes[objects[MAGNUM].mesh_index + THIGH_L];
        lara.mesh_ptrs[THIGH_R] = meshes[objects[MAGNUM].mesh_index + THIGH_R];
        break;
    case LG_UZIS:
        lara.mesh_ptrs[THIGH_L] = meshes[objects[UZI].mesh_index + THIGH_L];
        lara.mesh_ptrs[THIGH_R] = meshes[objects[UZI].mesh_index + THIGH_R];
        break;
    default:
        lara.mesh_ptrs[THIGH_L] = meshes[objects[PISTOLS].mesh_index + THIGH_L];
        lara.mesh_ptrs[THIGH_R] = meshes[objects[PISTOLS].mesh_index + THIGH_R];
        break;
    }
    if (start->gun_type == LG_FLARE)
        lara.mesh_ptrs[HAND_L] = meshes[objects[FLARE].mesh_index + HAND_L];
    switch (start->gun_type) {
    case LG_M16:
        lara.back_gun = M16;
        break;
    case LG_HARPOON:
        lara.back_gun = HARPOON_GUN;
        break;
    case LG_ROCKET:
        lara.back_gun = ROCKET_GUN;
        break;
    case LG_GRENADE:
        lara.back_gun = GRENADE_GUN;
        break;
    default:
        if (start->got_shotgun)
            lara.back_gun = SHOTGUN;
        else if (start->got_m16)
            lara.back_gun = M16;
        else if (start->got_rocket)
            lara.back_gun = ROCKET_GUN;
        else if (start->got_grenade)
            lara.back_gun = GRENADE_GUN;
        else if (start->got_harpoon)
            lara.back_gun = HARPOON_GUN;
        break;
    }
}

#define SWIMGLIDE_A 87
#define SWIMGLIDE_F GF(SWIMGLIDE_A, 0)

void LaraCheatyBits(void)
{
    if (!gameflow.cheat_enable && !gameflow.dozy_cheat_enabled)
        return;

    if (input & IN_D) {
        LaraCheatGetStuff();
        lara_item->hit_points = 1000;
    }

    if (input & IN_C) {
        lara_item->pos.y_pos -= STEP_L / 2;
        if (lara.water_status != LARA_CHEAT) {
            lara.water_status = LARA_CHEAT;
            lara_item->frame_number = SWIMGLIDE_F;
            lara_item->anim_number = SWIMGLIDE_A;
            lara_item->current_anim_state = AS_SWIM;
            lara_item->goal_anim_state = AS_SWIM;
            lara_item->gravity_status = 0;
            lara_item->pos.x_rot = 30 * ONE_DEGREE;
            lara_item->fallspeed = 30;
            lara.air = 1800;
            lara.death_count = 0;
            lara.torso_x_rot = lara.torso_y_rot = 0;
            lara.head_x_rot = lara.head_y_rot = 0;
            cheat_hit_points = lara_item->hit_points;
        }
    }
}

#define LARA_HITPOINTS 1000

int Inv_RemoveItem(int item_num);

void UseItem(sint16 object_num)
{
    if (object_num < 0 || object_num >= NUMBER_OBJECTS)
        return;
    switch (object_num) {
    case 160:
    case 185:
        lara.request_gun_type = LG_PISTOLS;
        break;
    case 162:
    case 187:
        lara.request_gun_type = LG_MAGNUMS;
        break;
    case 163:
    case 188:
        lara.request_gun_type = LG_UZIS;
        break;
    case 161:
    case 186:
        lara.request_gun_type = LG_SHOTGUN;
        break;
    case 164:
    case 189:
        lara.request_gun_type = LG_HARPOON;
        break;
    case 165:
    case 190:
        lara.request_gun_type = LG_M16;
        break;
    case 166:
    case 191:
        lara.request_gun_type = LG_ROCKET;
        break;
    case 167:
    case 192:
        lara.request_gun_type = LG_GRENADE;
        break;
    case 178:
    case 203:
        lara.request_gun_type = LG_FLARE;
        break;
    case 176:
    case 201:
        if ((lara_item->hit_points > 0 && lara_item->hit_points < LARA_HITPOINTS) || lara.poisoned) {
            lara.poisoned = 0;
            lara_item->hit_points += LARA_HITPOINTS / 2;
            if (lara_item->hit_points > LARA_HITPOINTS)
                lara_item->hit_points = LARA_HITPOINTS;
            Inv_RemoveItem(176);
            SoundEffect(116, NULL, 2);
            savegame.health_used++;
        }
        break;
    case 177:
    case 202:
        if ((lara_item->hit_points > 0 && lara_item->hit_points < LARA_HITPOINTS) || lara.poisoned) {
            lara.poisoned = 0;
            lara_item->hit_points += LARA_HITPOINTS;
            if (lara_item->hit_points > LARA_HITPOINTS)
                lara_item->hit_points = LARA_HITPOINTS;
            Inv_RemoveItem(177);
            SoundEffect(116, NULL, 2);
            savegame.health_used += 2;
        }
        break;
    default:
        break;
    }
}

void ControlLaraExtra(sint16 item_num)
{
    AnimateItem(&items[item_num]);
}
