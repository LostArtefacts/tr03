// cflags: -/O2 /Od
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
