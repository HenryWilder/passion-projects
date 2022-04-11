#include <thread>
#include "Blueprint.h"

// Position on screen
IVec2 IconPos::Pos() const
{
    return IVec2(x * g_unit, y * g_unit);
}

void IconPos::Draw(IVec2 start, Color tint) const
{
    if (id != NULL)
        BlueprintIcon::DrawBPIcon(id, start + Pos(), tint);
}

IVec2 BlueprintIcon::ColRowFromIcon(IconID_t icon)
{
    return IVec2((int)icon % g_iconSheetDimensions.x, (int)icon / g_iconSheetDimensions.y);
}


BlueprintIcon::IconID_t BlueprintIcon::GetIconAtColRow(IVec2 colRow)
{
    if (colRow.x < 0 || colRow.x >= g_iconSheetDimensions.x ||
        colRow.y < 0 || colRow.y >= g_iconSheetDimensions.y)
        return NULL;
    return (IconID_t)((colRow.y * g_iconSheetDimensions.x) + colRow.x);
}
IVec2 BlueprintIcon::PixelToColRow(IVec2 sheetPos, IVec2 selectPos)
{
    return IVec2Divide_i(selectPos - sheetPos, g_size);
}
IVec2 BlueprintIcon::GetSheetSize_RC() // Rows and columns
{
    return g_iconSheetDimensions;
}
IVec2 BlueprintIcon::GetSheetSize_Px() // Pixels
{
    return IVec2Scale_i(g_iconSheetDimensions, g_size);
}
void BlueprintIcon::DrawBPIcon(IconID_t icon, IVec2 pos, Color tint)
{
    if (icon != NULL)
        DrawIcon<g_size>(g_iconSheet, ColRowFromIcon(icon), pos, tint);
}

void BlueprintIcon::DrawSheet(IVec2 pos, Color background, Color tint)
{
    DrawRectangle(pos.x, pos.y, g_iconSheet.width, g_iconSheet.height, background);
    DrawTexture(g_iconSheet, pos.x, pos.y, tint);
}


// Global
void BlueprintIcon::Load(const char* filename)
{
    g_iconSheet = LoadTexture(filename);
    g_iconSheetDimensions.x = g_iconSheet.width / g_size;
    g_iconSheetDimensions.y = g_iconSheet.height / g_size;
}
void BlueprintIcon::Unload()
{
    UnloadTexture(g_iconSheet);
}

// Instance
BlueprintIcon::BlueprintIcon() = default;
BlueprintIcon::BlueprintIcon(const std::vector<IconPos>&icons)
{
    _ASSERT_EXPR(icons.size() <= 4, "Icon vector too large");
    size_t i = 0;
    for (; i < icons.size(); ++i)
    {
        combo[i] = icons[i];
    }
    for (; i < 4; ++i)
    {
        combo[i] = { NULL, 0,0 };
    }

}
BlueprintIcon::BlueprintIcon(IconPos(&icons)[4])
{
    memcpy(combo, icons, sizeof(IconPos) * 4);
}

void BlueprintIcon::DrawBackground(IVec2 pos, Color color) const
{
    constexpr int width = g_size * 2;
    DrawRectangle(pos.x, pos.y, width, width, color);
}
void BlueprintIcon::Draw(IVec2 pos, Color tint) const
{
    // Draw
    for (const IconPos& icon : combo)
    {
        icon.Draw(pos, tint);
    }
}

Texture2D BlueprintIcon::g_iconSheet;
IVec2 BlueprintIcon::g_iconSheetDimensions = IVec2Zero();


void Blueprint::PopulateNodes(const std::vector<Node*>& src)
{
    constexpr IRect boundsInit = IRect(
        std::numeric_limits<int>::max(),
        std::numeric_limits<int>::max(),
        std::numeric_limits<int>::min(),
        std::numeric_limits<int>::min());
    IRect bounds = boundsInit;
    for (Node* node : src)
    {
        const IVec2& compare = node->GetPosition();
        if (compare.x < bounds.x)
            bounds.x = compare.x;
        if (compare.y < bounds.y)
            bounds.y = compare.y;

        // Abusing width and height as max x/y
        if (compare.x > bounds.w)
            bounds.w = compare.x;
        if (compare.y > bounds.h)
            bounds.h = compare.y;
    }
    // Disabuse
    bounds.w -= bounds.x;
    bounds.h -= bounds.y;
    extents = IVec2(bounds.w, bounds.h);

    IVec2 min = IVec2(bounds.x, bounds.y);
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

        nodes.emplace_back(
            isIO,
            node->GetGate(),
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
    extents = IVec2Zero();
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
