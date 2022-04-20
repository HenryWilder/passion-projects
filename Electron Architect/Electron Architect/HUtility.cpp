#include "HUtility.h"
#include <raylib.h>

bool Between_Inclusive(int x, int a, int b)
{
    auto [min, max] = std::minmax(a, b);
    return min <= x && x <= max;
}
bool Between_Exclusive(int x, int a, int b)
{
    auto [min, max] = std::minmax(a, b);
    return min < x&& x < max;
}

bool IsKeyDown_Shift()
{
    return IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
}
bool IsKeyDown_Ctrl()
{
    return IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
}
bool IsKeyDown_Alt()
{
    return IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT);
}
