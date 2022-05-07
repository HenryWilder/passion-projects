#include "Buttons.h"

Button::Button(IVec2 relativePos, const char* tooltip, const char* description) :
    relativePos(relativePos), tooltip(tooltip), description(description) {}

IRect Button::Bounds() const { return IRect(relativePos * g_width, g_width); }

IconButton::IconButton(IVec2 relativePos, const char* tooltip, const char* description, IVec2 textureSheetPos, const Texture2D* textureSheet) :
    Button(relativePos, tooltip, description), textureSheetPos(textureSheetPos), textureSheet(textureSheet) {}

TextButton::TextButton(IVec2 relativePos, const char* tooltip, const char* description, const char* buttonText, int width) :
    Button(relativePos, tooltip, description), width(width), buttonText(buttonText) {}

IRect TextButton::Bounds() const { return IRect(relativePos * g_width, width * g_width, g_width); }
