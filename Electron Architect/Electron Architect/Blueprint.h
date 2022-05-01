#pragma once
#ifndef RAYLIB_H
#include <raylib.h>
#endif
#include "HUtility.h"
#include "IVec.h"
#include "Node.h"
#include "Wire.h"

struct IconPos
{
    uint16_t id;
    uint8_t x; // 0 for left, 1 for center, 2 for right
    uint8_t y; // 0 for top,  1 for center, 2 for bottom

    // Position on screen
    IVec2 Pos() const;
    void Draw(IVec2 start, Color tint) const;
};

// Combo of up to four icons for representing a blueprint
struct BlueprintIcon
{
public:
    static constexpr int g_size = 16;
private:
    static Texture2D g_iconSheet;
    static IVec2 g_iconSheetDimensions; // Rows and columns, not pixels
public:
    IconPos combo[4] = { IconPos{ NULL, 0,0 }, IconPos{ NULL, 0,0 }, IconPos{ NULL, 0,0 }, IconPos{ NULL, 0,0 }, };


private:
    static IVec2 ColRowFromIcon(uint16_t icon);
public:
    static uint16_t GetIconAtColRow(IVec2 colRow);
    static IVec2 PixelToColRow(IVec2 sheetPos, IVec2 selectPos);
    static IVec2 GetSheetSize_RC(); // Rows and columns
    static IVec2 GetSheetSize_Px(); // Pixels
    static void DrawBPIcon(uint16_t icon, IVec2 pos, Color tint);

    static void DrawSheet(IVec2 pos, Color background, Color tint);

    static void Load(const char* filename);
    static void Unload();

    BlueprintIcon();
    BlueprintIcon(const std::vector<IconPos>& icons);
    BlueprintIcon(IconPos(&icons)[4]);

    void DrawBackground(IVec2 pos, Color color) const;
    void Draw(IVec2 pos, Color tint) const;
};

struct NodeBP
{
    NodeBP() = default;
    constexpr NodeBP(bool b_io, Gate gate, IVec2 relativePosition) :
        b_io(b_io), gate(gate), extraParam(0), relativePosition(relativePosition) {}
    constexpr NodeBP(bool b_io, Gate gate, uint8_t extraParam, IVec2 relativePosition) :
        b_io(b_io), gate(gate), extraParam(extraParam), relativePosition(relativePosition) {}

    bool b_io;
    Gate gate;
    uint8_t extraParam;
    IVec2 relativePosition;
};

struct WireBP
{
    WireBP() = default;
    constexpr WireBP(size_t startNodeIndex, size_t endNodeIndex, ElbowConfig elbowConfig) :
        startNodeIndex(startNodeIndex), endNodeIndex(endNodeIndex), elbowConfig(elbowConfig) {}

    size_t startNodeIndex, endNodeIndex;
    ElbowConfig elbowConfig;
};

struct Blueprint
{
private: // Multithread functions
    void PopulateNodes(const std::vector<Node*>& src);
    void PopulateWires(const std::vector<Node*>& src);

public:
    Blueprint(const std::vector<Node*>& src);
    template<size_t NODECOUNT, size_t WIRECOUNT>
    constexpr Blueprint(IVec2 extents, NodeBP(&nodes)[NODECOUNT], WireBP(&wires)[WIRECOUNT]) :
        extents(extents), nodes(std::begin(nodes), std::end(nodes)), wires(std::begin(wires), std::end(wires)) {}

    IVec2 extents;
    std::vector<NodeBP> nodes;
    std::vector<WireBP> wires;

    void DrawPreview(IVec2 pos, Color boxColor, Color nodeColor) const;
};
