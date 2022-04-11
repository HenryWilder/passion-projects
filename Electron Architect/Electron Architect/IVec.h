#pragma once
#ifndef RAYLIB_H
#include <raylib.h>
#endif

struct IVec2
{
    IVec2() = default;
    constexpr IVec2(int x);
    constexpr IVec2(int x, int y);
    constexpr IVec2(Vector2 v);

    int x, y;

    bool operator==(IVec2 b) const;
    bool operator!=(IVec2 b) const;

    static constexpr IVec2 One();
    static constexpr IVec2 Zero();
    static constexpr IVec2 UnitX();
    static constexpr IVec2 UnitY();

    constexpr operator Vector2();
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

IVec2 operator+(IVec2 a, IVec2 b);
IVec2 operator-(IVec2 a, IVec2 b);
IVec2 operator*(IVec2 a, IVec2 b);
IVec2 operator/(IVec2 a, IVec2 b);

IVec2 operator*(IVec2 a, int b);
IVec2 operator/(IVec2 a, int b);
IVec2 operator*(int a, IVec2 b);
IVec2 operator/(int a, IVec2 b);

IVec2 IVec2Scale_f(IVec2 a, float b);

bool CheckCollisionIVecPointLine(IVec2 pt, IVec2 p1, IVec2 p2);

void DrawLineIV(IVec2 start, IVec2 end, Color color);
void DrawCircleIV(IVec2 origin, float radius, Color color);
void DrawTextureIV(Texture2D texture, IVec2 pos, Color tint);

struct IRect
{
    IRect() = default;
    constexpr IRect(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {}

    union
    {
        struct { int x, y, w, h; };
        struct { IVec2 xy, wh; };
    };
};

bool InBoundingBox(IRect bounds, IVec2 pt);

void DrawRectangleIRect(IRect rec, Color color);

void BeginScissorMode(IRect area);

template<int width, int height = width>
void DrawIcon(Texture2D iconSheet, IVec2 iconColRow, IVec2 pos, Color tint)
{
    BeginScissorMode(pos.x, pos.y, width, height);
    DrawTexture(iconSheet,
        pos.x - iconColRow.x * width,
        pos.y - iconColRow.y * height,
        tint);
    EndScissorMode();
}
