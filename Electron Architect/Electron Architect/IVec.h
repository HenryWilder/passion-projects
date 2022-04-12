#pragma once
#ifndef RAYLIB_H
#include <raylib.h>
#endif

struct IVec2
{
    IVec2() = default;
    constexpr IVec2(int x)
        : x(x), y(x) {}
    constexpr IVec2(int x, int y)
        : x(x), y(y) {}
    constexpr IVec2(Vector2 v)
        : x((int)v.x), y((int)v.y) {}

    int x, y;

    bool operator==(IVec2 b) const;
    bool operator!=(IVec2 b) const;

    constexpr IVec2 operator+=(IVec2 b) { return x += b.x, y += b.y, *this; }
    constexpr IVec2 operator-=(IVec2 b) { return x -= b.x, y -= b.y, *this; }
    constexpr IVec2 operator*=(IVec2 b) { return x *= b.x, y *= b.y, *this; }
    constexpr IVec2 operator/=(IVec2 b) { return x /= b.x, y /= b.y, *this; }

    constexpr IVec2 operator*=(int b) { return x += b, y += b, *this; }
    constexpr IVec2 operator/=(int b) { return x += b, y += b, *this; }

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

bool InBoundingBox(IVec2 p, IVec2 a, IVec2 b);

constexpr IVec2 operator+(IVec2 a, IVec2 b) { return IVec2(a.x + b.x, a.y + b.y); }
constexpr IVec2 operator-(IVec2 a, IVec2 b) { return IVec2(a.x - b.x, a.y - b.y); }
constexpr IVec2 operator*(IVec2 a, IVec2 b) { return IVec2(a.x * b.x, a.y * b.y); }
constexpr IVec2 operator/(IVec2 a, IVec2 b) { return IVec2(a.x / b.x, a.y / b.y); }

constexpr IVec2 operator*(IVec2 a, int b) { return IVec2(a.x * b, a.y * b); }
constexpr IVec2 operator/(IVec2 a, int b) { return IVec2(a.x / b, a.y / b); }
constexpr IVec2 operator*(int a, IVec2 b) { return IVec2(a * b.x, a * b.y); }
constexpr IVec2 operator/(int a, IVec2 b) { return IVec2(a / b.x, a / b.y); }

IVec2 IVec2Scale_f(IVec2 a, float b);

bool CheckCollisionIVecPointLine(IVec2 pt, IVec2 p1, IVec2 p2);

void DrawLineIV(IVec2 start, IVec2 end, Color color);
void DrawCircleIV(IVec2 origin, float radius, Color color);
void DrawTextureIV(Texture2D texture, IVec2 pos, Color tint);

struct IRect
{
    IRect() = default;
    constexpr IRect(int w)
        : x(0), y(0), w(w), h(w) {}
    constexpr IRect(int w, int h)
        : x(0), y(0), w(w), h(h) {}
    constexpr IRect(int x, int y, int w)
        : x(x), y(y), w(w), h(w) {}
    constexpr IRect(int x, int y, int w, int h)
        : x(x), y(y), w(w), h(h) {}
    constexpr IRect(Rectangle r)
        : x((int)r.x), y((int)r.y), w((int)r.width), h((int)r.height) {}

    union
    {
        struct { int x, y, w, h; };
        struct { int x, y, width, height; };
        struct { IVec2 xy, wh; };
        struct { IVec2 position, extents; };
    };

    constexpr operator Rectangle() { return Rectangle{ (float)x, (float)y, (float)w, (float)h }; }

    IRect& Expand(int outline);
};

constexpr IRect ExpandIRect(IRect rec, int outline)
{
    return IRect(
        rec.x - outline,
        rec.y - outline,
        rec.w + 2 * outline,
        rec.h + 2 * outline);
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
