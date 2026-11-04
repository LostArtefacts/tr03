#include "game.h"

extern sint16 draw_rooms[];
extern int number_draw_rooms;

void GetNewRoom(sint32 x, sint32 y, sint32 z, sint16 room_number);
int CollideStaticObjects(COLL_INFO *coll, sint32 x, sint32 y, sint32 z, sint16 room_number, sint32 hite);
void UpdateLaraRoom(ITEM_INFO *item, int height);
int TestBoundsCollide(ITEM_INFO *item, ITEM_INFO *laraitem, sint32 radius);
int TestCollision(ITEM_INFO *item, ITEM_INFO *laraitem);
void ItemPushLara(ITEM_INFO *item, ITEM_INFO *laraitem, COLL_INFO *coll, int spaz, int big_push);
sint16 GetTiltType(FLOOR_INFO *floor, sint32 x, sint32 y, sint32 z);
void LaraBaddieCollision(ITEM_INFO *laraitem, COLL_INFO *coll);
void CreatureCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void TrapCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
int TestLaraPosition(sint16 *bounds, ITEM_INFO *item, ITEM_INFO *laraitem);
void AlignLaraPosition(PHD_VECTOR *vec, ITEM_INFO *item, ITEM_INFO *laraitem);
int MoveLaraPosition(PHD_VECTOR *vec, ITEM_INFO *item, ITEM_INFO *laraitem);
void ObjectCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void DoorCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void ObjectCollisionSub(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);

sint32 FindGridShift(sint32 src, sint32 dst)
{
    sint32 srcw, dstw;

    srcw = src >> WALL_SHIFT;
    dstw = dst >> WALL_SHIFT;
    if (srcw == dstw)
        return 0;

    src &= WALL_L - 1;
    if (dstw > srcw)
        return WALL_L - (src - 1);
    else
        return -(src + 1);
}

void GetNearByRooms(sint32 x, sint32 y, sint32 z, sint32 r, sint32 h, sint16 room_number)
{
    draw_rooms[0] = room_number;
    number_draw_rooms = 1;
    GetNewRoom(x + r, y, z + r, room_number);
    GetNewRoom(x - r, y, z + r, room_number);
    GetNewRoom(x + r, y, z - r, room_number);
    GetNewRoom(x - r, y, z - r, room_number);
    GetNewRoom(x + r, y - h, z + r, room_number);
    GetNewRoom(x - r, y - h, z + r, room_number);
    GetNewRoom(x + r, y - h, z - r, room_number);
    GetNewRoom(x - r, y - h, z - r, room_number);
}

void GetNewRoom(sint32 x, sint32 y, sint32 z, sint16 room_number)
{
    int i;

    GetFloor(x, y, z, &room_number);
    for (i = 0; i < number_draw_rooms; i++)
    {
        if (draw_rooms[i] == room_number)
            break;
    }
    if (i == number_draw_rooms)
        draw_rooms[number_draw_rooms++] = room_number;
}

void ShiftItem(ITEM_INFO *item, COLL_INFO *coll)
{
    item->pos.x_pos += coll->shift.x;
    item->pos.y_pos += coll->shift.y;
    item->pos.z_pos += coll->shift.z;
    coll->shift.x = coll->shift.y = coll->shift.z = 0;
}

void UpdateLaraRoom(ITEM_INFO *item, int height)
{
    sint16 room_number;
    FLOOR_INFO *floor;
    sint32 x, y, z;

    x = item->pos.x_pos;
    y = item->pos.y_pos + height;
    z = item->pos.z_pos;
    room_number = item->room_number;
    floor = GetFloor(x, y, z, &room_number);
    item->floor = GetHeight(floor, x, y, z);
    if (item->room_number != room_number)
        ItemNewRoom(lara.item_number, room_number);
}

void ObjectCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll)
{
    ITEM_INFO *item;

    item = &items[item_num];
    if (!TestBoundsCollide(item, laraitem, coll->radius))
        return;
    if (!TestCollision(item, laraitem))
        return;
    if (coll->enable_baddie_push)
        ItemPushLara(item, laraitem, coll, 0, 1);
}

void ObjectCollisionSub(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll)
{
    ITEM_INFO *item;

    item = &items[item_num];
    if (!TestBoundsCollide(item, laraitem, coll->radius))
        return;
    if (!TestCollision(item, laraitem))
        return;
    ItemPushLara(item, laraitem, coll, 0, 0);
}

void DoorCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll)
{
    ITEM_INFO *item;

    item = &items[item_num];
    if (!TestBoundsCollide(item, laraitem, coll->radius))
        return;
    if (!TestCollision(item, laraitem))
        return;
    if (coll->enable_baddie_push)
    {
        if (item->current_anim_state != item->goal_anim_state)
            ItemPushLara(item, laraitem, coll, coll->enable_hit, 1);
        else
            ItemPushLara(item, laraitem, coll, 0, 1);
    }
}

void TrapCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll)
{
    ITEM_INFO *item;

    item = &items[item_num];
    if (item->status == ACTIVE)
    {
        if (!TestBoundsCollide(item, laraitem, coll->radius))
            return;
        TestCollision(item, laraitem);
        if (item->object_number == FAN && item->current_anim_state == 1)
            ObjectCollision(item_num, laraitem, coll);
    }
    else if (item->status != INVISIBLE)
        ObjectCollision(item_num, laraitem, coll);
}

sint16 *GetBestFrame(ITEM_INFO *item);

int TestBoundsCollide(ITEM_INFO *item, ITEM_INFO *laraitem, sint32 radius)
{
    sint32 s, c, rz, z, rx, x;
    sint32 minx, minz, maxz, maxx;
    sint16 *larabounds, *bounds;
    OBJECT_INFO *object;

    object = &objects[item->object_number];
    bounds = GetBestFrame(item);
    object = &objects[laraitem->object_number];
    larabounds = GetBestFrame(laraitem);

    if (item->pos.y_pos + bounds[3] <= laraitem->pos.y_pos + larabounds[2]
        || item->pos.y_pos + bounds[2] >= laraitem->pos.y_pos + larabounds[3])
        return 0;

    c = phd_cos(item->pos.y_rot);
    s = phd_sin(item->pos.y_rot);
    x = laraitem->pos.x_pos - item->pos.x_pos;
    z = laraitem->pos.z_pos - item->pos.z_pos;
    rx = (c * x - s * z) >> W2V_SHIFT;
    rz = (c * z + s * x) >> W2V_SHIFT;
    minx = bounds[0] - radius;
    maxx = bounds[1] + radius;
    minz = bounds[4] - radius;
    maxz = bounds[5] + radius;
    if (rx >= minx && rx <= maxx && rz >= minz && rz <= maxz)
        return 1;
    return 0;
}

#define M00 0
#define M01 1
#define M02 2
#define M10 4
#define M11 5
#define M12 6
#define M20 8
#define M21 9
#define M22 10

void phd_PushUnitMatrix(void);
void phd_RotYXZ(PHD_ANGLE ry, PHD_ANGLE rx, PHD_ANGLE rz);

int TestLaraPosition(sint16 *bounds, ITEM_INFO *item, ITEM_INFO *laraitem)
{
    PHD_ANGLE yrotrel, zrotrel, xrotrel;
    sint32 ry, rz, rx;
    sint32 *mptr;
    sint32 x, z, y;

    xrotrel = laraitem->pos.x_rot - item->pos.x_rot;
    yrotrel = laraitem->pos.y_rot - item->pos.y_rot;
    zrotrel = laraitem->pos.z_rot - item->pos.z_rot;
    if (xrotrel < bounds[6] || xrotrel > bounds[7] || yrotrel < bounds[8] || yrotrel > bounds[9]
        || zrotrel < bounds[10] || zrotrel > bounds[11])
        return 0;

    phd_PushUnitMatrix();
    phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);
    x = laraitem->pos.x_pos - item->pos.x_pos;
    y = laraitem->pos.y_pos - item->pos.y_pos;
    z = laraitem->pos.z_pos - item->pos.z_pos;
    mptr = phd_mxptr;
    rx = (mptr[M00] * x + mptr[M10] * y + mptr[M20] * z) >> W2V_SHIFT;
    ry = (mptr[M01] * x + mptr[M11] * y + mptr[M21] * z) >> W2V_SHIFT;
    rz = (mptr[M02] * x + mptr[M12] * y + mptr[M22] * z) >> W2V_SHIFT;
    phd_PopMatrix();
    if (rx < (sint32)bounds[0] || rx > (sint32)bounds[1] || ry < (sint32)bounds[2] || ry > (sint32)bounds[3]
        || rz < (sint32)bounds[4] || rz > (sint32)bounds[5])
        return 0;
    return 1;
}

void AlignLaraPosition(PHD_VECTOR *vec, ITEM_INFO *item, ITEM_INFO *laraitem)
{
    sint32 x, y, z, height, ceiling;
    FLOOR_INFO *floor;
    sint32 *mptr;
    sint16 room_number;

    laraitem->pos.x_rot = item->pos.x_rot;
    laraitem->pos.y_rot = item->pos.y_rot;
    laraitem->pos.z_rot = item->pos.z_rot;
    phd_PushUnitMatrix();
    phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);
    mptr = phd_mxptr;
    x = item->pos.x_pos + ((mptr[M00] * vec->x + mptr[M01] * vec->y + mptr[M02] * vec->z) >> W2V_SHIFT);
    y = item->pos.y_pos + ((mptr[M10] * vec->x + mptr[M11] * vec->y + mptr[M12] * vec->z) >> W2V_SHIFT);
    z = item->pos.z_pos + ((mptr[M20] * vec->x + mptr[M21] * vec->y + mptr[M22] * vec->z) >> W2V_SHIFT);
    phd_PopMatrix();

    room_number = laraitem->room_number;
    floor = GetFloor(x, y, z, &room_number);
    height = GetHeight(floor, x, y, z);
    ceiling = GetCeiling(floor, x, y, z);
    if (ABS(height - laraitem->pos.y_pos) > STEP_L || ABS(ceiling - laraitem->pos.y_pos) < LARA_HITE)
        return;

    laraitem->pos.x_pos = x;
    laraitem->pos.y_pos = y;
    laraitem->pos.z_pos = z;
}

int Move3DPosTo3DPos(PHD_3DPOS *srcpos, PHD_3DPOS *destpos, int velocity, PHD_ANGLE angadd);

int MoveLaraPosition(PHD_VECTOR *vec, ITEM_INFO *item, ITEM_INFO *laraitem)
{
    sint16 room_number;
    PHD_3DPOS dest;
    FLOOR_INFO *floor;
    sint32 dist, x, *mptr, height, y, z;

    dest.x_rot = item->pos.x_rot;
    dest.y_rot = item->pos.y_rot;
    dest.z_rot = item->pos.z_rot;
    phd_PushUnitMatrix();
    phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);
    mptr = phd_mxptr;
    dest.x_pos = item->pos.x_pos + ((mptr[M00] * vec->x + mptr[M01] * vec->y + mptr[M02] * vec->z) >> W2V_SHIFT);
    dest.y_pos = item->pos.y_pos + ((mptr[M10] * vec->x + mptr[M11] * vec->y + mptr[M12] * vec->z) >> W2V_SHIFT);
    dest.z_pos = item->pos.z_pos + ((mptr[M20] * vec->x + mptr[M21] * vec->y + mptr[M22] * vec->z) >> W2V_SHIFT);
    phd_PopMatrix();

    if (item->object_number == 179) {
        room_number = laraitem->room_number;
        floor = GetFloor(dest.x_pos, dest.y_pos, dest.z_pos, &room_number);
        height = GetHeight(floor, dest.x_pos, dest.y_pos, dest.z_pos);
        if (ABS(height - laraitem->pos.y_pos) > STEP_L * 2)
            return 0;
        x = dest.x_pos - laraitem->pos.x_pos;
        y = dest.y_pos - laraitem->pos.y_pos;
        z = dest.z_pos - laraitem->pos.z_pos;
        dist = phd_sqrt(x * x + y * y + z * z);
        if (dist < 128)
            return 1;
    }

    return Move3DPosTo3DPos(&laraitem->pos, &dest, 16, 2 * ONE_DEGREE);
}

int Move3DPosTo3DPos(PHD_3DPOS *srcpos, PHD_3DPOS *destpos, int velocity, PHD_ANGLE angadd)
{
    int dist, x, y, z;
    PHD_ANGLE angdif;

    x = destpos->x_pos - srcpos->x_pos;
    y = destpos->y_pos - srcpos->y_pos;
    z = destpos->z_pos - srcpos->z_pos;
    dist = phd_sqrt(x * x + y * y + z * z);
    if (velocity >= dist) {
        srcpos->x_pos = destpos->x_pos;
        srcpos->y_pos = destpos->y_pos;
        srcpos->z_pos = destpos->z_pos;
    } else {
        srcpos->x_pos += (x * velocity) / dist;
        srcpos->y_pos += (y * velocity) / dist;
        srcpos->z_pos += (z * velocity) / dist;
    }

    angdif = destpos->x_rot - srcpos->x_rot;
    if (angdif > angadd)
        srcpos->x_rot += angadd;
    else if (angdif < -angadd)
        srcpos->x_rot -= angadd;
    else
        srcpos->x_rot = destpos->x_rot;

    angdif = destpos->y_rot - srcpos->y_rot;
    if (angdif > angadd)
        srcpos->y_rot += angadd;
    else if (angdif < -angadd)
        srcpos->y_rot -= angadd;
    else
        srcpos->y_rot = destpos->y_rot;

    angdif = destpos->z_rot - srcpos->z_rot;
    if (angdif > angadd)
        srcpos->z_rot += angadd;
    else if (angdif < -angadd)
        srcpos->z_rot -= angadd;
    else
        srcpos->z_rot = destpos->z_rot;

    if (srcpos->x_pos == destpos->x_pos && srcpos->y_pos == destpos->y_pos && srcpos->z_pos == destpos->z_pos
        && srcpos->x_rot == destpos->x_rot && srcpos->y_rot == destpos->y_rot && srcpos->z_rot == destpos->z_rot)
        return 1;
    else
        return 0;
}
