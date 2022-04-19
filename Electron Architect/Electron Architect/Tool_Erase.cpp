#include <raylib.h>
#include "HUtility.h"
#include "IVec.h"
#include "Node.h"
#include "Wire.h"
#include "Blueprint.h"
#include "Group.h"
#include "NodeWorld.h"
#include "ProgramData.h"
#include "ToolModes.h"

Tool_Erase::Tool_Erase()
{
    // Start of mode
}
Tool_Erase::~Tool_Erase()
{
    // End of mode
}

void Tool_Erase::Update()
{
    if (data.b_cursorMoved)
    {
        data.hoveredWire = nullptr;
        data.hoveredNode = NodeWorld::Get().FindNodeAtPos(data.cursorPos);
        if (!data.hoveredNode)
            data.hoveredWire = NodeWorld::Get().FindWireAtPos(data.cursorPos);
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if (!!data.hoveredNode)
        {
            // Special erase
            if ((IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) &&
                data.hoveredNode->IsSpecialErasable())
                NodeWorld::Get().BypassNode(data.hoveredNode);
            // Complex bipass
            else if (
                (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) &&
                (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) &&
                data.hoveredNode->IsComplexBipassable())
                NodeWorld::Get().BypassNode_Complex(data.hoveredNode);
            else
                NodeWorld::Get().DestroyNode(data.hoveredNode);
        }
        else if (!!data.hoveredWire)
            NodeWorld::Get().DestroyWire(data.hoveredWire);

        data.hoveredNode = nullptr;
        data.hoveredWire = nullptr;
    }
}
void Tool_Erase::Draw()
{
    auto DrawCross = [](IVec2 center, Color color)
    {
        int radius = 3;
        int expandedRad = radius + 1;
        DrawRectangle(center.x - expandedRad, center.y - expandedRad, expandedRad * 2, expandedRad * 2, BLACK);
        DrawLine(center.x - radius, center.y - radius,
            center.x + radius, center.y + radius,
            color);
        DrawLine(center.x - radius, center.y + radius,
            center.x + radius, center.y - radius,
            color);
    };

    NodeWorld::Get().DrawWires();

    if (!!data.hoveredWire)
    {
        data.hoveredWire->Draw(MAGENTA);
        DrawCross(data.cursorPos, DESTRUCTIVERED);
    }
    else if (!!data.hoveredNode)
    {
        Color color;
        if ((IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)))
        {
            if (data.hoveredNode->IsSpecialErasable())
                color = WIPBLUE;
            else if (data.hoveredNode->IsComplexBipassable() && (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)))
                color = VIOLET;
            else
                color = DESTRUCTIVERED;
        }
        else
            color = MAGENTA;

        for (Wire* wire : data.hoveredNode->GetWires())
        {
            wire->Draw(color);
        }
    }

    NodeWorld::Get().DrawNodes();

    if (!!data.hoveredNode)
    {
        data.hoveredNode->Draw(BLACK);
        DrawCross(data.hoveredNode->GetPosition(), DESTRUCTIVERED);

        if ((IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) && !data.hoveredNode->IsSpecialErasable())
        {
            const char* text;
            Color color;
            if (data.hoveredNode->IsComplexBipassable() && (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)))
            {
                color = VIOLET;
                text = "Complex bipass";
            }
            else
            {
                color = DESTRUCTIVERED;
                size_t iCount = data.hoveredNode->GetInputCount();
                size_t oCount = data.hoveredNode->GetOutputCount();

                if (iCount == 0 && oCount == 0)
                    text =
                    "All wires connected to this node will be destroyed in this action!\n"
                    "Cannot bypass a node with no connections.";

                if (iCount == 0)
                    text =
                    "All wires connected to this node will be destroyed in this action!\n"
                    "Cannot bypass a node with no inputs.";

                else if (oCount == 0)
                    text =
                    "All wires connected to this node will be destroyed in this action!\n"
                    "Cannot bypass a node with no outputs.";

                else if (iCount > 1 && oCount > 1)
                    text =
                    "All wires connected to this node will be destroyed in this action!\n"
                    "Cannot implicitly bypass a node with complex throughput (multiple inputs AND outputs).\n"
                    "Hold [alt] while shift-clicking this node to allow complex bipass.\n"
                    "! Otherwise the node will be deleted as normal !";

                else
                    text = TextFormat(
                        "All wires connected to this node will be destroyed in this action!\n"
                        "Cannot bypass this node -- the reason is unknown.\n"
                        "\"Special erase error: i=%i, o=%i\"", iCount, oCount);

            }
            data.DrawTooltipAtCursor(text, color);
        }
    }
}
