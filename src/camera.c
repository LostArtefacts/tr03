// cflags: -/O2 /Od
#include "game.h"

extern GAME_VECTOR ForcedFixedCamera;
extern char UseForcedFixedCamera;

void MoveCamera(GAME_VECTOR *ideal, int speed);

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
