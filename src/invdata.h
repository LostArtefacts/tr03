#ifndef INVDATA_H
#define INVDATA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "text.h"
#include "3dglodef.h"

#define MAX_REQLINES 24

enum {
    IT_NAME,
    IT_QTY,
    IT_NUMBEROF
};

typedef struct request_info {
    uint16 noselector:1;
    uint16 items;
    uint16 selected;
    uint16 vis_lines;
    uint16 line_offset;
    uint16 line_oldoffset;
    uint16 pixwidth;
    uint16 line_height;
    sint16 xpos;
    sint16 ypos;
    sint16 zpos;
    sint16 itemtextlen;
    char *itemtexts1;
    char *itemtexts2;
    uint32 *itemtexts1_flags;
    uint32 *itemtexts2_flags;
    uint32 heading1_flags;
    uint32 heading2_flags;
    uint32 background_flags;
    uint32 moreup_flags;
    uint32 moredown_flags;
    uint32 texts1_flags[MAX_REQLINES];
    uint32 texts2_flags[MAX_REQLINES];
    TEXTSTRING *heading1text;
    TEXTSTRING *heading2text;
    TEXTSTRING *backgroundtext;
    TEXTSTRING *moreuptext;
    TEXTSTRING *moredowntext;
    TEXTSTRING *texts1[MAX_REQLINES];
    TEXTSTRING *texts2[MAX_REQLINES];
    char heading1_str[32];
    char heading2_str[32];
    int original_render_width;
    int original_render_height;
} REQUEST_INFO;

typedef struct imotion_info {
    sint16 count;
    sint16 status;
    sint16 status_target;
    sint16 radius_target;
    sint16 radius_rate;
    sint16 camera_ytarget;
    sint16 camera_yrate;
    sint16 camera_pitch_target;
    sint16 camera_pitch_rate;
    sint16 rotate_target;
    sint16 rotate_rate;
    sint16 item_ptxrot_target;
    sint16 item_ptxrot_rate;
    sint16 item_xrot_target;
    sint16 item_xrot_rate;
    sint32 item_ytrans_target;
    sint32 item_ytrans_rate;
    sint32 item_ztrans_target;
    sint32 item_ztrans_rate;
    sint32 misc;
} IMOTION_INFO;

typedef struct inventory_sprite {
    sint16 shape;
    sint16 x;
    sint16 y;
    sint16 z;
    sint32 param1;
    sint32 param2;
    struct gouraud_fill *grdptr;
    sint16 sprnum;
} INVENTORY_SPRITE;

typedef struct inventory_item {
    char *itemText;
    sint16 object_number;
    sint16 frames_total;
    sint16 current_frame;
    sint16 goal_frame;
    sint16 open_frame;
    sint16 anim_direction;
    sint16 anim_speed;
    sint16 anim_count;
    sint16 pt_xrot_sel;
    sint16 pt_xrot;
    sint16 x_rot_sel;
    sint16 x_rot_nosel;
    sint16 x_rot;
    sint16 y_rot_sel;
    sint16 y_rot;
    sint32 ytrans_sel;
    sint32 ytrans;
    sint32 ztrans_sel;
    sint32 ztrans;
    uint32 which_meshes;
    uint32 drawn_meshes;
    sint16 inv_pos;
    INVENTORY_SPRITE **sprlist;
    sint32 misc_data[4];
} INVENTORY_ITEM;

typedef struct ring_info {
    INVENTORY_ITEM **list;
    sint16 type;
    sint16 radius;
    sint16 camera_pitch;
    sint16 rotating;
    sint16 rot_count;
    sint16 current_object;
    sint16 target_object;
    sint16 number_of_objects;
    sint16 angle_adder;
    sint16 rot_adder;
    sint16 rot_adderL;
    sint16 rot_adderR;
    PHD_3DPOS ringpos;
    PHD_3DPOS camerapos;
    PHD_VECTOR light;
    IMOTION_INFO *imo;
} RING_INFO;

extern sint16 inv_main_objects, inv_main_current;
extern sint16 inv_keys_objects, inv_keys_current;
extern sint32 oldInputDB;
extern TEXTSTRING *Inv_itemText[IT_NUMBEROF];

sint32 GetDebouncedInput(sint32 in);
void Inv_RemoveAllItems(void);
void ReqItemCentreAlign(REQUEST_INFO *req, TEXTSTRING *txt);
void RemoveAllReqItems(REQUEST_INFO *req);

#ifdef __cplusplus
}
#endif

#endif
