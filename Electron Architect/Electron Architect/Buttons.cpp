#include "Buttons.h"

int Button::g_width = 16;


Button::Button(IVec2 relativePos, const char* tooltip, const char* description, std::function<void()> clickCallback) :
    relativePos(relativePos), tooltip(tooltip), description(description), OnClick(clickCallback) {}

IRect Button::Bounds() const { return IRect(relativePos * g_width, g_width); }

IconButton::IconButton(IVec2 relativePos, const char* tooltip, const char* description, std::function<void()> clickCallback, IVec2 textureSheetPos, const Texture2D* textureSheet) :
    Button(relativePos, tooltip, description, clickCallback), textureSheetPos(textureSheetPos), textureSheet(textureSheet) {}

TextButton::TextButton(IVec2 relativePos, const char* tooltip, const char* description, std::function<void()> clickCallback, const char* buttonText, int width) :
    Button(relativePos, tooltip, description, clickCallback), width(width), buttonText(buttonText) {}

IRect TextButton::Bounds() const { return IRect(relativePos * g_width, width * g_width, g_width); }
