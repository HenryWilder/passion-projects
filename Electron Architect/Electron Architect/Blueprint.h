#pragma once
#ifndef RAYLIB_H
#include <raylib.h>
#endif
#include "HUtility.h"
#include "IVec.h"
#include "Node.h"
#include "Wire.h"

struct NodeBP
{
    NodeBP() : b_io(), gate(), extraParam(), relativePosition(), name("") {}
    constexpr NodeBP(bool b_io, Gate gate, IVec2 relativePosition) :
        b_io(b_io), gate(gate), extraParam(0), relativePosition(relativePosition), name() {}
    constexpr NodeBP(bool b_io, Gate gate, uint8_t extraParam, IVec2 relativePosition) :
        b_io(b_io), gate(gate), extraParam(extraParam), relativePosition(relativePosition), name() {}
    constexpr NodeBP(const char* name, bool b_io, Gate gate, IVec2 relativePosition) :
        b_io(b_io), gate(gate), extraParam(0), relativePosition(relativePosition), name(name) {}
    constexpr NodeBP(const char* name, bool b_io, Gate gate, uint8_t extraParam, IVec2 relativePosition) :
        b_io(b_io), gate(gate), extraParam(extraParam), relativePosition(relativePosition), name(name) {}

    bool b_io; // Whether the node is an input/output to the entire system (Should it be shown on the paste preview?)
    Gate gate;
    uint8_t extraParam;
    IVec2 relativePosition;
    const char* name;
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
    Blueprint() : name("Unnamed blueprint"), extents() {}
    Blueprint(const std::vector<Node*>& src);
    Blueprint(const char* name, std::vector<NodeBP>&& nodes, std::vector<WireBP>&& wires) :
        name(name), nodes(std::begin(nodes), std::end(nodes)), wires(std::begin(wires), std::end(wires))
    {
        extents = IVec2::Zero();
        for (const NodeBP& node_bp : nodes)
        {
            if (node_bp.relativePosition.x > extents.x)
                extents.x = node_bp.relativePosition.x;
            if (node_bp.relativePosition.y > extents.y)
                extents.y = node_bp.relativePosition.y;
        }
    }

    std::string name;
    IVec2 extents;
    std::vector<NodeBP> nodes;
    std::vector<WireBP> wires;

    void DrawSelectionPreview(IVec2 pos, Color backgroundColor, Color nodeColor, Color ioNodeColor, Color wireColor, uint8_t lod) const;
    IRect GetSelectionPreviewRect(IVec2 pos) const;

    void Save() const;
};

void LoadBlueprint(const char* filename, Blueprint& dest);
