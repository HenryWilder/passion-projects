#pragma once
#ifndef RAYLIB_H
#include <raylib.h>
#endif
#include "IVec.h"

class Node;

enum class ElbowConfig : uint8_t
{
    horizontal,
    vertical,
    diagonalA,
    diagonalB,
};
ElbowConfig& operator++(ElbowConfig& ec);
ElbowConfig& operator--(ElbowConfig& ec);

struct Wire
{
    Wire();
    Wire(Node* start, Node* end);
    Wire(Node* start, Node* end, ElbowConfig elbowConfig);

    static constexpr Color g_wireColorActive = REDSTONE;
    static constexpr Color g_wireColorInactive = DEADCABLE;
    static constexpr float g_elbowRadius = 2.0f;

    IVec2 elbow;
    ElbowConfig elbowConfig;
    Node* start;
    Node* end;

    bool GetState() const;
    static void Draw(IVec2 start, IVec2 joint, IVec2 end, Color color);
    void Draw(Color color) const;
    static void DrawElbow(IVec2 pos, Color color);
    void DrawElbow(Color color) const;
    
    IVec2 GetStartPos() const;

    int GetStartX() const;
    int GetStartY() const;

    IVec2 GetElbowPos() const;
    int GetElbowX() const;
    int GetElbowY() const;

    IVec2 GetEndPos() const;
    int GetEndX() const;
    int GetEndY() const;

    static IVec2 GetLegalElbowPosition(IVec2 start, IVec2 end, ElbowConfig config);
    IVec2 GetLegalElbowPosition(ElbowConfig config) const;
    void GetLegalElbowPositions(IVec2(&legal)[4]) const;
    void UpdateElbowToLegal();
    void SnapElbowToLegal(IVec2 pos);
};
