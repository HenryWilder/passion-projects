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
        if (node->HasName())
        {
            nodes.emplace_back(
                node->GetName(),
                isIO,
                node->GetGate(),
                extraParam,
                node->GetPosition() - min);
        }
        else
        {
            nodes.emplace_back(
                isIO,
                node->GetGate(),
                extraParam,
                node->GetPosition() - min);
        }
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

void Blueprint::DrawSelectionPreview(IVec2 pos, Color backgroundColor, Color nodeColor, Color ioNodeColor, Color wireColor, uint8_t lod) const
{
    IVec2 offset = pos + IVec2(g_gridSize);
    DrawRectangleIRect(GetSelectionPreviewRect(pos), backgroundColor);

    // Wires
    switch (lod)
    {
    case 0: // Full wire quality
        for (const WireBP& wire_bp : wires)
        {
            IVec2 start = nodes[wire_bp.startNodeIndex].relativePosition + offset;
            IVec2 end = nodes[wire_bp.endNodeIndex].relativePosition + offset;
            IVec2 elbow = Wire::GetLegalElbowPosition(start, end, wire_bp.elbowConfig);
            Wire::Draw(start, elbow, end, wireColor);
        }
        break;
    case 1: // Wires are straight lines without elbows
        for (const WireBP& wire_bp : wires)
        {
            IVec2 start = nodes[wire_bp.startNodeIndex].relativePosition + offset;
            IVec2 end = nodes[wire_bp.endNodeIndex].relativePosition + offset;
            DrawLineIV(start, end, wireColor);
        }
        break;
    default: // Wires are not drawn
        break;
    }

    // Nodes
    switch (lod)
    {
    case 0: // Full node quality
    case 1: // Full node quality
        for (const NodeBP& node_bp : nodes)
        {
            Color color;
            if (node_bp.b_io)
                color = ioNodeColor;
            else
                color = nodeColor;
            Node::Draw(node_bp.relativePosition + offset, node_bp.gate, color, backgroundColor);
        }
        break;
    case 2: // All nodes are circles, io are colored
        for (const NodeBP& node_bp : nodes)
        {
            Color color;
            if (node_bp.b_io)
                color = ioNodeColor;
            else
                color = nodeColor;
            DrawCircleIV(node_bp.relativePosition + offset, Node::g_nodeRadius, color);
        }
        break;
    case 3: // All nodes are circles
        for (const NodeBP& node_bp : nodes)
        {
            DrawCircleIV(node_bp.relativePosition + offset, Node::g_nodeRadius, nodeColor);
        }
        break;
    case 4: // Only io is drawn, and they are circles
        for (const NodeBP& node_bp : nodes)
        {
            if (node_bp.b_io)
                DrawCircleIV(node_bp.relativePosition + offset, Node::g_nodeRadius, nodeColor);
        }
        break;
    default: // Nodes are not drawn
        break;
    }
}

IRect Blueprint::GetSelectionPreviewRect(IVec2 pos) const
{
    return IRect(pos, extents + IVec2(g_gridSize * 2));
}

void Blueprint::Save() const
{
    std::ofstream file(TextFormat(".\\blueprints\\%s.bp", name.c_str()));
    file << "n " << nodes.size() << '\n';
    for (const NodeBP& node : nodes)
    {
        file << node.b_io << ' ' << (char)node.gate << ' ' << (unsigned)node.extraParam << ' '
            << node.relativePosition.x << ' ' << node.relativePosition.y;
        if (!node.name.empty())
            file << ' ' << node.name;
        file << '\n';
    }
    file << "w " << wires.size() << '\n';
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

    file.ignore(64, 'n');
    size_t nodeCount;
    file >> nodeCount;
    dest.nodes.reserve(nodeCount);
    for (size_t i = 0; i < nodeCount; ++i)
    {
        bool io;
        char gate;
        unsigned ep;
        IVec2 pos;
        std::string name;
        file >> io >> gate >> ep >> pos.x >> pos.y;
        if (file.peek() == ' ')
        {
            file.ignore(1);
            std::getline(file, name);
            dest.nodes.emplace_back(name, io, (Gate)gate, ep, pos);
        }
        else
            dest.nodes.emplace_back(io, (Gate)gate, ep, pos);

        extents.x = std::max(pos.x, extents.x);
        extents.y = std::max(pos.y, extents.y);
    }
    dest.extents = extents;
    file.ignore(64, 'w');
    size_t wireCount;
    file >> wireCount;
    dest.wires.reserve(wireCount);
    for (size_t i = 0; i < wireCount; ++i)
    {
        size_t startNodeIndex, endNodeIndex;
        unsigned elbowConfig;
        file >> startNodeIndex >> endNodeIndex >> elbowConfig;
        _ASSERT_EXPR(elbowConfig < 4, L"Elbow config out of range");
        dest.wires.emplace_back(startNodeIndex, endNodeIndex, (ElbowConfig)elbowConfig);
    }
    file.close();
}
