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
    _ASSERT_EXPR((uint8_t)config < 4, L"Elbow index out of bounds");

    bool startFirst(config == ElbowConfig::diagonalA || config == ElbowConfig::horizontal);
    auto [first, second] = (startFirst ? std::pair{ start, end } : std::pair{ end, start });

    bool cardinal(config == ElbowConfig::horizontal || config == ElbowConfig::vertical);
    if (cardinal) return IVec2(first.x, second.y);

    IVec2 diff(end - start);
    int shortLength = std::min(abs(diff.x), abs(diff.y));
    IVec2 diagonal(shortLength);

    if (second.x < first.x) diagonal.x *= -1;
    if (second.y < first.y) diagonal.y *= -1;

    return first + diagonal;
}
IVec2 Wire::GetLegalElbowPosition(ElbowConfig config) const
{
    return GetLegalElbowPosition(GetStartPos(), GetEndPos(), config);
}
void Wire::UpdateElbowToLegal()
{
    elbow = GetLegalElbowPosition(elbowConfig);
}
void Wire::SnapElbowToLegal(IVec2 pos)
{
    constexpr ElbowConfig configOrder[] =
    {
        ElbowConfig::horizontal,
        ElbowConfig::diagonalA,
        ElbowConfig::vertical,
        ElbowConfig::diagonalB,
    };
    long shortestDist = LONG_MAX;
    for (int i = 0; i < _countof(configOrder); ++i)
    {
        IVec2 propPosed = GetLegalElbowPosition(configOrder[i]);
        long dist = DistanceSqr(pos, propPosed);
        if (dist < shortestDist)
        {
            shortestDist = dist;
            elbowConfig = configOrder[i];
            elbow = propPosed;
        }
    }
}
