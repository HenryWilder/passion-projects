#pragma once
#include "IVec.h"

class Group
{
private:
    static constexpr int g_fontSize = g_gridSize;
    static constexpr int g_labelHeight = g_fontSize * 2;
    static constexpr int g_padding = g_labelHeight / 4;

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

    Color GetColor() const;

    IVec2 GetPosition() const;
    void SetPosition(IVec2 pos);

    IRect GetBounds() const;

    IRect GetLabelBounds() const;
    const std::string& GetLabel() const;

    IRect GetCaptureBounds() const;
    void SetCaptureBounds(IRect bounds);

    IRect GetResizeCollision_TopL() const;
    IRect GetResizeCollision_TopR() const;
    IRect GetResizeCollision_BotL() const;
    IRect GetResizeCollision_BotR() const;
    void GetResizeCollisions(_Out_ IRect(&output)[4]) const;
    IRect GetResizeCollision(uint8_t index) const;
};

struct GroupCorner
{
    Group* group = nullptr;
    uint8_t cornerIndex = 0;
    bool Valid() const;
    IRect GetCollisionRect() const;
};
