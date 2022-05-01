#include <thread>
#include <fstream>
#include "Blueprint.h"

void Blueprint::PopulateNodes(const std::vector<Node*>& src)
{
    constexpr IRect boundsInit = IRect(
        INT_MAX,
        std::numeric_limits<int>::max(),
        std::numeric_limits<int>::min(),
        std::numeric_limits<int>::min());
    IRect bounds = boundsInit;
    for (Node* node : src)
    {
        const IVec2& compare = node->GetPosition();
        if (compare.x < bounds.minx) bounds.minx = compare.x;
        if (compare.y < bounds.miny) bounds.miny = compare.y;
        if (compare.x > bounds.maxx) bounds.maxx = compare.x;
        if (compare.y > bounds.maxy) bounds.maxy = compare.y;
    }
    bounds.DeAbuse();
    extents = bounds.wh;

    IVec2 min = bounds.xy;
    nodes.reserve(src.size());
    std::unordered_set<Node*> nodeSet(src.begin(), src.end());
    for (Node* node : src)
    {
        bool isIO = node->IsOutputOnly() || node->IsInputOnly();
        if (!isIO)
        {
            for (Wire* wire : node->GetWires())
            {
                if (nodeSet.find(wire->start == node ? wire->end : wire->start) == nodeSet.end())
                {
                    isIO = true;
                    break;
                }
            }
        }

        uint8_t extraParam;
        switch (node->GetGate())
        {
        case Gate::RESISTOR:    extraParam = node->GetResistance(); break;
        case Gate::CAPACITOR:   extraParam = node->GetCapacity();   break;
        case Gate::LED:         extraParam = node->GetColorIndex(); break;
        }
        nodes.emplace_back(
            isIO,
            node->GetGate(),
            extraParam,
            node->GetPosition() - min);
    }
}
void Blueprint::PopulateWires(const std::vector<Node*>& src)
{
    std::unordered_map<Node*, size_t> nodeIndices;
    std::unordered_map<Wire*, bool> visitedWires;

    // Populate nodeIndices
    for (size_t i = 0; i < src.size(); ++i)
    {
        nodeIndices.emplace(src[i], i);
    }

    // Count wires
    {
        {
            size_t totalWires = 0;
            for (Node* node : src)
            {
                for (Wire* wire : node->GetWires())
                {
                    ++totalWires;
                }
            }
            visitedWires.reserve(totalWires);
        }

        {
            size_t uniqueWires = 0;
            for (Node* node : src)
            {
                for (Wire* wire : node->GetWires())
                {
                    if (nodeIndices.find(wire->start) == nodeIndices.end() ||
                        nodeIndices.find(wire->end) == nodeIndices.end())
                        continue;
                    if (visitedWires.find(wire) == visitedWires.end())
                    {
                        visitedWires.insert({ wire, false });
                        ++uniqueWires;
                    }
                }
            }
            wires.reserve(uniqueWires);
        }
    }

    for (Node* node : src)
    {
        for (Wire* wire : node->GetWires())
        {
            if (visitedWires.find(wire) != visitedWires.end() && // Is an internal wire
                !visitedWires.find(wire)->second) // Has not been visited in this loop
            {
                visitedWires[wire] = true;
                wires.emplace_back(
                    nodeIndices.find(wire->start)->second,
                    nodeIndices.find(wire->end)->second,
                    wire->elbowConfig);
            }
        }
    }
}

Blueprint::Blueprint(const std::vector<Node*>& src)
{
    name = "Unnamed blueprint";
    extents = IVec2::Zero();
    std::thread nodeThread(&Blueprint::PopulateNodes, this, std::ref(src));
    std::thread wireThread(&Blueprint::PopulateWires, this, std::ref(src));
    nodeThread.join();
    wireThread.join();
}

void Blueprint::DrawPreview(IVec2 pos, Color boxColor, Color nodeColor) const
{
    constexpr int halfGrid = g_gridSize / 2;
    DrawRectangle(pos.x - halfGrid, pos.y - halfGrid, extents.x + g_gridSize, extents.y + g_gridSize, boxColor);
    for (const NodeBP& node_bp : nodes)
    {
        if (node_bp.b_io)
        {
            DrawCircleIV(node_bp.relativePosition + pos, Node::g_nodeRadius, nodeColor);
            DrawCircleIV(node_bp.relativePosition + pos, Node::g_nodeRadius - 1.0f, BLACK); // In case boxColor is transparent
            DrawCircleIV(node_bp.relativePosition + pos, Node::g_nodeRadius - 1.0f, boxColor);
        }
    }
}

// Draws at a 50% scale
// Returns the containing rectangle
void Blueprint::DrawSelectionPreview(IVec2 pos, Color backgroundColor, Color nodeColor, Color wireColor) const
{
    constexpr int halfGrid = g_gridSize / 2;
    constexpr float halfRadius = Node::g_nodeRadius * 0.5f;
    IVec2 offset = pos + IVec2(halfGrid);
    DrawRectangleIRect(GetSelectionPreviewRect(pos), backgroundColor);
    for (const WireBP& wire_bp : wires)
    {
        IVec2 start = nodes[wire_bp.startNodeIndex].relativePosition / 2 + offset - IVec2::One();
        IVec2 end   = nodes[wire_bp.  endNodeIndex].relativePosition / 2 + offset - IVec2::One();
        DrawLineIV(start, end, nodeColor);
    }
    for (const NodeBP& node_bp : nodes)
    {
        DrawCircleIV(node_bp.relativePosition / 2 + offset, halfRadius, nodeColor);
    }
}

IRect Blueprint::GetSelectionPreviewRect(IVec2 pos) const
{
    return IRect(pos, extents / 2 + IVec2(g_gridSize));
}

void Blueprint::Save() const
{
    std::ofstream file(TextFormat("%s.bp", name));
    file << nodes.size() << '\n';
    for (const NodeBP& node : nodes)
    {
        file << node.b_io << ' ' << (char)node.gate << ' ' << node.extraParam << ' ' << node.relativePosition.x << ' ' << node.relativePosition.y << '\n';
    }
    file << wires.size() << '\n';
    for (const WireBP& wire : wires)
    {
        file << wire.startNodeIndex << ' ' << wire.endNodeIndex << ' ' << (int)wire.elbowConfig << '\n';
    }
    file.close();
}
