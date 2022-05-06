#include "HUtility.h"
#include "Group.h"

// Takes captureBounds as rec
Group::Group(IRect rec, Color color) : labelBounds(rec.x, rec.y - g_labelHeight, rec.w, g_labelHeight), captureBounds(rec), color(color), label() {}
Group::Group(IRect rec, Color color, const std::string & label) : labelBounds(rec.x, rec.y - g_labelHeight, rec.w, g_labelHeight), captureBounds(rec), color(color), label(label) {}

void Group::Draw() const
{
    DrawRectangleIRect(captureBounds, ColorAlpha(color, 0.25));
    DrawRectangleLines(captureBounds.x, captureBounds.y, captureBounds.w, captureBounds.h, color);
    DrawRectangleIRect(labelBounds, color);
    DrawText(label.c_str(), labelBounds.x + g_padding, labelBounds.y + g_padding, g_fontSize, WHITE);
}
void Group::Highlight(Color highlight) const
{
    DrawRectangleIRect(ExpandIRect(labelBounds), highlight);
    DrawText(label.c_str(), labelBounds.x + g_padding, labelBounds.y + g_padding, g_fontSize, color);
    DrawRectangleLines(captureBounds.x - 1, captureBounds.y - 1, captureBounds.w + 2, captureBounds.h + 2, highlight);
}

Color Group::GetColor() const
{
    return color;
}

IVec2 Group::GetPosition() const
{
    return captureBounds.xy;
}
void Group::SetPosition(IVec2 pos)
{
    labelBounds.xy = captureBounds.xy = pos;
    labelBounds.y -= g_labelHeight;
}

IRect Group::GetBounds() const
{
    return labelBounds + captureBounds.height;
}

IRect Group::GetLabelBounds() const
{
    return labelBounds;
}

const std::string& Group::GetLabel() const
{
    return label;
}

IRect Group::GetCaptureBounds() const
{
    return captureBounds;
}

void Group::SetCaptureBounds(IRect bounds)
{
    captureBounds = bounds;
    labelBounds.x = bounds.x;
    labelBounds.y = bounds.y - g_labelHeight;
    labelBounds.w = bounds.w;
}

IRect Group::GetResizeCollision_TopL() const
{
    return IRect(captureBounds.xy, g_gridSize);
}

IRect Group::GetResizeCollision_TopR() const
{
    return IRect(captureBounds.xy + (captureBounds.width - g_gridSize), g_gridSize);
}

IRect Group::GetResizeCollision_BotL() const
{
    return IRect(captureBounds.xy + (captureBounds.height - g_gridSize), g_gridSize);
}

IRect Group::GetResizeCollision_BotR() const
{
    return IRect(captureBounds.xy + captureBounds.wh - IVec2(g_gridSize), g_gridSize);
}

void Group::GetResizeCollisions(_Out_ IRect(&output)[4]) const
{
    output[0] = GetResizeCollision_TopL();
    output[1] = GetResizeCollision_TopR();
    output[2] = GetResizeCollision_BotL();
    output[3] = GetResizeCollision_BotR();
}

using namespace std;
IRect Group::GetResizeCollision(uint8_t index) const
{
    _ASSERT_EXPR(index < 4, L"Index out of range");

#pragma warning(disable:4715)
    switch (index)
    {
    case 0: return GetResizeCollision_TopL();
    case 1: return GetResizeCollision_TopR();
    case 2: return GetResizeCollision_BotL();
    case 3: return GetResizeCollision_BotR();
    }
    return IRect(0);
#pragma warning(default:4715)
}

bool GroupCorner::Valid() const
{
    return !!group;
}
IRect GroupCorner::GetCollisionRect() const
{
    return group->GetResizeCollision(cornerIndex);
}
