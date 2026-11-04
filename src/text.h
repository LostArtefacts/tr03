#ifndef TEXT_H
#define TEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

#define MAX_TEXT_STRINGS 64
#define MAX_STRING_SIZE 64
#define TEXT_HEIGHT 11

typedef struct {
    uint32 clr[4][4];
} GOURAUD_FILL;

typedef struct {
    uint32 clr[9];
} GOURAUD_OUTLINE;

typedef struct textstringinfo {
    uint32 flags;
    uint16 textflags;
    uint16 bgndflags;
    uint16 outlflags;
    sint16 xpos;
    sint16 ypos;
    sint16 zpos;
    sint16 letterSpacing;
    sint16 wordSpacing;
    sint16 flashRate;
    sint16 flashCount;
    sint16 bgndColour;
    GOURAUD_FILL *bgndGour;
    sint16 outlColour;
    GOURAUD_OUTLINE *outlGour;
    sint16 bgndSizeX;
    sint16 bgndSizeY;
    sint16 bgndOffX;
    sint16 bgndOffY;
    sint16 bgndOffZ;
    sint32 scaleH;
    sint32 scaleV;
    sint16 Colour;
    char *string;
} TEXTSTRING;

void T_InitPrint(void);
void T_ChangeText(TEXTSTRING *textString, char *string);
void T_FlashText(TEXTSTRING *textString, sint16 ok, sint16 rate);
void T_RemoveBackground(TEXTSTRING *textString);
void T_AddOutline(TEXTSTRING *textString, sint16 ok, sint16 colour, GOURAUD_OUTLINE *gourptr, uint16 flags);
void T_RemoveOutline(TEXTSTRING *textString);
void T_CentreH(TEXTSTRING *textString, sint16 ok);
void T_CentreV(TEXTSTRING *textString, sint16 ok);
void T_RightAlign(TEXTSTRING *textString, sint16 ok);
void T_BottomAlign(TEXTSTRING *textString, sint16 ok);
int T_RemovePrint(TEXTSTRING *textString);
sint32 T_GetTextWidth(TEXTSTRING *textString);
uint32 GetTextScaleH(uint32 scaleH);
sint16 T_GetStringLen(char *string);
void T_DrawText(void);

#ifdef __cplusplus
}
#endif

#endif
