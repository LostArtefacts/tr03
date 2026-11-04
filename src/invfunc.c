#include "game.h"
#include <stdio.h>

#define CAMERA_YOFFSET -96
#define NUM_ASS_TIMES 10

void phd_GetVectorAngles(sint32 x, sint32 y, sint32 z, PHD_ANGLE *dest);

extern TEXTSTRING *Inv_ringText;
extern TEXTSTRING *Inv_tagText;
extern TEXTSTRING *Inv_upArrow1;
extern TEXTSTRING *Inv_upArrow2;
extern TEXTSTRING *Inv_downArrow1;
extern TEXTSTRING *Inv_downArrow2;

void RingIsNotOpen(RING_INFO *ring)
{
    T_RemovePrint(Inv_tagText);
    Inv_tagText = NULL;
    T_RemovePrint(Inv_ringText);
    Inv_ringText = NULL;
    T_RemovePrint(Inv_upArrow1);
    Inv_upArrow1 = NULL;
    T_RemovePrint(Inv_upArrow2);
    Inv_upArrow2 = NULL;
    T_RemovePrint(Inv_downArrow1);
    Inv_downArrow1 = NULL;
    T_RemovePrint(Inv_downArrow2);
    Inv_downArrow2 = NULL;
}

void RingActive(void)
{
    T_RemovePrint(Inv_itemText[IT_NAME]);
    Inv_itemText[IT_NAME] = NULL;
    T_RemovePrint(Inv_itemText[IT_QTY]);
    Inv_itemText[IT_QTY] = NULL;
}

extern INVENTORY_ITEM *inv_main_list[];
extern sint16 inv_main_qtys[];
extern INVENTORY_ITEM *inv_keys_list[];
extern sint16 inv_keys_qtys[];

int Inv_GetItemOption(int item_num);

int Inv_RequestItem(int item_num)
{
    int n;
    int option;

    option = Inv_GetItemOption(item_num);
    for (n = 0; n < inv_main_objects; n++) {
        if (inv_main_list[n]->object_number == option)
            return inv_main_qtys[n];
    }
    for (n = 0; n < inv_keys_objects; n++) {
        if (inv_keys_list[n]->object_number == option)
            return inv_keys_qtys[n];
    }
    return 0;
}

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

void Inv_RingGetView(RING_INFO *ring, PHD_3DPOS *viewer)
{
    PHD_ANGLE angles[2];

    phd_GetVectorAngles(0 - ring->camerapos.x_pos, CAMERA_YOFFSET - ring->camerapos.y_pos, ring->radius - ring->camerapos.z_pos, angles);
    viewer->x_pos = ring->camerapos.x_pos;
    viewer->y_pos = ring->camerapos.y_pos;
    viewer->z_pos = ring->camerapos.z_pos;
    viewer->x_rot = angles[1] + ring->camera_pitch;
    viewer->y_rot = angles[0];
    viewer->z_rot = 0;
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

void Inv_RingMotionInit(RING_INFO *ring, sint16 frames, sint16 status, sint16 status_target)
{
    IMOTION_INFO *imo;

    imo = ring->imo;
    imo->count = frames;
    imo->status = status;
    imo->status_target = status_target;
    imo->radius_target = 0;
    imo->radius_rate = 0;
    imo->camera_ytarget = 0;
    imo->camera_yrate = 0;
    imo->camera_pitch_target = 0;
    imo->camera_pitch_rate = 0;
    imo->rotate_target = 0;
    imo->rotate_rate = 0;
    imo->item_ptxrot_target = 0;
    imo->item_ptxrot_rate = 0;
    imo->item_xrot_target = 0;
    imo->item_xrot_rate = 0;
    imo->item_ytrans_target = 0;
    imo->item_ytrans_rate = 0;
    imo->item_ztrans_target = 0;
    imo->item_ztrans_rate = 0;
    imo->misc = 0;
}

void Inv_RingMotionSetup(RING_INFO *ring, sint16 status, sint16 status_target, sint16 frames)
{
    IMOTION_INFO *imo;

    imo = ring->imo;
    imo->count = frames;
    imo->status = status;
    imo->status_target = status_target;
    imo->radius_rate = 0;
    imo->camera_yrate = 0;
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

void Inv_RingMotionItemSelect(RING_INFO *ring, INVENTORY_ITEM *inv_item)
{
    IMOTION_INFO *imo;

    imo = ring->imo;
    imo->item_ptxrot_target = inv_item->pt_xrot_sel;
    imo->item_ptxrot_rate = inv_item->pt_xrot_sel / imo->count;
    imo->item_xrot_target = inv_item->x_rot_sel;
    imo->item_xrot_rate = (inv_item->x_rot_sel - inv_item->x_rot_nosel) / imo->count;
    imo->item_ytrans_target = inv_item->ytrans_sel;
    imo->item_ytrans_rate = inv_item->ytrans_sel / imo->count;
    imo->item_ztrans_target = inv_item->ztrans_sel;
    imo->item_ztrans_rate = inv_item->ztrans_sel / imo->count;
}

void Inv_RingMotionItemDeselect(RING_INFO *ring, INVENTORY_ITEM *inv_item)
{
    IMOTION_INFO *imo;

    imo = ring->imo;
    imo->item_ptxrot_target = 0;
    imo->item_ptxrot_rate = 0 - inv_item->pt_xrot_sel / imo->count;
    imo->item_xrot_target = inv_item->x_rot_nosel;
    imo->item_xrot_rate = (inv_item->x_rot_nosel - inv_item->x_rot_sel) / imo->count;
    imo->item_ytrans_target = 0;
    imo->item_ytrans_rate = 0 - inv_item->ytrans_sel / imo->count;
    imo->item_ztrans_target = 0;
    imo->item_ztrans_rate = 0 - inv_item->ztrans_sel / imo->count;
}

extern uint32 RequesterFlags1[];
extern uint32 RequesterFlags2[];

void Init_Requester(REQUEST_INFO *req)
{
    int n;

    req->items = 0;
    req->heading1text = NULL;
    req->heading2text = NULL;
    req->heading1_flags = 0;
    req->heading2_flags = 0;
    req->backgroundtext = NULL;
    req->background_flags = 1;
    req->moreuptext = NULL;
    req->moreup_flags = 1;
    req->moredowntext = NULL;
    req->moredown_flags = 1;
    for (n = 0; n < MAX_REQLINES; n++) {
        req->texts1[n] = NULL;
        req->texts1_flags[n] = 0;
        req->texts2[n] = NULL;
        req->texts2_flags[n] = 0;
    }
    req->itemtexts1_flags = RequesterFlags1;
    req->itemtexts2_flags = RequesterFlags2;
    req->original_render_width = GetRenderWidth();
    req->original_render_height = GetRenderHeight();
}

void Remove_Requester(REQUEST_INFO *req)
{
    int n;

    T_RemovePrint(req->heading1text);
    req->heading1text = NULL;
    T_RemovePrint(req->heading2text);
    req->heading2text = NULL;
    T_RemovePrint(req->backgroundtext);
    req->backgroundtext = NULL;
    T_RemovePrint(req->moreuptext);
    req->moreuptext = NULL;
    T_RemovePrint(req->moredowntext);
    req->moredowntext = NULL;
    for (n = 0; n < MAX_REQLINES; n++) {
        T_RemovePrint(req->texts1[n]);
        req->texts1[n] = NULL;
        T_RemovePrint(req->texts2[n]);
        req->texts2[n] = NULL;
    }
}

void ReqItemCentreAlign(REQUEST_INFO *req, TEXTSTRING *txt)
{
    if (txt)
    {
        txt->xpos = req->xpos;
        txt->bgndOffX = 0;
    }
}

void ReqItemLeftalign(REQUEST_INFO *req, TEXTSTRING *txt)
{
    int x;
    uint32 scaleH = GetTextScaleH(txt->scaleH);

    if (txt)
    {
        x = (((req->pixwidth * scaleH) >> 16) / 2 - T_GetTextWidth(txt) / 2) - ((8 * scaleH) >> 16);
        txt->xpos = req->xpos - x;
        txt->bgndOffX = x;
    }
}

void ReqItemRightalign(REQUEST_INFO *req, TEXTSTRING *txt)
{
    int x;
    uint32 scaleH = GetTextScaleH(txt->scaleH);

    if (txt)
    {
        x = (((req->pixwidth * scaleH) >> 16) / 2 - T_GetTextWidth(txt) / 2) - ((8 * scaleH) >> 16);
        txt->xpos = req->xpos + x;
        txt->bgndOffX = -x;
    }
}

void SetRequesterHeading(REQUEST_INFO *req, char *text1, uint32 flags1, char *text2, uint32 flags2)
{
    T_RemovePrint(req->heading1text);
    req->heading1text = NULL;
    T_RemovePrint(req->heading2text);
    req->heading2text = NULL;
    if (text1) {
        sprintf(req->heading1_str, text1);
        req->heading1_flags = flags1 | 1;
    } else {
        sprintf(req->heading1_str, "u");
        req->heading1_flags = 0;
    }
    if (text2) {
        sprintf(req->heading2_str, text2);
        req->heading2_flags = flags2 | 1;
    } else {
        sprintf(req->heading2_str, "u");
        req->heading2_flags = 0;
    }
}

void RemoveAllReqItems(REQUEST_INFO *req)
{
    req->items = 0;
    req->line_offset = 0;
    req->selected = 0;
}

void AddRequesterItem(REQUEST_INFO *req, char *text1, uint32 flags1, char *text2, uint32 flags2)
{
    req->itemtexts1_flags = RequesterFlags1;
    req->itemtexts2_flags = RequesterFlags2;
    if (text1) {
        sprintf(req->itemtexts1 + req->items * req->itemtextlen, text1);
        req->itemtexts1_flags[req->items] = flags1 | 1;
    } else {
        req->itemtexts1_flags[req->items] = 0;
    }
    if (text2) {
        sprintf(req->itemtexts2 + req->items * req->itemtextlen, text2);
        req->itemtexts2_flags[req->items] = flags2 | 1;
    } else {
        req->itemtexts2_flags[req->items] = 0;
    }
    req->items++;
}

void SetPCRequesterSize(REQUEST_INFO *req, int maxlines, int ypos)
{
    int height = GetRenderHeight();
    int lines = (height / 2) / (TEXT_HEIGHT + 7);

    if (lines > maxlines)
        lines = maxlines;
    req->ypos = ypos;
    req->vis_lines = lines;
}

int AddAssaultTime(uint32 time)
{
    int i, j, insert = 0;

    for (i = 0; i < NUM_ASS_TIMES; i++)
    {
        if (savegame.best_assault_times[i] == 0 || time < savegame.best_assault_times[i])
        {
            insert = 1;
            break;
        }
    }
    if (insert)
    {
        for (j = NUM_ASS_TIMES - 1; j > i; j--)
            savegame.best_assault_times[j] = savegame.best_assault_times[j - 1];
        savegame.best_assault_times[i] = time;
        return 1;
    }
    return 0;
}

int AddQuadbikeTime(uint32 time)
{
    int i, j, insert = 0;

    for (i = 0; i < NUM_ASS_TIMES; i++)
    {
        if (savegame.best_quadbike_times[i] == 0 || time < savegame.best_quadbike_times[i])
        {
            insert = 1;
            break;
        }
    }
    if (insert)
    {
        for (j = NUM_ASS_TIMES - 1; j > i; j--)
            savegame.best_quadbike_times[j] = savegame.best_quadbike_times[j - 1];
        savegame.best_quadbike_times[i] = time;
        return 1;
    }
    return 0;
}
