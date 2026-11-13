#include "specific/standard.h"
#include "game.h"

#ifdef TR3_EXE
extern char Save_Game_Strings[];
extern char Save_Game_Strings2[];
extern char Valid_Level_Strings[];
extern char Valid_Level_Strings2[];

sint32 inv_nframes = 2;
uint16 req_bgnd_gour1[16] = {
    0x1E00, 0x1E00, 0x1A00, 0x1E00, 0x1E00, 0x1E00, 0x1E00, 0x1A00, 0x1A00, 0x1E00, 0x1E00,
    0x1E00, 0x1E00, 0x1A00, 0x1E00, 0x1E00,
};
uint16 req_bgnd_gour2[9] = {
    0x1A00, 0x1800, 0x1E00, 0x2000, 0x2000, 0x1E00, 0x1C00, 0x1C00, 0x1A00,
};
uint16 req_main_gour1[16] = {
    0x2000, 0x2000, 0x1800, 0x2000, 0x2000, 0x2000, 0x2000, 0x1800, 0x1800, 0x2000, 0x2000,
    0x2000, 0x2000, 0x1800, 0x2000, 0x2000,
};
uint16 req_main_gour2[9] = {
    0x2000, 0x2000, 0x2000, 0x2000, 0x2000, 0x2000, 0x2000, 0x2000, 0x2000,
};
uint16 req_sel_gour1[16] = {
    0x2000, 0x2000, 0x1400, 0x2000, 0x2000, 0x2000, 0x2000, 0x1400, 0x1400, 0x2000, 0x2000,
    0x2000, 0x2000, 0x1400, 0x2000, 0x2000,
};
uint16 req_sel_gour2[9] = {
    0x2000, 0x1010, 0x2000, 0x1400, 0x2000, 0x1010, 0x2000, 0x1400, 0x2000,
};
uint16 req_unsel_gour1[16] = {
    0x2000, 0x2000, 0x1A00, 0x2000, 0x2000, 0x2000, 0x2000, 0x1A00, 0x1A00, 0x2000, 0x2000,
    0x2000, 0x2000, 0x1A00, 0x2000, 0x2000,
};
uint16 req_unsel_gour2[9] = {
    0x2000, 0x1C40, 0x2000, 0x1C40, 0x2000, 0x1C40, 0x2000, 0x1C40, 0x2000,
};
uint16 is_gour1[2] = { 0x1010, 0x1600 };
uint16 is_gour2[2] = { 0x1A00, 0x1E00 };
uint16 is_gour3[4] = { 0x1600, 0x1800, 0x1A00, 0x1800 };
uint16 is_gour4[4] = { 0x2000, 0x1010, 0x1010, 0x2000 };
uint16 is_gour5[4] = { 0x2000, 0x1010, 0x1010, 0x2000 };
uint16 is_gour6[4] = { 0x2000, 0x1010, 0x1010, 0x2000 };
uint16 is_gour7[4] = { 0x2000, 0x1010, 0x1010, 0x2000 };
uint16 is_gour8[2] = { 0x1210, 0x1800 };
uint16 is_gour9[2] = { 0x1C00, 0x2000 };
uint16 is_gour10[4] = { 0x1800, 0x1A00, 0x1C00, 0x1A00 };
uint16 is_gour11[4] = { 0x2000, 0x1800, 0x1800, 0x2000 };
uint16 is_gour12[4] = { 0x2000, 0x1800, 0x1800, 0x2000 };
INVENTORY_SPRITE is_detail_level[10] = {
    { 2, -66, -72, 32, 132, 0, (struct gouraud_fill *)is_gour1, 1 },
    { 2, -66, -56, 32, 132, 0, (struct gouraud_fill *)is_gour2, 1 },
    { 2, -66, -72, 32, 0, 16, (struct gouraud_fill *)is_gour1, 1 },
    { 2, 66, -72, 32, 0, 16, (struct gouraud_fill *)is_gour2, 1 },
    { 3, -65, -71, 32, 130, 14, (struct gouraud_fill *)is_gour3, 1 },
    { 3, -64, -70, 32, 128, 12, (struct gouraud_fill *)is_gour3, 1 },
    { 4, -63, -69, 32, 126, 10, (struct gouraud_fill *)is_gour7, 16 },
    { 4, -63, -69, 33, 126, 10 },
    { 2, 1, -69, 31, 0, 10, 0, 2 },
    { 0 },
};
INVENTORY_SPRITE *is_detail_list[2] = { is_detail_level, 0 };
INVENTORY_SPRITE is_sound_muslevel[9] = {
    { 2, -66, -80, 32, 132, 0, (struct gouraud_fill *)is_gour1, 1 },
    { 2, -66, -65, 32, 132, 0, (struct gouraud_fill *)is_gour2, 1 },
    { 2, -66, -80, 32, 0, 16, (struct gouraud_fill *)is_gour1, 1 },
    { 2, 66, -80, 32, 0, 16, (struct gouraud_fill *)is_gour2, 1 },
    { 3, -65, -79, 32, 131, 14, (struct gouraud_fill *)is_gour3, 1 },
    { 3, -64, -78, 32, 129, 12, (struct gouraud_fill *)is_gour3, 1 },
    { 4, -63, -77, 32, 126, 10, (struct gouraud_fill *)is_gour5, 3 },
    { 4, -63, -77, 33, 127, 10 },
    { 0 },
};
INVENTORY_SPRITE is_sound_sfxlevel[9] = {
    { 2, -66, -56, 32, 132, 0, (struct gouraud_fill *)is_gour1, 1 },
    { 2, -66, -41, 32, 132, 0, (struct gouraud_fill *)is_gour2, 1 },
    { 2, -66, -56, 32, 0, 16, (struct gouraud_fill *)is_gour1, 1 },
    { 2, 66, -56, 32, 0, 16, (struct gouraud_fill *)is_gour2, 1 },
    { 3, -65, -55, 32, 131, 14, (struct gouraud_fill *)is_gour3, 1 },
    { 3, -64, -54, 32, 129, 12, (struct gouraud_fill *)is_gour3, 1 },
    { 4, -63, -53, 32, 126, 10, (struct gouraud_fill *)is_gour6, 15 },
    { 4, -63, -53, 33, 127, 10 },
    { 0 },
};
INVENTORY_SPRITE is_sound_muslevellow[9] = {
    { 2, -66, -80, 32, 132, 0, (struct gouraud_fill *)is_gour8, 1 },
    { 2, -66, -65, 32, 132, 0, (struct gouraud_fill *)is_gour9, 1 },
    { 2, -66, -80, 32, 0, 16, (struct gouraud_fill *)is_gour8, 1 },
    { 2, 66, -80, 32, 0, 16, (struct gouraud_fill *)is_gour9, 1 },
    { 3, -65, -79, 32, 131, 14, (struct gouraud_fill *)is_gour10, 1 },
    { 3, -64, -78, 32, 129, 12, (struct gouraud_fill *)is_gour10, 1 },
    { 4, -63, -77, 32, 126, 10, (struct gouraud_fill *)is_gour11, 3 },
    { 4, -63, -77, 33, 127, 10 },
    { 0 },
};
INVENTORY_SPRITE is_sound_sfxlevellow[9] = {
    { 2, -66, -56, 32, 132, 0, (struct gouraud_fill *)is_gour8, 1 },
    { 2, -66, -41, 32, 132, 0, (struct gouraud_fill *)is_gour9, 1 },
    { 2, -66, -56, 32, 0, 16, (struct gouraud_fill *)is_gour8, 1 },
    { 2, 66, -56, 32, 0, 16, (struct gouraud_fill *)is_gour9, 1 },
    { 3, -65, -55, 32, 131, 14, (struct gouraud_fill *)is_gour10, 1 },
    { 3, -64, -54, 32, 129, 12, (struct gouraud_fill *)is_gour10, 1 },
    { 4, -63, -53, 32, 126, 10, (struct gouraud_fill *)is_gour12, 15 },
    { 4, -63, -53, 33, 127, 10 },
    { 0 },
};
INVENTORY_SPRITE is_sound_names[3] = {
    { 1, -75, -63, 32, 524288, 524288, 0, 101 },
    { 1, -75, -40, 32, 524288, 524288, 0, 102 },
    { 0 },
};
INVENTORY_SPRITE *is_sound_list[4] = { is_sound_names, is_sound_muslevel, is_sound_sfxlevel, 0 };
INVENTORY_ITEM icompass_option = {
    0, 146, 1, 0, 0, 0, 1, 1, 0, 4352, 0, -1536, 0, 0, 0, 0, -170, 0, 320, 0, 0xFFFFFFFF,
    0xFFFFFFFF,
};
INVENTORY_ITEM igun_option = {
    0, 185, 12, 0, 0, 11, 1, 1, 0, 3200, 0, 2848, 0, 0, -32768, 0, 38, 0, 352, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 1,
};
INVENTORY_ITEM iflare_option = {
    0, 203, 31, 0, 0, 30, 1, 1, 0, 3200, 0, 0, 0, 0, -8192, 0, 0, 0, 296, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 9,
};
INVENTORY_ITEM ishotgun_option = {
    0, 186, 13, 0, 0, 12, 1, 1, 0, 3200, 0, 5120, 0, 0, 30720, 0, 0, 0, 228, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 2,
};
INVENTORY_ITEM imagnum_option = {
    0, 187, 12, 0, 0, 11, 1, 1, 0, 3200, 0, 3360, 0, 0, -32768, 0, 0, 0, 362, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 3,
};
INVENTORY_ITEM iuzi_option = {
    0, 188, 13, 0, 0, 12, 1, 1, 0, 3200, 0, 2336, 0, 0, -32768, 0, 56, 0, 322, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 4,
};
INVENTORY_ITEM iharpoon_option = {
    0, 189, 12, 0, 0, 11, 1, 1, 0, 3200, 0, -736, 0, 0, -19456, 0, 58, 0, 296, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 8,
};
INVENTORY_ITEM im16_option = {
    0, 190, 12, 0, 0, 11, 1, 1, 0, 3200, 0, -224, 0, 0, -18432, 0, 84, 0, 296, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 5,
};
INVENTORY_ITEM irocket_option = {
    0, 191, 12, 0, 0, 11, 1, 1, 0, 3200, 0, -224, 0, 0, 14336, 0, 56, 0, 296, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 6,
};
INVENTORY_ITEM igrenade_option = {
    0, 192, 12, 0, 0, 11, 1, 1, 0, 3200, 0, -224, 0, 0, 14336, 0, 56, 0, 296, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 7,
};
INVENTORY_ITEM igunammo_option = {
    0, 193, 1, 0, 0, 0, 1, 1, 0, 3200, 0, -3808, 0, 0, 0, 0, 0, 0, 296, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 1,
};
INVENTORY_ITEM isgunammo_option = {
    0, 194, 1, 0, 0, 0, 1, 1, 0, 3200, 0, -3808, 0, 0, 0, 0, 0, 0, 296, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 2,
};
INVENTORY_ITEM imagammo_option = {
    0, 195, 1, 0, 0, 0, 1, 1, 0, 3200, 0, -3808, 0, 0, 0, 0, 0, 0, 296, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 3,
};
INVENTORY_ITEM iuziammo_option = {
    0, 196, 1, 0, 0, 0, 1, 1, 0, 3200, 0, -3808, 0, 0, 0, 0, 0, 0, 296, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 4,
};
INVENTORY_ITEM iharpoonammo_option = {
    0, 197, 1, 0, 0, 0, 1, 1, 0, 3200, 0, -3808, 0, 0, 0, 0, 0, 0, 296, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 8,
};
INVENTORY_ITEM im16ammo_option = {
    0, 198, 1, 0, 0, 0, 1, 1, 0, 3200, 0, -3808, 0, 0, 0, 0, 0, 0, 296, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 5,
};
INVENTORY_ITEM irocketammo_option = {
    0, 199, 1, 0, 0, 0, 1, 1, 0, 3200, 0, -3808, 0, 0, 0, 0, 0, 0, 296, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 6,
};
INVENTORY_ITEM igrenadeammo_option = {
    0, 200, 1, 0, 0, 0, 1, 1, 0, 3200, 0, -3808, 0, 0, 0, 0, 0, 0, 296, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 7,
};
INVENTORY_ITEM imedi_option = {
    0, 201, 26, 0, 0, 25, 1, 1, 0, 4032, 0, -7296, 0, 0, -4096, 0, 0, 0, 216, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 12,
};
INVENTORY_ITEM ibigmedi_option = {
    0, 202, 20, 0, 0, 19, 1, 1, 0, 3616, 0, -8160, 0, 0, -4096, 0, 0, 0, 352, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 11,
};
INVENTORY_ITEM ipickup1_option = {
    0, 238, 1, 0, 0, 0, 1, 1, 0, 7200, 0, 7200, 0, 0, -32768, 0, 0, 0, 256, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 111,
};
INVENTORY_ITEM ipickup2_option = {
    0, 239, 1, 0, 0, 0, 1, 1, 0, 7200, 0, -4352, 0, 0, 0, 0, 0, 0, 256, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 110,
};
INVENTORY_ITEM ipuzzle1_option = {
    0, 209, 1, 0, 0, 0, 1, 1, 0, 7200, 0, -4352, 0, 0, 0, 0, 0, 0, 256, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 108,
};
INVENTORY_ITEM ipuzzle2_option = {
    0, 210, 1, 0, 0, 0, 1, 1, 0, 7200, 0, -4352, 0, 0, 0, 0, 0, 0, 256, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 107,
};
INVENTORY_ITEM ipuzzle3_option = {
    0, 211, 1, 0, 0, 0, 1, 1, 0, 7200, 0, -4352, 0, 0, 0, 0, 0, 0, 256, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 106,
};
INVENTORY_ITEM ipuzzle4_option = {
    0, 212, 1, 0, 0, 0, 1, 1, 0, 7200, 0, -4352, 0, 0, 0, 0, 0, 0, 256, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 105,
};
INVENTORY_ITEM ikey1_option = {
    0, 228, 1, 0, 0, 0, 1, 1, 0, 7200, 0, -4352, 0, 0, 0, 0, 0, 0, 256, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 101,
};
INVENTORY_ITEM ikey2_option = {
    0, 229, 1, 0, 0, 0, 1, 1, 0, 7200, 0, -4352, 0, 0, 0, 0, 0, 0, 256, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 102,
};
INVENTORY_ITEM ikey3_option = {
    0, 230, 1, 0, 0, 0, 1, 1, 0, 7200, 0, -4352, 0, 0, 0, 0, 0, 0, 256, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 103,
};
INVENTORY_ITEM ikey4_option = {
    0, 231, 1, 0, 0, 0, 1, 1, 0, 7200, 0, -4352, 0, 0, 0, 0, 0, 0, 256, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 104,
};
INVENTORY_ITEM icon1_option = {
    0, 244, 1, 0, 0, 0, 1, 1, 0, 7200, 0, -4352, 0, 0, 0, 0, 0, 0, 256, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 112,
};
INVENTORY_ITEM icon2_option = {
    0, 245, 1, 0, 0, 0, 1, 1, 0, 7200, 0, -4352, 0, 0, 0, 0, 0, 0, 256, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 113,
};
INVENTORY_ITEM icon3_option = {
    0, 246, 1, 0, 0, 0, 1, 1, 0, 7200, 0, -4352, 0, 0, 0, 0, 0, 0, 256, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 114,
};
INVENTORY_ITEM icon4_option = {
    0, 247, 1, 0, 0, 0, 1, 1, 0, 7200, 0, -4352, 0, 0, 0, 0, 0, 0, 256, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 115,
};
INVENTORY_ITEM sgcrystal_option = {
    0, 204, 1, 0, 0, 0, 1, 1, 0, 7200, 0, -4352, 0, 0, 0, 0, 0, 0, 256, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 13,
};
INVENTORY_ITEM ipassport_option = {
    0, 158, 30, 0, 0, 14, 1, 1, 0, 4640, 0, -4320, 0, 0, 0, 0, 0, 0, 384, 0, 0x13, 0x13,
};
INVENTORY_ITEM idetail_option = {
    0, 181, 1, 0, 0, 0, 1, 1, 0, 4224, 0, -7232, 0, 0, 0, 0, -16, 0, 444, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 1,
};
INVENTORY_ITEM isound_option = {
    0, 182, 1, 0, 0, 0, 1, 1, 0, 4832, 0, -5408, 0, 0, -3072, 0, -18, 0, 350, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 2,
};
INVENTORY_ITEM icontrol_option = {
    0, 183, 1, 0, 0, 0, 1, 1, 0, 5504, 0, -2560, 5632, 5632, 13312, 0, 46, 0, 508, 0,
    0xFFFFFFFF, 0xFFFFFFFF, 3,
};
INVENTORY_ITEM igamma_option = {
    0, 184, 160, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, -176, 0, 256, 0, 0xFFFFFFF7,
    0xFFFFFFF7, 4,
};
INVENTORY_ITEM iphoto_option = {
    0, 147, 1, 0, 0, 0, 1, 1, 0, 4640, 0, -4320, 0, 0, 0, 0, 16, 0, 448, 0, 0xFFFFFFFF,
    0xFFFFFFFF, 5,
};
sint16 inv_main_objects = 8;
sint16 inv_main_select = -1;
sint16 inv_main_qtys[23] = { 1, 1, 1, 1, 1, 1, 1, 1, 1 };
INVENTORY_ITEM *inv_main_list[23] = {
    &icompass_option, &iflare_option, &igun_option, &ishotgun_option, &imagnum_option,
    &iuzi_option, &im16_option, &irocket_option, &igrenade_option, &iharpoon_option,
    &ibigmedi_option, &imedi_option, &sgcrystal_option,
};
sint16 inv_keys_select = -1;
sint16 inv_keys_qtys[23] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
INVENTORY_ITEM *inv_keys_list[23] = {
    &ipuzzle1_option, &ipuzzle2_option, &ipuzzle3_option, &ipuzzle4_option, &ikey1_option,
    &ikey2_option, &ikey3_option, &ikey4_option, &ipickup1_option, &ipickup2_option,
    &icon1_option, &icon2_option, &icon3_option, &icon4_option,
};
sint16 inv_option_objects = 5;
INVENTORY_ITEM *inv_option_list[5] = {
    &ipassport_option, &icontrol_option, &isound_option, &idetail_option, &iphoto_option,
};
INVENTORY_ITEM *inv_levelselect_list[1] = {
    &igamma_option,
};
int OpenInvOnGym = 1;
REQUEST_INFO Load_Game_Requester = {
    0, 1, 0, 5, 0, 0, 296, 18, 0, -32, 0, 50, Save_Game_Strings, Save_Game_Strings2,
};
REQUEST_INFO Level_Select_Requester = {
    0, 1, 0, 5, 0, 0, 272, 18, 0, -32, 0, 50, Valid_Level_Strings, Valid_Level_Strings2,
};
#endif
