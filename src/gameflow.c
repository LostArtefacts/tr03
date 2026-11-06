#include "specific/standard.h"
#include "game.h"

extern sint16 *GF_level_sequence_list[];
extern int title_loaded;

sint32 GF_DoFrontEndSequence(void)
{
    sint32 option;

    option = GF_InterpretSequence(GF_frontendSequence, IL_NORMAL, 1);
    if (option == EXITGAME)
        return 1;
    return 0;
}

sint32 GF_DoLevelSequence(int level, int type)
{
    sint16 *ptr;
    sint32 option;

    do
    {
        if (level > gameflow.num_levels - 1)
        {
            title_loaded = 0;
            return EXIT_TO_TITLE;
        }
        ptr = GF_level_sequence_list[level];
        option = GF_InterpretSequence(ptr, type, 0);
        level++;
        if (gameflow.singlelevel >= 0)
            return option;
    } while ((option & ~0xff) == LEVELCOMPLETE);
    return option;
}

#define MT_START 0
#define MT_SECRET 1

#define ADDINV_PISTOLS 0
#define ADDINV_SHOTGUN 1
#define ADDINV_AUTOPISTOLS 2
#define ADDINV_UZIS 3
#define ADDINV_HARPOON 4
#define ADDINV_M16 5
#define ADDINV_ROCKET 6
#define ADDINV_GRENADE 7
#define ADDINV_PISTOL_AMMO 8
#define ADDINV_SHOTGUN_AMMO 9
#define ADDINV_AUTOPISTOLS_AMMO 10
#define ADDINV_UZI_AMMO 11
#define ADDINV_HARPOON_AMMO 12
#define ADDINV_M16_AMMO 13
#define ADDINV_ROCKET_AMMO 14
#define ADDINV_GRENADE_AMMO 15
#define ADDINV_FLARES 16
#define ADDINV_MEDI 17
#define ADDINV_BIGMEDI 18
#define ADDINV_PICKUP1 19
#define ADDINV_PICKUP2 20
#define ADDINV_PUZZLE1 21
#define ADDINV_PUZZLE2 22
#define ADDINV_PUZZLE3 23
#define ADDINV_PUZZLE4 24
#define ADDINV_KEY1 25
#define ADDINV_KEY2 26
#define ADDINV_KEY3 27
#define ADDINV_KEY4 28
#define ADDINV_SAVEGAME_CRYSTAL 29
#define ADDINV_NUMBEROF 30

#define SHOTGUN_AMMO_QTY 12
#define MAGNUM_AMMO_QTY 10
#define UZI_AMMO_QTY 40
#define HARPOON_AMMO_QTY 3
#define M16_AMMO_QTY 60
#define ROCKET_AMMO_QTY 1
#define GRENADE_AMMO_QTY 2

extern char GF_Add2InvItems[];
extern char GF_SecretInvItems[];

int Inv_AddItem(int itemNum);
int Inv_RequestItem(int item_num);
void AddDisplayPickup(sint16 objnum);

void GF_ModifyInventory(int level, int type)
{
    int i;
    START_INFO *start = &savegame.start[level];

    if (!start->got_pistols && GF_Add2InvItems[ADDINV_PISTOLS]) {
        start->got_pistols = 1;
        Inv_AddItem(160);
    }

    if (Inv_RequestItem(161)) {
        if (type == MT_START)
            lara.shotgun.ammo += GF_Add2InvItems[ADDINV_SHOTGUN_AMMO] * SHOTGUN_AMMO_QTY;
        else {
            lara.shotgun.ammo += GF_SecretInvItems[ADDINV_SHOTGUN_AMMO] * SHOTGUN_AMMO_QTY;
            for (i = 0; i < GF_SecretInvItems[ADDINV_SHOTGUN_AMMO]; i++)
                AddDisplayPickup(169);
        }
    } else if ((type == MT_START && GF_Add2InvItems[ADDINV_SHOTGUN]) || (type == MT_SECRET && GF_SecretInvItems[ADDINV_SHOTGUN])) {
        start->got_shotgun = 1;
        Inv_AddItem(161);
        if (type == MT_START)
            lara.shotgun.ammo += GF_Add2InvItems[ADDINV_SHOTGUN_AMMO] * SHOTGUN_AMMO_QTY;
        else {
            lara.shotgun.ammo += GF_SecretInvItems[ADDINV_SHOTGUN_AMMO] * SHOTGUN_AMMO_QTY;
            AddDisplayPickup(161);
        }
    } else {
        if (type == MT_START) {
            for (i = 0; i < GF_Add2InvItems[ADDINV_SHOTGUN_AMMO]; i++)
                Inv_AddItem(169);
        } else {
            for (i = 0; i < GF_SecretInvItems[ADDINV_SHOTGUN_AMMO]; i++) {
                AddDisplayPickup(169);
                Inv_AddItem(169);
            }
        }
    }

    if (Inv_RequestItem(162)) {
        if (type == MT_START)
            lara.magnums.ammo += GF_Add2InvItems[ADDINV_AUTOPISTOLS_AMMO] * MAGNUM_AMMO_QTY;
        else {
            lara.magnums.ammo += GF_SecretInvItems[ADDINV_AUTOPISTOLS_AMMO] * MAGNUM_AMMO_QTY;
            for (i = 0; i < GF_SecretInvItems[ADDINV_AUTOPISTOLS_AMMO]; i++)
                AddDisplayPickup(170);
        }
    } else if ((type == MT_START && GF_Add2InvItems[ADDINV_AUTOPISTOLS]) || (type == MT_SECRET && GF_SecretInvItems[ADDINV_AUTOPISTOLS])) {
        start->got_magnums = 1;
        Inv_AddItem(162);
        if (type == MT_START)
            lara.magnums.ammo += GF_Add2InvItems[ADDINV_AUTOPISTOLS_AMMO] * MAGNUM_AMMO_QTY;
        else {
            AddDisplayPickup(162);
            lara.magnums.ammo += GF_Add2InvItems[ADDINV_AUTOPISTOLS_AMMO] * MAGNUM_AMMO_QTY;
            for (i = 0; i < GF_SecretInvItems[ADDINV_AUTOPISTOLS]; i++)
                AddDisplayPickup(170);
        }
    } else {
        if (type == MT_START) {
            for (i = 0; i < GF_Add2InvItems[ADDINV_AUTOPISTOLS_AMMO]; i++)
                Inv_AddItem(170);
        } else {
            for (i = 0; i < GF_SecretInvItems[ADDINV_AUTOPISTOLS_AMMO]; i++) {
                Inv_AddItem(170);
                AddDisplayPickup(170);
            }
        }
    }

    if (Inv_RequestItem(163)) {
        if (type == MT_START)
            lara.uzis.ammo += GF_Add2InvItems[ADDINV_UZI_AMMO] * UZI_AMMO_QTY;
        else {
            lara.uzis.ammo += GF_SecretInvItems[ADDINV_UZI_AMMO] * UZI_AMMO_QTY;
            for (i = 0; i < GF_SecretInvItems[ADDINV_UZI_AMMO]; i++)
                AddDisplayPickup(171);
        }
    } else if ((type == MT_START && GF_Add2InvItems[ADDINV_UZIS]) || (type == MT_SECRET && GF_SecretInvItems[ADDINV_UZIS])) {
        start->got_uzis = 1;
        Inv_AddItem(163);
        if (type == MT_START)
            lara.uzis.ammo += GF_Add2InvItems[ADDINV_UZI_AMMO] * UZI_AMMO_QTY;
        else {
            AddDisplayPickup(163);
            lara.uzis.ammo += GF_SecretInvItems[ADDINV_UZI_AMMO] * UZI_AMMO_QTY;
            for (i = 0; i < GF_SecretInvItems[ADDINV_UZI_AMMO]; i++)
                AddDisplayPickup(171);
        }
    } else {
        if (type == MT_START) {
            for (i = 0; i < GF_Add2InvItems[ADDINV_UZI_AMMO]; i++)
                Inv_AddItem(171);
        } else {
            for (i = 0; i < GF_SecretInvItems[ADDINV_UZI_AMMO]; i++) {
                Inv_AddItem(171);
                AddDisplayPickup(171);
            }
        }
    }

    if (Inv_RequestItem(164)) {
        if (type == MT_START)
            lara.harpoon.ammo += GF_Add2InvItems[ADDINV_HARPOON_AMMO] * HARPOON_AMMO_QTY;
        else {
            lara.harpoon.ammo += GF_SecretInvItems[ADDINV_HARPOON_AMMO] * HARPOON_AMMO_QTY;
            for (i = 0; i < GF_SecretInvItems[ADDINV_HARPOON_AMMO]; i++)
                AddDisplayPickup(172);
        }
    } else if ((type == MT_START && GF_Add2InvItems[ADDINV_HARPOON]) || (type == MT_SECRET && GF_SecretInvItems[ADDINV_HARPOON])) {
        start->got_harpoon = 1;
        Inv_AddItem(164);
        if (type == MT_START)
            lara.harpoon.ammo += GF_Add2InvItems[ADDINV_HARPOON_AMMO] * HARPOON_AMMO_QTY;
        else {
            AddDisplayPickup(164);
            lara.harpoon.ammo += GF_SecretInvItems[ADDINV_HARPOON_AMMO] * HARPOON_AMMO_QTY;
            for (i = 0; i < GF_SecretInvItems[ADDINV_HARPOON_AMMO]; i++)
                AddDisplayPickup(172);
        }
    } else {
        if (type == MT_START) {
            for (i = 0; i < GF_Add2InvItems[ADDINV_HARPOON_AMMO]; i++) {
                if (i == 0) {
                    Inv_AddItem(172);
                    lara.harpoon.ammo -= HARPOON_AMMO_QTY;
                }
                lara.harpoon.ammo += HARPOON_AMMO_QTY;
            }
        } else {
            for (i = 0; i < GF_SecretInvItems[ADDINV_HARPOON_AMMO]; i++) {
                if (i == 0) {
                    Inv_AddItem(172);
                    lara.harpoon.ammo -= HARPOON_AMMO_QTY;
                }
                lara.harpoon.ammo += HARPOON_AMMO_QTY;
                AddDisplayPickup(172);
            }
        }
    }

    if (Inv_RequestItem(165)) {
        if (type == MT_START)
            lara.m16.ammo += GF_Add2InvItems[ADDINV_M16_AMMO] * M16_AMMO_QTY;
        else {
            lara.m16.ammo += GF_SecretInvItems[ADDINV_M16_AMMO] * M16_AMMO_QTY;
            for (i = 0; i < GF_SecretInvItems[ADDINV_M16_AMMO]; i++)
                AddDisplayPickup(173);
        }
    } else if ((type == MT_START && GF_Add2InvItems[ADDINV_M16]) || (type == MT_SECRET && GF_SecretInvItems[ADDINV_M16])) {
        start->got_m16 = 1;
        Inv_AddItem(165);
        if (type == MT_START)
            lara.m16.ammo += GF_Add2InvItems[ADDINV_M16_AMMO] * M16_AMMO_QTY;
        else {
            AddDisplayPickup(165);
            lara.m16.ammo += GF_SecretInvItems[ADDINV_M16_AMMO] * M16_AMMO_QTY;
            for (i = 0; i < GF_SecretInvItems[ADDINV_M16_AMMO]; i++)
                AddDisplayPickup(173);
        }
    } else {
        if (type == MT_START) {
            for (i = 0; i < GF_Add2InvItems[ADDINV_M16_AMMO]; i++)
                Inv_AddItem(173);
        } else {
            for (i = 0; i < GF_SecretInvItems[ADDINV_M16_AMMO]; i++) {
                Inv_AddItem(173);
                AddDisplayPickup(173);
            }
        }
    }

    if (Inv_RequestItem(166)) {
        if (type == MT_START)
            lara.rocket.ammo += GF_Add2InvItems[ADDINV_ROCKET_AMMO] * ROCKET_AMMO_QTY;
        else {
            lara.rocket.ammo += GF_SecretInvItems[ADDINV_ROCKET_AMMO] * ROCKET_AMMO_QTY;
            for (i = 0; i < GF_SecretInvItems[ADDINV_ROCKET_AMMO]; i++)
                AddDisplayPickup(174);
        }
    } else if ((type == MT_START && GF_Add2InvItems[ADDINV_ROCKET]) || (type == MT_SECRET && GF_SecretInvItems[ADDINV_ROCKET])) {
        start->got_rocket = 1;
        Inv_AddItem(166);
        if (type == MT_START)
            lara.rocket.ammo += GF_Add2InvItems[ADDINV_ROCKET_AMMO] * ROCKET_AMMO_QTY;
        else {
            lara.rocket.ammo += GF_SecretInvItems[ADDINV_ROCKET_AMMO] * ROCKET_AMMO_QTY;
            AddDisplayPickup(166);
            for (i = 0; i < GF_SecretInvItems[ADDINV_ROCKET_AMMO]; i++)
                AddDisplayPickup(174);
        }
    } else {
        if (type == MT_START) {
            for (i = 0; i < GF_Add2InvItems[ADDINV_ROCKET_AMMO]; i++)
                Inv_AddItem(174);
        } else {
            for (i = 0; i < GF_SecretInvItems[ADDINV_ROCKET_AMMO]; i++) {
                Inv_AddItem(174);
                AddDisplayPickup(174);
            }
        }
    }

    if (Inv_RequestItem(167)) {
        if (type == MT_START)
            lara.grenade.ammo += GF_Add2InvItems[ADDINV_GRENADE_AMMO] * GRENADE_AMMO_QTY;
        else {
            lara.grenade.ammo += GF_SecretInvItems[ADDINV_GRENADE_AMMO] * GRENADE_AMMO_QTY;
            for (i = 0; i < GF_SecretInvItems[ADDINV_GRENADE_AMMO]; i++)
                AddDisplayPickup(175);
        }
    } else if ((type == MT_START && GF_Add2InvItems[ADDINV_GRENADE]) || (type == MT_SECRET && GF_SecretInvItems[ADDINV_GRENADE])) {
        start->got_grenade = 1;
        Inv_AddItem(167);
        if (type == MT_START)
            lara.grenade.ammo += GF_Add2InvItems[ADDINV_GRENADE_AMMO] * GRENADE_AMMO_QTY;
        else {
            lara.grenade.ammo += GF_SecretInvItems[ADDINV_GRENADE_AMMO] * GRENADE_AMMO_QTY;
            AddDisplayPickup(167);
            for (i = 0; i < GF_SecretInvItems[ADDINV_GRENADE_AMMO]; i++)
                AddDisplayPickup(175);
        }
    } else {
        if (type == MT_START) {
            for (i = 0; i < GF_Add2InvItems[ADDINV_GRENADE_AMMO]; i++)
                Inv_AddItem(175);
        } else {
            for (i = 0; i < GF_SecretInvItems[ADDINV_GRENADE_AMMO]; i++) {
                Inv_AddItem(175);
                AddDisplayPickup(175);
            }
        }
    }

    if (type == MT_START) {
        for (i = 0; i < GF_Add2InvItems[ADDINV_FLARES]; i++)
            Inv_AddItem(179);
        for (i = 0; i < GF_Add2InvItems[ADDINV_MEDI]; i++)
            Inv_AddItem(176);
        for (i = 0; i < GF_Add2InvItems[ADDINV_BIGMEDI]; i++)
            Inv_AddItem(177);
        for (i = 0; i < GF_Add2InvItems[ADDINV_PICKUP1]; i++)
            Inv_AddItem(236);
        for (i = 0; i < GF_Add2InvItems[ADDINV_PICKUP2]; i++)
            Inv_AddItem(237);
        for (i = 0; i < GF_Add2InvItems[ADDINV_PUZZLE1]; i++)
            Inv_AddItem(205);
        for (i = 0; i < GF_Add2InvItems[ADDINV_PUZZLE2]; i++)
            Inv_AddItem(206);
        for (i = 0; i < GF_Add2InvItems[ADDINV_PUZZLE3]; i++)
            Inv_AddItem(207);
        for (i = 0; i < GF_Add2InvItems[ADDINV_PUZZLE4]; i++)
            Inv_AddItem(208);
        for (i = 0; i < GF_Add2InvItems[ADDINV_KEY1]; i++)
            Inv_AddItem(224);
        for (i = 0; i < GF_Add2InvItems[ADDINV_KEY2]; i++)
            Inv_AddItem(225);
        for (i = 0; i < GF_Add2InvItems[ADDINV_KEY3]; i++)
            Inv_AddItem(226);
        for (i = 0; i < GF_Add2InvItems[ADDINV_KEY4]; i++)
            Inv_AddItem(227);
        for (i = 0; i < GF_Add2InvItems[ADDINV_SAVEGAME_CRYSTAL]; i++)
            Inv_AddItem(180);
        for (i = 0; i < ADDINV_NUMBEROF; i++)
            GF_Add2InvItems[i] = 0;
    } else {
        for (i = 0; i < GF_SecretInvItems[ADDINV_FLARES]; i++) {
            Inv_AddItem(179);
            AddDisplayPickup(179);
        }
        for (i = 0; i < GF_SecretInvItems[ADDINV_MEDI]; i++) {
            Inv_AddItem(176);
            AddDisplayPickup(176);
        }
        for (i = 0; i < GF_SecretInvItems[ADDINV_BIGMEDI]; i++) {
            Inv_AddItem(177);
            AddDisplayPickup(177);
        }
        for (i = 0; i < GF_SecretInvItems[ADDINV_PICKUP1]; i++) {
            Inv_AddItem(236);
            AddDisplayPickup(236);
        }
        for (i = 0; i < GF_SecretInvItems[ADDINV_PICKUP2]; i++) {
            Inv_AddItem(237);
            AddDisplayPickup(237);
        }
        for (i = 0; i < GF_SecretInvItems[ADDINV_PUZZLE1]; i++) {
            Inv_AddItem(205);
            AddDisplayPickup(205);
        }
        for (i = 0; i < GF_SecretInvItems[ADDINV_PUZZLE2]; i++) {
            Inv_AddItem(206);
            AddDisplayPickup(206);
        }
        for (i = 0; i < GF_SecretInvItems[ADDINV_PUZZLE3]; i++) {
            Inv_AddItem(207);
            AddDisplayPickup(207);
        }
        for (i = 0; i < GF_SecretInvItems[ADDINV_PUZZLE4]; i++) {
            Inv_AddItem(208);
            AddDisplayPickup(208);
        }
        for (i = 0; i < GF_SecretInvItems[ADDINV_KEY1]; i++) {
            Inv_AddItem(224);
            AddDisplayPickup(224);
        }
        for (i = 0; i < GF_SecretInvItems[ADDINV_KEY2]; i++) {
            Inv_AddItem(225);
            AddDisplayPickup(225);
        }
        for (i = 0; i < GF_SecretInvItems[ADDINV_KEY3]; i++) {
            Inv_AddItem(226);
            AddDisplayPickup(226);
        }
        for (i = 0; i < GF_SecretInvItems[ADDINV_KEY4]; i++) {
            Inv_AddItem(227);
            AddDisplayPickup(227);
        }
        for (i = 0; i < ADDINV_NUMBEROF; i++)
            GF_SecretInvItems[i] = 0;
    }
}
