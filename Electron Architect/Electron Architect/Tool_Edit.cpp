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

Tool_Edit::Tool_Edit()
{
    // Start of mode
}
Tool_Edit::~Tool_Edit()
{
    // End of mode
}

void Tool_Edit::Update()
{
    if (data.b_cursorMoved)
    {
        if (!data.Edit_NodeBeingDragged() &&
            !data.Edit_WireBeingDragged() &&
            !data.Edit_DraggingGroup())
        {
            data.Edit_HoveredGroup() = nullptr;
            data.hoveredWire = nullptr;
            data.hoveredNode = NodeWorld::Get().FindNodeAtPos(data.cursorPos);
            if (!data.hoveredNode)
            {
                data.hoveredWire = NodeWorld::Get().FindWireElbowAtPos(data.cursorPos);
                if (!data.hoveredWire)
                {
                    data.Edit_HoveredGroup() = NodeWorld::Get().FindGroupAtPos(data.cursorPos);
                }
            }
        }
        else if (data.Edit_NodeBeingDragged())
        {
            data.Edit_HoveringMergable() = NodeWorld::Get().FindNodeAtPos(data.cursorPos); // This will come before updating the position of the dragged node
        }
    }

    // Press
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if (!data.selection.empty() && !!data.hoveredNode) // There is a selection, and a node has been pressed
        {
            data.Edit_NodeBeingDragged() = data.hoveredNode;
            data.Edit_WireBeingDragged() = nullptr;
            data.Edit_SelectionRec() = data.GetSelectionBounds();
        }
        else
        {
            data.selection.clear();
            data.Edit_NodeBeingDragged() = data.hoveredNode;
            data.Edit_WireBeingDragged() = data.hoveredWire;

            // selectionStart being used as an offset here
            if (data.Edit_DraggingGroup() = !!data.Edit_HoveredGroup())
            {
                NodeWorld::Get().FindNodesInGroup(data.selection, data.Edit_HoveredGroup());
                data.Edit_SelectionStart() = (data.cursorPos - (data.Edit_FallbackPos() = data.Edit_HoveredGroup()->GetPosition()));
            }

            data.Edit_FallbackPos() = data.cursorPos;
            if (data.Edit_SelectionWIP() = !(data.Edit_NodeBeingDragged() || data.Edit_WireBeingDragged() || data.Edit_DraggingGroup()))
                data.Edit_SelectionStart() = data.cursorPos;
        }
    }

    // Selection
    if (data.Edit_SelectionWIP())
    {
        auto [minx, maxx] = std::minmax(data.cursorPos.x, data.Edit_SelectionStart().x);
        auto [miny, maxy] = std::minmax(data.cursorPos.y, data.Edit_SelectionStart().y);
        data.Edit_SelectionRec().w = maxx - (data.Edit_SelectionRec().x = minx);
        data.Edit_SelectionRec().h = maxy - (data.Edit_SelectionRec().y = miny);
    }
    // Node
    else if (!!data.Edit_NodeBeingDragged())
    {
        // Multiple selection
        if (!data.selection.empty())
        {
            const IVec2 offset = data.GetCursorDelta();
            for (Node* node : data.selection)
            {
                node->SetPosition_Temporary(node->GetPosition() + offset);
            }
            data.Edit_SelectionRec().position += offset;
        }
        else
            data.Edit_NodeBeingDragged()->SetPosition_Temporary(data.cursorPos);
    }
    // Wire
    else if (!!data.Edit_WireBeingDragged())
    {
        data.Edit_WireBeingDragged()->SnapElbowToLegal(data.cursorPos);
    }
    // Group
    else if (data.Edit_DraggingGroup())
    {
        data.Edit_HoveredGroup()->SetPosition(data.cursorPos - data.Edit_SelectionStart());
        for (Node* node : data.selection)
        {
            const IVec2 offset = data.GetCursorDelta();
            node->SetPosition_Temporary(node->GetPosition() + offset);
        }
    }

    // Release
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) || (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)))
    {
        // Cancel
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        {
            if (!!data.Edit_NodeBeingDragged())
            {
                data.Edit_NodeBeingDragged()->SetPosition(data.Edit_FallbackPos());
            }
            else if (data.Edit_DraggingGroup())
            {
                data.Edit_HoveredGroup()->SetPosition(data.Edit_FallbackPos());
                for (Node* node : data.selection)
                {
                    IVec2 offset = (data.Edit_FallbackPos() + data.Edit_SelectionStart()) - data.cursorPos;
                    node->SetPosition_Temporary(node->GetPosition() + offset);
                }
            }
            else if (data.Edit_SelectionWIP())
            {
                data.Edit_SelectionRec() = IRect(0);
            }
        }
        // Finalize
        else
        {
            if (!!data.Edit_NodeBeingDragged())
            {
                _ASSERT_EXPR(data.Edit_HoveringMergable() != data.Edit_NodeBeingDragged(), L"Node being dragged is trying to merge with itself");
                if (!!data.Edit_HoveringMergable())
                {
                    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
                    {
                        data.hoveredNode = NodeWorld::Get().MergeNodes(data.Edit_HoveringMergable(), data.Edit_NodeBeingDragged());
                        data.Edit_HoveringMergable() = data.Edit_NodeBeingDragged() = nullptr;
                    }
                    else
                    {
                        NodeWorld::Get().SwapNodes(data.Edit_HoveringMergable(), data.Edit_NodeBeingDragged());
                        data.Edit_NodeBeingDragged()->SetPosition(data.Edit_FallbackPos());
                    }
                }
                else
                    data.Edit_NodeBeingDragged()->SetPosition(data.cursorPos);
            }
            else if (data.Edit_DraggingGroup())
            {
                for (Node* node : data.selection)
                {
                    node->SetPosition(node->GetPosition());
                }
            }
            else if (data.Edit_SelectionWIP())
            {
                NodeWorld::Get().FindNodesInRect(data.selection, data.Edit_SelectionRec());
            }
        }
        if (data.Edit_DraggingGroup())
            data.ClearSelection();
        data.Edit_NodeBeingDragged() = nullptr;
        data.Edit_SelectionWIP() = false;
        data.Edit_DraggingGroup() = false;
        data.Edit_WireBeingDragged() = nullptr;
    }
    // Right click
    else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && !!data.hoveredNode)
    {
        data.hoveredNode->SetGate(data.gatePick);
        switch (data.hoveredNode->GetGate())
        {
        case Gate::RESISTOR:  data.hoveredNode->SetResistance(data.storedExtraParam); break;
        case Gate::LED:       data.hoveredNode->SetColorIndex(data.storedExtraParam); break;
        case Gate::CAPACITOR: data.hoveredNode->SetCapacity(data.storedExtraParam);   break;
        }
    }
}
void Tool_Edit::Draw()
{
    if (!!data.Edit_HoveredGroup())
        data.Edit_HoveredGroup()->Highlight(INTERFERENCEGRAY);

    DrawRectangleIRect(data.Edit_SelectionRec(), ColorAlpha(SPACEGRAY, 0.5));
    DrawRectangleLines(data.Edit_SelectionRec().x, data.Edit_SelectionRec().y, data.Edit_SelectionRec().w, data.Edit_SelectionRec().h, LIFELESSNEBULA);

    NodeWorld::Get().DrawWires();

    for (Node* node : data.selection)
    {
        DrawCircleIV(node->GetPosition(), node->g_nodeRadius + 3, WIPBLUE);
    }

    if (!!data.hoveredWire)
    {
        IVec2 pts[4];
        data.hoveredWire->GetLegalElbowPositions(pts);
        for (const IVec2& p : pts)
        {
            Wire::Draw(data.hoveredWire->GetStartPos(), p, data.hoveredWire->GetEndPos(), ColorAlpha(WIPBLUE, 0.25f));
            DrawCircle(p.x, p.y, Wire::g_elbowRadius, ColorAlpha(WIPBLUE, 0.5f));
        }

        data.hoveredWire->Draw(WIPBLUE);
        Color elbowColor;
        if (!!data.Edit_WireBeingDragged())
            elbowColor = WIPBLUE;
        else
            elbowColor = CAUTIONYELLOW;
        data.hoveredWire->DrawElbow(elbowColor);
    }

    NodeWorld::Get().DrawNodes();

    if (!!data.hoveredNode)
    {
        data.hoveredNode->Draw(CAUTIONYELLOW);
    }
    if (!!data.hoveredWire)
    {
        data.hoveredWire->start->Draw(INPUTLAVENDER);
        data.hoveredWire->end->Draw(OUTPUTAPRICOT);
    }

    if (!!data.Edit_NodeBeingDragged() && data.Edit_HoveringMergable())
    {
        DrawCircleIV(data.Edit_NodeBeingDragged()->GetPosition(), Node::g_nodeRadius * 2.0f, VIOLET);
        data.DrawTooltipAtCursor(
            "Hold [shift] to merge on release.\n"
            "Otherwise, nodes will only be swapped.", VIOLET);
    }
}
