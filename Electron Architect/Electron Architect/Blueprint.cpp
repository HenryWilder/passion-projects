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

// Returns the containing rectangle
void Blueprint::DrawSelectionPreview(IVec2 pos, Color backgroundColor, Color nodeColor, Color ioNodeColor, Color wireColor) const
{
    IVec2 offset = pos + IVec2(g_gridSize);
    DrawRectangleIRect(GetSelectionPreviewRect(pos), backgroundColor);
    for (const WireBP& wire_bp : wires)
    {
        IVec2 start = nodes[wire_bp.startNodeIndex].relativePosition + offset - IVec2::One();
        IVec2 end   = nodes[wire_bp.  endNodeIndex].relativePosition + offset - IVec2::One();
        DrawLineIV(start, end, wireColor);
    }
    for (const NodeBP& node_bp : nodes)
    {
        Color color;
        if (node_bp.b_io)
            color = ioNodeColor;
        else
            color = nodeColor;
        Node::Draw(node_bp.relativePosition + offset, node_bp.gate, color);
    }
}

IRect Blueprint::GetSelectionPreviewRect(IVec2 pos) const
{
    return IRect(pos, extents + IVec2(g_gridSize * 2));
}

void Blueprint::Save() const
{
    std::ofstream file(TextFormat(".\\blueprints\\%s.bp", name.c_str()));
    file << nodes.size() << '\n';
    for (const NodeBP& node : nodes)
    {
        file << node.b_io << ' ' << (char)node.gate << ' ' << (unsigned)node.extraParam << ' ' << node.relativePosition.x << ' ' << node.relativePosition.y << '\n';
    }
    file << wires.size() << '\n';
    for (const WireBP& wire : wires)
    {
        file << wire.startNodeIndex << ' ' << wire.endNodeIndex << ' ' << (int)wire.elbowConfig << '\n';
    }
    file.close();
}

void LoadBlueprint(const char* filename, Blueprint& dest)
{
    dest = Blueprint(); // Reset in case of edge cases
    std::ifstream file(TextFormat("%s", filename));
    if (file.bad())
        return;
    std::string name = filename;
    size_t start = name.find_last_of('\\') + 1;
    size_t end = name.size() - 3;
    if (start != name.npos)
        dest.name = name.substr(start, end - start);
    else
        dest.name = name;
    IVec2 extents = IVec2::Zero();
    size_t nodeCount;
    file >> nodeCount;
    try
    {
        dest.nodes.reserve(nodeCount);
    }
    catch (std::length_error e)
    {
        file.close();
        throw e;
    }
    catch (...)
    {
        file.close();
        throw std::exception("unknown");
    }
    for (size_t i = 0; i < nodeCount; ++i)
    {
        bool io;
        char gate;
        uint8_t ep;
        IVec2 pos;
        file >> io >> gate >> ep >> pos.x >> pos.y;
        dest.nodes.emplace_back(io, (Gate)gate, ep, pos);
        extents.x = std::max(pos.x, extents.x);
        extents.y = std::max(pos.y, extents.y);
    }
    dest.extents = extents;
    size_t wireCount;
    file >> wireCount;
    try
    {
        dest.wires.reserve(wireCount);
    }
    catch (std::length_error e)
    {
        file.close();
        throw e;
    }
    catch (...)
    {
        file.close();
        throw std::exception("unknown");
    }
    for (size_t i = 0; i < wireCount; ++i)
    {
        size_t startNodeIndex, endNodeIndex;
        uint8_t elbowConfig;
        file >> startNodeIndex >> endNodeIndex >> elbowConfig;
        dest.wires.emplace_back(startNodeIndex, endNodeIndex, (ElbowConfig)elbowConfig);
    }
    file.close();
}
