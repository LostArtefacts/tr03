#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "game.h"
#include <string.h>

#define T_ACTIVE (1 << 0)
#define T_FLASH (1 << 1)
#define T_ROTATE_H (1 << 2)
#define T_ROTATE_V (1 << 3)
#define T_CENTRE_H (1 << 4)
#define T_CENTRE_V (1 << 5)
#define T_RIGHTALIGN (1 << 7)
#define T_BOTTOMALIGN (1 << 8)
#define T_ADDBACKGROUND (1 << 9)
#define T_ADDOUTLINE (1 << 10)
#define T_RIGHTJUSTIFY (1 << 11)

extern TEXTSTRING T_textStrings[MAX_TEXT_STRINGS];
extern sint16 T_numStrings;
extern char T_theStrings[MAX_TEXT_STRINGS * MAX_STRING_SIZE];
extern int phd_znear;
extern void (*InsertLine)(int x1, int y1, int x2, int y2, int z, int c0, int c1);

void DisplayModeInfo(char *string);
void T_DrawThisText(TEXTSTRING *textString);

void T_InitPrint(void)
{
    int n;
    TEXTSTRING *textString = T_textStrings;

    DisplayModeInfo(0);

    for (n = 0; n < MAX_TEXT_STRINGS; n++)
    {
        textString->flags = 0;
        textString++;
    }
    T_numStrings = 0;
}

TEXTSTRING *T_Print(sint32 xpos, sint32 ypos, sint32 zpos, char *string)
{
    int n, length;
    TEXTSTRING *textString = T_textStrings;

    if (string == NULL)
        return 0;
    if (T_numStrings >= MAX_TEXT_STRINGS)
        return 0;
    for (n = 0; n < MAX_TEXT_STRINGS; n++)
    {
        if (!(textString->flags & T_ACTIVE))
            break;
        textString++;
    }
    if (n >= MAX_TEXT_STRINGS)
        return 0;
    length = T_GetStringLen(string);
    if (length >= MAX_STRING_SIZE)
        length = MAX_STRING_SIZE - 1;
    textString->string = &T_theStrings[n * MAX_STRING_SIZE];
    textString->xpos = (sint16)xpos;
    textString->ypos = (sint16)ypos;
    textString->zpos = 0;
    textString->letterSpacing = 0;
    textString->wordSpacing = 6;
    textString->scaleV = 0x10000;
    textString->scaleH = 0x10000;
    textString->Colour = (sint16)zpos;
    memcpy(textString->string, string, length + 1);
    textString->flags = T_ACTIVE;
    textString->textflags = 0;
    textString->outlflags = 0;
    textString->bgndflags = 0;
    textString->bgndSizeX = 0;
    textString->bgndSizeY = 0;
    textString->bgndOffX = 0;
    textString->bgndOffY = 0;
    textString->bgndOffZ = 0;
    T_numStrings++;
    return textString;
}

void T_ChangeText(TEXTSTRING *textString, char *string)
{
    if (string == NULL || textString == NULL)
        return;

    if (textString->flags & T_ACTIVE)
    {
        if (T_GetStringLen(string) > MAX_STRING_SIZE)
            string[MAX_STRING_SIZE - 1] = 0;
        memcpy(textString->string, string, MAX_STRING_SIZE);
    }
}

void T_FlashText(TEXTSTRING *textString, sint16 ok, sint16 rate)
{
    if (textString == NULL)
        return;

    if (ok)
    {
        textString->flags |= T_FLASH;
        textString->flashRate = rate;
        textString->flashCount = rate;
    }
    else
        textString->flags &= ~T_FLASH;
}

uint32 GetTextScaleV(uint32 scaleV);

void T_AddBackground(TEXTSTRING *textString, sint16 xsize, sint16 ysize, sint16 xoff, sint16 yoff, sint16 zoff, sint16 colour, GOURAUD_FILL *gourptr, uint16 flags)
{
    uint32 scaleV, scaleH;

    scaleH = GetTextScaleH(textString->scaleH);
    scaleV = GetTextScaleV(textString->scaleV);
    if (textString) {
        textString->flags |= T_ADDBACKGROUND;
        textString->bgndSizeX = (sint16)((scaleH * xsize) >> 16);
        textString->bgndSizeY = (sint16)((scaleV * ysize) >> 16);
        textString->bgndOffX = (sint16)((scaleH * xoff) >> 16);
        textString->bgndOffY = (sint16)((scaleV * yoff) >> 16);
        textString->bgndOffZ = zoff;
        textString->bgndColour = colour;
        textString->bgndGour = gourptr;
        textString->bgndflags = flags;
    }
}

void T_RemoveBackground(TEXTSTRING *textString)
{
    if (textString)
        textString->flags &= ~T_ADDBACKGROUND;
}

void T_AddOutline(TEXTSTRING *textString, sint16 ok, sint16 colour, GOURAUD_OUTLINE *gourptr, uint16 flags)
{
    if (textString)
    {
        textString->flags |= T_ADDOUTLINE;
        textString->outlColour = colour;
        textString->outlGour = gourptr;
        textString->outlflags = flags;
    }
}

void T_RemoveOutline(TEXTSTRING *textString)
{
    if (textString)
        textString->flags &= ~T_ADDOUTLINE;
}

void T_CentreH(TEXTSTRING *textString, sint16 ok)
{
    if (textString == NULL)
        return;
    if (ok)
        textString->flags |= T_CENTRE_H;
    else
        textString->flags &= ~T_CENTRE_H;
}

void T_CentreV(TEXTSTRING *textString, sint16 ok)
{
    if (textString == NULL)
        return;
    if (ok)
        textString->flags |= T_CENTRE_V;
    else
        textString->flags &= ~T_CENTRE_V;
}

void T_RightAlign(TEXTSTRING *textString, sint16 ok)
{
    if (textString == NULL)
        return;
    if (ok)
        textString->flags |= T_RIGHTALIGN;
    else
        textString->flags &= ~T_RIGHTALIGN;
}

void T_BottomAlign(TEXTSTRING *textString, sint16 ok)
{
    if (textString == NULL)
        return;
    if (ok)
        textString->flags |= T_BOTTOMALIGN;
    else
        textString->flags &= ~T_BOTTOMALIGN;
}

extern char T_textSpacing[];
extern char T_remapASCII[];

uint32 GetTextScaleV(uint32 scaleV);

sint32 T_GetTextWidth(TEXTSTRING *textString)
{
    uint8 *string;
    uint32 letter;
    sint32 width;
    uint32 scaleV, scaleH;

    letter = 0;
    width = 0;
    scaleH = GetTextScaleH(textString->scaleH);
    scaleV = GetTextScaleV(textString->scaleV);
    width = 0;
    string = (uint8 *)textString->string;
    while (*string) {
        letter = *(string++);
        if (letter == 17 || letter == 18) {
            width += 14;
            continue;
        }
        if (letter > 129)
            continue;
        if (letter > 10 && letter < 32)
            continue;
        if (letter == '(' || letter == ')' || letter == '$' || letter == '~')
            continue;
        if (letter == 32) {
            if (scaleH == 0x10000)
                width += textString->wordSpacing;
            else
                width += (textString->wordSpacing * scaleH) >> 16;
            continue;
        }
        if (letter >= 127 && letter <= 129) {
            if (scaleH == 0x10000)
                width += 16;
            else
                width += (16 * scaleH) >> 16;
            continue;
        }
        if (letter < 11)
            letter += 81;
        else if (letter < 16)
            letter += 91;
        else
            letter = T_remapASCII[letter - 32];
        if (letter >= '0' && letter <= '9')
            width += (12 * scaleH) >> 16;
        else {
            if (scaleH == 0x10000)
                width += T_textSpacing[letter] + textString->letterSpacing;
            else
                width += (((sint32)T_textSpacing[letter] + textString->letterSpacing) * scaleH) >> 16;
        }
    }
    width -= textString->letterSpacing;
    width &= 0xfffe;
    return width;
}

int T_RemovePrint(TEXTSTRING *textString)
{
    if (!textString)
        return 0;

    if (!(textString->flags & T_ACTIVE))
        return 0;

    textString->flags &= ~T_ACTIVE;
    T_numStrings--;
    return 1;
}

sint16 T_GetStringLen(char *string)
{
    sint16 len = 1;

    while (*string++)
    {
        len++;
        if (len > MAX_STRING_SIZE)
            return MAX_STRING_SIZE;
    }
    return len;
}

void T_DrawText(void)
{
    int n;
    TEXTSTRING *textString = T_textStrings;

    for (n = 0; n < MAX_TEXT_STRINGS; n++)
    {
        if (textString)
        {
            if (textString->flags & T_ACTIVE)
                T_DrawThisText(textString);
        }
        textString++;
    }
}

void S_DrawScreenSprite2d(sint32 x, sint32 y, sint32 z, sint32 scaleH, sint32 scaleV, sint16 sprnum, sint16 shade, uint16 flags);
void S_DrawScreenFBox(sint32 sx, sint32 sy, sint32 z, sint32 w, sint32 h, sint32 col, uint16 *grdptr, sint32 flags);

void draw_border(sint32 x, sint32 y, sint32 z, sint32 w, sint32 h);

void T_DrawThisText(TEXTSTRING *textString)
{
    uint8 *string;
    uint32 letter = 0;
    int sprite;
    int hw, hh, hw2, hh2, textheight;
    sint32 xpos, ypos, zpos;
    sint32 bxpos, bypos, bwidth = 0, bheight = 0;
    sint32 pixWidth;
    uint32 scaleV, scaleH;

    scaleH = GetTextScaleH(textString->scaleH);
    scaleV = GetTextScaleV(textString->scaleV);
    textheight = (11 * scaleV) >> 16;
    if (textString->flags & T_FLASH) {
        textString->flashCount -= (sint16)camera.number_frames;
        if (textString->flashCount <= -textString->flashRate)
            textString->flashCount = textString->flashRate;
        else if (textString->flashCount < 0)
            return;
    }
    string = (uint8 *)textString->string;
    xpos = textString->xpos;
    ypos = textString->ypos;
    zpos = textString->zpos;
    pixWidth = T_GetTextWidth(textString);
    if (textString->flags & T_CENTRE_H)
        xpos += (GetRenderWidth() - pixWidth) / 2;
    else if (textString->flags & T_RIGHTALIGN)
        xpos += GetRenderWidth() - pixWidth;
    else if (textString->flags & T_RIGHTJUSTIFY)
        xpos -= pixWidth;
    if (textString->flags & T_CENTRE_V)
        ypos += GetRenderHeight() / 2;
    else if (textString->flags & T_BOTTOMALIGN)
        ypos += GetRenderHeight();
    bxpos = xpos - ((2 * scaleH) >> 16) + textString->bgndOffX;
    bypos = ypos - ((4 * scaleV) >> 16) + textString->bgndOffY - textheight;
    while (*string) {
        letter = *(string++);
        if (letter > 18 && letter < 32)
            continue;
        if (letter == 32) {
            xpos += (textString->wordSpacing * scaleH) >> 16;
            continue;
        }
        if (letter >= 127 && letter <= 129) {
            xpos += (16 * scaleH) >> 16;
            continue;
        }
        if (letter >= 127)
            continue;
        if (letter < 11)
            sprite = letter + 81;
        else if (letter <= 18)
            sprite = letter + 91;
        else
            sprite = T_remapASCII[letter - 32];
        if (letter >= '0' && letter <= '9')
            xpos += (((12 - T_textSpacing[sprite]) / 2) * scaleH) >> 16;
        if (xpos > 0 && xpos < GetRenderWidth() && ypos > 0 && ypos < GetRenderHeight())
            S_DrawScreenSprite2d(xpos, ypos, zpos, scaleH, scaleV, (sint16)(objects[356].mesh_index + sprite), textString->Colour, textString->textflags);
        if (letter == '(' || letter == ')' || letter == '$' || letter == '~')
            continue;
        if (letter >= '0' && letter <= '9') {
            xpos += ((12 - (12 - T_textSpacing[sprite]) / 2) * scaleH) >> 16;
        } else {
            if (scaleH == 0x10000) {
                if (sprite == 108 || sprite == 109)
                    xpos += 14;
                else
                    xpos += T_textSpacing[sprite] + textString->letterSpacing;
            } else
                xpos += (((sint32)T_textSpacing[sprite] + textString->letterSpacing) * scaleH) >> 16;
        }
    }
    if ((textString->flags & T_ADDBACKGROUND) || (textString->flags & T_ADDOUTLINE)) {
        if (textString->bgndSizeX) {
            bxpos += pixWidth / 2;
            pixWidth = textString->bgndSizeX;
            bxpos -= pixWidth / 2;
        }
        bwidth = pixWidth + 4;
        if (textString->bgndSizeY)
            bheight = textString->bgndSizeY - 2;
        else
            bheight = (16 * scaleV) >> 16;
    }
    if (textString->flags & T_ADDBACKGROUND) {
        if (textString->bgndGour) {
            hw = bwidth / 2;
            hh = bheight / 2;
            hw2 = bwidth - hw;
            hh2 = bheight - hh;
            S_DrawScreenFBox(bxpos, bypos, zpos + textString->bgndOffZ + 2, hw + hw2, hh + hh2, textString->bgndColour, (uint16 *)textString->bgndGour, textString->bgndflags);
        } else {
            S_DrawScreenFBox(bxpos, bypos, zpos + textString->bgndOffZ + 2, bwidth, bheight, textString->bgndColour, (uint16 *)textString->bgndGour, textString->bgndflags);
        }
    }
    if (textString->flags & T_ADDOUTLINE)
        draw_border(bxpos, bypos, 0, bwidth, bheight);
}

void draw_border(sint32 x, sint32 y, sint32 z, sint32 w, sint32 h)
{
    x -= 2;
    y -= 2;
    w += 4;
    h += 4;
    InsertLine(x - 1, y - 1, x + w + 2, y - 1, phd_znear + 0xc000, 0x1040, 0x1040);
    InsertLine(x, y, x + w, y, phd_znear + 0xc000, 0x4080c0, 0x4080c0);
    InsertLine(x + 1, y + 1, x + w, y + 1, phd_znear + 0xc000, 0x1040, 0x1040);
    InsertLine(x - 1, y, x - 1, y + h + 2, phd_znear + 0xc000, 0x1040, 0x1040);
    InsertLine(x, y, x, y + h, phd_znear + 0xc000, 0x4080c0, 0x4080c0);
    InsertLine(x + 1, y, x + 1, y + h - 1, phd_znear + 0xc000, 0x1040, 0x1040);
    InsertLine(x + w - 1, y, x + w - 1, y + h, phd_znear + 0xc000, 0x1040, 0x1040);
    InsertLine(x + w, y, x + w, y + h + 1, phd_znear + 0xc000, 0x4080c0, 0x4080c0);
    InsertLine(x + w + 1, y, x + w + 1, y + h + 2, phd_znear + 0xc000, 0x1040, 0x1040);
    InsertLine(x - 1, y + h - 1, x + w + 1, y + h - 1, phd_znear + 0xc000, 0x1040, 0x1040);
    InsertLine(x, y + h, x + w + 1, y + h, phd_znear + 0xc000, 0x4080c0, 0x4080c0);
    InsertLine(x, y + h + 1, x + w + 2, y + h + 1, phd_znear + 0xc000, 0x1040, 0x1040);
}

uint32 GetTextScaleH(uint32 scaleH)
{
    uint32 scale;
    int width;

    width = GetRenderWidth();
    if (width < 640)
        width = 640;
    scale = (width << 16) / width;
    return (scaleH >> 8) * (scale >> 8);
}

uint32 GetTextScaleV(uint32 scaleV)
{
    uint32 scale;
    int height;

    height = GetRenderHeight();
    if (height < 480)
        height = 480;
    scale = (height << 16) / height;
    return (scaleV >> 8) * (scale >> 8);
}
