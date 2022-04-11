#pragma once
#include "IVec.h"

class Group
{
private:
    static constexpr int g_fontSize = g_gridSize;
    static constexpr int g_labelHeight = g_fontSize * 2;

    IRect labelBounds;
    IRect captureBounds;
    Color color;
    std::string label;

    friend class NodeWorld;

public:
    Group() = default;
    // Takes captureBounds as rec
    Group(IRect rec, Color color);
    Group(IRect rec, Color color, const std::string& label);

    void Draw() const;
    void Highlight(Color highlight) const;

    IVec2 GetPosition() const;
    void SetPosition(IVec2 pos);
};

