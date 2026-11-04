#ifndef INVDATA_H
#define INVDATA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "text.h"

#define MAX_REQLINES 24

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

extern sint16 inv_main_objects, inv_main_current;
extern sint16 inv_keys_objects, inv_keys_current;
extern sint32 oldInputDB;

sint32 GetDebouncedInput(sint32 in);
void Inv_RemoveAllItems(void);
void ReqItemCentreAlign(REQUEST_INFO *req, TEXTSTRING *txt);
void RemoveAllReqItems(REQUEST_INFO *req);

#ifdef __cplusplus
}
#endif

#endif
