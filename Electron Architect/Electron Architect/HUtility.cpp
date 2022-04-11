#include "HUtility.h"

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
