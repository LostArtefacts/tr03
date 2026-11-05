// cflags: -/O2 /Od
#include "specific/standard.h"
#include "game.h"

extern GAME_VECTOR ForcedFixedCamera;
extern char UseForcedFixedCamera;

typedef struct {
    sint16 current_anim_state;
    sint16 goal_anim_state;
    PHD_3DPOS pos;
    PHD_3DPOS pos2;
} OLD_CAMERA;

extern GAME_VECTOR last_target;
extern GAME_VECTOR last_ideal;
extern OLD_CAMERA old_cam;
extern uint8 camerasnaps;
extern GAME_VECTOR static_lookcamp;
extern PHD_VECTOR static_lookcamt;
extern sint32 phd_persp;
extern PHD_VECTOR old_target;
extern sint16 Option_Music_Volume;
extern int chunky_flag;

void CalculateCamera(void);
void InGameCinematicCamera(void);
void S_CDVolume(int volume);
void FixedCamera(void);
sint32 mgLOS(GAME_VECTOR *start, GAME_VECTOR *target, sint32 push);
void phd_LookAt(sint32 xsrc, sint32 ysrc, sint32 zsrc, sint32 xtar, sint32 ytar, sint32 ztar, PHD_ANGLE roll);
int abs(int n);
void GetLaraHandAbsPosition(PHD_VECTOR *vec, int hand);
sint32 CameraCollisionBounds(GAME_VECTOR *ideal, sint32 push, sint32 yfirst);

void InitialiseCamera(void)
{
    camera.target.x = last_target.x = lara_item->pos.x_pos;
    camera.target.y = last_target.y = camera.shift = lara_item->pos.y_pos - 1024;
    camera.target.z = last_target.z = lara_item->pos.z_pos;
    camera.target.room_number = last_target.room_number = lara_item->room_number;
    camera.pos.x = camera.target.x;
    camera.pos.y = camera.target.y;
    camera.pos.z = camera.target.z - 100;
    camera.pos.room_number = camera.target.room_number;
    camera.target_distance = 1536;
    camera.item = NULL;
    camera.number_frames = 1;
    if (!lara.extra_anim)
        camera.type = CHASE_CAMERA;
    camera.speed = 1;
    camera.flags = 0;
    camera.bounce = 0;
    camera.number = -1;
    camera.fixed_camera = 0;
    AlterFOV(GAME_FOV * 182);
    UseForcedFixedCamera = 0;
    CalculateCamera();
}

void MoveCamera(GAME_VECTOR *ideal, int speed)
{
    FLOOR_INFO *floor;
    sint32 height, ceiling, shake;

    if (old_cam.pos.x_rot != lara_item->pos.x_rot
        || old_cam.pos.y_rot != lara_item->pos.y_rot
        || old_cam.pos.z_rot != lara_item->pos.z_rot
        || old_cam.pos2.x_rot != lara.head_x_rot
        || old_cam.pos2.y_rot != lara.head_y_rot
        || old_cam.pos2.x_pos != lara.torso_x_rot
        || old_cam.pos2.y_pos != lara.torso_y_rot
        || old_cam.pos.x_pos != lara_item->pos.x_pos
        || old_cam.pos.y_pos != lara_item->pos.y_pos
        || old_cam.pos.z_pos != lara_item->pos.z_pos
        || old_cam.current_anim_state != lara_item->current_anim_state
        || old_cam.goal_anim_state != lara_item->goal_anim_state
        || camera.old_type != camera.type) {
        old_cam.pos.x_rot = lara_item->pos.x_rot;
        old_cam.pos.y_rot = lara_item->pos.y_rot;
        old_cam.pos.z_rot = lara_item->pos.z_rot;
        old_cam.pos2.x_rot = lara.head_x_rot;
        old_cam.pos2.y_rot = lara.head_y_rot;
        old_cam.pos2.x_pos = lara.torso_x_rot;
        old_cam.pos2.y_pos = lara.torso_y_rot;
        old_cam.pos.x_pos = lara_item->pos.x_pos;
        old_cam.pos.y_pos = lara_item->pos.y_pos;
        old_cam.pos.z_pos = lara_item->pos.z_pos;
        old_cam.current_anim_state = lara_item->current_anim_state;
        old_cam.goal_anim_state = lara_item->goal_anim_state;
        last_ideal.x = ideal->x;
        last_ideal.y = ideal->y;
        last_ideal.z = ideal->z;
        last_ideal.room_number = ideal->room_number;
    } else {
        ideal->x = last_ideal.x;
        ideal->y = last_ideal.y;
        ideal->z = last_ideal.z;
        ideal->room_number = last_ideal.room_number;
    }

    camera.pos.x += (ideal->x - camera.pos.x) / speed;
    camera.pos.y += (ideal->y - camera.pos.y) / speed;
    camera.pos.z += (ideal->z - camera.pos.z) / speed;
    camera.pos.room_number = ideal->room_number;

    if (camera.bounce) {
        if (camera.bounce > 0) {
            camera.pos.y += camera.bounce;
            camera.target.y += camera.bounce;
            camera.bounce = 0;
        } else {
            shake = (GetRandomControl() - 0x4000) * camera.bounce / 0x7fff;
            camera.pos.x += shake;
            camera.target.y += shake;
            shake = (GetRandomControl() - 0x4000) * camera.bounce / 0x7fff;
            camera.pos.y += shake;
            camera.target.y += shake;
            shake = (GetRandomControl() - 0x4000) * camera.bounce / 0x7fff;
            camera.pos.z += shake;
            camera.target.z += shake;
            camera.bounce += 5;
        }
    }

    {
        sint16 room_number;
        sint32 wx, wy, wz;

        wx = camera.pos.x;
        wy = camera.pos.y;
        wz = camera.pos.z;
        room_number = camera.pos.room_number;
        floor = GetFloor(wx, wy + 256, wz, &room_number);
        if (room[room_number].flags & 0x80) {
            wy = room[room_number].maxceiling - 256;
            floor = GetFloor(wx, wy, wz, &camera.pos.room_number);
        }

        room_number = camera.pos.room_number;
        floor = GetFloor(wx, wy, wz, &room_number);
        height = GetHeight(floor, wx, wy, wz);
        ceiling = GetCeiling(floor, wx, wy, wz);
        if (wy < ceiling || wy > height) {
            sint32 dx, dy, dz;
            GAME_VECTOR temp1, temp2;

            mgLOS(&camera.target, &camera.pos, 0);
            dx = abs(camera.pos.x - ideal->x);
            dy = abs(camera.pos.y - ideal->y);
            dz = abs(camera.pos.z - ideal->z);
            if (dx < 768 && dy < 768 && dz < 768) {
                temp1.x = camera.pos.x;
                temp1.y = camera.pos.y;
                temp1.z = camera.pos.z;
                temp1.room_number = camera.pos.room_number;
                temp2.x = ideal->x;
                temp2.y = ideal->y;
                temp2.z = ideal->z;
                temp2.room_number = ideal->room_number;
                if (!mgLOS(&temp2, &temp1, 0)) {
                    camerasnaps++;
                    if (camerasnaps >= 8) {
                        camera.pos.x = ideal->x;
                        camera.pos.y = ideal->y;
                        camera.pos.z = ideal->z;
                        camera.pos.room_number = ideal->room_number;
                        camerasnaps = 0;
                    }
                }
            }
        }
    }

    {
        sint16 room_number;
        sint32 wx, wy, wz;

        wx = camera.pos.x;
        wy = camera.pos.y;
        wz = camera.pos.z;
        room_number = camera.pos.room_number;
        floor = GetFloor(wx, wy, wz, &room_number);
        height = GetHeight(floor, wx, wy, wz);
        ceiling = GetCeiling(floor, wx, wy, wz);
        if (wy - 255 < ceiling && wy + 255 > height && ceiling < height
            && ceiling != NO_HEIGHT && height != NO_HEIGHT)
            camera.pos.y = (height + ceiling) >> 1;
        else if (wy + 255 > height && ceiling < height && ceiling != NO_HEIGHT
                 && height != NO_HEIGHT)
            camera.pos.y = height - 255;
        else if (wy - 255 < ceiling && ceiling < height && ceiling != NO_HEIGHT
                 && height != NO_HEIGHT)
            camera.pos.y = ceiling + 255;
        else if (ceiling >= height || height == NO_HEIGHT || ceiling == NO_HEIGHT) {
            camera.pos.x = ideal->x;
            camera.pos.y = ideal->y;
            camera.pos.z = ideal->z;
            camera.pos.room_number = ideal->room_number;
        }
    }

    GetFloor(camera.pos.x, camera.pos.y, camera.pos.z, &camera.pos.room_number);
    phd_LookAt(camera.pos.x, camera.pos.y, camera.pos.z, camera.target.x,
               camera.target.y, camera.target.z, 0);

    if (camera.mike_at_lara) {
        camera.actual_angle = lara_item->pos.y_rot + lara.head_y_rot + lara.torso_y_rot;
        camera.mike_pos.x = lara_item->pos.x_pos;
        camera.mike_pos.y = lara_item->pos.y_pos;
        camera.mike_pos.z = lara_item->pos.z_pos;
    } else {
        camera.actual_angle = phd_atan(camera.target.z - camera.pos.z, camera.target.x - camera.pos.x);
        camera.mike_pos.x = camera.pos.x + ((phd_persp * phd_sin(camera.actual_angle)) >> W2V_SHIFT);
        camera.mike_pos.z = camera.pos.z + ((phd_persp * phd_cos(camera.actual_angle)) >> W2V_SHIFT);
        camera.mike_pos.y = camera.pos.y;
    }

    camera.old_type = camera.type;
}

void ChaseCamera(ITEM_INFO *item)
{
    FLOOR_INFO *floor;
    GAME_VECTOR ideal;
    GAME_VECTOR ideals[5];
    GAME_VECTOR temp[2];
    sint32 distance, lp, dx, dz, farthest, farthestnum, h, c;
    sint16 angle, room_number;

    if (!camera.target_elevation)
        camera.target_elevation = -10 * 182;

    camera.target_elevation += item->pos.x_rot;
    if (camera.target_elevation > 85 * 182)
        camera.target_elevation = 85 * 182;
    else if (camera.target_elevation < -85 * 182)
        camera.target_elevation = -85 * 182;

    distance = camera.target_distance * phd_cos(camera.target_elevation) >> W2V_SHIFT;
    room_number = camera.target.room_number;
    floor = GetFloor(camera.target.x, camera.target.y + 256, camera.target.z, &room_number);
    if (room[room_number].flags & 0x80)
        camera.target.y = room[room_number].maxceiling - 256;

    floor = GetFloor(camera.target.x, camera.target.y, camera.target.z, &camera.target.room_number);
    h = GetHeight(floor, camera.target.x, camera.target.y, camera.target.z);
    c = GetCeiling(floor, camera.target.x, camera.target.y, camera.target.z);
    if (c + 16 > h - 16 && h != NO_HEIGHT && c != NO_HEIGHT) {
        camera.target.y = (c + h) >> 1;
        camera.target_elevation = 0;
    } else if (camera.target.y > h - 16 && h != NO_HEIGHT) {
        camera.target.y = h - 16;
        camera.target_elevation = 0;
    } else if (camera.target.y < c + 16 && c != NO_HEIGHT) {
        camera.target.y = c + 16;
        camera.target_elevation = 0;
    }

    floor = GetFloor(camera.target.x, camera.target.y, camera.target.z, &camera.target.room_number);

    {
        sint16 room_number;
        sint32 wx, wy, wz;

        wx = camera.target.x;
        wy = camera.target.y;
        wz = camera.target.z;
        room_number = camera.target.room_number;
        floor = GetFloor(wx, wy, wz, &room_number);
        h = GetHeight(floor, wx, wy, wz);
        c = GetCeiling(floor, wx, wy, wz);
        if (wy < c || wy > h || c >= h || h == NO_HEIGHT || c == NO_HEIGHT) {
            camera.target.x = last_target.x;
            camera.target.y = last_target.y;
            camera.target.z = last_target.z;
            camera.target.room_number = last_target.room_number;
        }
    }

    for (lp = 0; lp < 5; lp++)
        ideals[lp].y = camera.target.y + (camera.target_distance * phd_sin(camera.target_elevation) >> W2V_SHIFT);

    farthest = 0x7fffffff;
    farthestnum = 0;

    for (lp = 0; lp < 5; lp++) {
        if (lp == 0)
            angle = item->pos.y_rot + camera.target_angle;
        else
            angle = (lp - 1) * 0x4000;

        ideals[lp].x = camera.target.x - ((distance * phd_sin(angle)) >> W2V_SHIFT);
        ideals[lp].z = camera.target.z - ((distance * phd_cos(angle)) >> W2V_SHIFT);
        ideals[lp].room_number = camera.target.room_number;

        if (mgLOS(&camera.target, &ideals[lp], 200)) {
            temp[0].x = ideals[lp].x;
            temp[0].y = ideals[lp].y;
            temp[0].z = ideals[lp].z;
            temp[0].room_number = ideals[lp].room_number;
            temp[1].x = camera.pos.x;
            temp[1].y = camera.pos.y;
            temp[1].z = camera.pos.z;
            temp[1].room_number = camera.pos.room_number;
            if (lp == 0 || mgLOS(&temp[0], &temp[1], 0)) {
                if (lp == 0) {
                    farthestnum = 0;
                    break;
                }
                dx = (camera.pos.x - ideals[lp].x) * (camera.pos.x - ideals[lp].x);
                dx += (camera.pos.z - ideals[lp].z) * (camera.pos.z - ideals[lp].z);
                if (dx < farthest) {
                    farthest = dx;
                    farthestnum = lp;
                }
            }
        } else if (lp == 0) {
            temp[0].x = ideals[lp].x;
            temp[0].y = ideals[lp].y;
            temp[0].z = ideals[lp].z;
            temp[0].room_number = ideals[lp].room_number;
            temp[1].x = camera.pos.x;
            temp[1].y = camera.pos.y;
            temp[1].z = camera.pos.z;
            temp[1].room_number = camera.pos.room_number;
            if (lp == 0 || mgLOS(&temp[0], &temp[1], 0)) {
                dx = (camera.target.x - ideals[lp].x) * (camera.target.x - ideals[lp].x);
                dz = (camera.target.z - ideals[lp].z) * (camera.target.z - ideals[lp].z);
                if (dx + dz > 768 * 768) {
                    farthestnum = 0;
                    break;
                }
            }
        }
    }

    ideal.x = ideals[farthestnum].x;
    ideal.y = ideals[farthestnum].y;
    ideal.z = ideals[farthestnum].z;
    ideal.room_number = ideals[farthestnum].room_number;

    CameraCollisionBounds(&ideal, 384, 1);

    if (camera.old_type == FIXED_CAMERA)
        camera.speed = 1;
    MoveCamera(&ideal, camera.speed);
}

void CombatCamera(ITEM_INFO *item)
{
    FLOOR_INFO *floor;
    GAME_VECTOR ideal;
    GAME_VECTOR ideals[9];
    GAME_VECTOR temp[2];
    sint32 distance, lp, dx, dz, farthest, farthestnum, h, c;
    sint16 angle, room_number;

    camera.target.z = item->pos.z_pos;
    camera.target.x = item->pos.x_pos;

    if (lara.target) {
        camera.target_angle = item->pos.y_rot + lara.target_angles[0];
        camera.target_elevation = item->pos.x_rot + lara.target_angles[1];
    } else {
        camera.target_angle = item->pos.y_rot + lara.head_y_rot + lara.torso_y_rot;
        camera.target_elevation = item->pos.x_rot + lara.head_x_rot + lara.torso_x_rot - 15 * 182;
    }

    room_number = camera.target.room_number;
    floor = GetFloor(camera.target.x, camera.target.y + 256, camera.target.z, &room_number);
    if (room[room_number].flags & 0x80)
        camera.target.y = room[room_number].maxceiling - 256;

    floor = GetFloor(camera.target.x, camera.target.y, camera.target.z, &camera.target.room_number);
    h = GetHeight(floor, camera.target.x, camera.target.y, camera.target.z);
    c = GetCeiling(floor, camera.target.x, camera.target.y, camera.target.z);
    if (c + 64 > h - 64 && h != NO_HEIGHT && c != NO_HEIGHT) {
        camera.target.y = (c + h) >> 1;
        camera.target_elevation = 0;
    } else if (camera.target.y > h - 64 && h != NO_HEIGHT) {
        camera.target.y = h - 64;
        camera.target_elevation = 0;
    } else if (camera.target.y < c + 64 && c != NO_HEIGHT) {
        camera.target.y = c + 64;
        camera.target_elevation = 0;
    }

    floor = GetFloor(camera.target.x, camera.target.y, camera.target.z, &camera.target.room_number);

    {
        sint16 room_number;
        sint32 wx, wy, wz;

        wx = camera.target.x;
        wy = camera.target.y;
        wz = camera.target.z;
        room_number = camera.target.room_number;
        floor = GetFloor(wx, wy, wz, &room_number);
        h = GetHeight(floor, wx, wy, wz);
        c = GetCeiling(floor, wx, wy, wz);
        if (wy < c || wy > h || c >= h || h == NO_HEIGHT || c == NO_HEIGHT) {
            camera.target.x = last_target.x;
            camera.target.y = last_target.y;
            camera.target.z = last_target.z;
            camera.target.room_number = last_target.room_number;
        }
    }

    camera.target_distance = 1536;
    distance = camera.target_distance * phd_cos(camera.target_elevation) >> W2V_SHIFT;

    for (lp = 0; lp < 5; lp++)
        ideals[lp].y = camera.target.y + (camera.target_distance * phd_sin(camera.target_elevation) >> W2V_SHIFT);

    farthest = 0x7fffffff;
    farthestnum = 0;

    for (lp = 0; lp < 5; lp++) {
        if (lp == 0)
            angle = camera.target_angle;
        else
            angle = (lp - 1) * 0x4000;

        ideals[lp].x = camera.target.x - ((distance * phd_sin(angle)) >> W2V_SHIFT);
        ideals[lp].z = camera.target.z - ((distance * phd_cos(angle)) >> W2V_SHIFT);
        ideals[lp].room_number = camera.target.room_number;

        if (mgLOS(&camera.target, &ideals[lp], 200)) {
            temp[0].x = ideals[lp].x;
            temp[0].y = ideals[lp].y;
            temp[0].z = ideals[lp].z;
            temp[0].room_number = ideals[lp].room_number;
            temp[1].x = camera.pos.x;
            temp[1].y = camera.pos.y;
            temp[1].z = camera.pos.z;
            temp[1].room_number = camera.pos.room_number;
            if (lp == 0 || mgLOS(&temp[0], &temp[1], 0)) {
                if (lp == 0) {
                    farthestnum = 0;
                    break;
                }
                dx = (camera.pos.x - ideals[lp].x) * (camera.pos.x - ideals[lp].x);
                dx += (camera.pos.z - ideals[lp].z) * (camera.pos.z - ideals[lp].z);
                if (dx < farthest) {
                    farthest = dx;
                    farthestnum = lp;
                }
            }
        } else if (lp == 0) {
            temp[0].x = ideals[lp].x;
            temp[0].y = ideals[lp].y;
            temp[0].z = ideals[lp].z;
            temp[0].room_number = ideals[lp].room_number;
            temp[1].x = camera.pos.x;
            temp[1].y = camera.pos.y;
            temp[1].z = camera.pos.z;
            temp[1].room_number = camera.pos.room_number;
            if (lp == 0 || mgLOS(&temp[0], &temp[1], 0)) {
                dx = (camera.target.x - ideals[lp].x) * (camera.target.x - ideals[lp].x);
                dz = (camera.target.z - ideals[lp].z) * (camera.target.z - ideals[lp].z);
                if (dx + dz > 768 * 768) {
                    farthestnum = 0;
                    break;
                }
            }
        }
    }

    ideal.x = ideals[farthestnum].x;
    ideal.y = ideals[farthestnum].y;
    ideal.z = ideals[farthestnum].z;
    ideal.room_number = ideals[farthestnum].room_number;

    CameraCollisionBounds(&ideal, 384, 1);

    if (camera.old_type == FIXED_CAMERA)
        camera.speed = 1;
    MoveCamera(&ideal, camera.speed);
}

void LookCamera(ITEM_INFO *item)
{
    GAME_VECTOR oldt, oldp, ideal;
    PHD_VECTOR pos1, pos2, pos3;
    FLOOR_INFO *floor;
    sint32 shake, lp;
    sint16 room_number, room_number2;
    sint32 dx, dy, dz, wx, wy, wz, clipped, h, c, hxrot, txrot, hyrot, tyrot, speed;

    hxrot = lara.head_x_rot;
    hyrot = lara.head_y_rot;
    txrot = lara.torso_x_rot;
    tyrot = lara.torso_y_rot;
    lara.torso_x_rot = 0;
    lara.torso_y_rot = 0;
    lara.head_x_rot <<= 1;
    lara.head_y_rot <<= 1;

    if (lara.head_x_rot > 55 * 182)
        lara.head_x_rot = 55 * 182;
    else if (lara.head_x_rot < -75 * 182)
        lara.head_x_rot = -75 * 182;
    if (lara.head_y_rot < -80 * 182)
        lara.head_y_rot = -80 * 182;
    else if (lara.head_y_rot > 80 * 182)
        lara.head_y_rot = 80 * 182;

    pos1.x = 0;
    pos1.y = 16;
    pos1.z = 64;
    GetLaraHandAbsPosition(&pos1, 4);

    clipped = 0;
    room_number = lara_item->room_number;
    floor = GetFloor(pos1.x, pos1.y + 256, pos1.z, &room_number);
    if (room[room_number].flags & 0x80) {
        pos1.y = room[room_number].maxceiling - 256;
        floor = GetFloor(pos1.x, pos1.y, pos1.z, &room_number);
    } else {
        floor = GetFloor(pos1.x, pos1.y, pos1.z, &room_number);
    }
    h = GetHeight(floor, pos1.x, pos1.y, pos1.z);
    c = GetCeiling(floor, pos1.x, pos1.y, pos1.z);
    if (h == NO_HEIGHT || c == NO_HEIGHT || c >= h || pos1.y > h || pos1.y < c)
        clipped = 1;

    if (clipped) {
        pos1.x = 0;
        pos1.y = 16;
        pos1.z = 0;
        GetLaraHandAbsPosition(&pos1, 4);

        clipped = 0;
        room_number = lara_item->room_number;
        floor = GetFloor(pos1.x, pos1.y + 256, pos1.z, &room_number);
        if (room[room_number].flags & 0x80) {
            pos1.y = room[room_number].maxceiling - 256;
            floor = GetFloor(pos1.x, pos1.y, pos1.z, &room_number);
        } else {
            floor = GetFloor(pos1.x, pos1.y, pos1.z, &room_number);
        }
        h = GetHeight(floor, pos1.x, pos1.y, pos1.z);
        c = GetCeiling(floor, pos1.x, pos1.y, pos1.z);
        if (h == NO_HEIGHT || c == NO_HEIGHT || c >= h || pos1.y > h || pos1.y < c)
            clipped = 1;

        if (clipped) {
            pos1.x = 0;
            pos1.y = 16;
            pos1.z = -64;
            GetLaraHandAbsPosition(&pos1, 4);
        }
    }

    pos2.x = 0;
    pos2.y = 0;
    pos2.z = -1024;
    GetLaraHandAbsPosition(&pos2, 4);
    pos3.x = 0;
    pos3.y = 0;
    pos3.z = 2048;
    GetLaraHandAbsPosition(&pos3, 4);

    dx = (pos2.x - pos1.x) >> 3;
    dy = (pos2.y - pos1.y) >> 3;
    dz = (pos2.z - pos1.z) >> 3;
    wx = pos1.x;
    wy = pos1.y;
    wz = pos1.z;

    room_number = room_number2 = lara_item->room_number;
    for (lp = 0; lp < 8; lp++) {
        room_number = room_number2;
        floor = GetFloor(wx, wy + 256, wz, &room_number2);
        if (room[room_number2].flags & 0x80) {
            wy = room[room_number2].maxceiling - 256;
            break;
        } else {
            floor = GetFloor(wx, wy, wz, &room_number2);
        }
        h = GetHeight(floor, wx, wy, wz);
        c = GetCeiling(floor, wx, wy, wz);
        if (h == NO_HEIGHT || c == NO_HEIGHT || c >= h || wy > h || wy < c)
            break;
        wx += dx;
        wy += dy;
        wz += dz;
    }

    if (lp) {
        wx -= dx;
        wy -= dy;
        wz -= dz;
    }

    ideal.x = wx;
    ideal.y = wy;
    ideal.z = wz;
    ideal.room_number = room_number;

    if (old_cam.pos.x_rot == lara.head_x_rot
        && old_cam.pos.y_rot == lara.head_y_rot
        && old_cam.pos.x_pos == lara_item->pos.x_pos
        && old_cam.pos.y_pos == lara_item->pos.y_pos
        && old_cam.pos.z_pos == lara_item->pos.z_pos
        && old_cam.current_anim_state == lara_item->current_anim_state
        && old_cam.goal_anim_state == lara_item->goal_anim_state
        && camera.old_type == LOOK_CAMERA) {
        ideal.x = static_lookcamp.x;
        ideal.y = static_lookcamp.y;
        ideal.z = static_lookcamp.z;
        ideal.room_number = static_lookcamp.room_number;
        pos3.x = static_lookcamt.x;
        pos3.y = static_lookcamt.y;
        pos3.z = static_lookcamt.z;
    } else {
        old_cam.pos.x_rot = lara.head_x_rot;
        old_cam.pos.y_rot = lara.head_y_rot;
        old_cam.pos.x_pos = lara_item->pos.x_pos;
        old_cam.pos.y_pos = lara_item->pos.y_pos;
        old_cam.pos.z_pos = lara_item->pos.z_pos;
        old_cam.current_anim_state = lara_item->current_anim_state;
        old_cam.goal_anim_state = lara_item->goal_anim_state;
        static_lookcamp.x = ideal.x;
        static_lookcamp.y = ideal.y;
        static_lookcamp.z = ideal.z;
        static_lookcamp.room_number = ideal.room_number;
        static_lookcamt.x = pos3.x;
        static_lookcamt.y = pos3.y;
        static_lookcamt.z = pos3.z;
    }

    CameraCollisionBounds(&ideal, 224, 1);

    if (camera.old_type == FIXED_CAMERA) {
        camera.pos.x = ideal.x;
        camera.pos.y = ideal.y;
        camera.pos.z = ideal.z;
        camera.target.x = pos3.x;
        camera.target.y = pos3.y;
        camera.target.z = pos3.z;
        camera.target.room_number = lara_item->room_number;
    } else {
        camera.pos.x += (ideal.x - camera.pos.x) >> 2;
        camera.pos.y += (ideal.y - camera.pos.y) >> 2;
        camera.pos.z += (ideal.z - camera.pos.z) >> 2;
        camera.target.x += (pos3.x - camera.target.x) >> 2;
        camera.target.y += (pos3.y - camera.target.y) >> 2;
        camera.target.z += (pos3.z - camera.target.z) >> 2;
        camera.target.room_number = lara_item->room_number;
    }

    if (camera.bounce && camera.type == camera.old_type) {
        if (camera.bounce > 0) {
            camera.pos.y += camera.bounce;
            camera.target.y += camera.bounce;
            camera.bounce = 0;
        } else {
            shake = ((GetRandomControl() - 0x4000) * camera.bounce) >> 15;
            camera.pos.x += shake;
            camera.target.y += shake;
            shake = ((GetRandomControl() - 0x4000) * camera.bounce) >> 15;
            camera.pos.y += shake;
            camera.target.y += shake;
            shake = ((GetRandomControl() - 0x4000) * camera.bounce) >> 15;
            camera.pos.z += shake;
            camera.target.z += shake;
            camera.bounce += 5;
        }
    }

    GetFloor(camera.pos.x, camera.pos.y, camera.pos.z, &camera.pos.room_number);

    {
        sint16 room_number;
        sint32 wx, wy, wz;

        wx = camera.pos.x;
        wy = camera.pos.y;
        wz = camera.pos.z;
        room_number = camera.pos.room_number;
        floor = GetFloor(wx, wy, wz, &room_number);
        h = GetHeight(floor, wx, wy, wz);
        c = GetCeiling(floor, wx, wy, wz);
        if (wy - 255 < c && wy + 255 > h && c < h && c != NO_HEIGHT && h != NO_HEIGHT)
            camera.pos.y = (h + c) >> 1;
        else if (wy + 255 > h && c < h && c != NO_HEIGHT && h != NO_HEIGHT)
            camera.pos.y = h - 255;
        else if (wy - 255 < c && c < h && c != NO_HEIGHT && h != NO_HEIGHT)
            camera.pos.y = c + 255;
    }

    {
        sint16 room_number;
        sint32 wx, wy, wz;

        wx = camera.pos.x;
        wy = camera.pos.y;
        wz = camera.pos.z;
        room_number = camera.pos.room_number;
        floor = GetFloor(wx, wy, wz, &room_number);
        h = GetHeight(floor, wx, wy, wz);
        c = GetCeiling(floor, wx, wy, wz);
        if (room[room_number].flags & 0x80)
            camera.pos.y = room[room_number].y - 256;
        else if (wy < c || wy > h || c >= h || h == NO_HEIGHT || c == NO_HEIGHT)
            mgLOS(&camera.target, &camera.pos, 0);
    }

    {
        sint16 room_number;
        sint32 wx, wy, wz;

        wx = camera.pos.x;
        wy = camera.pos.y;
        wz = camera.pos.z;
        room_number = camera.pos.room_number;
        floor = GetFloor(wx, wy, wz, &room_number);
        h = GetHeight(floor, wx, wy, wz);
        c = GetCeiling(floor, wx, wy, wz);
        if (wy < c || wy > h || c >= h || h == NO_HEIGHT || c == NO_HEIGHT
            || (room[room_number].flags & 0x80)) {
            camera.pos.x = pos1.x;
            camera.pos.y = pos1.y;
            camera.pos.z = pos1.z;
            camera.pos.room_number = lara_item->room_number;
        }
    }

    GetFloor(camera.pos.x, camera.pos.y, camera.pos.z, &camera.pos.room_number);
    phd_LookAt(camera.pos.x, camera.pos.y, camera.pos.z, camera.target.x,
               camera.target.y, camera.target.z, 0);

    if (camera.mike_at_lara) {
        camera.actual_angle = lara_item->pos.y_rot + lara.head_y_rot + lara.torso_y_rot;
        camera.mike_pos.x = lara_item->pos.x_pos;
        camera.mike_pos.y = lara_item->pos.y_pos;
        camera.mike_pos.z = lara_item->pos.z_pos;
    } else {
        camera.actual_angle = phd_atan(camera.target.z - camera.pos.z, camera.target.x - camera.pos.x);
        camera.mike_pos.x = camera.pos.x + ((phd_persp * phd_sin(camera.actual_angle)) >> W2V_SHIFT);
        camera.mike_pos.z = camera.pos.z + ((phd_persp * phd_cos(camera.actual_angle)) >> W2V_SHIFT);
        camera.mike_pos.y = camera.pos.y;
    }

    camera.old_type = camera.type;
    lara.head_x_rot = hxrot;
    lara.head_y_rot = hyrot;
    lara.torso_x_rot = txrot;
    lara.torso_y_rot = tyrot;
}

void CalculateCamera(void)
{
    ITEM_INFO *item;
    OBJECT_INFO *object;
    FLOOR_INFO *floor;
    sint16 *bounds, angle, tilt, change;
    sint32 shift, fixed_camera, y;

    old_target.x = camera.target.x;
    old_target.y = camera.target.y;
    old_target.z = camera.target.z;

    if (UseForcedFixedCamera) {
        camera.type = FIXED_CAMERA;
        camera.speed = 1;
    }

    if (room[camera.pos.room_number].flags & UNDERWATER) {
        SoundEffect(60, NULL, 2);
        if (!camera.underwater) {
            S_CDVolume(0);
            camera.underwater = 1;
        }
    } else if (camera.underwater) {
        if (Option_Music_Volume)
            S_CDVolume(Option_Music_Volume * 25 + 5);
        camera.underwater = 0;
    }

    if (camera.type == CINEMATIC_CAMERA) {
        InGameCinematicCamera();
        return;
    }

    if (camera.flags != NO_CHUNKY)
        chunky_flag = 0;

    fixed_camera = camera.item && (camera.type == FIXED_CAMERA || camera.type == HEAVY_CAMERA);
    item = fixed_camera ? camera.item : lara_item;

    object = &objects[item->object_number];
    bounds = GetBoundsAccurate(item);

    y = item->pos.y_pos;
    if (!fixed_camera)
        y += bounds[3] + ((bounds[2] - bounds[3]) * 3 >> 2);
    else
        y += (bounds[2] + bounds[3]) / 2;

    if (camera.item && !fixed_camera) {
        shift = phd_sqrt((camera.item->pos.z_pos - item->pos.z_pos) * (camera.item->pos.z_pos - item->pos.z_pos)
                         + (camera.item->pos.x_pos - item->pos.x_pos) * (camera.item->pos.x_pos - item->pos.x_pos));
        angle = phd_atan(camera.item->pos.z_pos - item->pos.z_pos, camera.item->pos.x_pos - item->pos.x_pos) - item->pos.y_rot;
        bounds = GetBoundsAccurate(camera.item);
        tilt = phd_atan(shift, y - (camera.item->pos.y_pos + (bounds[2] + bounds[3]) / 2));
        angle >>= 1;
        tilt >>= 1;
        if (angle > -50 * 182 && angle < 50 * 182 && tilt > -85 * 182 && tilt < 85 * 182) {
            change = angle - lara.head_y_rot;
            if (change > 4 * 182)
                lara.head_y_rot += 4 * 182;
            else if (change < -4 * 182)
                lara.head_y_rot -= 4 * 182;
            else
                lara.head_y_rot += change;
            lara.torso_y_rot = lara.head_y_rot;

            change = tilt - lara.head_x_rot;
            if (change > 4 * 182)
                lara.head_x_rot += 4 * 182;
            else if (change < -4 * 182)
                lara.head_x_rot -= 4 * 182;
            else
                lara.head_x_rot += change;
            lara.torso_x_rot = lara.head_x_rot;

            camera.type = LOOK_CAMERA;
            camera.item->looked_at = 1;
        }
    }

    if (camera.type == LOOK_CAMERA || camera.type == COMBAT_CAMERA) {
        if (camera.type == COMBAT_CAMERA) {
            last_target.x = camera.target.x;
            last_target.y = camera.target.y;
            last_target.z = camera.target.z;
            last_target.room_number = camera.target.room_number;
        }

        y -= 256;
        camera.target.room_number = item->room_number;

        if (camera.fixed_camera) {
            camera.target.y = y;
            camera.speed = 1;
        } else {
            camera.target.y += (y - camera.target.y) >> 2;
            camera.speed = camera.type == LOOK_CAMERA ? 4 : 8;
        }

        camera.fixed_camera = 0;

        if (camera.type == LOOK_CAMERA)
            LookCamera(item);
        else
            CombatCamera(item);
    } else {
        last_target.x = camera.target.x;
        last_target.y = camera.target.y;
        last_target.z = camera.target.z;
        last_target.room_number = camera.target.room_number;

        camera.target.x = item->pos.x_pos;
        camera.target.z = item->pos.z_pos;

        if (camera.flags == FOLLOW_CENTRE) {
            shift = (bounds[4] + bounds[5]) / 2;
            camera.target.z += (phd_cos(item->pos.y_rot) * shift) >> W2V_SHIFT;
            camera.target.x += (phd_sin(item->pos.y_rot) * shift) >> W2V_SHIFT;
        }
        camera.target.room_number = item->room_number;

        if (camera.fixed_camera ^ fixed_camera) {
            camera.target.y = y;
            camera.fixed_camera = 1;
            camera.speed = 1;
        } else {
            camera.target.y = y;
            camera.fixed_camera = 0;
        }

        if (camera.speed != 1 && camera.old_type != LOOK_CAMERA) {
            camera.target.x = old_target.x + ((camera.target.x - old_target.x) >> 2);
            camera.target.y = old_target.y + ((camera.target.y - old_target.y) >> 2);
            camera.target.z = old_target.z + ((camera.target.z - old_target.z) >> 2);
        }

        floor = GetFloor(camera.target.x, camera.target.y, camera.target.z, &camera.target.room_number);

        if (camera.type == CHASE_CAMERA || camera.flags == CHASE_OBJECT)
            ChaseCamera(item);
        else
            FixedCamera();
    }

    camera.last = camera.number;
    camera.fixed_camera = fixed_camera;

    if (camera.type != HEAVY_CAMERA || camera.timer == -1) {
        camera.type = CHASE_CAMERA;
        camera.speed = 10;
        camera.number = -1;
        camera.last_item = camera.item;
        camera.item = NULL;
        camera.target_angle = camera.target_elevation = 0;
        camera.target_distance = 1536;
        camera.flags = 0;
    }

    chunky_flag = 0;
}

void FixedCamera(void)
{
    GAME_VECTOR ideal;
    OBJECT_VECTOR *fixed;

    if (UseForcedFixedCamera) {
        ideal.x = ForcedFixedCamera.x;
        ideal.y = ForcedFixedCamera.y;
        ideal.z = ForcedFixedCamera.z;
        ideal.room_number = ForcedFixedCamera.room_number;
    } else {
        fixed = &camera.fixed[camera.number];
        ideal.x = fixed->x;
        ideal.y = fixed->y;
        ideal.z = fixed->z;
        ideal.room_number = fixed->data;
    }
    camera.fixed_camera = 1;
    MoveCamera(&ideal, 1);
    if (camera.timer) {
        camera.timer--;
        if (!camera.timer)
            camera.timer = -1;
    }
}

sint32 CameraCollisionBounds(GAME_VECTOR *ideal, sint32 push, sint32 yfirst)
{
    FLOOR_INFO *floor;
    sint32 wx, wy, wz, h, c;
    sint16 room_number;

    wx = ideal->x;
    wy = ideal->y;
    wz = ideal->z;

    if (yfirst) {
        room_number = ideal->room_number;
        floor = GetFloor(wx, wy, wz, &room_number);
        h = GetHeight(floor, wx, wy, wz);
        c = GetCeiling(floor, wx, wy, wz);
        if (wy - 255 < c && wy + 255 > h && c < h && c != NO_HEIGHT && h != NO_HEIGHT)
            wy = (h + c) >> 1;
        else if (wy + 255 > h && c < h && c != NO_HEIGHT && h != NO_HEIGHT)
            wy = h - 255;
        else if (wy - 255 < c && c < h && c != NO_HEIGHT && h != NO_HEIGHT)
            wy = c + 255;
    }

    room_number = ideal->room_number;
    floor = GetFloor(wx - push, wy, wz, &room_number);
    h = GetHeight(floor, wx - push, wy, wz);
    c = GetCeiling(floor, wx - push, wy, wz);
    if (wy > h || h == NO_HEIGHT || c == NO_HEIGHT || c >= h || wy < c)
        wx = (wx & ~1023) + push;

    room_number = ideal->room_number;
    floor = GetFloor(wx, wy, wz - push, &room_number);
    h = GetHeight(floor, wx, wy, wz - push);
    c = GetCeiling(floor, wx, wy, wz - push);
    if (wy > h || h == NO_HEIGHT || c == NO_HEIGHT || c >= h || wy < c)
        wz = (wz & ~1023) + push;

    room_number = ideal->room_number;
    floor = GetFloor(wx + push, wy, wz, &room_number);
    h = GetHeight(floor, wx + push, wy, wz);
    c = GetCeiling(floor, wx + push, wy, wz);
    if (wy > h || h == NO_HEIGHT || c == NO_HEIGHT || c >= h || wy < c)
        wx = (wx | 1023) - push;

    room_number = ideal->room_number;
    floor = GetFloor(wx, wy, wz + push, &room_number);
    h = GetHeight(floor, wx, wy, wz + push);
    c = GetCeiling(floor, wx, wy, wz + push);
    if (wy > h || h == NO_HEIGHT || c == NO_HEIGHT || c >= h || wy < c)
        wz = (wz | 1023) - push;

    if (!yfirst) {
        room_number = ideal->room_number;
        floor = GetFloor(wx, wy, wz, &room_number);
        h = GetHeight(floor, wx, wy, wz);
        c = GetCeiling(floor, wx, wy, wz);
        if (wy - 255 < c && wy + 255 > h && c < h && c != NO_HEIGHT && h != NO_HEIGHT)
            wy = (h + c) >> 1;
        else if (wy + 255 > h && c < h && c != NO_HEIGHT && h != NO_HEIGHT)
            wy = h - 255;
        else if (wy - 255 < c && c < h && c != NO_HEIGHT && h != NO_HEIGHT)
            wy = c + 255;
    }

    room_number = ideal->room_number;
    floor = GetFloor(wx, wy, wz, &room_number);
    h = GetHeight(floor, wx, wy, wz);
    c = GetCeiling(floor, wx, wy, wz);
    if (wy > h || wy < c || h == NO_HEIGHT || c == NO_HEIGHT || c >= h)
        return 1;

    floor = GetFloor(wx, wy, wz, &ideal->room_number);
    ideal->x = wx;
    ideal->y = wy;
    ideal->z = wz;
    return 0;
}

sint32 mgLOS(GAME_VECTOR *start, GAME_VECTOR *target, sint32 push)
{
    FLOOR_INFO *floor;
    sint32 x, y, z, h, c, cdiff, hdiff;
    sint32 dx, dy, dz, lp, clipped, nc;
    sint16 room_number, room_number2;

    dx = (target->x - start->x) >> 3;
    dy = (target->y - start->y) >> 3;
    dz = (target->z - start->z) >> 3;
    x = start->x;
    y = start->y;
    z = start->z;
    room_number = room_number2 = start->room_number;
    clipped = nc = 0;
    for (lp = 0; lp < 8; lp++) {
        room_number = room_number2;
        floor = GetFloor(x, y, z, &room_number2);
        if (room[room_number2].flags & 0x80) {
            clipped = 1;
            break;
        }
        h = GetHeight(floor, x, y, z);
        c = GetCeiling(floor, x, y, z);
        if (h == NO_HEIGHT || c == NO_HEIGHT || c >= h) {
            if (!nc) {
                x += dx;
                y += dy;
                z += dz;
                continue;
            }
            clipped = 1;
            break;
        }
        if (y > h) {
            hdiff = y - h;
            if (hdiff < push) {
                y = h;
            } else {
                clipped = 1;
                break;
            }
        }
        if (y < c) {
            cdiff = c - y;
            if (cdiff < push) {
                y = c;
            } else {
                clipped = 1;
                break;
            }
        }
        nc = 1;
        x += dx;
        y += dy;
        z += dz;
    }

    if (lp) {
        x -= dx;
        y -= dy;
        z -= dz;
    }

    target->x = x;
    target->y = y;
    target->z = z;
    GetFloor(target->x, target->y, target->z, &room_number);
    target->room_number = room_number;
    return !clipped;
}
