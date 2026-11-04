#include "game.h"

void Inv_RemoveAllItems(void)
{
    inv_main_objects = 0;
    inv_main_current = 0;
    inv_keys_objects = 0;
    inv_keys_current = 0;
}

void RemoveInventoryText(void)
{
    int n;

    for (n = 0; n < IT_NUMBEROF; n++)
    {
        T_RemovePrint(Inv_itemText[n]);
        Inv_itemText[n] = NULL;
    }
}

void Inv_RingCalcAdders(RING_INFO *ring, sint16 rotation_duration)
{
    ring->angle_adder = 0x10000 / ring->number_of_objects;
    ring->rot_adderL = ring->angle_adder / rotation_duration;
    ring->rot_adderR = -ring->rot_adderL;
}

void Inv_RingRotateLeft(RING_INFO *ring)
{
    ring->rotating = 1;
    ring->target_object = ring->current_object - 1;
    if (ring->target_object < 0)
        ring->target_object = ring->number_of_objects - 1;
    ring->rot_count = ROTATE_DURATION;
    ring->rot_adder = ring->rot_adderL;
}

void Inv_RingRotateRight(RING_INFO *ring)
{
    ring->rotating = 1;
    ring->target_object = ring->current_object + 1;
    if (ring->target_object >= ring->number_of_objects)
        ring->target_object = 0;
    ring->rot_count = ROTATE_DURATION;
    ring->rot_adder = ring->rot_adderR;
}

void Inv_RingMotionRadius(RING_INFO *ring, sint16 target)
{
    IMOTION_INFO *imo;

    imo = ring->imo;
    imo->radius_target = target;
    imo->radius_rate = (target - ring->radius) / imo->count;
}

void Inv_RingMotionRotation(RING_INFO *ring, sint16 rotation, sint16 target)
{
    IMOTION_INFO *imo;

    imo = ring->imo;
    imo->rotate_rate = rotation / imo->count;
    imo->rotate_target = target;
}

void Inv_RingMotionCameraPos(RING_INFO *ring, sint16 target)
{
    IMOTION_INFO *imo;

    imo = ring->imo;
    imo->camera_ytarget = target;
    imo->camera_yrate = (target - ring->camerapos.y_pos) / imo->count;
}

void Inv_RingMotionCameraPitch(RING_INFO *ring, sint16 target)
{
    IMOTION_INFO *imo;

    imo = ring->imo;
    imo->camera_pitch_target = target;
    imo->camera_pitch_rate = target / imo->count;
}

void ReqItemCentreAlign(REQUEST_INFO *req, TEXTSTRING *txt)
{
    if (txt)
    {
        txt->xpos = req->xpos;
        txt->bgndOffX = 0;
    }
}

void RemoveAllReqItems(REQUEST_INFO *req)
{
    req->items = 0;
    req->line_offset = 0;
    req->selected = 0;
}
