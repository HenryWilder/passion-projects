#include <algorithm>
#include "IVec_Test.h"

bool IVec2::operator==(IVec2 b) const
{
    return x == b.x && y == b.y;
}
bool IVec2::operator!=(IVec2 b) const
{
    return x != b.x || y != b.y;
}

namespace std
{
    size_t hash<IVec2>::operator()(const IVec2& k) const
    {
        size_t first = hash<int>{}(k.x);
        size_t second = hash<int>{}(k.y);
        return first ^ (second << 1);
    }
}

int IntGridDistance(IVec2 a, IVec2 b)
{
    if ((a.x == b.x) && (a.y == b.y))
        return 0;
    if (a.x == b.x)
        return abs(a.y - b.y);
    else if (a.y == b.y)
        return abs(a.x - b.x);
    else
        return std::min(abs(b.y - a.y), abs(b.x - a.x));
}
long DistanceSqr(IVec2 a, IVec2 b)
{
    long x = b.x - a.x;
    long y = b.y - a.y;
    return x * x + y * y;
}
IVec2 Normal(IVec2 vec)
{
    int xSqr = vec.x * vec.x;
    int ySqr = vec.y * vec.y;
    int lenSqr = xSqr + ySqr;
    return {
        (int)sqrtf(xSqr / lenSqr),
        (int)sqrtf(ySqr / lenSqr)
    };
}

IVec2 IVec2Scale_f(IVec2 a, float b)
{
    return IVec2((int)((float)a.x * b), (int)((float)a.y * b));
}

bool CheckCollisionIVecPointLine(IVec2 pt, IVec2 p1, IVec2 p2)
{
    auto [minX, maxX] = std::minmax(p1.x, p2.x);
    auto [minY, maxY] = std::minmax(p1.y, p2.y);
    if (pt.x < minX || pt.x > maxX || pt.y < minY || pt.y > maxY)
        return false;

    // Cardinal
    if (p1.x == p2.x || p1.y == p2.y)
        return true;

    // Diagonal
    auto [a, b] = (p1.x < p2.x) ?
        std::pair<const IVec2&, const IVec2&>{ p1, p2 } :
        std::pair<const IVec2&, const IVec2&>{ p2, p1 };

    /*****************
    *   It is either
    *
    *   a
    *    \
    *     \
    *      \
    *       b
    *
    *   Or
    *
    *       b
    *      /
    *     /
    *    /
    *   a
    *
    *****************/

    if (b.y > a.y)
    {
        /*************
        *
        *   a
        *    \
        *     \
        *      \
        *       b
        *
        *************/
        return abs((pt.x - a.x) - (pt.y - a.y)) <= 1;
    }
    else
    {
        /*************
        *
        *       b
        *      /
        *     /
        *    /
        *   a
        *
        *************/
        return abs((pt.x - a.x) - -(pt.y - a.y)) <= 1;
    }
}

IRect IRectFromTwoPoints(IVec2 a, IVec2 b)
{
    auto [minx, maxx] = std::minmax(a.x, b.x);
    auto [miny, maxy] = std::minmax(a.y, b.y);
    return IRect(minx, miny, maxx - minx, maxy - miny);
}

bool InBoundingBox(IRect bounds, IVec2 pt)
{
    return
        pt.x >= bounds.x &&
        pt.y >= bounds.y &&
        pt.x < bounds.x + bounds.w &&
        pt.y < bounds.y + bounds.h;
}

IRect& IRect::Expand(int outline)
{
    x -= outline;
    y -= outline;
    w += outline * 2;
    h += outline * 2;
    return *this;
}

IRect& IRect::Shrink(int outline)
{
    return Expand(-outline);
}
