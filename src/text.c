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
