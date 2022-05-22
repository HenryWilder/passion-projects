#pragma once
#ifndef RAYLIB_H
#include <raylib.h>
#endif
#include <xhash>

struct Width
{
    constexpr Width(int x) : x(x) {}
    int x;
};

constexpr Width operator+(Width a, int b) { return Width(a.x + b); }
constexpr Width operator-(Width a, int b) { return Width(a.x - b); }
constexpr Width operator*(Width a, int b) { return Width(a.x * b); }
constexpr Width operator/(Width a, int b) { return Width(a.x / b); }

struct Height
{
    constexpr Height(int y) : y(y) {}
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
    constexpr IVec2(int x, int y)
        : x(x), y(y) {}
    constexpr IVec2(Vector2 v)
        : x((int)v.x), y((int)v.y) {}

    int x, y;

    bool operator==(IVec2 b) const;
    bool operator!=(IVec2 b) const;

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

    static constexpr IVec2 One()   { return IVec2(1);   }
    static constexpr IVec2 Zero()  { return IVec2(0);   }
    static constexpr IVec2 UnitX() { return IVec2(1,0); }
    static constexpr IVec2 UnitY() { return IVec2(0,1); }

    constexpr operator Vector2() { return Vector2{ (float)x, (float)y }; }
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

IVec2 IVec2Scale_f(IVec2 a, float b);

bool CheckCollisionIVecPointLine(IVec2 pt, IVec2 p1, IVec2 p2);

inline void DrawLineIV(IVec2 start, IVec2 end, Color color)
{
    DrawLine(start.x, start.y, end.x, end.y, color);
}
inline void DrawLineIV(IVec2 start, Width width, Color color)
{
    DrawLine(start.x, start.y, start.x + width.x, start.y, color);
}
inline void DrawLineIV(IVec2 start, Height height, Color color)
{
    DrawLine(start.x, start.y, start.x, start.y + height.y, color);
}
inline void DrawCircleIV(IVec2 origin, float radius, Color color)
{
    DrawCircle(origin.x, origin.y, radius, color);
}
inline void DrawTextureIV(Texture2D texture, IVec2 pos, Color tint)
{
    DrawTexture(texture, pos.x, pos.y, tint);
}
inline void DrawTextIV(const char* text, IVec2 pos, int fontSize, Color color)
{
    DrawText(text, pos.x, pos.y, fontSize, color);
}

struct IRect
{
    IRect() : x(), y(), w(), h() {}
    constexpr IRect(int w)
        : x(0), y(0), w(w), h(w) {}
    constexpr IRect(int w, int h)
        : x(0), y(0), w(w), h(h) {}
    constexpr IRect(int x, int y, int w)
        : x(x), y(y), w(w), h(w) {}
    constexpr IRect(int x, int y, int w, int h)
        : x(x), y(y), w(w), h(h) {}
    constexpr IRect(IVec2 v, int w)
        : x(v.x), y(v.y), w(w), h(w) {}
    constexpr IRect(IVec2 v, int w, int h)
        : x(v.x), y(v.y), w(w), h(h) {}
    constexpr IRect(IVec2 v, IVec2 e)
        : x(v.x), y(v.y), w(e.x), h(e.y) {}
    constexpr IRect(Rectangle r)
        : x((int)r.x), y((int)r.y), w((int)r.width), h((int)r.height) {}

    union
    {
        struct { int x, y, w, h; };
        struct { int minx, miny, maxx, maxy; }; // Be careful when using these! It's just a rename, not a function!
        struct { int x, y; Width width; Height height; }; // width and height are of the Width/Height types, whereas w and h are regular ints.
        struct { IVec2 xy, wh; };
        struct { IVec2 position, extents; };
    };

    inline constexpr int Right()  { return x + w; }
    inline constexpr int Bottom() { return y + h; }
    // Top-left
    inline constexpr IVec2 TL() { return xy; }
    // Top-right
    inline constexpr IVec2 TR() { return xy + width; }
    // Bottom-left
    inline constexpr IVec2 BL() { return xy + height; }
    // Bottom-right
    inline constexpr IVec2 BR() { return xy + wh; }

    inline constexpr operator Rectangle() { return Rectangle{ (float)x, (float)y, (float)w, (float)h }; }

    inline constexpr operator Width() { return Width(w); }
    inline constexpr operator Height() { return Height(h); }

    IRect& Expand(int outline = 1);
    inline IRect& Shrink(int outline = 1)
    {
        return Expand(-outline);
    }

    // Abuse as min and max instead of width and height
    // Returns an IRect with INT_MIN and INT_MAX components for comparing
    inline static consteval IRect Abused()
    {
        return IRect(INT_MAX, INT_MAX, INT_MIN, INT_MIN);
    }
    // Changes width and height from being abused maxx and maxy into normal width/height
    inline void DeAbuse()
    {
        w = maxx - minx;
        h = maxy - miny;
    }
};

IRect IRectFromTwoPoints(IVec2 a, IVec2 b);

constexpr IRect operator+(IRect a, Width b) { return IRect(a.x, a.y, a.w + b.x, a.h); }
constexpr IRect operator-(IRect a, Width b) { return IRect(a.x, a.y, a.w - b.x, a.h); }
constexpr IRect operator*(IRect a, Width b) { return IRect(a.x, a.y, a.w * b.x, a.h); }
constexpr IRect operator/(IRect a, Width b) { return IRect(a.x, a.y, a.w / b.x, a.h); }

constexpr IRect operator+(IRect a, Height b) { return IRect(a.x, a.y, a.w, a.h + b.y); }
constexpr IRect operator-(IRect a, Height b) { return IRect(a.x, a.y, a.w, a.h - b.y); }
constexpr IRect operator*(IRect a, Height b) { return IRect(a.x, a.y, a.w, a.h * b.y); }
constexpr IRect operator/(IRect a, Height b) { return IRect(a.x, a.y, a.w, a.h / b.y); }

inline constexpr IRect ExpandIRect(IRect rec, int outline = 1)
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

bool InBoundingBox(IRect bounds, IVec2 pt);

inline void DrawRectangleIRect(IRect rec, Color color)
{
    DrawRectangle(rec.x, rec.y, rec.w, rec.h, color);
}

void DrawRectangleLinesIRect(IRect rec, Color color);

inline void BeginScissorMode(IRect area)
{
    BeginScissorMode(area.x, area.y, area.w, area.h);
}

template<int W, int H = W>
void DrawIconPro(Texture2D iconSheet, IVec2 iconColRow, IVec2 pos, float scale, Color tint)
{
    DrawTexturePro(iconSheet,
        Rectangle{ (float)iconColRow.x * (float)W, (float)iconColRow.y * (float)H, (float)W, (float)H },
        Rectangle{ (float)pos.x, (float)pos.y, (float)W * scale, (float)H * scale },
        Vector2{ 0, 0 }, 0.0f, tint);
}

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
