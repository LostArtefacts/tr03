#include "specific/standard.h"
#include "game.h"

#define PASSPORT_OPTION 145
#define MAP_OPTION 146
#define GAMMA_OPTION 184

extern sint32 CurPicTexIndices[5];
extern float f_zfar;

void HWR_EnableZBuffer(int write, int compare);
void TRDrawPicture(sint32 col, sint32 *indices, float z);

extern sint32 Inventory_Mode;
extern sint16 Inventory_Displaying;
extern sint16 Inventory_Chosen;
extern sint32 Inventory_ExtraData[8];
extern sint16 inv_main_objects, inv_main_current;
extern sint16 inv_option_objects, inv_option_current;
extern INVENTORY_ITEM *inv_main_list[];
extern INVENTORY_ITEM *inv_option_list[];
extern INVENTORY_ITEM *inv_levelselect_list[];
extern sint32 OpenInvOnGym;
extern sint16 item_data;
extern sint32 HiResFlag;
extern sint32 phd_left, phd_right, phd_top, phd_bottom;
extern sint16 phd_winxmax, phd_winymax;

void S_SetupAboveWater(int underwater);
void TempVideoAdjust(sint32 a, double sizer);

#define INV_GAME_MODE 0
#define INV_TITLE_MODE 1
#define INV_KEYS_MODE 2
#define INV_SAVE_MODE 3
#define INV_LOAD_MODE 4
#define INV_DEATH_MODE 5
#define INV_LEVELSELECT_MODE 7

#define MAIN_RING 0
#define OPTION_RING 1
#define KEYS_RING 2
#define LEVELSELECT_RING 3

#define RNG_OPENING 0
#define RNG_OPEN 1
#define RNG_CLOSING 2
#define RNG_MAIN2OPTION 3
#define RNG_MAIN2KEYS 4
#define RNG_KEYS2MAIN 5
#define RNG_OPTION2MAIN 6
#define RNG_SELECTING 7
#define RNG_SELECTED 8
#define RNG_DESELECTING 9
#define RNG_DESELECT 10
#define RNG_CLOSING_ITEM 11
#define RNG_EXITING_INVENTORY 12
#define RNG_DONE 13

#define CLOSE_FRAMES 32
#define OPEN_ROTATION (-0x8000)
#define CLOSE_ROTATION (-0x8000)
#define RINGSWITCH_FRAMES 48
#define SELECTING_FRAMES 16
#define CAMERA_STARTHEIGHT (-0x600)
#define CAMERA_2_RING 0x256
#define RING_RADIUS 0x2b0
#define ITEM_TILT 0
#define LOW_LIGHT 0x1400
#define HIGH_LIGHT 0x1000

#define SELECT_ITEM IN_SELECT
#define DESELECT_ITEM IN_DESELECT
#define EXIT_INVENTORY (IN_DESELECT | IN_OPTION)
#define ROTATE_LEFT IN_LEFT
#define ROTATE_RIGHT IN_RIGHT

#define PHOTO_OPTION 147
#define PASSPORT_CLOSED 158
#define DETAIL_OPTION 181
#define SOUND_OPTION 182
#define CONTROL_OPTION 183
#define GUN_OPTION 185
#define SHOTGUN_OPTION 186
#define MAGNUM_OPTION 187
#define UZI_OPTION 188
#define HARPOON_OPTION 189
#define M16_OPTION 190
#define ROCKET_OPTION 191
#define GRENADE_OPTION 192
#define MEDI_OPTION 201
#define BIGMEDI_OPTION 202
#define FLAREBOX_OPTION 203

extern sint16 inv_keys_objects, inv_keys_current;
extern INVENTORY_ITEM *inv_keys_list[];
extern sint32 inv_nframes;
extern sint32 GlobePointLight;
extern sint16 JustSaved, NewAdventureLoadGame;
extern sint32 ls_adder;
extern TEXTSTRING *ammotext;
extern sint32 Inventory_DemoMode;
extern int noinput_count;
extern int reset_flag;
extern int game_closedown;
extern sint16 cdtrack;
extern sint32 inputDB;
extern sint32 idelay, idcount;
extern sint16 Option_Music_Volume;
extern uint32 CurrentLevel;

void GetGlobeMeshFlags(void);
void S_SetReverbType(int reverb);
void CreateMonoScreen(void);
void RemoveMonoScreen(int fade);
void S_FadeInInventory(int fade);
void S_FadeOutInventory(int fade);
void SOUND_Stop(void);
void SOUND_EndScene(void);
void S_CDVolume(int volume);
void S_CDLoop(void);
int S_UpdateInput(void);
void GetDemoInput(void);
void S_InitialisePolyList(int clear);
void DoInventoryPicture(void);
void DrawMonoScreen(int r, int g, int b);
void S_AnimateTextures(int nframes);
void phd_GenerateW2V(PHD_3DPOS *viewpos);
void phd_PushMatrix(void);
void phd_TranslateAbs(sint32 x, sint32 y, sint32 z);
void phd_RotYXZ(sint16 ry, sint16 rx, sint16 rz);
void phd_TranslateRel(sint32 x, sint32 y, sint32 z);
void DrawModeInfo(void);
void S_OutputPolyList(void);
int S_DumpScreen(void);
void Construct_Inventory(void);
sint32 AnimateInventoryItem(INVENTORY_ITEM *inv_item);
void DrawInventoryItem(INVENTORY_ITEM *inv_item);
void Inv_RingInit(RING_INFO *ring, sint16 type, INVENTORY_ITEM **list, sint16 qty, sint16 current, IMOTION_INFO *imo);
void Inv_RingCalcAdders(RING_INFO *ring, sint16 rotation_duration);
void Inv_RingDoMotions(RING_INFO *ring);
void Inv_RingGetView(RING_INFO *ring, PHD_3DPOS *viewer);
void Inv_RingLight(RING_INFO *ring, sint16 object_number);
void RingNotActive(INVENTORY_ITEM *inv_item);
void RingIsOpen(RING_INFO *ring);
void RingIsNotOpen(RING_INFO *ring);
void RingActive(void);
void Inv_RingRotateLeft(RING_INFO *ring);
void Inv_RingRotateRight(RING_INFO *ring);
void Inv_RingMotionSetup(RING_INFO *ring, sint16 status, sint16 status_target, sint16 frames);
void Inv_RingMotionRadius(RING_INFO *ring, sint16 target);
void Inv_RingMotionCameraPos(RING_INFO *ring, sint16 target);
void Inv_RingMotionRotation(RING_INFO *ring, sint16 rotation, sint16 target);
void Inv_RingMotionItemSelect(RING_INFO *ring, INVENTORY_ITEM *inv_item);
void Inv_RingMotionItemDeselect(RING_INFO *ring, INVENTORY_ITEM *inv_item);
void Inv_RingMotionCameraPitch(RING_INFO *ring, sint16 target);
void do_inventory_options(INVENTORY_ITEM *inv_item);
void RemoveInventoryText(void);
void S_FinishInventory(void);
void UseItem(sint16 object_num);
int SoundEffect(int nSFX, PHD_3DPOS *pPos, int flags);

int Display_Inventory(int inventory_mode)
{
    int demo_needed = 0;
    int n, frm, busy = 0, pass_open = 0;
    PHD_3DPOS viewer;
    sint16 angle, first_time = 0;
    sint32 tempSint32;
    sint16 icam2obj = CAMERA_2_RING;
    INVENTORY_ITEM *inv_item;
    sint16 itilt = ITEM_TILT;
    IMOTION_INFO imo = { 0 };
    RING_INFO ring = { 0 };

    if (inventory_mode == INV_LEVELSELECT_MODE) {
        first_time = 1;
        GetGlobeMeshFlags();
        JustSaved = NewAdventureLoadGame = 0;
        GlobePointLight = 0;
    }

    S_SetReverbType(0);
    if (inventory_mode == INV_KEYS_MODE && !inv_keys_objects) {
        Inventory_Chosen = -1;
        return 0;
    }

    if (inventory_mode != INV_TITLE_MODE && inventory_mode != INV_LEVELSELECT_MODE)
        CreateMonoScreen();

    T_RemovePrint(ammotext);
    ammotext = NULL;
    AlterFOV(80 * 182);
    Inventory_Mode = inventory_mode;
    inv_nframes = 2;
    Construct_Inventory();

    if (inventory_mode != INV_TITLE_MODE && inventory_mode != INV_LEVELSELECT_MODE)
        S_FadeInInventory(1);
    else
        S_FadeInInventory(0);
    SOUND_Stop();
    if (inventory_mode != INV_TITLE_MODE && inventory_mode != INV_LEVELSELECT_MODE)
        S_CDVolume(0);

    switch (inventory_mode) {
    case INV_DEATH_MODE:
    case INV_SAVE_MODE:
    case INV_LOAD_MODE:
    case INV_TITLE_MODE:
        Inv_RingInit(&ring, OPTION_RING, inv_option_list, inv_option_objects, inv_option_current, &imo);
        break;
    case INV_KEYS_MODE:
        Inv_RingInit(&ring, KEYS_RING, inv_keys_list, inv_keys_objects, inv_main_current, &imo);
        break;
    case INV_LEVELSELECT_MODE:
        Inv_RingInit(&ring, LEVELSELECT_RING, inv_levelselect_list, 1, 0, &imo);
        break;
    default:
        if (inv_main_objects)
            Inv_RingInit(&ring, MAIN_RING, inv_main_list, inv_main_objects, inv_main_current, &imo);
        else
            Inv_RingInit(&ring, OPTION_RING, inv_option_list, inv_option_objects, inv_option_current, &imo);
        break;
    }

    if (inventory_mode != INV_TITLE_MODE)
        SoundEffect(111, NULL, 2);
    inv_nframes = 2;

    do {
        GlobePointLight += 16;
        GlobePointLight &= 255;
        if ((inventory_mode == INV_TITLE_MODE || inventory_mode == INV_LEVELSELECT_MODE) && cdtrack > 0)
            S_CDLoop();

        Inv_RingCalcAdders(&ring, ROTATE_DURATION);
        S_UpdateInput();
        if (Inventory_DemoMode) {
            if (input)
                return gameflow.on_demo_interrupt;
            GetDemoInput();
            if (input == -1)
                return gameflow.on_demo_end;
        } else {
            if (input)
                noinput_count = 0;
        }
        inputDB = GetDebouncedInput(input);

        if (Inventory_Mode == INV_TITLE_MODE && !input && !inputDB) {
            if (gameflow.num_demos || gameflow.noinput_timeout) {
                noinput_count++;
                if (noinput_count > gameflow.noinput_time)
                    reset_flag = demo_needed = 1;
            }
        } else {
            noinput_count = 0;
            reset_flag = 0;
        }
        if (game_closedown)
            return EXIT_TO_TITLE;

        if ((Inventory_Mode == INV_SAVE_MODE || Inventory_Mode == INV_LOAD_MODE || Inventory_Mode == INV_DEATH_MODE)
            && !pass_open) {
            inputDB = SELECT_ITEM;
            input = 0;
        }
        if (Inventory_Mode == INV_LEVELSELECT_MODE && first_time && !ring.imo->count) {
            inputDB = SELECT_ITEM;
            first_time = input = 0;
        }

        for (frm = 0; frm < inv_nframes; frm++) {
            if (idelay) {
                if (idcount)
                    idcount--;
                else
                    idelay = 0;
            }
            Inv_RingDoMotions(&ring);
        }

        ring.camerapos.z_pos = ring.radius + icam2obj;
        S_InitialisePolyList(0);
        if (Inventory_Mode == INV_TITLE_MODE || Inventory_Mode == INV_LEVELSELECT_MODE)
            DoInventoryPicture();
        else
            DrawMonoScreen(0x50, 0x50, 0x70);
        S_AnimateTextures(inv_nframes);
        Inv_RingGetView(&ring, &viewer);
        phd_GenerateW2V(&viewer);
        Inv_RingLight(&ring, 0);

        phd_PushMatrix();
        phd_TranslateAbs(ring.ringpos.x_pos, ring.ringpos.y_pos, ring.ringpos.z_pos);
        phd_RotYXZ(ring.ringpos.y_rot, ring.ringpos.x_rot, ring.ringpos.z_rot);
        for (n = 0, angle = 0; n < ring.number_of_objects; n++) {
            inv_item = *(ring.list + n);
            if (n == ring.current_object) {
                for (frm = 0; frm < inv_nframes; frm++) {
                    if (ring.rotating) {
                        ls_adder = LOW_LIGHT;
                        if (inv_item->y_rot != 0) {
                            if (inv_item->y_rot < 0)
                                inv_item->y_rot += 0x200;
                            else
                                inv_item->y_rot -= 0x200;
                        }
                    } else if (imo.status == RNG_SELECTED || imo.status == RNG_DESELECTING
                        || imo.status == RNG_SELECTING || imo.status == RNG_DESELECT
                        || imo.status == RNG_CLOSING_ITEM) {
                        ls_adder = HIGH_LIGHT;
                        if (inv_item->y_rot != inv_item->y_rot_sel) {
                            tempSint32 = inv_item->y_rot_sel - inv_item->y_rot;
                            if (tempSint32 > 0 && tempSint32 < 0x8000)
                                inv_item->y_rot += 0x400;
                            else
                                inv_item->y_rot -= 0x400;
                            inv_item->y_rot &= 0xfc00;
                        }
                    } else if (ring.number_of_objects == 1 || !(input & IN_LEFT || input & IN_RIGHT)) {
                        ls_adder = HIGH_LIGHT;
                        inv_item->y_rot += 0x100;
                    }
                }
                if ((imo.status == RNG_OPEN || imo.status == RNG_SELECTING || imo.status == RNG_SELECTED
                        || imo.status == RNG_DESELECTING || imo.status == RNG_DESELECT
                        || imo.status == RNG_CLOSING_ITEM)
                    && !ring.rotating && !(input & IN_LEFT || input & IN_RIGHT))
                    RingNotActive(inv_item);
            } else {
                ls_adder = LOW_LIGHT;
                for (frm = 0; frm < inv_nframes; frm++) {
                    if (inv_item->y_rot != 0) {
                        if (inv_item->y_rot < 0)
                            inv_item->y_rot += 0x100;
                        else
                            inv_item->y_rot -= 0x100;
                    }
                }
            }

            if (imo.status == RNG_OPEN || imo.status == RNG_SELECTING || imo.status == RNG_SELECTED
                || imo.status == RNG_DESELECTING || imo.status == RNG_DESELECT || imo.status == RNG_CLOSING_ITEM)
                RingIsOpen(&ring);
            else
                RingIsNotOpen(&ring);

            if (imo.status == RNG_OPENING || imo.status == RNG_CLOSING || imo.status == RNG_MAIN2OPTION
                || imo.status == RNG_OPTION2MAIN || imo.status == RNG_EXITING_INVENTORY || imo.status == RNG_DONE
                || ring.rotating)
                RingActive();

            phd_PushMatrix();
            phd_RotYXZ(angle, 0, 0);
            phd_TranslateRel(ring.radius, 0, 0);
            phd_RotYXZ(0x4000, (sint16)(itilt + inv_item->pt_xrot), 0);
            DrawInventoryItem(inv_item);
            phd_PopMatrix();
            angle += ring.angle_adder;
            angle &= 0xffff;
        }
        phd_PopMatrix();

        DrawModeInfo();
        T_DrawText();
        S_OutputPolyList();
        SOUND_EndScene();
        camera.number_frames = inv_nframes = S_DumpScreen();
        if (CurrentLevel != 0)
            savegame.timer += inv_nframes / 2;

        if (!ring.rotating) {
            switch (imo.status) {
            case RNG_OPEN:
                if ((input & ROTATE_RIGHT) && ring.number_of_objects > 1) {
                    Inv_RingRotateLeft(&ring);
                    SoundEffect(108, NULL, 2);
                    break;
                }
                if ((input & ROTATE_LEFT) && ring.number_of_objects > 1) {
                    Inv_RingRotateRight(&ring);
                    SoundEffect(108, NULL, 2);
                    break;
                }

                if (reset_flag || (inputDB & EXIT_INVENTORY)) {
                    if (reset_flag || (Inventory_Mode != INV_TITLE_MODE && Inventory_Mode != INV_LEVELSELECT_MODE)) {
                        SoundEffect(112, NULL, 2);
                        Inventory_Chosen = -1;
                        if (ring.type == MAIN_RING)
                            inv_main_current = ring.current_object;
                        else
                            inv_option_current = ring.current_object;
                        if (inventory_mode != INV_TITLE_MODE && inventory_mode != INV_LEVELSELECT_MODE)
                            S_FadeOutInventory(1);
                        else
                            S_FadeOutInventory(0);
                        Inv_RingMotionSetup(&ring, RNG_CLOSING, RNG_DONE, CLOSE_FRAMES);
                        Inv_RingMotionRadius(&ring, 0);
                        Inv_RingMotionCameraPos(&ring, CAMERA_STARTHEIGHT);
                        Inv_RingMotionRotation(&ring, CLOSE_ROTATION, (sint16)(ring.ringpos.y_rot - CLOSE_ROTATION));
                        input = 0;
                        inputDB = 0;
                    }
                }

                if (inputDB & SELECT_ITEM) {
                    if ((Inventory_Mode == INV_SAVE_MODE || Inventory_Mode == INV_LOAD_MODE
                            || Inventory_Mode == INV_DEATH_MODE)
                        && !pass_open)
                        pass_open = 1;
                    item_data = 0;
                    if (ring.type == MAIN_RING) {
                        inv_main_current = ring.current_object;
                        inv_item = inv_main_list[ring.current_object];
                    } else if (ring.type == OPTION_RING) {
                        inv_option_current = ring.current_object;
                        inv_item = inv_option_list[ring.current_object];
                    } else if (ring.type == LEVELSELECT_RING) {
                        inv_option_current = ring.current_object;
                        inv_item = inv_levelselect_list[ring.current_object];
                    } else {
                        inv_keys_current = ring.current_object;
                        inv_item = inv_keys_list[ring.current_object];
                    }
                    inv_item->goal_frame = inv_item->open_frame;
                    inv_item->anim_direction = 1;
                    Inv_RingMotionSetup(&ring, RNG_SELECTING, RNG_SELECTED, SELECTING_FRAMES);
                    Inv_RingMotionRotation(&ring, 0, (sint16)(0xc000 - ring.current_object * ring.angle_adder));
                    Inv_RingMotionItemSelect(&ring, inv_item);
                    input = 0;
                    inputDB = 0;

                    switch (inv_item->object_number) {
                    case MAP_OPTION:
                        SoundEffect(113, NULL, 2);
                        break;
                    case GUN_OPTION:
                    case SHOTGUN_OPTION:
                    case MAGNUM_OPTION:
                    case UZI_OPTION:
                    case HARPOON_OPTION:
                    case M16_OPTION:
                    case ROCKET_OPTION:
                    case GRENADE_OPTION:
                        SoundEffect(114, NULL, 2);
                        break;
                    default:
                        SoundEffect(109, NULL, 2);
                        break;
                    }
                }

                if (inputDB & IN_FORWARD && inventory_mode != INV_TITLE_MODE && inventory_mode != INV_LEVELSELECT_MODE
                    && inventory_mode != INV_KEYS_MODE) {
                    if (ring.type == MAIN_RING) {
                        if (inv_keys_objects) {
                            Inv_RingMotionSetup(&ring, RNG_CLOSING, RNG_MAIN2KEYS, RINGSWITCH_FRAMES / 2);
                            Inv_RingMotionRadius(&ring, 0);
                            Inv_RingMotionRotation(&ring, CLOSE_ROTATION, (sint16)(ring.ringpos.y_rot - CLOSE_ROTATION));
                            Inv_RingMotionCameraPitch(&ring, 0x2000);
                            imo.misc = 0x2000;
                        }
                        input = 0;
                        inputDB = 0;
                    } else if (ring.type == OPTION_RING) {
                        if (inv_main_objects) {
                            Inv_RingMotionSetup(&ring, RNG_CLOSING, RNG_OPTION2MAIN, RINGSWITCH_FRAMES / 2);
                            Inv_RingMotionRadius(&ring, 0);
                            Inv_RingMotionRotation(&ring, CLOSE_ROTATION, (sint16)(ring.ringpos.y_rot - CLOSE_ROTATION));
                            Inv_RingMotionCameraPitch(&ring, 0x2000);
                            imo.misc = 0x2000;
                        }
                        inputDB = 0;
                    }
                } else if (inputDB & IN_BACK && inventory_mode != INV_TITLE_MODE
                    && inventory_mode != INV_LEVELSELECT_MODE && inventory_mode != INV_KEYS_MODE) {
                    if (ring.type == KEYS_RING) {
                        if (inv_main_objects) {
                            Inv_RingMotionSetup(&ring, RNG_CLOSING, RNG_KEYS2MAIN, RINGSWITCH_FRAMES / 2);
                            Inv_RingMotionRadius(&ring, 0);
                            Inv_RingMotionRotation(&ring, CLOSE_ROTATION, (sint16)(ring.ringpos.y_rot - CLOSE_ROTATION));
                            Inv_RingMotionCameraPitch(&ring, 0 - 0x2000);
                            imo.misc = 0 - 0x2000;
                        }
                        input = 0;
                        inputDB = 0;
                    } else if (ring.type == MAIN_RING) {
                        if (inv_option_objects && !gameflow.lockout_optionring) {
                            Inv_RingMotionSetup(&ring, RNG_CLOSING, RNG_MAIN2OPTION, RINGSWITCH_FRAMES / 2);
                            Inv_RingMotionRadius(&ring, 0);
                            Inv_RingMotionRotation(&ring, CLOSE_ROTATION, (sint16)(ring.ringpos.y_rot - CLOSE_ROTATION));
                            Inv_RingMotionCameraPitch(&ring, 0 - 0x2000);
                            imo.misc = 0 - 0x2000;
                        }
                        inputDB = 0;
                    }
                }
                break;

            case RNG_MAIN2OPTION:
                Inv_RingMotionSetup(&ring, RNG_OPENING, RNG_OPEN, RINGSWITCH_FRAMES / 2);
                Inv_RingMotionRadius(&ring, RING_RADIUS);
                imo.camera_pitch_target = 0;
                ring.camera_pitch = -(sint16)imo.misc;
                imo.camera_pitch_rate = imo.misc / (RINGSWITCH_FRAMES / 2);
                ring.list = inv_option_list;
                ring.type = OPTION_RING;
                inv_main_current = ring.current_object;
                ring.number_of_objects = inv_option_objects;
                ring.current_object = inv_option_current;
                Inv_RingCalcAdders(&ring, ROTATE_DURATION);
                Inv_RingMotionRotation(&ring, OPEN_ROTATION, (sint16)(0xc000 - ring.current_object * ring.angle_adder));
                ring.ringpos.y_rot = imo.rotate_target + OPEN_ROTATION;
                break;

            case RNG_OPTION2MAIN:
                Inv_RingMotionSetup(&ring, RNG_OPENING, RNG_OPEN, RINGSWITCH_FRAMES / 2);
                Inv_RingMotionRadius(&ring, RING_RADIUS);
                imo.camera_pitch_target = 0;
                ring.camera_pitch = -(sint16)imo.misc;
                imo.camera_pitch_rate = imo.misc / (RINGSWITCH_FRAMES / 2);
                ring.list = inv_main_list;
                ring.type = MAIN_RING;
                inv_option_objects = ring.number_of_objects;
                inv_option_current = ring.current_object;
                ring.number_of_objects = inv_main_objects;
                ring.current_object = inv_main_current;
                Inv_RingCalcAdders(&ring, ROTATE_DURATION);
                Inv_RingMotionRotation(&ring, OPEN_ROTATION, (sint16)(0xc000 - ring.current_object * ring.angle_adder));
                ring.ringpos.y_rot = imo.rotate_target + OPEN_ROTATION;
                break;

            case RNG_MAIN2KEYS:
                Inv_RingMotionSetup(&ring, RNG_OPENING, RNG_OPEN, RINGSWITCH_FRAMES / 2);
                Inv_RingMotionRadius(&ring, RING_RADIUS);
                imo.camera_pitch_target = 0;
                ring.camera_pitch = -(sint16)imo.misc;
                imo.camera_pitch_rate = imo.misc / (RINGSWITCH_FRAMES / 2);
                ring.list = inv_keys_list;
                ring.type = KEYS_RING;
                inv_main_objects = ring.number_of_objects;
                inv_main_current = ring.current_object;
                ring.number_of_objects = inv_keys_objects;
                ring.current_object = inv_keys_current;
                Inv_RingCalcAdders(&ring, ROTATE_DURATION);
                Inv_RingMotionRotation(&ring, OPEN_ROTATION, (sint16)(0xc000 - ring.current_object * ring.angle_adder));
                ring.ringpos.y_rot = imo.rotate_target + OPEN_ROTATION;
                break;

            case RNG_KEYS2MAIN:
                Inv_RingMotionSetup(&ring, RNG_OPENING, RNG_OPEN, RINGSWITCH_FRAMES / 2);
                Inv_RingMotionRadius(&ring, RING_RADIUS);
                imo.camera_pitch_target = 0;
                ring.camera_pitch = -(sint16)imo.misc;
                imo.camera_pitch_rate = imo.misc / (RINGSWITCH_FRAMES / 2);
                ring.list = inv_main_list;
                ring.type = MAIN_RING;
                inv_keys_current = ring.current_object;
                ring.number_of_objects = inv_main_objects;
                ring.current_object = inv_main_current;
                Inv_RingCalcAdders(&ring, ROTATE_DURATION);
                Inv_RingMotionRotation(&ring, OPEN_ROTATION, (sint16)(0xc000 - ring.current_object * ring.angle_adder));
                ring.ringpos.y_rot = imo.rotate_target + OPEN_ROTATION;
                break;

            case RNG_CLOSING_ITEM:
                inv_item = *(ring.list + ring.current_object);
                for (frm = 0; frm < inv_nframes; frm++) {
                    if (!AnimateInventoryItem(inv_item)) {
                        if (inv_item->object_number == PASSPORT_OPTION) {
                            inv_item->object_number = PASSPORT_CLOSED;
                            inv_item->current_frame = 0;
                        }
                        imo.count = SELECTING_FRAMES;
                        imo.status = imo.status_target;
                        Inv_RingMotionItemDeselect(&ring, inv_item);
                        break;
                    }
                }
                break;

            case RNG_DESELECT:
                SoundEffect(112, NULL, 2);
                Inv_RingMotionSetup(&ring, RNG_DESELECTING, RNG_OPEN, SELECTING_FRAMES);
                Inv_RingMotionRotation(&ring, 0, (sint16)(0xc000 - ring.current_object * ring.angle_adder));
                input = 0;
                inputDB = 0;
                break;

            case RNG_SELECTED:
                inv_item = *(ring.list + ring.current_object);
                if (inv_item->object_number == PASSPORT_CLOSED)
                    inv_item->object_number = PASSPORT_OPTION;
                for (frm = 0; frm < inv_nframes; frm++) {
                    busy = 0;
                    if (inv_item->y_rot == inv_item->y_rot_sel)
                        busy = AnimateInventoryItem(inv_item);
                }
                if (!busy && !idelay) {
                    do_inventory_options(inv_item);
                    if (inputDB & DESELECT_ITEM) {
                        inv_item->sprlist = NULL;
                        Inv_RingMotionSetup(&ring, RNG_CLOSING_ITEM, RNG_DESELECT, 0);
                        input = 0;
                        inputDB = 0;
                        if (Inventory_Mode == INV_LOAD_MODE || Inventory_Mode == INV_SAVE_MODE) {
                            Inv_RingMotionSetup(&ring, RNG_CLOSING_ITEM, RNG_EXITING_INVENTORY, 0);
                            input = inputDB = 0;
                        }
                    }
                    if (inputDB & SELECT_ITEM) {
                        inv_item->sprlist = NULL;
                        Inventory_Chosen = inv_item->object_number;
                        if (ring.type == MAIN_RING)
                            inv_main_current = ring.current_object;
                        else
                            inv_option_current = ring.current_object;
                        if (Inventory_Mode == INV_LEVELSELECT_MODE) {
                            if (inv_item->object_number == PASSPORT_OPTION && JustSaved)
                                Inv_RingMotionSetup(&ring, RNG_CLOSING_ITEM, RNG_DESELECT, 0);
                            else
                                Inv_RingMotionSetup(&ring, RNG_CLOSING_ITEM, RNG_EXITING_INVENTORY, 0);
                            JustSaved = 0;
                        } else if (Inventory_Mode == INV_TITLE_MODE
                            && (inv_item->object_number == DETAIL_OPTION || inv_item->object_number == SOUND_OPTION
                                || inv_item->object_number == CONTROL_OPTION))
                            Inv_RingMotionSetup(&ring, RNG_CLOSING_ITEM, RNG_DESELECT, 0);
                        else
                            Inv_RingMotionSetup(&ring, RNG_CLOSING_ITEM, RNG_EXITING_INVENTORY, 0);
                        input = 0;
                        inputDB = 0;
                    }
                }
                break;

            case RNG_EXITING_INVENTORY:
                if (!imo.count) {
                    if (inventory_mode != INV_TITLE_MODE && inventory_mode != INV_LEVELSELECT_MODE)
                        S_FadeOutInventory(1);
                    else
                        S_FadeOutInventory(0);
                    Inv_RingMotionSetup(&ring, RNG_CLOSING, RNG_DONE, CLOSE_FRAMES);
                    Inv_RingMotionRadius(&ring, 0);
                    Inv_RingMotionCameraPos(&ring, CAMERA_STARTHEIGHT);
                    Inv_RingMotionRotation(&ring, CLOSE_ROTATION, (sint16)(ring.ringpos.y_rot - CLOSE_ROTATION));
                }
                break;

            default:
                break;
            }
        }
    } while (imo.status != RNG_DONE);

    if (inventory_mode != INV_TITLE_MODE && inventory_mode != INV_LEVELSELECT_MODE) {
        if (Inventory_Chosen != PASSPORT_OPTION)
            RemoveMonoScreen(0);
        else {
            if (Inventory_ExtraData[0] == 1)
                RemoveMonoScreen(0);
            else
                RemoveMonoScreen(1);
        }
    }

    RemoveInventoryText();
    S_FinishInventory();
    Inventory_Displaying = 0;
    if (reset_flag)
        return EXIT_TO_TITLE;
    if (demo_needed)
        return STARTDEMO;

    if (Inventory_Chosen == -1) {
        if (inventory_mode != INV_TITLE_MODE && Option_Music_Volume)
            S_CDVolume(Option_Music_Volume * 25 + 5);
        return 0;
    }

    switch (Inventory_Chosen) {
    case PASSPORT_OPTION:
        if (Inventory_ExtraData[0] == 1 && Option_Music_Volume)
            S_CDVolume(Option_Music_Volume * 25 + 5);
        return 1;
        break;
    case PHOTO_OPTION:
        if (gameflow.gym_enabled) {
            Inventory_ExtraData[1] = 0;
            return 1;
        } else
            break;
    case DETAIL_OPTION:
    case SOUND_OPTION:
    case CONTROL_OPTION:
    case GAMMA_OPTION:
        break;
    case GUN_OPTION:
        UseItem(GUN_OPTION);
        break;
    case SHOTGUN_OPTION:
        UseItem(SHOTGUN_OPTION);
        break;
    case MAGNUM_OPTION:
        UseItem(MAGNUM_OPTION);
        break;
    case UZI_OPTION:
        UseItem(UZI_OPTION);
        break;
    case HARPOON_OPTION:
        UseItem(HARPOON_OPTION);
        break;
    case M16_OPTION:
        UseItem(M16_OPTION);
        break;
    case ROCKET_OPTION:
        UseItem(ROCKET_OPTION);
        break;
    case GRENADE_OPTION:
        UseItem(GRENADE_OPTION);
        break;
    case FLAREBOX_OPTION:
        UseItem(FLAREBOX_OPTION);
        break;
    case MEDI_OPTION:
        UseItem(MEDI_OPTION);
        break;
    case BIGMEDI_OPTION:
        UseItem(BIGMEDI_OPTION);
        break;
    default:
        break;
    }

    if (inventory_mode != INV_TITLE_MODE && inventory_mode != INV_LEVELSELECT_MODE && Option_Music_Volume)
        S_CDVolume(Option_Music_Volume * 25 + 5);
    return 0;
}

void Construct_Inventory(void)
{
    int n;
    INVENTORY_ITEM *inv_item;

    S_SetupAboveWater(0);
    if (Inventory_Mode != 1)
        TempVideoAdjust(HiResFlag, 1.0);
    phd_left = 0;
    phd_top = 0;
    phd_right = phd_winxmax;
    phd_bottom = phd_winymax;
    Inventory_Displaying = 1;
    Inventory_Chosen = 0;
    for (n = 0; n < 8; n++)
        Inventory_ExtraData[n] = 0;
    if (Inventory_Mode == 1) {
        if (gameflow.gym_enabled)
            inv_option_objects = 5;
        else
            inv_option_objects = 5;
    } else
        inv_option_objects = 4;
    for (n = 0; n < inv_main_objects; n++) {
        inv_item = inv_main_list[n];
        inv_item->drawn_meshes = inv_item->which_meshes;
        inv_item->current_frame = 0;
        inv_item->goal_frame = inv_item->current_frame;
        inv_item->anim_count = 0;
        inv_item->y_rot = 0;
    }
    for (n = 0; n < inv_option_objects; n++) {
        inv_item = inv_option_list[n];
        inv_item->current_frame = 0;
        inv_item->goal_frame = inv_item->current_frame;
        inv_item->anim_count = 0;
        inv_item->y_rot = 0;
    }
    for (n = 0; n < 1; n++) {
        inv_item = inv_levelselect_list[n];
        if (n == 0)
            inv_item->drawn_meshes = 0xffffffff;
        else
            inv_item->drawn_meshes = 0xf81;
        inv_item->current_frame = 0;
        inv_item->goal_frame = inv_item->current_frame;
        inv_item->anim_count = 0;
        inv_item->y_rot = 0;
    }
    inv_main_current = 0;
    if (OpenInvOnGym && Inventory_Mode == 1 && !gameflow.loadsave_disabled && gameflow.gym_enabled)
        inv_option_current = 4;
    else {
        inv_option_current = 0;
        OpenInvOnGym = 0;
    }
    item_data = 0;
}

void SelectMeshes(INVENTORY_ITEM *inv_item)
{
    int frame;

    if (inv_item->object_number == PASSPORT_OPTION) {
        frame = inv_item->current_frame;
        if (frame < 4)
            inv_item->drawn_meshes = 0x17;
        else if (frame <= 16)
            inv_item->drawn_meshes = 0x57;
        else if (frame < 19)
            inv_item->drawn_meshes = 0x5f;
        else if (frame == 19)
            inv_item->drawn_meshes = 0x5b;
        else if (frame < 24)
            inv_item->drawn_meshes = 0x7b;
        else if (frame < 29)
            inv_item->drawn_meshes = 0x3b;
        else if (frame == 29)
            inv_item->drawn_meshes = 0x13;
    } else if (inv_item->object_number == MAP_OPTION) {
        frame = inv_item->current_frame;
        if (frame && frame < 18)
            inv_item->drawn_meshes = 0xffffffff;
        else
            inv_item->drawn_meshes = inv_item->which_meshes;
    } else if (inv_item->object_number != GAMMA_OPTION) {
        inv_item->drawn_meshes = 0xffffffff;
    }
}

sint32 AnimateInventoryItem(INVENTORY_ITEM *inv_item)
{
    if (inv_item->current_frame != inv_item->goal_frame) {
        if (inv_item->anim_count) {
            inv_item->anim_count--;
        } else {
            inv_item->anim_count = inv_item->anim_speed;
            inv_item->current_frame += inv_item->anim_direction;
            if (inv_item->current_frame >= inv_item->frames_total)
                inv_item->current_frame = 0;
            else if (inv_item->current_frame < 0)
                inv_item->current_frame = inv_item->frames_total - 1;
        }
        SelectMeshes(inv_item);
        return 1;
    }
    SelectMeshes(inv_item);
    return 0;
}

extern uint8 GlobeLevelComplete[];
extern sint16 GlobeLevel;

extern char bInvItemsOff;
extern sint32 nPolyType;
extern sint16 GlobeXRot, GlobeYRot, GlobeZRot;
extern sint32 *bones;
extern sint32 phd_znear, phd_zfar, phd_persp, phd_centerx, phd_centery;
#define M03 3
#define M13 7
#define M23 11

typedef struct static_info {
    sint16 mesh_number;
    sint16 flags;
    sint16 x_minp;
    sint16 x_maxp;
    sint16 y_minp;
    sint16 y_maxp;
    sint16 z_minp;
    sint16 z_maxp;
    sint16 x_minc;
    sint16 x_maxc;
    sint16 y_minc;
    sint16 y_maxc;
    sint16 z_minc;
    sint16 z_maxc;
} STATIC_INFO;

extern STATIC_INFO static_objects[];
void phd_PushMatrix(void);
void phd_TranslateRel(sint32 x, sint32 y, sint32 z);
void phd_RotZ(sint16 rz);
void phd_RotYXZ(sint16 ry, sint16 rx, sint16 rz);
void gar_RotYXZsuperpack(sint16 **pprot, int skip);
void phd_PutPolygons(sint16 *objptr, int clip);
int S_GetObjectBounds(sint16 *bptr);
void S_DrawSprite(uint32 flags, sint32 x, sint32 y, sint32 z, sint16 sprnum, sint16 shade, sint16 scale);
void S_DrawScreenSprite(sint32 sx, sint32 sy, sint32 z, sint32 scaleH, sint32 scaleV, sint16 sprnum, sint16 shade, uint16 flags);
void S_DrawScreenLine(sint32 sx, sint32 sy, sint32 z, sint32 w, sint32 h, sint32 col, uint16 *grdptr, uint16 flags);
void S_DrawScreenBox(sint32 sx, sint32 sy, sint32 z, sint32 w, sint32 h, sint32 col, uint16 *grdptr, uint16 flags);
void S_DrawScreenFBox(sint32 sx, sint32 sy, sint32 z, sint32 w, sint32 h, sint32 col, uint16 *grdptr, uint16 flags);
void Inv_GlobeLight(sint16 mesh_num);

void DrawInventoryItem(INVENTORY_ITEM *inv_item)
{
    int hours, minutes, seconds, totsec;
    int clip, i, poppush;
    sint16 mesh, mesh_num;
    OBJECT_INFO *object;
    sint32 *bone;
    sint16 *rotation;
    sint16 *frame;
    INVENTORY_SPRITE *isprite, **isprite_list;
    sint32 xv, yv, zv, zp;
    sint32 sx, sy;

    nPolyType = 7;
    if (bInvItemsOff)
        return;
    if (inv_item->object_number == 146) {
        totsec = savegame.timer / 30;
        seconds = totsec % 60;
        minutes = ((totsec % 3600) * -91) / 5;
        hours = ((totsec / 12) * -91) / 5;
        seconds *= -1092;
    } else
        hours = minutes = seconds = totsec = 0;
    phd_TranslateRel(0, inv_item->ytrans, inv_item->ztrans);
    phd_RotYXZ(inv_item->y_rot, inv_item->x_rot, 0);
    if (inv_item->object_number == 184)
        phd_RotYXZ(GlobeYRot, GlobeXRot, GlobeZRot);
    object = &objects[inv_item->object_number];
    if (!object->loaded)
        return;
    if (object->nmeshes < 0) {
        S_DrawSprite(0, 0, 0, 0, object->mesh_index, 0, 0);
        return;
    }
    if ((isprite_list = inv_item->sprlist)) {
        zv = *(phd_mxptr + M23);
        xv = *(phd_mxptr + M03);
        yv = *(phd_mxptr + M13);
        zp = zv / phd_persp;
        sx = xv / zp + phd_centerx;
        sy = yv / zp + phd_centery;
        while ((isprite = *isprite_list++)) {
            if (zv < phd_znear || zv > phd_zfar)
                break;
            while (isprite->shape) {
                switch (isprite->shape) {
                case 1:
                    S_DrawScreenSprite(sx + isprite->x, sy + isprite->y, isprite->z, isprite->param1, isprite->param2, (sint16)(static_objects[356].mesh_number + isprite->sprnum), 4096, 0);
                    break;
                case 2:
                    S_DrawScreenLine(sx + isprite->x, sy + isprite->y, isprite->z, isprite->param1, isprite->param2, isprite->sprnum, (uint16 *)isprite->grdptr, 0);
                    break;
                case 3:
                    S_DrawScreenBox(sx + isprite->x, sy + isprite->y, isprite->z, isprite->param1, isprite->param2, isprite->sprnum, (uint16 *)isprite->grdptr, 0);
                    break;
                case 4:
                    S_DrawScreenFBox(sx + isprite->x, sy + isprite->y, isprite->z, isprite->param1, isprite->param2, isprite->sprnum, (uint16 *)isprite->grdptr, 0);
                    break;
                }
                isprite++;
            }
        }
    }
    frame = object->frame_base + inv_item->current_frame * (anims[object->anim_index].interpolation >> 8);
    phd_PushMatrix();
    if ((clip = S_GetObjectBounds(frame))) {
        phd_TranslateRel(*(frame + 6), *(frame + 7), *(frame + 8));
        rotation = frame + 9;
        gar_RotYXZsuperpack(&rotation, 0);
        mesh = object->mesh_index;
        bone = bones + object->bone_index;
        mesh_num = 1;
        if (inv_item->object_number == 184)
            inv_item->drawn_meshes = ~8;
        if (mesh_num & inv_item->drawn_meshes) {
            if (inv_item->object_number == 184) {
                Inv_GlobeLight(mesh_num);
                phd_PutPolygons(meshes[mesh], clip);
            } else
                phd_PutPolygons(meshes[mesh], clip);
        }
        for (i = object->nmeshes - 1; i > 0; i--) {
            mesh++;
            mesh_num <<= 1;
            poppush = *(bone++);
            if (poppush & 1)
                phd_PopMatrix();
            if (poppush & 2)
                phd_PushMatrix();
            phd_TranslateRel(*bone, *(bone + 1), *(bone + 2));
            gar_RotYXZsuperpack(&rotation, 0);
            if (inv_item->object_number == 146) {
                if (i == 1) {
                    phd_RotZ((sint16)seconds);
                    inv_item->misc_data[1] = inv_item->misc_data[0];
                    inv_item->misc_data[0] = seconds;
                }
                if (i == 2)
                    phd_RotZ((sint16)minutes);
                if (i == 3)
                    phd_RotZ((sint16)hours);
            }
            if (inv_item->object_number == 184)
                Inv_GlobeLight(mesh_num);
            if (mesh_num & inv_item->drawn_meshes) {
                if (inv_item->object_number == 184)
                    phd_PutPolygons(meshes[mesh], clip);
                else
                    phd_PutPolygons(meshes[mesh], clip);
            }
            bone += 3;
        }
    }
    phd_PopMatrix();
}

sint32 GetDebouncedInput(sint32 input)
{
    int j;

    j = input ^ (input & oldInputDB);
    oldInputDB = input;
    input &= j;
    return input;
}

void GetGlobeMeshFlags(void)
{
    GlobeLevelComplete[0] = savegame.IndiaComplete;
    GlobeLevelComplete[1] = savegame.SPacificComplete;
    GlobeLevelComplete[2] = savegame.NevadaComplete;
    GlobeLevelComplete[3] = 255;
    GlobeLevelComplete[4] = savegame.LondonComplete;
    if (savegame.IndiaComplete && savegame.SPacificComplete && savegame.NevadaComplete && savegame.LondonComplete)
        GlobeLevelComplete[5] = savegame.AntarcticaComplete;
    else
        GlobeLevelComplete[5] = 255;
    GlobeLevel = 0;
    while (GlobeLevelComplete[GlobeLevel])
        GlobeLevel++;
}

void DoInventoryPicture(void)
{
    HWR_EnableZBuffer(0, 0);
    TRDrawPicture(0, CurPicTexIndices, f_zfar);
}

#ifdef TR3_EXE
sint16 GlobeLevelAngles[7][4] = {
    { -1536, -7936, 1536, 85 },
    { 1024, -512, -256, 87 },
    { 2560, 21248, -4096, 86 },
    { -3328, 29440, 1024, 89 },
    { 3072, -20992, 6400, 85 },
    { -5120, -15360, -18688, 88 },
    { 0, 0, 0, 0 },
};
sint16 Inventory_Chosen = -1;
sint32 Inventory_Mode = INV_TITLE_MODE;
sint16 Option_Detail_Level = 255;
sint16 Option_SFX_Volume = 165;
sint16 Option_Music_Volume = 255;
#endif
