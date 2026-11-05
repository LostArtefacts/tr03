#include "specific/standard.h"
#include "game.h"

typedef struct {
    sint32 Speed;
    sint32 MidPos;
    sint32 FrontPos;
    sint32 TurnX;
    sint32 TurnZ;
    sint16 TurnLen;
    sint16 TurnRot;
    sint16 YVel;
    sint16 Gradient;
    char Flags;
    char StopDelay;
} CARTINFO;

sint16 GetCollision(ITEM_INFO *v, sint16 ang, sint32 dist, sint16 *ceiling)
{
    sint16 room_number;
    FLOOR_INFO *floor;
    sint32 x, y, z, height, cheight;

    x = v->pos.x_pos + ((phd_sin(ang) * dist) >> W2V_SHIFT);
    y = v->pos.y_pos - LARA_HITE;
    z = v->pos.z_pos + ((phd_cos(ang) * dist) >> W2V_SHIFT);
    room_number = v->room_number;
    floor = GetFloor(x, y, z, &room_number);
    height = GetHeight(floor, x, y, z);
    cheight = GetCeiling(floor, x, y, z);
    *ceiling = (sint16)cheight;
    if (height != NO_HEIGHT)
        height -= v->pos.y_pos;
    return (sint16)height;
}

void MineCartInitialise(sint16 item_number)
{
    ITEM_INFO *v;

    v = &items[item_number];
    v->data = game_malloc(sizeof(CARTINFO), 0);
}

int TestBoundsCollide(ITEM_INFO *item, ITEM_INFO *laraitem, sint32 radius);
int TestCollision(ITEM_INFO *item, ITEM_INFO *laraitem);

int GetInMineCart(ITEM_INFO *v, ITEM_INFO *l, COLL_INFO *coll);
void CreateFlare(int thrown);
sint32 mGetAngle(sint32 x1, sint32 y1, sint32 x2, sint32 y2);
void S_CDPlay(sint32 track, int mode);
void ObjectCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);

void MineCartCollision(sint16 item_number, ITEM_INFO *l, COLL_INFO *coll)
{
    int geton;
    ITEM_INFO *v;

    if (l->hit_points < 0 || lara.skidoo != NO_ITEM)
        return;

    v = &items[item_number];
    if ((geton = GetInMineCart(v, l, coll))) {
        sint16 ang;
        CARTINFO *cart;

        lara.skidoo = item_number;
        if (lara.gun_type == LG_FLARE) {
            CreateFlare(0);
            undraw_flare_meshes();
            lara.flare_control_left = 0;
            lara.request_gun_type = lara.gun_type = LG_ARMLESS;
        }
        lara.gun_status = LG_HANDSBUSY;

        ang = (sint16)mGetAngle(v->pos.x_pos, v->pos.z_pos, l->pos.x_pos, l->pos.z_pos) - v->pos.y_rot;
        if (ang > -(ONE_DEGREE * 45) && ang < ONE_DEGREE * 135)
            l->anim_number = objects[VEHICLE_ANIM].anim_index + 46;
        else
            l->anim_number = objects[VEHICLE_ANIM].anim_index;
        l->frame_number = anims[l->anim_number].frame_base;
        l->current_anim_state = l->goal_anim_state = 0;
        l->pos.x_pos = v->pos.x_pos;
        l->pos.y_pos = v->pos.y_pos;
        l->pos.z_pos = v->pos.z_pos;
        l->pos.x_rot = v->pos.x_rot;
        l->pos.y_rot = v->pos.y_rot;
        l->pos.z_rot = v->pos.z_rot;

        cart = (CARTINFO *)v->data;
        cart->Flags = 0;
        cart->Speed = 0;
        cart->YVel = 0;
        cart->Gradient = 0;
        S_CDPlay(12, 0);
    } else {
        ObjectCollision(item_number, l, coll);
    }
}

int GetInMineCart(ITEM_INFO *v, ITEM_INFO *l, COLL_INFO *coll)
{
    int dist;
    sint32 x, z;
    FLOOR_INFO *floor;
    sint16 room_number;

    if (!(input & IN_ACTION) || lara.gun_status != LG_ARMLESS || l->gravity_status)
        return 0;
    if (!TestBoundsCollide(v, l, coll->radius))
        return 0;
    if (!TestCollision(v, l))
        return 0;
    x = l->pos.x_pos - v->pos.x_pos;
    z = l->pos.z_pos - v->pos.z_pos;
    dist = x * x + z * z;
    if (dist > 200000)
        return 0;
    room_number = v->room_number;
    floor = GetFloor(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, &room_number);
    if (GetHeight(floor, v->pos.x_pos, v->pos.y_pos, v->pos.z_pos) < -32000)
        return 0;
    return 1;
}

void DoUserInput(ITEM_INFO *v, ITEM_INFO *l, CARTINFO *cart);
void MoveCart(ITEM_INFO *v, ITEM_INFO *l, CARTINFO *cart);

int MineCartControl(void)
{
    CARTINFO *cart;
    ITEM_INFO *v, *l;
    FLOOR_INFO *floor;
    sint16 room_number;

    l = lara_item;
    v = &items[lara.skidoo];
    cart = (CARTINFO *)v->data;

    DoUserInput(v, l, cart);
    if (cart->Flags & 16)
        MoveCart(v, l, cart);

    if (lara.skidoo != NO_ITEM) {
        l->pos.x_pos = v->pos.x_pos;
        l->pos.y_pos = v->pos.y_pos;
        l->pos.z_pos = v->pos.z_pos;
        l->pos.x_rot = v->pos.x_rot;
        l->pos.y_rot = v->pos.y_rot;
        l->pos.z_rot = v->pos.z_rot;
    }

    room_number = v->room_number;
    floor = GetFloor(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, &room_number);
    GetHeight(floor, v->pos.x_pos, v->pos.y_pos, v->pos.z_pos);
    if (room_number != v->room_number) {
        ItemNewRoom(lara.skidoo, room_number);
        ItemNewRoom(lara.item_number, room_number);
    }

    TestTriggers(trigger_index, 0);

    if (!(cart->Flags & 128)) {
        camera.target_elevation = -45 * ONE_DEGREE;
        camera.target_distance = WALL_L * 2;
    }

    return (lara.skidoo == NO_ITEM) ? 0 : 1;
}

int TestBoundsCollide(ITEM_INFO *item, ITEM_INFO *laraitem, sint32 radius);
void DoLotsOfBlood(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num, int number);

void CartToBaddieCollision(ITEM_INFO *v)
{
    int i;
    sint16 roomies[20], numroom, *door;
    sint16 item_num, nex;
    ITEM_INFO *item;
    OBJECT_INFO *object;
    int x, y, z;

    numroom = 1;
    roomies[0] = v->room_number;
    if ((door = room[v->room_number].door)) {
        for (i = (int)*(door++); i > 0; i--) {
            roomies[numroom++] = *door;
            door += 16;
        }
    }
    for (i = 0; i < numroom; i++) {
        item_num = room[roomies[i]].item_number;
        while (item_num != NO_ITEM) {
            item = &items[item_num];
            nex = item->next_item;
            if (item->collidable && item->status != INVISIBLE) {
                object = &objects[item->object_number];
                if (object->collision && (object->intelligent || item->object_number == 350)) {
                    x = v->pos.x_pos - item->pos.x_pos;
                    y = v->pos.y_pos - item->pos.y_pos;
                    z = v->pos.z_pos - item->pos.z_pos;
                    if (x > -2048 && x < 2048 && z > -2048 && z < 2048 && y > -2048 && y < 2048) {
                        if (TestBoundsCollide(item, v, 256)) {
                            if (item->object_number == 350) {
                                if (item->frame_number == anims[item->anim_number].frame_base &&
                                    lara_item->current_anim_state == 18 &&
                                    lara_item->anim_number == objects[11].anim_index + 6) {
                                    sint16 frame;
                                    FLOOR_INFO *floor;
                                    sint16 room_number;

                                    frame = lara_item->frame_number - anims[lara_item->anim_number].frame_base;
                                    if (frame >= 12 && frame <= 22) {
                                        SoundEffect(220, &item->pos, 2);
                                        room_number = item->room_number;
                                        floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
                                        GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
                                        TestTriggers(trigger_index, 1);
                                        item->frame_number++;
                                    }
                                }
                            } else {
                                DoLotsOfBlood(item->pos.x_pos, v->pos.y_pos - 256, item->pos.z_pos, v->speed,
                                              v->pos.y_rot, item->room_number, 3);
                                item->hit_points = 0;
                            }
                        }
                    }
                }
            }
            item_num = nex;
        }
    }
}
