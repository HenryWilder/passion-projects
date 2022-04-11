#include "HUtility.h"
#include "Group.h"

Group::Group() = default;
// Takes captureBounds as rec
Group::Group(IRect rec, Color color) : labelBounds(rec.x, rec.y - g_labelHeight, rec.w, g_labelHeight), captureBounds(rec), color(color), label() {}
Group::Group(IRect rec, Color color, const std::string & label) : labelBounds(rec.x, rec.y - g_labelHeight, rec.w, g_labelHeight), captureBounds(rec), color(color), label(label) {}

void Group::Draw() const
{
    DrawRectangleIRect(captureBounds, ColorAlpha(color, 0.25));
    DrawRectangleLines(captureBounds.x, captureBounds.y, captureBounds.w, captureBounds.h, color);
    DrawRectangleIRect(labelBounds, color);
    constexpr int padding = g_labelHeight / 4;
    DrawText(label.c_str(), labelBounds.x + padding, labelBounds.y + padding, g_fontSize, WHITE);
}
void Group::Highlight(Color highlight) const
{
    IRect rec = labelBounds;
    rec.h += captureBounds.h;
    DrawRectangleLines(rec.x - 1, rec.y - 1, rec.w + 2, rec.h + 2, highlight);
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
