#pragma once
#include <functional>
#include "IVec.h"

struct Button
{
    Button(
        IVec2 relativePos,
        const char* tooltip,
        const char* description);

    static int g_width;

    // Divided by g_width
    IVec2 relativePos;
    const char* tooltip;
    const char* description;

    virtual IRect Bounds() const;
};
int Button::g_width = 16;

struct IconButton : Button
{
    IconButton(
        IVec2 relativePos,
        const char* tooltip,
        const char* description,
        IVec2 textureSheetPos,
        const Texture2D* textureSheet);

    IVec2 textureSheetPos;
    const Texture2D* textureSheet;
};

struct TextButton : Button
{
    TextButton(
        IVec2 relativePos,
        const char* tooltip,
        const char* description,
        const char* buttonText,
        int width = 1);

    // Divided by g_width
    int width;
    const char* buttonText;

    IRect Bounds() const final;
};
