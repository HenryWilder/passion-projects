#include "HUtility.h"
#include "IVec.h"

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
        (int)sqrtf((float)xSqr / (float)lenSqr),
        (int)sqrtf((float)ySqr / (float)lenSqr)
    };
}

bool InBoundingBox(IVec2 p, IVec2 a, IVec2 b)
{
    return Between_Inclusive(p.x, a.x, b.x) &&
        Between_Inclusive(p.y, a.y, b.y);
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

void DrawLineIV(IVec2 start, IVec2 end, Color color)
{
    DrawLine(start.x, start.y, end.x, end.y, color);
}
void DrawLineIV(IVec2 start, Width width, Color color)
{
    DrawLine(start.x, start.y, start.x + width.x, start.y, color);
}
void DrawLineIV(IVec2 start, Height height, Color color)
{
    DrawLine(start.x, start.y, start.x, start.y + height.y, color);
}
void DrawCircleIV(IVec2 origin, float radius, Color color)
{
    DrawCircle(origin.x, origin.y, radius, color);
}

void DrawTextureIV(Texture2D texture, IVec2 pos, Color tint)
{
    DrawTexture(texture, pos.x, pos.y, tint);
}

void DrawTextIV(const char* text, IVec2 pos, int fontSize, Color color)
{
    DrawText(text, pos.x, pos.y, fontSize, color);
}

bool InBoundingBox(IRect bounds, IVec2 pt)
{
    return
        pt.x >= bounds.x &&
        pt.y >= bounds.y &&
        pt.x <= bounds.x + bounds.w &&
        pt.y <= bounds.y + bounds.h;
}

void DrawRectangleIRect(IRect rec, Color color)
{
    DrawRectangle(rec.x, rec.y, rec.w, rec.h, color);
}

void BeginScissorMode(IRect area)
{
    BeginScissorMode(area.x, area.y, area.w, area.h);
}

IRectIterator IRect::begin() const
{
    return IRectIterator(*this, xy);
}

IRectIterator IRect::end() const
{
    return IRectIterator(*this, BR());
}

IVec2 IRectIterator::operator*() const
{
    return pt;
}

IRectIterator& IRectIterator::operator++()
{
    ++pt.x;
    if (pt.x == rec.Right())
    {
        pt.x = rec.x;
        ++pt.y;
    }
    return *this;
}

bool IRectIterator::operator!=(const IRectIterator& other)
{
    return pt != other.pt;
}
