#include <raylib.h>
#include <fstream>
#include <filesystem>
#include <functional>
#include "HUtility.h"
#include "IVec.h"
#include "Node.h"
#include "Wire.h"
#include "Blueprint.h"
#include "Group.h"
#include "Tab.h"
#include "Buttons.h"
#include "UIColors.h"
#include "Mode.h"

PenTool::PenTool() {}
PenTool::~PenTool() {}
ModeType PenTool::GetModeType() const { return ModeType::Basic; }
Mode PenTool::GetMode() const { return Mode::PEN; }
void PenTool::Update(Window& data)
{
    if (data.b_cursorMoved) // On move
    {
        data.hoveredWire = nullptr;
        data.hoveredNode = Tab::Get().FindNodeAtPos(data.cursorPos);
        if (!data.hoveredNode)
            data.hoveredWire = Tab::Get().FindWireAtPos(data.cursorPos);
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        Node* newNode = data.hoveredNode;
        if (!newNode)
        {
            newNode = Tab::Get().CreateNode(data.cursorPos, data.gatePick, data.storedExtraParam);
            if (!!data.hoveredWire)
            {
                Tab::Get().BisectWire(data.hoveredWire, newNode);
                data.hoveredWire = nullptr;
            }
        }
        // Do not create a new node/wire if already hovering the start node
        if (!!data.Pen_CurrentWireStart() && newNode != data.Pen_CurrentWireStart())
        {
            Node* oldNode;
            if (!!data.Pen_PreviousWireStart() && (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)))
                oldNode = data.Pen_PreviousWireStart();
            else
                oldNode = data.Pen_CurrentWireStart();

            if (oldNode != newNode)
            {
                Wire* wire = Tab::Get().CreateWire(oldNode, newNode);
                wire->elbowConfig = data.Pen_CurrentWireElbowConfig();
                wire->UpdateElbowToLegal();
                data.Pen_PreviousWireStart() = oldNode;
            }
        }
        data.Pen_CurrentWireStart() = newNode;
    }
    else if (IsKeyPressed(KEY_R))
    {
        if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
            --data.Pen_CurrentWireElbowConfig();
        else
            ++data.Pen_CurrentWireElbowConfig();
    }
    else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    {
        data.Pen_PreviousWireStart() = data.Pen_CurrentWireStart() = nullptr;
    }
}
void PenTool::Draw(Window& data)
{
    Tab::Get().DrawWires(UIColor(UIColorID::UI_COLOR_ACTIVE), UIColor(UIColorID::UI_COLOR_FOREGROUND3));

    if (!!data.Pen_CurrentWireStart())
    {
        IVec2 start;
        if (!!data.Pen_PreviousWireStart() && (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)))
            start = data.Pen_PreviousWireStart()->GetPosition();
        else
            start = data.Pen_CurrentWireStart()->GetPosition();
        IVec2 elbow;
        IVec2 end = data.cursorPos;
        elbow = Wire::GetLegalElbowPosition(start, end, data.Pen_CurrentWireElbowConfig());
        Wire::Draw(start, elbow, end, UIColor(UIColorID::UI_COLOR_AVAILABLE));
        Node::Draw(end, data.gatePick, UIColor(UIColorID::UI_COLOR_AVAILABLE));
    }

    if (!!data.hoveredWire)
    {
        data.hoveredWire->Draw(UIColor(UIColorID::UI_COLOR_AVAILABLE));
    }
    else if (!!data.hoveredNode)
    {
        // Todo: Can this please be based on the input and output ranges and not a test being run on an already partitioned vector?...
        for (const Wire* wire : data.hoveredNode->GetWires())
        {
            Color color;
            if (wire->start == data.hoveredNode)
                color = UIColor(UIColorID::UI_COLOR_OUTPUT); // Output
            else
                color = UIColor(UIColorID::UI_COLOR_INPUT); // Input

            wire->Draw(color);
        }
    }

    Tab::Get().DrawNodes(UIColor(UIColorID::UI_COLOR_ACTIVE), UIColor(UIColorID::UI_COLOR_FOREGROUND));

    if (!!data.hoveredWire)
    {
        data.hoveredWire->start->Draw(UIColor(UIColorID::UI_COLOR_INPUT));
        data.hoveredWire->end->Draw(UIColor(UIColorID::UI_COLOR_OUTPUT));
    }
    else if (!!data.hoveredNode)
    {
        data.hoveredNode->Draw(UIColor(UIColorID::UI_COLOR_AVAILABLE));
    }

    EndMode2D();

    // Stats
    {
        int i = 0;
        // Cursor stats
        DrawText(TextFormat("y: %i", data.cursorPos.y / g_gridSize), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND));
        DrawText(TextFormat("x: %i", data.cursorPos.x / g_gridSize), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND));

        // Node hover stats
        if (!!data.hoveredNode)
        {
            const char* gateName;
            switch (data.hoveredNode->GetGate())
            {
            case Gate::OR:        gateName = "or";        break;
            case Gate::AND:       gateName = "and";       break;
            case Gate::NOR:       gateName = "nor";       break;
            case Gate::XOR:       gateName = "xor";       break;
            case Gate::RESISTOR:  gateName = "resistor";  break;
            case Gate::CAPACITOR: gateName = "capacitor"; break;
            case Gate::LED:       gateName = "LED";       break;
            case Gate::DELAY:     gateName = "delay";     break;
            case Gate::BATTERY:   gateName = "battery";   break;
            default:
                _ASSERT_EXPR(false, L"Missing specialization for gate name");
                gateName = "ERROR";
                break;
            }
            // State
            {
                const char* stateName;
                if (data.hoveredNode->GetState())
                    stateName = "\tactive";
                else
                    stateName = "\tinactive";
                DrawText(stateName, 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND2));
            }
            DrawText(TextFormat("\toutputs: %i", data.hoveredNode->GetOutputCount()), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_OUTPUT));
            DrawText(TextFormat("\tinputs: %i", data.hoveredNode->GetInputCount()), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_INPUT));
            DrawText(TextFormat("\ttype: %s", gateName), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND2));
            DrawText(TextFormat("\tserial: %u", Tab::Get().NodeID(data.hoveredNode)), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND2));
            DrawText(TextFormat("\tptr: %p", data.hoveredNode), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND2));
            DrawText("hovered node", 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND));
        }
        // Wire hover stats
        else if (!!data.hoveredWire)
        {
            DrawText(TextFormat("\t\tptr: %p", data.hoveredWire->end), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND2));
            DrawText(TextFormat("\t\tserial: %u", Tab::Get().NodeID(data.hoveredWire->end)), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND2));
            DrawText("\toutput", 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_OUTPUT));
            DrawText(TextFormat("\t\tptr: %p", data.hoveredWire->start), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND2));
            DrawText(TextFormat("\t\tserial: %u", Tab::Get().NodeID(data.hoveredWire->start)), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND2));
            DrawText("\tinput", 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_INPUT));
            // State
            {
                const char* stateName;
                if (data.hoveredWire->GetState())
                    stateName = "\tactive";
                else
                    stateName = "\tinactive";
                DrawText(stateName, 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND2));
            }
            DrawText(TextFormat("\tptr: %p", data.hoveredWire), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND2));
            DrawText("hovered wire", 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND));
        }
    }
}

EditTool::EditTool() {}
EditTool::~EditTool() {}
ModeType EditTool::GetModeType() const { return ModeType::Basic; }
Mode EditTool::GetMode() const { return Mode::EDIT; }
void EditTool::Update(Window& data)
{
    // Todo: fix bug with canceling multiple-drag (And update group dragging to match!!)

    if (data.b_cursorMoved && !data.Edit_SelectionWIP())
    {
        if (!data.Edit_NodeBeingDragged() &&
            !data.Edit_WireBeingDragged() &&
            !data.Edit_DraggingGroup() &&
            !data.Edit_DraggingGroupCorner())
        {
            data.Edit_GroupCorner().group = nullptr;
            data.hoveredGroup = nullptr;
            data.hoveredWire = nullptr;
            data.hoveredNode = Tab::Get().FindNodeAtPos(data.cursorPos);
            if (!data.hoveredNode)
            {
                data.hoveredWire = Tab::Get().FindWireElbowAtPos(data.cursorPos);
                if (!data.hoveredWire)
                {
                    data.hoveredGroup = Tab::Get().FindGroupAtPos(data.cursorPos);
                    if (!data.hoveredGroup)
                    {
                        data.Edit_GroupCorner() = Tab::Get().FindGroupCornerAtPos(data.cursorPos);
                    }
                }
            }
        }
        else if (data.Edit_NodeBeingDragged() && !data.SelectionExists())
        {
            data.Edit_HoveringMergable() = Tab::Get().FindNodeAtPos(data.cursorPos); // This will come before updating the position of the dragged node
        }
    }

    // Press
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if (!data.selection.empty() && !!data.hoveredNode) // There is a selection, and a node has been pressed (move selected)
        {
            data.Edit_NodeBeingDragged() = data.hoveredNode;
            data.Edit_WireBeingDragged() = nullptr;
            data.Edit_SelectionRec() = data.GetSelectionBounds();
        }
        else if (data.IsSelectionRectValid() && (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)))
        {
            data.selection.clear();
            data.Edit_SelectionWIP() = true;
        }
        else
        {
            data.selection.clear();
            data.Edit_NodeBeingDragged() = data.hoveredNode;
            data.Edit_WireBeingDragged() = data.hoveredWire;

            // selectionStart being used as an offset here
            if (data.Edit_DraggingGroup() = !!data.hoveredGroup)
            {
                Tab::Get().FindNodesInGroup(data.selection, data.hoveredGroup);
                data.Edit_SelectionStart() = (data.cursorPos - (data.Edit_FallbackPos() = data.hoveredGroup->GetPosition()));
            }
            else if (data.Edit_DraggingGroupCorner() = data.Edit_GroupCorner().Valid())
            {
                // Todo
            }

            data.Edit_FallbackPos() = data.cursorPos;
            if (data.Edit_SelectionWIP() = !(data.Edit_NodeBeingDragged() || data.Edit_WireBeingDragged() || data.Edit_DraggingGroup() || data.Edit_DraggingGroupCorner()))
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
        data.hoveredGroup->SetPosition(data.cursorPos - data.Edit_SelectionStart());
        for (Node* node : data.selection)
        {
            const IVec2 offset = data.GetCursorDelta();
            node->SetPosition_Temporary(node->GetPosition() + offset);
        }
    }
    // Resize group
    else if (data.Edit_DraggingGroupCorner())
    {
        _ASSERT_EXPR(data.Edit_GroupCorner().cornerIndex < 4, L"Index out of range");
        constexpr int minWidth = g_gridSize * 2;
        IRect captureBounds = data.Edit_GroupCorner().group->GetCaptureBounds();
        IVec2 cursorEnd;
        IVec2 otherEnd;
        switch (data.Edit_GroupCorner().cornerIndex)
        {
        case 0:
            cursorEnd.x = std::min(data.cursorPos.x, captureBounds.Right() - minWidth);
            cursorEnd.y = std::min(data.cursorPos.y, captureBounds.Bottom() - minWidth);
            otherEnd = captureBounds.BR();
            break;
        case 1:
            cursorEnd.x = std::max(data.cursorPos.x, captureBounds.x + minWidth);
            cursorEnd.y = std::min(data.cursorPos.y, captureBounds.Bottom() - minWidth);
            otherEnd = captureBounds.BL();
            break;
        case 2:
            cursorEnd.x = std::min(data.cursorPos.x, captureBounds.Right() - minWidth);
            cursorEnd.y = std::max(data.cursorPos.y, captureBounds.y + minWidth);
            otherEnd = captureBounds.TR();
            break;
        case 3:
            cursorEnd.x = std::max(data.cursorPos.x, captureBounds.x + minWidth);
            cursorEnd.y = std::max(data.cursorPos.y, captureBounds.y + minWidth);
            otherEnd = captureBounds.TL();
            break;
        }
        captureBounds = IRectFromTwoPoints(cursorEnd, otherEnd);
        data.Edit_GroupCorner().group->SetCaptureBounds(captureBounds);
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
                data.hoveredGroup->SetPosition(data.Edit_FallbackPos());
                for (Node* node : data.selection)
                {
                    IVec2 offset = (data.Edit_FallbackPos() + data.Edit_SelectionStart()) - data.cursorPos;
                    node->SetPosition_Temporary(node->GetPosition() + offset);
                }
            }
            else if (data.Edit_DraggingGroupCorner())
            {
                // Todo
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
                        data.hoveredNode = Tab::Get().MergeNodes(data.Edit_HoveringMergable(), data.Edit_NodeBeingDragged());
                        data.Edit_HoveringMergable() = data.Edit_NodeBeingDragged() = nullptr;
                    }
                    else
                    {
                        Tab::Get().SwapNodes(data.Edit_HoveringMergable(), data.Edit_NodeBeingDragged());
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
            else if (data.Edit_DraggingGroupCorner())
            {
                // Todo
            }
            else if (data.Edit_SelectionWIP())
            {
                data.Edit_SelectionWIP() = false;
                if (data.IsSelectionRectValid())
                    Tab::Get().FindNodesInRect(data.selection, data.Edit_SelectionRec());
                else
                    data.Edit_SelectionRec() = IRect(0);
            }
        }
        if (data.Edit_DraggingGroup())
            data.ClearSelection();
        data.Edit_NodeBeingDragged() = nullptr;
        data.Edit_SelectionWIP() = false;
        data.Edit_DraggingGroup() = false;
        data.Edit_DraggingGroupCorner() = false;
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
void EditTool::Draw(Window& data)
{
    if (!!data.hoveredGroup)
    {
        data.hoveredGroup->Highlight(UIColor(UIColorID::UI_COLOR_FOREGROUND1));
    }
    else if (data.Edit_GroupCorner().Valid())
    {
        Color color;
        if (data.Edit_DraggingGroupCorner())
            color = UIColor(UIColorID::UI_COLOR_FOREGROUND1);
        else
            color = data.Edit_GroupCorner().group->GetColor();
        DrawRectangleIRect(data.Edit_GroupCorner().GetCollisionRect(), color);
    }

    DrawRectangleIRect(data.Edit_SelectionRec(), ColorAlpha(UIColor(UIColorID::UI_COLOR_BACKGROUND1), 0.5));
    DrawRectangleLines(data.Edit_SelectionRec().x, data.Edit_SelectionRec().y, data.Edit_SelectionRec().w, data.Edit_SelectionRec().h, UIColor(UIColorID::UI_COLOR_BACKGROUND2));

    Tab::Get().DrawWires(UIColor(UIColorID::UI_COLOR_ACTIVE), UIColor(UIColorID::UI_COLOR_FOREGROUND3));

    for (Node* node : data.selection)
    {
        DrawCircleIV(node->GetPosition(), node->g_nodeRadius + 3, UIColor(UIColorID::UI_COLOR_AVAILABLE));
    }

    if (!!data.hoveredWire)
    {
        constexpr ElbowConfig configOrder[] =
        {
            ElbowConfig::horizontal,
            ElbowConfig::diagonalA,
            ElbowConfig::vertical,
            ElbowConfig::diagonalB,
        };
        for (int i = 0; i < _countof(configOrder); ++i)
        {
            IVec2 p = data.hoveredWire->GetLegalElbowPosition(configOrder[i]);
            Wire::Draw(data.hoveredWire->GetStartPos(), p, data.hoveredWire->GetEndPos(), ColorAlpha(UIColor(UIColorID::UI_COLOR_AVAILABLE), 0.25f));
            DrawCircle(p.x, p.y, Wire::g_elbowRadius, ColorAlpha(UIColor(UIColorID::UI_COLOR_AVAILABLE), 0.5f));
        }

        data.hoveredWire->Draw(UIColor(UIColorID::UI_COLOR_AVAILABLE));
        Color elbowColor;
        if (!!data.Edit_WireBeingDragged())
            elbowColor = UIColor(UIColorID::UI_COLOR_AVAILABLE);
        else
            elbowColor = UIColor(UIColorID::UI_COLOR_CAUTION);
        data.hoveredWire->DrawElbow(elbowColor);
    }

    Tab::Get().DrawNodes(UIColor(UIColorID::UI_COLOR_ACTIVE), UIColor(UIColorID::UI_COLOR_FOREGROUND));

    if (!!data.hoveredNode)
    {
        data.hoveredNode->Draw(UIColor(UIColorID::UI_COLOR_CAUTION));
    }
    if (!!data.hoveredWire)
    {
        data.hoveredWire->start->Draw(UIColor(UIColorID::UI_COLOR_INPUT));
        data.hoveredWire->end->Draw(UIColor(UIColorID::UI_COLOR_OUTPUT));
    }

    if (!!data.Edit_NodeBeingDragged() && data.Edit_HoveringMergable())
    {
        DrawCircleIV(data.Edit_NodeBeingDragged()->GetPosition(), Node::g_nodeRadius * 2.0f, UIColor(UIColorID::UI_COLOR_SPECIAL));
        data.DrawTooltipAtCursor(
            "Hold [shift] to merge on release.\n"
            "Otherwise, nodes will only be swapped.", UIColor(UIColorID::UI_COLOR_SPECIAL));
    }

    EndMode2D();

    // Stats
    {
        int i = 0;
        // Cursor stats
        DrawText(TextFormat("y: %i", data.cursorPos.y / g_gridSize), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND));
        DrawText(TextFormat("x: %i", data.cursorPos.x / g_gridSize), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND));

        // Selection stats
        if (data.SelectionExists())
        {
            unsigned ORs = 0;
            unsigned ANDs = 0;
            unsigned NORs = 0;
            unsigned XORs = 0;
            unsigned RESs = 0;
            unsigned CAPs = 0;
            unsigned LEDs = 0;
            unsigned DELs = 0;
            unsigned BATs = 0;

            for (Node* node : data.selection)
            {
                switch (node->GetGate())
                {
                case Gate::OR:        ++ORs;  break;
                case Gate::AND:       ++ANDs; break;
                case Gate::NOR:       ++NORs; break;
                case Gate::XOR:       ++XORs; break;
                case Gate::RESISTOR:  ++RESs; break;
                case Gate::CAPACITOR: ++CAPs; break;
                case Gate::LED:       ++LEDs; break;
                case Gate::DELAY:     ++DELs; break;
                case Gate::BATTERY:   ++BATs; break;
                }
            }

            if (DELs) DrawText(TextFormat("\t\tbattery: %i", BATs), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND2));
            if (DELs) DrawText(TextFormat("\t\tdelay: %i", DELs), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND2));
            if (LEDs) DrawText(TextFormat("\t\tLED: %i", LEDs), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND2));
            if (CAPs) DrawText(TextFormat("\t\tcapacitor: %i", CAPs), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND2));
            if (RESs) DrawText(TextFormat("\t\tresistor: %i", RESs), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND2));
            if (XORs) DrawText(TextFormat("\t\txor: %i", XORs), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND2));
            if (NORs) DrawText(TextFormat("\t\tnor: %i", NORs), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND2));
            if (ANDs) DrawText(TextFormat("\t\tand: %i", ANDs), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND2));
            if (ORs)  DrawText(TextFormat("\t\tor: %i", ORs), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND2));
            DrawText(TextFormat("\ttotal: %i", data.selection.size()), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND1));
            DrawText("selection", 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND));
        }


        // Node hover stats
        if (!!data.hoveredNode)
        {
            const char* gateName;
            switch (data.hoveredNode->GetGate())
            {
            case Gate::OR:        gateName = "or";        break;
            case Gate::AND:       gateName = "and";       break;
            case Gate::NOR:       gateName = "nor";       break;
            case Gate::XOR:       gateName = "xor";       break;
            case Gate::RESISTOR:  gateName = "resistor";  break;
            case Gate::CAPACITOR: gateName = "capacitor"; break;
            case Gate::LED:       gateName = "LED";       break;
            case Gate::DELAY:     gateName = "delay";     break;
            case Gate::BATTERY:   gateName = "battery";   break;
            default:
                _ASSERT_EXPR(false, L"Missing specialization for gate name");
                gateName = "ERROR";
                break;
            }
            // State
            {
                const char* stateName;
                if (data.hoveredNode->GetState())
                    stateName = "\tactive";
                else
                    stateName = "\tinactive";
                DrawText(stateName, 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND2));
            }
            DrawText(TextFormat("\toutputs: %i", data.hoveredNode->GetOutputCount()), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_OUTPUT));
            DrawText(TextFormat("\tinputs: %i", data.hoveredNode->GetInputCount()), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_INPUT));
            DrawText(TextFormat("\ttype: %s", gateName), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND2));
            DrawText(TextFormat("\tserial: %u", Tab::Get().NodeID(data.hoveredNode)), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND2));
            DrawText(TextFormat("\tptr: %p", data.hoveredNode), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND2));
            DrawText("hovered node", 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND));
        }
        // Joint hover stats
        else if (!!data.hoveredWire)
        {
            const char* configurationName;
            switch (data.hoveredWire->elbowConfig)
            {
            case ElbowConfig::horizontal: configurationName = "horizontal"; break;
            case ElbowConfig::diagonalA: configurationName = "diagonal from input"; break;
            case ElbowConfig::vertical: configurationName = "vertical"; break;
            case ElbowConfig::diagonalB: configurationName = "diagonal from output"; break;
            default: configurationName = "ERROR"; break;
            }
            DrawText(TextFormat("\tconfiguration: %s", configurationName), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND2));
            DrawText(TextFormat("\twire ptr: %p", data.hoveredWire), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND2));
            DrawText("hovered wire-joint", 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND));
        }
        // Group hover stats
        else if (!!data.hoveredGroup)
        {
            DrawText(TextFormat("\tlabel: %s", data.hoveredGroup->GetLabel().c_str()), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND2));
            DrawText(TextFormat("\tptr: %p", data.hoveredGroup), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND2));
            DrawText("hovered group", 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND));
        }
    }
}

EraseTool::EraseTool() {}
EraseTool::~EraseTool() {}
ModeType EraseTool::GetModeType() const { return ModeType::Basic; }
Mode EraseTool::GetMode() const { return Mode::ERASE; }
void EraseTool::Update(Window& data)
{
    if (data.b_cursorMoved)
    {
        data.hoveredWire = nullptr;
        data.hoveredNode = Tab::Get().FindNodeAtPos(data.cursorPos);
        if (!data.hoveredNode)
        {
            data.hoveredWire = Tab::Get().FindWireAtPos(data.cursorPos);
            if (!data.hoveredWire)
                data.hoveredGroup = Tab::Get().FindGroupAtPos(data.cursorPos);
        }
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if (!!data.hoveredNode)
        {
            // Special erase
            if ((IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) &&
                data.hoveredNode->IsSpecialErasable())
                Tab::Get().BypassNode(data.hoveredNode);
            // Complex bipass
            else if (
                (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) &&
                (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) &&
                data.hoveredNode->IsComplexBipassable())
                Tab::Get().BypassNode_Complex(data.hoveredNode);
            else
                Tab::Get().DestroyNode(data.hoveredNode);
        }
        else if (!!data.hoveredWire)
            Tab::Get().DestroyWire(data.hoveredWire);
        else if (!!data.hoveredGroup)
            Tab::Get().DestroyGroup(data.hoveredGroup);

        data.hoveredNode = nullptr;
        data.hoveredWire = nullptr;
        data.hoveredGroup = nullptr;
    }
}
void EraseTool::Draw(Window& data)
{
    auto DrawCross = [](IVec2 center, Color color)
    {
        int radius = 3;
        int expandedRad = radius + 1;
        DrawRectangle(center.x - expandedRad, center.y - expandedRad, expandedRad * 2, expandedRad * 2, UIColor(UIColorID::UI_COLOR_BACKGROUND));
        DrawLine(center.x - radius, center.y - radius,
            center.x + radius, center.y + radius,
            color);
        DrawLine(center.x - radius, center.y + radius,
            center.x + radius, center.y - radius,
            color);
    };

    if (!!data.hoveredGroup)
    {
        IRect rec = data.hoveredGroup->GetLabelBounds();
        data.hoveredGroup->Highlight(UIColor(UIColorID::UI_COLOR_ERROR));
        DrawLineEx({ (float)rec.x, (float)rec.y }, { (float)rec.x + (float)rec.h, (float)rec.Bottom() }, 3, UIColor(UIColorID::UI_COLOR_DESTRUCTIVE));
        DrawLineEx({ (float)rec.x, (float)rec.Bottom() }, { (float)rec.x + (float)rec.h, (float)rec.y }, 3, UIColor(UIColorID::UI_COLOR_DESTRUCTIVE));
    }

    Tab::Get().DrawWires(UIColor(UIColorID::UI_COLOR_ACTIVE), UIColor(UIColorID::UI_COLOR_FOREGROUND3));

    if (!!data.hoveredWire)
    {
        data.hoveredWire->Draw(UIColor(UIColorID::UI_COLOR_ERROR));
        DrawCross(data.cursorPos, UIColor(UIColorID::UI_COLOR_DESTRUCTIVE));
    }
    else if (!!data.hoveredNode)
    {
        Color color;
        if ((IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)))
        {
            if (data.hoveredNode->IsSpecialErasable())
                color = UIColor(UIColorID::UI_COLOR_AVAILABLE);
            else if (data.hoveredNode->IsComplexBipassable() && (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)))
                color = UIColor(UIColorID::UI_COLOR_SPECIAL);
            else
                color = UIColor(UIColorID::UI_COLOR_DESTRUCTIVE);
        }
        else
            color = UIColor(UIColorID::UI_COLOR_ERROR);

        for (Wire* wire : data.hoveredNode->GetWires())
        {
            wire->Draw(color);
        }
    }

    Tab::Get().DrawNodes(UIColor(UIColorID::UI_COLOR_ACTIVE), UIColor(UIColorID::UI_COLOR_FOREGROUND));

    if (!!data.hoveredNode)
    {
        data.hoveredNode->Draw(UIColor(UIColorID::UI_COLOR_BACKGROUND));
        DrawCross(data.hoveredNode->GetPosition(), UIColor(UIColorID::UI_COLOR_DESTRUCTIVE));

        if ((IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)))
        {
            const char* text;
            Color color;
            if (data.hoveredNode->IsSpecialErasable())
            {
                color = UIColor(UIColorID::UI_COLOR_AVAILABLE);
                text = "Simple bipass";
            }
            else if (data.hoveredNode->IsComplexBipassable() && (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)))
            {
                color = UIColor(UIColorID::UI_COLOR_SPECIAL);
                text = "Complex bipass";
            }
            else
            {
                color = UIColor(UIColorID::UI_COLOR_DESTRUCTIVE);
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
            data.DrawTooltipAtCursor_Shadowed(text, color);
        }
    }
}

InteractTool::InteractTool() {}
InteractTool::~InteractTool() {}
ModeType InteractTool::GetModeType() const { return ModeType::Basic; }
Mode InteractTool::GetMode() const { return Mode::INTERACT; }
void InteractTool::Update(Window& data)
{
    data.hoveredNode = Tab::Get().FindNodeAtPos(data.cursorPos);
    if (!!data.hoveredNode && !data.hoveredNode->IsOutputOnly())
        data.hoveredNode = nullptr;

    if (!!data.hoveredNode && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        data.hoveredNode->SetGate(data.hoveredNode->GetGate() == Gate::NOR ? Gate::OR : Gate::NOR);
}
void InteractTool::Draw(Window& data)
{
    Tab::Get().DrawWires(UIColor(UIColorID::UI_COLOR_ACTIVE), UIColor(UIColorID::UI_COLOR_FOREGROUND3));
    Tab::Get().DrawNodes(UIColor(UIColorID::UI_COLOR_ACTIVE), UIColor(UIColorID::UI_COLOR_FOREGROUND));

    for (const Node* node : Tab::Get().GetStartNodes())
    {
        node->Draw(UIColor(UIColorID::UI_COLOR_AVAILABLE));
    }

    if (!!data.hoveredNode)
    {
        data.hoveredNode->Draw(UIColor(UIColorID::UI_COLOR_CAUTION));
    }

    EndMode2D();

    // Stats
    {
        int i = 0;
        // Cursor stats
        DrawText(TextFormat("y: %i", data.cursorPos.y / g_gridSize), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND));
        DrawText(TextFormat("x: %i", data.cursorPos.x / g_gridSize), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND));

        // Node hover stats
        if (!!data.hoveredNode)
        {
            // State
            {
                const char* stateName;
                if (data.hoveredNode->GetState())
                    stateName = "\tactive";
                else
                    stateName = "\tinactive";
                DrawText(stateName, 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND2));
            }
            DrawText(TextFormat("\toutputs: %i", data.hoveredNode->GetOutputCount()), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_OUTPUT));
            DrawText(TextFormat("\tinteraction serial: %u", Tab::Get().StartNodeID(data.hoveredNode)), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND2));
            DrawText(TextFormat("\tserial: %u", Tab::Get().NodeID(data.hoveredNode)), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND2));
            DrawText(TextFormat("\tptr: %p", data.hoveredNode), 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND2));
            DrawText("hovered interactable node", 2, data.windowHeight - (++i * 12), 8, UIColor(UIColorID::UI_COLOR_FOREGROUND));
        }
    }
}

ButtonOverlay::ButtonOverlay() {}
ButtonOverlay::~ButtonOverlay() {}
ModeType ButtonOverlay::GetModeType() const { return ModeType::Overlay; }
Mode ButtonOverlay::GetMode() const { return Mode::BUTTON; }
void ButtonOverlay::Update(Window& data)
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        IRect rec = Window::dropdownBounds[data.Button_DropdownActive()];
        if (data.CursorInUIBounds(rec))
        {
            rec.h = data.ButtonWidth();

            switch (data.Button_DropdownActive())
            {
            case 0: // Mode
            {
                for (Mode m : Window::sidebarModeOrder)
                {
                    if (m == data.baseMode)
                        continue;

                    if (data.CursorInUIBounds(rec))
                    {
                        data.SetMode(m);
                        break;
                    }

                    rec.y += data.ButtonWidth();
                }
            }
            break;

            case 1: // Gate
            {
                for (Gate g : Window::sidebarGateOrder)
                {
                    if (g == data.gatePick)
                        continue;

                    if (data.CursorInUIBounds(rec))
                    {
                        data.SetGate(g);
                        break;
                    }

                    rec.y += data.ButtonWidth();
                }

                data.ClearOverlayMode();
            }
            break;

            case 2: // Resistance
            {
                for (uint8_t v = 0; v < _countof(Node::g_resistanceBands); ++v)
                {
                    if (v == data.storedExtraParam)
                        continue;

                    if (data.CursorInUIBounds(rec))
                    {
                        data.storedExtraParam = v;
                        break;
                    }

                    rec.y += data.ButtonWidth();
                }

                data.ClearOverlayMode();
            }
            break;
            }
        }
        else
            data.ClearOverlayMode();
    }
    else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    {
        data.ClearOverlayMode();
    }
}
void ButtonOverlay::Draw(Window& data)
{
    Tab::Get().DrawWires(UIColor(UIColorID::UI_COLOR_ACTIVE), UIColor(UIColorID::UI_COLOR_FOREGROUND3));
    Tab::Get().DrawNodes(UIColor(UIColorID::UI_COLOR_ACTIVE), UIColor(UIColorID::UI_COLOR_FOREGROUND));

    EndMode2D();

    IRect rec = data.dropdownBounds[data.Button_DropdownActive()];
    DrawRectangleIRect(rec, UIColor(UIColorID::UI_COLOR_BACKGROUND1));
    rec.h = data.ButtonWidth();

    switch (data.Button_DropdownActive())
    {
    case 0: // Mode
    {
        for (Mode m : Window::sidebarModeOrder)
        {
            if (m == data.baseMode)
                continue;
            Color color;
            if (InBoundingBox(rec, data.cursorUIPos))
            {
                color = UIColor(UIColorID::UI_COLOR_FOREGROUND);
                DrawText(Window::GetModeTooltipName(m), data.ButtonWidth() + (data.FontSize() / 2), data.ButtonWidth() + (data.FontSize() / 8) + rec.y, data.FontSize(), UIColor(UIColorID::UI_COLOR_FOREGROUND));
            }
            else
                color = UIColor(UIColorID::UI_COLOR_FOREGROUND3);
            data.DrawModeIcon(m, rec.xy, color);
            rec.y += data.ButtonWidth();
        }
    }
    break;

    case 1: // Gate
    {
        for (Gate g : Window::sidebarGateOrder)
        {
            if (g == data.gatePick)
                continue;
            Color color;
            if (InBoundingBox(rec, data.cursorUIPos))
            {
                color = UIColor(UIColorID::UI_COLOR_FOREGROUND);
                DrawText(Window::GetGateTooltipName(g), data.ButtonWidth() * 2 + (data.FontSize() / 2), data.ButtonWidth() + (data.FontSize() / 8) + rec.y, data.FontSize(), UIColor(UIColorID::UI_COLOR_FOREGROUND));
            }
            else
                color = UIColor(UIColorID::UI_COLOR_FOREGROUND3);
            data.DrawGateIcon(g, rec.xy, color);
            rec.y += data.ButtonWidth();
        }
    }
    break;

    case 2: // Resistance
    {
        for (uint8_t v = 0; v < _countof(Node::g_resistanceBands); ++v)
        {
            if (v == data.storedExtraParam)
                continue;
            Color color = Node::g_resistanceBands[v];
            if (InBoundingBox(rec, data.cursorUIPos))
            {
                DrawRectangleIRect(rec, UIColor(UIColorID::UI_COLOR_AVAILABLE));
                DrawRectangleIRect(ExpandIRect(rec, -2), color);
                const char* text;
                if (data.gatePick == Gate::LED)
                    text = TextFormat(data.deviceParameterTextFmt, Node::GetColorName(v));
                else
                    text = TextFormat(data.deviceParameterTextFmt, v);
                DrawText(text, data.ButtonWidth() * 3 + (data.FontSize() / 2), data.ButtonWidth() + (data.FontSize() / 8) + rec.y, data.FontSize(), UIColor(UIColorID::UI_COLOR_FOREGROUND));
            }
            else
                DrawRectangleIRect(rec, color);
            rec.y += data.ButtonWidth();
        }
    }
    break;
    }
}

PasteOverlay::PasteOverlay() {}
PasteOverlay::~PasteOverlay() {}
ModeType PasteOverlay::GetModeType() const { return ModeType::Overlay; }
Mode PasteOverlay::GetMode() const { return Mode::PASTE; }
void PasteOverlay::Update(Window& data)
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            Tab::Get().SpawnBlueprint(data.clipboard, data.cursorPos);
        data.ClearSelection();
        data.ClearOverlayMode();
    }
}
void PasteOverlay::Draw(Window& data)
{
    Tab::Get().DrawWires(UIColor(UIColorID::UI_COLOR_ACTIVE), UIColor(UIColorID::UI_COLOR_FOREGROUND3));
    Tab::Get().DrawNodes(UIColor(UIColorID::UI_COLOR_ACTIVE), UIColor(UIColorID::UI_COLOR_FOREGROUND));

    data.clipboard->DrawSelectionPreview(data.cursorPos - IVec2(g_gridSize), ColorAlpha(UIColor(UIColorID::UI_COLOR_BACKGROUND2), 0.5f), UIColor(UIColorID::UI_COLOR_FOREGROUND2), UIColor(UIColorID::UI_COLOR_BACKGROUND2), UIColor(UIColorID::UI_COLOR_FOREGROUND3), data.pastePreviewLOD);
}

BlueprintMenu::BlueprintMenu() {}
BlueprintMenu::~BlueprintMenu() {}
ModeType BlueprintMenu::GetModeType() const { return ModeType::Menu; }
Mode BlueprintMenu::GetMode() const { return Mode::BP_SELECT; }
void BlueprintMenu::Update(Window& data)
{
    constexpr int halfGrid = g_gridSize / 2;
    if (data.b_cursorMoved)
    {
        IVec2 pos(0, data.ButtonWidth());
        int maxY = 0; // I know there must be a better algorithm, but this will at least be progress.
        data.BPSelect_Hovering() = nullptr;
        for (Blueprint* bp : Tab::Get().GetBlueprints())
        {
            IRect rec = bp->GetSelectionPreviewRect(pos);
            if (rec.Right() > data.windowWidth)
            {
                pos = IVec2(0, maxY);
                rec = bp->GetSelectionPreviewRect(pos);
            }

            if (data.CursorInUIBounds(rec))
                data.BPSelect_Hovering() = bp;

            pos += rec.width;
            int recBottom = rec.y + rec.h;
            maxY = std::max(maxY, recBottom);
        }
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !!data.BPSelect_Hovering())
    {
        data.clipboard = data.BPSelect_Hovering();
        data.SetMode(Mode::PASTE);
    }
}
void BlueprintMenu::Draw(Window& data)
{
    constexpr int halfGrid = g_gridSize / 2;
    IVec2 pos(0, data.ButtonWidth());
    int maxY = 0; // I know there must be a better algorithm, but this will at least be progress.
    ClearBackground(UIColor(UIColorID::UI_COLOR_BLUEPRINTS_BACKGROUND));
    for (int y = 0; y < data.windowHeight; y += g_gridSize)
    {
        DrawLine(0, y, data.windowWidth, y, UIColor(UIColorID::UI_COLOR_BACKGROUND1));
    }
    for (int x = 0; x < data.windowWidth; x += g_gridSize)
    {
        DrawLine(x, 0, x, data.windowHeight, UIColor(UIColorID::UI_COLOR_BACKGROUND1));
    }
    DrawRectangle(0, 0, data.windowWidth, data.ButtonWidth(), UIColor(UIColorID::UI_COLOR_BACKGROUND1));
    const int padding = data.ButtonWidth() / 2 - (data.FontSize() / 2);
    DrawText("Blueprints", padding, padding, data.FontSize(), UIColor(UIColorID::UI_COLOR_FOREGROUND));
    for (Blueprint* bp : Tab::Get().GetBlueprints())
    {
        IRect rec = bp->GetSelectionPreviewRect(pos);
        if (rec.Right() > data.windowWidth)
        {
            pos = IVec2(0, maxY);
            rec = bp->GetSelectionPreviewRect(pos);
        }

        Color background;
        Color foreground;
        Color foregroundIO;
        if (!!data.BPSelect_Hovering() && bp == data.BPSelect_Hovering()) [[unlikely]]
        {
            background = UIColor(UIColorID::UI_COLOR_AVAILABLE);
            foreground = UIColor(UIColorID::UI_COLOR_FOREGROUND1);
            foregroundIO = UIColor(UIColorID::UI_COLOR_FOREGROUND2);
        }
        else [[likely]]
        {
            background = UIColor(UIColorID::UI_COLOR_BACKGROUND1);
            foreground = UIColor(UIColorID::UI_COLOR_FOREGROUND3);
            foregroundIO = UIColor(UIColorID::UI_COLOR_BACKGROUND2);
        }

        bp->DrawSelectionPreview(pos, background, foreground, foregroundIO, ColorAlpha(foreground, 0.25f), data.blueprintLOD);
        DrawRectangleLines(rec.x, rec.y, rec.w, rec.h, foreground);

        pos += rec.width;
        maxY = std::max(maxY, rec.Bottom());
    }
    if (!!data.BPSelect_Hovering())
        data.DrawTooltipAtCursor_Shadowed(data.BPSelect_Hovering()->name.c_str(), UIColor(UIColorID::UI_COLOR_FOREGROUND));
}
