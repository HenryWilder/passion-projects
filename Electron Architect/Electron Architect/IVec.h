#pragma once
#ifndef RAYLIB_H
#include <raylib.h>
#endif

struct Width
{
    template<std::integral T>
    constexpr Width(T x) : x(x) {}
    int x;
};

constexpr Width operator+(Width a, int b) { return Width(a.x + b); }
constexpr Width operator-(Width a, int b) { return Width(a.x - b); }
constexpr Width operator*(Width a, int b) { return Width(a.x * b); }
constexpr Width operator/(Width a, int b) { return Width(a.x / b); }

struct Height
{
    template<std::integral T>
    constexpr Height(T y) : y(y) {}
    int y;
};

constexpr Height operator+(Height a, int b) { return Height(a.y + b); }
constexpr Height operator-(Height a, int b) { return Height(a.y - b); }
constexpr Height operator*(Height a, int b) { return Height(a.y * b); }
constexpr Height operator/(Height a, int b) { return Height(a.y / b); }

struct IVec2
{
    IVec2() : x(), y() {}

    constexpr IVec2(int x)
        : x(x), y(x) {}

    constexpr IVec2(Width w)
        : x(w.x), y(0) {}

    constexpr IVec2(Height h)
        : x(0), y(h.y) {}

    constexpr IVec2(Width w, Height h)
        : x(w.x), y(h.y) {}

    constexpr IVec2(int x, int y)
        : x(x), y(y) {}

    explicit constexpr IVec2(Vector2 v)
        : x((int)v.x), y((int)v.y) {}

    union
    {
        struct { int x, y; };
        struct { Width w; Height h; };
    };

    constexpr IVec2& operator+=(IVec2 b) { x += b.x; y += b.y; return *this; }
    constexpr IVec2& operator-=(IVec2 b) { x -= b.x; y -= b.y; return *this; }
    constexpr IVec2& operator*=(IVec2 b) { x *= b.x; y *= b.y; return *this; }
    constexpr IVec2& operator/=(IVec2 b) { x /= b.x; y /= b.y; return *this; }

    constexpr IVec2& operator*=(int b) { x *= b; y *= b; return *this; }
    constexpr IVec2& operator/=(int b) { x /= b; y /= b; return *this; }

    constexpr IVec2& operator+=(Width b) { x += b.x; return *this; }
    constexpr IVec2& operator-=(Width b) { x -= b.x; return *this; }
    constexpr IVec2& operator*=(Width b) { x *= b.x; return *this; }
    constexpr IVec2& operator/=(Width b) { x /= b.x; return *this; }

    constexpr IVec2& operator+=(Height b) { y += b.y; return *this; }
    constexpr IVec2& operator-=(Height b) { y -= b.y; return *this; }
    constexpr IVec2& operator*=(Height b) { y *= b.y; return *this; }
    constexpr IVec2& operator/=(Height b) { y /= b.y; return *this; }

    static consteval IVec2 Zero()  { return IVec2(0);   }
    static consteval IVec2 UnitX() { return IVec2(1,0); }
    static consteval IVec2 UnitY() { return IVec2(0,1); }
    static consteval IVec2 One()   { return IVec2(1);   }

    explicit constexpr operator Vector2() { return Vector2{ (float)x, (float)y }; }
};

namespace std
{
    template<> struct hash<IVec2>
    {
        size_t operator()(const IVec2& k) const;
    };
}

// Distance between points in an integer number of squares
// I.E. a diagonal line is measured as the number of square-grid points it passes through, rather than the length of its hypotenuse
// Assumes that the points could take each other in chess if the two were the positions of queens
int IntGridDistance(IVec2 a, IVec2 b);
long DistanceSqr(IVec2 a, IVec2 b);
IVec2 Normal(IVec2 vec);

constexpr IVec2 Snap(IVec2 pt, IVec2 grid)  { return IVec2((int)(pt.x / grid.x) * grid.x, (int)(pt.y / grid.y) * grid.y); }
constexpr IVec2 Snap(IVec2 pt, int grid)    { return IVec2((int)(pt.x / grid) * grid, (int)(pt.y / grid) * grid); }
constexpr IVec2 Snap(IVec2 pt, Width grid)  { return IVec2((int)(pt.x / grid.x) * grid.x, pt.y); }
constexpr IVec2 Snap(IVec2 pt, Height grid) { return IVec2(pt.x, (int)(pt.y / grid.y) * grid.y); }

bool InBoundingBox(IVec2 p, IVec2 a, IVec2 b);

constexpr bool operator==(IVec2 a, IVec2 b) { return a.x == b.x && a.y == b.y; }
constexpr bool operator!=(IVec2 a, IVec2 b) { return a.x != b.x || a.y != b.y; }

constexpr IVec2 operator+(IVec2 a, IVec2 b) { return IVec2(a.x + b.x, a.y + b.y); }
constexpr IVec2 operator-(IVec2 a, IVec2 b) { return IVec2(a.x - b.x, a.y - b.y); }
constexpr IVec2 operator*(IVec2 a, IVec2 b) { return IVec2(a.x * b.x, a.y * b.y); }
constexpr IVec2 operator/(IVec2 a, IVec2 b) { return IVec2(a.x / b.x, a.y / b.y); }

constexpr IVec2 operator*(IVec2 a, int b) { return IVec2(a.x * b, a.y * b); }
constexpr IVec2 operator/(IVec2 a, int b) { return IVec2(a.x / b, a.y / b); }
constexpr IVec2 operator*(int a, IVec2 b) { return IVec2(a * b.x, a * b.y); }
constexpr IVec2 operator/(int a, IVec2 b) { return IVec2(a / b.x, a / b.y); }

constexpr IVec2 operator+(IVec2 a, Width b) { return IVec2(a.x + b.x, a.y); }
constexpr IVec2 operator-(IVec2 a, Width b) { return IVec2(a.x - b.x, a.y); }
constexpr IVec2 operator*(IVec2 a, Width b) { return IVec2(a.x * b.x, a.y); }
constexpr IVec2 operator/(IVec2 a, Width b) { return IVec2(a.x / b.x, a.y); }

constexpr IVec2 operator+(IVec2 a, Height b) { return IVec2(a.x, a.y + b.y); }
constexpr IVec2 operator-(IVec2 a, Height b) { return IVec2(a.x, a.y - b.y); }
constexpr IVec2 operator*(IVec2 a, Height b) { return IVec2(a.x, a.y * b.y); }
constexpr IVec2 operator/(IVec2 a, Height b) { return IVec2(a.x, a.y / b.y); }

constexpr IVec2 operator+(Width a, Height b) { return IVec2(a.x, b.y); }
constexpr IVec2 operator-(Width a, Height b) { return IVec2(a.x, b.y); }
constexpr IVec2 operator*(Width a, Height b) { return IVec2(a.x, b.y); }
constexpr IVec2 operator/(Width a, Height b) { return IVec2(a.x, b.y); }

constexpr IVec2 operator+(Height a, Width b) { return IVec2(b.x, a.y); }
constexpr IVec2 operator-(Height a, Width b) { return IVec2(b.x, a.y); }
constexpr IVec2 operator*(Height a, Width b) { return IVec2(b.x, a.y); }
constexpr IVec2 operator/(Height a, Width b) { return IVec2(b.x, a.y); }

template<typename T>
concept IVec2Operable = requires(IVec2 v, T t)
{
    { v += t } -> std::convertible_to<IVec2>;
};

IVec2 IVec2Scale_f(IVec2 a, float b);

bool CheckCollisionIVecPointLine(IVec2 pt, IVec2 p1, IVec2 p2);

void DrawLineIV(IVec2 start, IVec2 end, Color color);
void DrawLineIV(IVec2 start, Width width, Color color);
void DrawLineIV(IVec2 start, Height height, Color color);
void DrawCircleIV(IVec2 origin, float radius, Color color);
void DrawTextureIV(Texture2D texture, IVec2 pos, Color tint);
void DrawTextIV(const char* text, IVec2 pos, int fontSize, Color color);

struct IRect;
class IRectIterator
{
    IRectIterator(const IRect& rec, IVec2 pt) : rec(rec), pt(pt) {}
    friend struct IRect;
    const IRect& rec;
    IVec2 pt;

public:
    IVec2 operator*() const;
    IRectIterator& operator++();
    bool operator!=(const IRectIterator& other);
};
struct IRect
{
    IRect() : x(), y(), w(), h() {}

    constexpr IRect(int w)
        : x(0), y(0), w(w), h(w) {}

    constexpr IRect(int w, int h)
        : x(0), y(0), w(w), h(h) {}

    constexpr IRect(Width w, Height h)
        : x(0), y(0), w(w.x), h(h.y) {}

    constexpr IRect(int x, int y, int w)
        : x(x), y(y), w(w), h(w) {}

    constexpr IRect(IVec2 v, int w)
        : x(v.x), y(v.y), w(w), h(w) {}

    constexpr IRect(int x, int y, int w, int h)
        : x(x), y(y), w(w), h(h) {}

    constexpr IRect(IVec2 v, int w, int h)
        : x(v.x), y(v.y), w(w), h(h) {}

    constexpr IRect(IVec2 v, IVec2 extents)
        : x(v.x), y(v.y), w(extents.x), h(extents.y) {}

    constexpr IRect(IVec2 v, Width w, Height h)
        : x(v.x), y(v.y), w(w.x), h(h.y) {}

    constexpr IRect(std::pair<int, int> minmaxX, std::pair<int, int> minmaxY)
        : x(minmaxX.first), y(minmaxY.first), w(minmaxX.second), h(minmaxY.second) {}

    explicit constexpr IRect(Rectangle r)
        : x((int)r.x), y((int)r.y), w((int)r.width), h((int)r.height) {}

    union
    {
        struct { int x, y, w, h; };
        struct { int minx, miny, maxx, maxy; }; // Be careful when using these! It's just a rename, not a function!
        struct { int x, y, width, height; };
        struct { IVec2 xy, wh; };
        struct { IVec2 position, extents; };
    };
    constexpr int Right()  const { return x + w; }
    constexpr int Bottom() const { return y + h; }
    // Bottom-Right point
    constexpr IVec2 BR()   const { return xy + wh; }

    constexpr operator Rectangle() { return Rectangle{ (float)x, (float)y, (float)w, (float)h }; }

    constexpr operator Width()  { return Width(w); }
    constexpr operator Height() { return Height(h); }

    template<typename T>
    IRect& Expand(T outline = 1) requires std::convertible_to<T, IVec2>
    {
        xy -= IVec2(outline);
        wh += IVec2(outline * 2);
        return *this;
    }

    template<typename T>
    IRect& Shrink(int outline = 1) requires std::convertible_to<T, IVec2>
    {
        return Expand(-outline);
    }

    // Abuse as min and max instead of width and height
    // Returns an IRect with INT_MIN and INT_MAX components for comparing
    static consteval IRect Abused()
    {
        return IRect(INT_MAX, INT_MAX, INT_MIN, INT_MIN);
    }
    // Changes width and height from being abused maxx and maxy into normal width/height
    void DeAbuse()
    {
        w = maxx - minx;
        h = maxy - miny;
    }

    static consteval IRect Zero()  { return IRect(0); }
    static consteval IRect Pixel() { return IRect(0,0,1); }

    IRectIterator begin() const;
    IRectIterator end() const;
};

constexpr IRect operator+(IRect a, Width b) { return IRect(a.x, a.y, a.w + b.x, a.h); }
constexpr IRect operator-(IRect a, Width b) { return IRect(a.x, a.y, a.w - b.x, a.h); }
constexpr IRect operator*(IRect a, Width b) { return IRect(a.x, a.y, a.w * b.x, a.h); }
constexpr IRect operator/(IRect a, Width b) { return IRect(a.x, a.y, a.w / b.x, a.h); }

constexpr IRect operator+(IRect a, Height b) { return IRect(a.x, a.y, a.w, a.h + b.y); }
constexpr IRect operator-(IRect a, Height b) { return IRect(a.x, a.y, a.w, a.h - b.y); }
constexpr IRect operator*(IRect a, Height b) { return IRect(a.x, a.y, a.w, a.h * b.y); }
constexpr IRect operator/(IRect a, Height b) { return IRect(a.x, a.y, a.w, a.h / b.y); }

constexpr IRect ExpandIRect(IRect rec, int outline = 1)
{
    return IRect(
        rec.x - outline,
        rec.y - outline,
        rec.w + 2 * outline,
        rec.h + 2 * outline);
}
inline constexpr IRect ShrinkIRect(IRect rec, int outline = 1)
{
    return ExpandIRect(rec, -outline);
}

/*
*  2 | 1
* ---+---
*  3 | 4
*/
constexpr IRect IRectQuadrant(IRect rec, uint8_t q)
{
    _ASSERT_EXPR(q <= 4, L"Quadrant cannot be greater than 4");
    switch (q)
    {
    case 1: return IRect(rec.xy + Width(rec.w / 2),  rec.wh / 2);
    case 2: return IRect(rec.xy,                     rec.wh / 2);
    case 3: return IRect(rec.xy + Height(rec.h / 2), rec.wh / 2);
    case 4: return IRect(rec.xy + rec.wh / 2,        rec.wh / 2);
    }
}

/*
*  2 | 1
* ---+---
*  3 | 4
*/
constexpr struct QuadSet { IRect q1, q2, q3, q4; } SubdivideIRect(IRect rec)
{
    IVec2 halfExt = rec.wh / 2;
    return {
        { rec.xy + halfExt.w, halfExt },
        { rec.xy,             halfExt },
        { rec.xy + halfExt.h, halfExt },
        { rec.xy + halfExt,   halfExt }
    };
}

bool InBoundingBox(IRect bounds, IVec2 pt);

void DrawRectangleIRect(IRect rec, Color color);

void BeginScissorMode(IRect area);

template<int W, int H = W>
void DrawIcon(Texture2D iconSheet, IVec2 iconColRow, IVec2 pos, Color tint)
{
    BeginScissorMode(pos.x, pos.y, W, H);
    DrawTexture(iconSheet,
        pos.x - iconColRow.x * W,
        pos.y - iconColRow.y * H,
        tint);
    EndScissorMode();
}
