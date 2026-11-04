#include "game.h"

extern BITE_INFO seal_gas_boomute;

sint16 TriggerSealmuteGasThrower_boomute(ITEM_INFO *item, BITE_INFO *bite, sint16 speed);

void BoomuteControl(sint16 item_number)
{
    ITEM_INFO *item;
    uint32 length;

    item = &items[item_number];
    if (item->status == DEACTIVATED)
        return;
    if (item->status == ACTIVE) {
        if (item->frame_number >= anims[item->anim_number].frame_base + 1 && item->frame_number <= anims[item->anim_number].frame_end - 8) {
            length = item->frame_number - anims[item->anim_number].frame_base + 1;
            if (length > 24) {
                length = anims[item->anim_number].frame_end - item->frame_number - 8;
                if (length <= 0)
                    length = 1;
                else if (length > 24)
                    length = (GetRandomControl() & 15) + 8;
            }
            if (!length)
                length = 1;
            TriggerSealmuteGasThrower_boomute(item, &seal_gas_boomute, length);
        }
        AnimateItem(item);
    }
}
