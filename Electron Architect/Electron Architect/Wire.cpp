#include "HUtility.h"
#include "Node.h"
#include "Wire.h"

ElbowConfig& operator++(ElbowConfig& ec)
{
    if ((uint8_t)ec == 3)
        return ec = (ElbowConfig)0;
    return ec = (ElbowConfig)((uint8_t)ec + 1);
}
ElbowConfig& operator--(ElbowConfig& ec)
{
    if ((uint8_t)ec == 0)
        return ec = (ElbowConfig)3;
    return ec = (ElbowConfig)((uint8_t)ec - 1);
}

Wire::Wire(Node * start, Node * end) : elbow(), elbowConfig((ElbowConfig)0), start(start), end(end) {}
Wire::Wire(Node * start, Node * end, ElbowConfig elbowConfig) : elbow(), elbowConfig(elbowConfig), start(start), end(end) { UpdateElbowToLegal(); }

bool Wire::GetState() const
{
    return start->GetState();
}
void Wire::Draw(IVec2 start, IVec2 joint, IVec2 end, Color color)
{
    DrawLineIV(start, joint, color);
    DrawLineIV(joint, end, color);
}
void Wire::Draw(Color color) const
{
    constexpr IVec2 activeOffset = IVec2(1, -1);
    if (GetState())
        Draw(GetStartPos() + activeOffset, elbow + activeOffset, GetEndPos() + activeOffset, color);
    else
        Draw(GetStartPos(), elbow, GetEndPos(), color);
}
void Wire::DrawElbow(IVec2 pos, Color color)
{
    DrawCircle(pos.x, pos.y, g_elbowRadius, color);
}
void Wire::DrawElbow(Color color) const
{
    DrawElbow(elbow, color);
}

IVec2 Wire::GetStartPos() const
{
    return start->GetPosition();
}

int Wire::GetStartX() const
{
    return GetStartPos().x;
}
int Wire::GetStartY() const
{
    return GetStartPos().y;
}

IVec2 Wire::GetElbowPos() const
{
    return elbow;
}
int Wire::GetElbowX() const
{
    return GetElbowPos().x;
}
int Wire::GetElbowY() const
{
    return GetElbowPos().y;
}

IVec2 Wire::GetEndPos() const
{
    return end->GetPosition();
}
int Wire::GetEndX() const
{
    return GetEndPos().x;
}
int Wire::GetEndY() const
{
    return GetEndPos().y;
}

IVec2 Wire::GetLegalElbowPosition(IVec2 start, IVec2 end, ElbowConfig config)
{
    uint8_t index = (uint8_t)config;
    _ASSERT_EXPR(index < 4, L"Elbow index out of bounds");
    if (index == 0)
        return IVec2(start.x, end.y);
    else if (index == 1)
        return IVec2(end.x, start.y);
    else
    {
        int shortLength = std::min(
            abs(end.x - start.x),
            abs(end.y - start.y)
        );
        IVec2 pos;
        if (index == 2)
        {
            pos = start;

            if (end.x < start.x)
                pos.x -= shortLength;
            else
                pos.x += shortLength;

            if (end.y < start.y)
                pos.y -= shortLength;
            else
                pos.y += shortLength;
        }
        else // index == 3
        {
            pos = end;

            if (start.x < end.x)
                pos.x -= shortLength;
            else
                pos.x += shortLength;

            if (start.y < end.y)
                pos.y -= shortLength;
            else
                pos.y += shortLength;
        }
        return pos;
    }
}
IVec2 Wire::GetLegalElbowPosition(ElbowConfig config) const
{
    return GetLegalElbowPosition(GetStartPos(), GetEndPos(), config);
}
void Wire::GetLegalElbowPositions(IVec2(&legal)[4]) const
{
    int shortLength = std::min(
        abs(GetEndX() - GetStartX()),
        abs(GetEndY() - GetStartY())
    );

    legal[0] = IVec2(GetStartX(), GetEndY());
    legal[1] = IVec2(GetEndX(), GetStartY());
    legal[2] = GetStartPos() + IVec2(GetEndX() < GetStartX() ? -shortLength : shortLength,
                                     GetEndY() < GetStartY() ? -shortLength : shortLength);
    legal[3] = GetEndPos()   + IVec2(GetStartX() < GetEndX() ? -shortLength : shortLength,
                                     GetStartY() < GetEndY() ? -shortLength : shortLength);
}
void Wire::UpdateElbowToLegal()
{
    elbow = GetLegalElbowPosition(elbowConfig);
}
void Wire::SnapElbowToLegal(IVec2 pos)
{
    IVec2 legal[4];
    GetLegalElbowPositions(legal);

    uint8_t pick = 0;
    long shortestDist = LONG_MAX;
    for (uint8_t i = 0; i < 4; ++i)
    {
        long dist = DistanceSqr(pos, legal[i]);
        if (dist < shortestDist)
        {
            shortestDist = dist;
            pick = i;
        }
    }
    elbowConfig = (ElbowConfig)pick; // Index of pick
    elbow = legal[pick];
}
