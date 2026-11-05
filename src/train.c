#include "game.h"

extern GAME_VECTOR ForcedFixedCamera;
extern char UseForcedFixedCamera;

void TriggerDynamic(sint32 x, sint32 y, sint32 z, sint32 falloff, sint32 r, sint32 g, sint32 b);
sint32 TestHeight_train(ITEM_INFO *item, sint32 x, sint32 z, sint16 *room_number);

void TrainControl(sint16 train_number)
{
    ITEM_INFO *t;
    sint32 s, c;
    sint32 fh, rh;
    FLOOR_INFO *floor;
    sint16 room_number;

    t = &items[train_number];

    if (!TriggerActive(t)) {
        return;
    }

    if (t->item_flags[0] == 0) {
        t->item_flags[0] = t->item_flags[1] = 260;
    }

    c = phd_cos(t->pos.y_rot);
    s = phd_sin(t->pos.y_rot);

    t->pos.x_pos += (t->item_flags[1] * s) >> W2V_SHIFT;
    t->pos.z_pos += (t->item_flags[1] * c) >> W2V_SHIFT;

    rh = TestHeight_train(t, 0, 5120, &room_number);
    t->pos.y_pos = fh = TestHeight_train(t, 0, 0, &room_number);

    if (fh == NO_HEIGHT) {
        KillItem(train_number);
        return;
    }

    t->pos.y_pos -= 32;

    room_number = t->room_number;
    GetFloor(t->pos.x_pos, t->pos.y_pos, t->pos.z_pos, &room_number);

    if (room_number != t->room_number) {
        ItemNewRoom(train_number, room_number);
    }

    t->pos.x_rot = -(rh - fh) << 1;

    TriggerDynamic(t->pos.x_pos + ((3072 * s) >> W2V_SHIFT), t->pos.y_pos, t->pos.z_pos + ((3072 * c) >> W2V_SHIFT), 16, 31, 31, 31);

    if (t->item_flags[1] != 260) {
        if ((t->item_flags[1] -= 48) < 0) {
            t->item_flags[1] = 0;
        }

        if (!UseForcedFixedCamera) {
            ForcedFixedCamera.x = t->pos.x_pos + ((8192 * s) >> W2V_SHIFT);
            ForcedFixedCamera.z = t->pos.z_pos + ((8192 * c) >> W2V_SHIFT);

            room_number = t->room_number;
            floor = GetFloor(ForcedFixedCamera.x, t->pos.y_pos - 512, ForcedFixedCamera.z, &room_number);
            ForcedFixedCamera.y = GetHeight(floor, ForcedFixedCamera.x, t->pos.y_pos - 512, ForcedFixedCamera.z);

            ForcedFixedCamera.room_number = room_number;

            UseForcedFixedCamera = 1;
        }
    } else {
        SoundEffect(148, &t->pos, 2);
    }
}
