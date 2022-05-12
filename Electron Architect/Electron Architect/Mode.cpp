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
#include "Graph.h"
#include "Tab.h"
#include "Buttons.h"
#include "UIColors.h"
#include "Window.h"
#include "Mode.h"

const std::string GateName(Gate gate)
{
    switch (gate)
    {
    case Gate::OR:        return "or";
    case Gate::AND:       return "and";
    case Gate::NOR:       return "nor";
    case Gate::XOR:       return "xor";
    case Gate::RESISTOR:  return "resistor";
    case Gate::CAPACITOR: return "capacitor";
    case Gate::LED:       return "LED";
    case Gate::DELAY:     return "delay";
    case Gate::BATTERY:   return "battery";
    default:
        _ASSERT_EXPR(false, L"Missing specialization for gate name");
        return "ERROR";
    }
}
const std::string ModeName(Mode mode)
{
    switch (mode)
    {
    case Mode::PEN: return "Pen";
    case Mode::EDIT: return "Edit";
    case Mode::ERASE: return "Erase";
    case Mode::INTERACT: return "Interact";
    case Mode::BUTTON: return "Button";
    case Mode::PASTE: return "Paste";
    case Mode::BP_SELECT: return "Blueprint Select";
    default:
        _ASSERT_EXPR(false, L"Missing specialization for mode name");
        return "ERROR";
    }
}
const std::string StateName(bool state)
{
    if (state)
        return "active";
    else
        return "inactive";
}
const std::string ElbowConfigName(ElbowConfig elbow)
{
    switch (elbow)
    {
    case ElbowConfig::horizontal: return "horizontal";
    case ElbowConfig::diagonalA:  return "diagonal from input";
    case ElbowConfig::vertical:   return "vertical";
    case ElbowConfig::diagonalB:  return "diagonal from output";
    default: return "ERROR";
    }
}

// Draw

PenTool::PenTool() :
    dragStart(0),
    previousWireStart(nullptr),
    currentWireStart(nullptr) {}
PenTool::~PenTool() {}
ModeType PenTool::GetModeType() const { return ModeType::Basic; }
Mode PenTool::GetMode() const { return Mode::PEN; }
void PenTool::Update(Window& window)
{
    if (window.b_cursorMoved) // On move
    {
        window.hoveredWire = nullptr;
        window.hoveredNode = window.CurrentTab().graph->FindNodeAtPos(window.cursorPos);
        if (!window.hoveredNode)
            window.hoveredWire = window.CurrentTab().graph->FindWireAtPos(window.cursorPos);
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        Node* newNode = window.hoveredNode;
        if (!newNode)
        {
            newNode = window.CurrentTab().graph->CreateNode(window.cursorPos, window.gatePick, window.storedExtraParam);
            if (!!window.hoveredWire && !(IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)))
            {
                window.CurrentTab().graph->BisectWire(window.hoveredWire, newNode);
                window.hoveredWire = nullptr;
            }
        }
        // Do not create a new node/wire if already hovering the start node
        if (!!currentWireStart && newNode != currentWireStart)
        {
            Node* oldNode;
            if (!!previousWireStart && (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)))
                oldNode = previousWireStart;
            else
                oldNode = currentWireStart;

            if (oldNode != newNode)
            {
                Wire* wire;
                // Reverse wire direction when holding ctrl
                if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))
                    wire = window.CurrentTab().graph->CreateWire(newNode, oldNode);
                else
                    wire = window.CurrentTab().graph->CreateWire(oldNode, newNode);
                
                wire->elbowConfig = window.currentWireElbowConfig;
                wire->UpdateElbowToLegal();
                previousWireStart = oldNode;
            }
        }
        currentWireStart = newNode;
    }
    else if (IsKeyPressed(KEY_R))
    {
        if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
            --window.currentWireElbowConfig;
        else
            ++window.currentWireElbowConfig;
    }
    else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    {
        previousWireStart = currentWireStart = nullptr;
    }
}
void PenTool::Draw(Window& window)
{
    window.CurrentTab().graph->DrawWires(UIColor(UIColorID::UI_COLOR_ACTIVE), UIColor(UIColorID::UI_COLOR_FOREGROUND3));

    // Draw node/wire preview
    {
        if (!!currentWireStart)
        {
            IVec2 start;
            IVec2 end = window.cursorPos;

            if (!!previousWireStart && (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)))
                start = previousWireStart->GetPosition();
            else
                start = currentWireStart->GetPosition();

            IVec2 elbow;
            if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))
                elbow = Wire::GetLegalElbowPosition(end, start, window.currentWireElbowConfig);
            else
                elbow = Wire::GetLegalElbowPosition(start, end, window.currentWireElbowConfig);

            Wire::Draw(start, elbow, end, UIColor(UIColorID::UI_COLOR_AVAILABLE));

            Node::Draw(end, window.gatePick, UIColor(UIColorID::UI_COLOR_AVAILABLE), UIColor(UIColorID::UI_COLOR_BACKGROUND));
        }
        else
            Node::Draw(window.cursorPos, window.gatePick, UIColor(UIColorID::UI_COLOR_AVAILABLE), UIColor(UIColorID::UI_COLOR_BACKGROUND));
    }

    if (!!window.hoveredWire && !(IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)))
    {
        window.hoveredWire->Draw(UIColor(UIColorID::UI_COLOR_AVAILABLE));
    }
    else if (!!window.hoveredNode)
    {
        // Todo: Can this please be based on the input and output ranges and not a test being run on an already partitioned vector?...
        for (const Wire* wire : window.hoveredNode->GetWires())
        {
            Color color;
            if (wire->start == window.hoveredNode)
                color = UIColor(UIColorID::UI_COLOR_OUTPUT); // Output
            else
                color = UIColor(UIColorID::UI_COLOR_INPUT); // Input

            wire->Draw(color);
        }
    }

    window.CurrentTab().graph->DrawNodes(UIColor(UIColorID::UI_COLOR_ACTIVE), UIColor(UIColorID::UI_COLOR_FOREGROUND));

    if (!!window.hoveredWire)
    {
        window.hoveredWire->start->DrawStateless(UIColor(UIColorID::UI_COLOR_INPUT), UIColor(UIColorID::UI_COLOR_BACKGROUND));
        window.hoveredWire->end->DrawStateless(UIColor(UIColorID::UI_COLOR_OUTPUT), UIColor(UIColorID::UI_COLOR_BACKGROUND));
    }
    else if (!!window.hoveredNode)
    {
        window.hoveredNode->DrawStateless(UIColor(UIColorID::UI_COLOR_AVAILABLE), UIColor(UIColorID::UI_COLOR_BACKGROUND));
    }
}
void PenTool::DrawProperties(Window& window)
{
    // Node hover stats
    window.PushPropertySection_Node("Hovered node", window.hoveredNode);
    // Wire hover stats
    window.PushPropertySection_Wire("Hovered wire", window.hoveredWire);
}

// Edit

EditTool::EditTool() :
    fallbackPos(0),
    selectionWIP(false),
    selectionStart(0),
    draggingGroup(false),
    draggingGroupCorner(false),
    groupCorner(),
    hoveringMergable(nullptr),
    nodeBeingDragged(nullptr),
    wireBeingDragged(nullptr) {}
EditTool::~EditTool() {}
ModeType EditTool::GetModeType() const { return ModeType::Basic; }
Mode EditTool::GetMode() const { return Mode::EDIT; }
void EditTool::Update(Window& window)
{
    // Todo: fix bug with canceling multiple-drag (And update group dragging to match!!)

    if (window.b_cursorMoved && !selectionWIP)
    {
        if (!nodeBeingDragged &&
            !wireBeingDragged &&
            !draggingGroup &&
            !draggingGroupCorner)
        {
            groupCorner.group = nullptr;
            window.hoveredGroup = nullptr;
            window.hoveredWire = nullptr;
            window.hoveredNode = window.CurrentTab().graph->FindNodeAtPos(window.cursorPos);
            if (!window.hoveredNode)
            {
                window.hoveredWire = window.CurrentTab().graph->FindWireElbowAtPos(window.cursorPos);
                if (!window.hoveredWire)
                {
                    window.hoveredGroup = window.CurrentTab().graph->FindGroupAtPos(window.cursorPos);
                    if (!window.hoveredGroup)
                    {
                        groupCorner = window.CurrentTab().graph->FindGroupCornerAtPos(window.cursorPos);
                    }
                }
            }
        }
        else if (nodeBeingDragged && !window.SelectionExists())
        {
            hoveringMergable = window.CurrentTab().graph->FindNodeAtPos(window.cursorPos); // This will come before updating the position of the dragged node
        }
    }

    // Press
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if (groupCorner.Valid())
        {
            draggingGroupCorner = true;
        }
        // There is a selection, and a node inside it has been pressed (move selected)
        else if (!!window.hoveredNode && std::find(window.CurrentTab().selection.begin(), window.CurrentTab().selection.end(), window.hoveredNode) != window.CurrentTab().selection.end())
        {
            nodeBeingDragged = window.hoveredNode;
            wireBeingDragged = nullptr;
        }
        // A node outside of the selection has been pressed (select exclusively it)
        else if (!!window.hoveredNode)
        {
            window.CurrentTab().selection.clear();
            window.CurrentTab().selection.push_back(window.hoveredNode);
            nodeBeingDragged = window.hoveredNode;
            wireBeingDragged = nullptr;
        }
        // Modify the last selection rectangle
        else if (window.IsSelectionRectValid() && (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)))
        {
            window.CurrentTab().selection.clear();
            for (const IRect& rec : window.CurrentTab().SelectionRecs())
            {
                window.CurrentTab().graph->FindNodesInRect(window.CurrentTab().selection, rec);
            }
            selectionWIP = true;
        }
        // Create a new selection rectangle
        else if (window.IsSelectionRectValid() && (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)))
        {
            window.CurrentTab().AddSelectionRec(IRect(window.cursorPos, 1));
            fallbackPos = window.cursorPos;
            if (selectionWIP = !(nodeBeingDragged || wireBeingDragged || draggingGroup || draggingGroupCorner))
                selectionStart = window.cursorPos;
        }
        // Clear selection and start anew
        else
        {
            window.ClearSelection();
            nodeBeingDragged = window.hoveredNode;
            wireBeingDragged = window.hoveredWire;

            // selectionStart being used as an offset here
            if (draggingGroup = !!window.hoveredGroup)
            {
                window.CurrentTab().graph->FindNodesInGroup(window.CurrentTab().selection, window.hoveredGroup);
                selectionStart = (window.cursorPos - (fallbackPos = window.hoveredGroup->GetPosition()));
            }

            fallbackPos = window.cursorPos;
            if (selectionWIP = !(nodeBeingDragged || wireBeingDragged || draggingGroup || draggingGroupCorner))
                selectionStart = window.cursorPos;
        }
    }

    // Selection
    if (selectionWIP)
    {
        auto [minx, maxx] = std::minmax(window.cursorPos.x, selectionStart.x);
        auto [miny, maxy] = std::minmax(window.cursorPos.y, selectionStart.y);
        if (!window.CurrentTab().GetLastSelectionRec())
            window.CurrentTab().AddSelectionRec(IRect(1));
        window.CurrentTab().GetLastSelectionRec()->w = maxx - (window.CurrentTab().GetLastSelectionRec()->x = minx);
        window.CurrentTab().GetLastSelectionRec()->h = maxy - (window.CurrentTab().GetLastSelectionRec()->y = miny);
    }
    // Node
    else if (!!nodeBeingDragged)
    {
        // Multiple selection
        if (window.CurrentTab().SelectionExists())
        {
            const IVec2 offset = window.GetCursorDelta();
            for (Node* node : window.CurrentTab().selection)
            {
                node->SetPosition_Temporary(node->GetPosition() + offset);
            }
            // Explicit exception to the rule of selectionRecs being modified without updating bridge cache
            // Please forgive my transgressions
            for (IRect& rec : const_cast<std::vector<IRect>&>(window.CurrentTab().SelectionRecs()))
            {
                rec.xy += offset;
            }
        }
        else
            nodeBeingDragged->SetPosition_Temporary(window.cursorPos);
    }
    // Wire
    else if (!!wireBeingDragged)
    {
        wireBeingDragged->SnapElbowToLegal(window.cursorPos);
    }
    // Group
    else if (draggingGroup)
    {
        window.hoveredGroup->SetPosition(window.cursorPos - selectionStart);
        for (Node* node : window.CurrentTab().selection)
        {
            const IVec2 offset = window.GetCursorDelta();
            node->SetPosition_Temporary(node->GetPosition() + offset);
        }
    }
    // Resize group
    else if (draggingGroupCorner)
    {
        _ASSERT_EXPR(groupCorner.cornerIndex < 4, L"Index out of range");
        constexpr int minWidth = g_gridSize * 2;
        IRect captureBounds = groupCorner.group->GetCaptureBounds();
        IVec2 cursorEnd;
        IVec2 otherEnd;
        switch (groupCorner.cornerIndex)
        {
        case 0:
            cursorEnd.x = std::min(window.cursorPos.x, captureBounds.Right() - minWidth);
            cursorEnd.y = std::min(window.cursorPos.y, captureBounds.Bottom() - minWidth);
            otherEnd = captureBounds.BR();
            break;
        case 1:
            cursorEnd.x = std::max(window.cursorPos.x, captureBounds.x + minWidth);
            cursorEnd.y = std::min(window.cursorPos.y, captureBounds.Bottom() - minWidth);
            otherEnd = captureBounds.BL();
            break;
        case 2:
            cursorEnd.x = std::min(window.cursorPos.x, captureBounds.Right() - minWidth);
            cursorEnd.y = std::max(window.cursorPos.y, captureBounds.y + minWidth);
            otherEnd = captureBounds.TR();
            break;
        case 3:
            cursorEnd.x = std::max(window.cursorPos.x, captureBounds.x + minWidth);
            cursorEnd.y = std::max(window.cursorPos.y, captureBounds.y + minWidth);
            otherEnd = captureBounds.TL();
            break;
        }
        captureBounds = IRectFromTwoPoints(cursorEnd, otherEnd);
        groupCorner.group->SetCaptureBounds(captureBounds);
    }

    // Release
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) || (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)))
    {
        // Cancel
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        {
            if (!!nodeBeingDragged)
            {
                nodeBeingDragged->SetPosition(fallbackPos);
            }
            else if (draggingGroup)
            {
                window.hoveredGroup->SetPosition(fallbackPos);
                for (Node* node : window.CurrentTab().selection)
                {
                    IVec2 offset = (fallbackPos + selectionStart) - window.cursorPos;
                    node->SetPosition_Temporary(node->GetPosition() + offset);
                }
            }
            else if (draggingGroupCorner)
            {
                // Todo
            }
            else if (selectionWIP)
            {
                window.ClearSelection();
            }
        }
        // Finalize
        else
        {
            if (!!nodeBeingDragged)
            {
                _ASSERT_EXPR(hoveringMergable != nodeBeingDragged, L"Node being dragged is trying to merge with itself");
                if (!!hoveringMergable)
                {
                    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
                    {
                        window.hoveredNode = window.CurrentTab().graph->MergeNodes(hoveringMergable, nodeBeingDragged);
                        hoveringMergable = nodeBeingDragged = nullptr;
                    }
                    else
                    {
                        window.CurrentTab().graph->SwapNodes(hoveringMergable, nodeBeingDragged);
                        nodeBeingDragged->SetPosition(fallbackPos);
                    }
                }
                else
                    nodeBeingDragged->SetPosition(window.cursorPos);
            }
            else if (draggingGroup)
            {
                for (Node* node : window.CurrentTab().selection)
                {
                    node->SetPosition(node->GetPosition());
                }
            }
            else if (draggingGroupCorner)
            {
                // Todo
            }
            else if (selectionWIP)
            {
                selectionWIP = false;
                if (!!window.CurrentTab().GetLastSelectionRec() &&
                    (window.CurrentTab().GetLastSelectionRec()->w == 0 ||
                    window.CurrentTab().GetLastSelectionRec()->h == 0))
                {
                    window.CurrentTab().PopSelectionRec();
                }
                window.CurrentTab().selection.clear();
                if (window.IsSelectionRectValid())
                {
                    for (const IRect& rec : window.CurrentTab().SelectionRecs())
                    {
                        window.CurrentTab().graph->FindNodesInRect(window.CurrentTab().selection, rec);
                    }
                }
                std::unordered_set<Node*> selectionUnique(window.CurrentTab().selection.begin(), window.CurrentTab().selection.end());
                window.CurrentTab().selection.clear();
                window.CurrentTab().selection = std::vector<Node*>(selectionUnique.begin(), selectionUnique.end());

                window.CurrentTab().UpdateBridgeCache();
            }
        }
        if (draggingGroup)
            window.ClearSelection();
        nodeBeingDragged = nullptr;
        selectionWIP = false;
        draggingGroup = false;
        draggingGroupCorner = false;
        wireBeingDragged = nullptr;
    }
    // Right click
    else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && !!window.hoveredNode)
    {
        window.hoveredNode->SetGate(window.gatePick);
        switch (window.hoveredNode->GetGate())
        {
        case Gate::RESISTOR:  window.hoveredNode->SetResistance(window.storedExtraParam); break;
        case Gate::LED:       window.hoveredNode->SetColorIndex(window.storedExtraParam); break;
        case Gate::CAPACITOR: window.hoveredNode->SetCapacity(window.storedExtraParam);   break;
        }
    }

    // Wire bridge
    if (IsKeyPressed(KEY_SPACE) && window.CurrentTab().IsSelectionBridgeable()) [[unlikely]]
    {
        window.CurrentTab().BridgeSelection(window.currentWireElbowConfig);
        window.ClearSelection();
    }
}
void EditTool::Draw(Window& window)
{
    if (!!window.hoveredGroup)
    {
        window.hoveredGroup->Highlight(UIColor(UIColorID::UI_COLOR_FOREGROUND1));
    }
    else if (groupCorner.Valid())
    {
        Color color;
        if (draggingGroupCorner)
            color = UIColor(UIColorID::UI_COLOR_FOREGROUND1);
        else
            color = groupCorner.group->GetColor();
        DrawRectangleIRect(groupCorner.GetCollisionRect(), color);
    }

    // Selection preview
    std::vector<Node*> selectionPreviewNodes;
    if (window.selectionPreview && selectionWIP)
    {
        const IRect* rec = window.CurrentTab().GetLastSelectionRecConst();
        if (!!rec)
            window.CurrentTab().graph->FindNodesInRect(selectionPreviewNodes, *rec);
    }

    // Selection rectangles
    {
        bool actuallyBridgeable = window.CurrentTab().IsSelectionBridgeable();

        bool previewBridgeable = 
            actuallyBridgeable ||
            (window.selectionPreview && selectionWIP &&
            window.CurrentTab().SelectionRectCount() == 2 &&
            window.CurrentTab().SelectionExists() &&
                ((window.CurrentTab().selection.size() == 1 && selectionPreviewNodes.size() > 1) ||
                 (window.CurrentTab().selection.size() > 1 && selectionPreviewNodes.size() == 1) ||
                 (window.CurrentTab().selection.size() == selectionPreviewNodes.size())));

        if (previewBridgeable) [[unlikely]]
        {
            DrawRectangleIRect(window.CurrentTab().SelectionRecs()[0], ColorAlpha(UIColor(UIColorID::UI_COLOR_INPUT), 0.5));
            DrawRectangleLinesIRect(window.CurrentTab().SelectionRecs()[0], UIColor(UIColorID::UI_COLOR_INPUT));
            DrawRectangleIRect(window.CurrentTab().SelectionRecs()[1], ColorAlpha(UIColor(UIColorID::UI_COLOR_OUTPUT), 0.5));
            DrawRectangleLinesIRect(window.CurrentTab().SelectionRecs()[1], UIColor(UIColorID::UI_COLOR_OUTPUT));

            if (actuallyBridgeable)
                window.CurrentTab().DrawBridgePreview(window.currentWireElbowConfig, UIColor(UIColorID::UI_COLOR_AVAILABLE));
        }
        else [[likely]]
        {
            for (const IRect& rec : window.CurrentTab().SelectionRecs())
            {
                DrawRectangleIRect(rec, ColorAlpha(UIColor(UIColorID::UI_COLOR_BACKGROUND1), 0.5));
                DrawRectangleLinesIRect(rec, UIColor(UIColorID::UI_COLOR_BACKGROUND2));
            }
        }
    }
    window.CurrentTab().graph->DrawWires(UIColor(UIColorID::UI_COLOR_ACTIVE), UIColor(UIColorID::UI_COLOR_FOREGROUND3));

    // Selection
    for (Node* node : window.CurrentTab().selection)
    {
        DrawCircleIV(node->GetPosition(), node->g_nodeRadius + 3, UIColor(UIColorID::UI_COLOR_AVAILABLE));
    }
    for (Node* node : selectionPreviewNodes)
    {
        DrawCircleIV(node->GetPosition(), node->g_nodeRadius + 3, UIColor(UIColorID::UI_COLOR_AVAILABLE));
    }

    if (!!window.hoveredWire)
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
            IVec2 p = window.hoveredWire->GetLegalElbowPosition(configOrder[i]);
            Wire::Draw(window.hoveredWire->GetStartPos(), p, window.hoveredWire->GetEndPos(), ColorAlpha(UIColor(UIColorID::UI_COLOR_AVAILABLE), 0.25f));
            DrawCircle(p.x, p.y, Wire::g_elbowRadius, ColorAlpha(UIColor(UIColorID::UI_COLOR_AVAILABLE), 0.5f));
        }

        window.hoveredWire->Draw(UIColor(UIColorID::UI_COLOR_AVAILABLE));
        Color elbowColor;
        if (!!wireBeingDragged)
            elbowColor = UIColor(UIColorID::UI_COLOR_AVAILABLE);
        else
            elbowColor = UIColor(UIColorID::UI_COLOR_CAUTION);
        window.hoveredWire->DrawElbow(elbowColor);
    }

    window.CurrentTab().graph->DrawNodes(UIColor(UIColorID::UI_COLOR_ACTIVE), UIColor(UIColorID::UI_COLOR_FOREGROUND));

    if (!!window.hoveredNode)
    {
        window.hoveredNode->DrawStateless(UIColor(UIColorID::UI_COLOR_CAUTION), UIColor(UIColorID::UI_COLOR_BACKGROUND));
    }
    if (!!window.hoveredWire)
    {
        window.hoveredWire->start->DrawStateless(UIColor(UIColorID::UI_COLOR_INPUT), UIColor(UIColorID::UI_COLOR_BACKGROUND));
        window.hoveredWire->end->DrawStateless(UIColor(UIColorID::UI_COLOR_OUTPUT), UIColor(UIColorID::UI_COLOR_BACKGROUND));
    }

    if (!!nodeBeingDragged && hoveringMergable)
    {
        DrawCircleIV(nodeBeingDragged->GetPosition(), Node::g_nodeRadius * 2.0f, UIColor(UIColorID::UI_COLOR_SPECIAL));
        window.DrawTooltipAtCursor(
            "Hold [shift] to merge on release.\n"
            "Otherwise, nodes will only be swapped.", UIColor(UIColorID::UI_COLOR_SPECIAL));
    }


    if (IsKeyPressed(KEY_R))
    {
        if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
            --window.currentWireElbowConfig;
        else
            ++window.currentWireElbowConfig;
    }
}
void EditTool::DrawProperties(Window& window)
{
    // Selection stats
    if (window.SelectionExists() && window.CurrentTab().SelectionSize() > 1)
        window.PushPropertySection_Selection("Selection", window.CurrentTab().selection);
    else if (window.CurrentTab().SelectionSize() == 1)
        window.PushPropertySection_Node("Selected node", window.CurrentTab().selection[0]);
    // Node hover stats
    window.PushPropertySection_Node("Hovered node", window.hoveredNode);
    // Joint hover stats
    window.PushPropertySection_Wire("Hovered wire joint", window.hoveredWire);
    // Group hover stats
    window.PushPropertySection_Group("Hovered group", window.hoveredGroup);
}

// Erase

EraseTool::EraseTool() {}
EraseTool::~EraseTool() {}
ModeType EraseTool::GetModeType() const { return ModeType::Basic; }
Mode EraseTool::GetMode() const { return Mode::ERASE; }
void EraseTool::Update(Window& window)
{
    if (window.b_cursorMoved)
    {
        window.hoveredWire = nullptr;
        window.hoveredNode = window.CurrentTab().graph->FindNodeAtPos(window.cursorPos);
        if (!window.hoveredNode)
        {
            window.hoveredWire = window.CurrentTab().graph->FindWireAtPos(window.cursorPos);
            if (!window.hoveredWire)
                window.hoveredGroup = window.CurrentTab().graph->FindGroupAtPos(window.cursorPos);
        }
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if (!!window.hoveredNode)
        {
            // Special erase
            if ((IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) &&
                window.hoveredNode->IsSpecialErasable())
                window.CurrentTab().graph->BypassNode(window.hoveredNode);
            // Complex bipass
            else if (
                (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) &&
                (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) &&
                window.hoveredNode->IsComplexBipassable())
                window.CurrentTab().graph->BypassNode_Complex(window.hoveredNode);
            else
                window.CurrentTab().graph->DestroyNode(window.hoveredNode);
        }
        else if (!!window.hoveredWire)
            window.CurrentTab().graph->DestroyWire(window.hoveredWire);
        else if (!!window.hoveredGroup)
            window.CurrentTab().graph->DestroyGroup(window.hoveredGroup);

        window.hoveredNode = nullptr;
        window.hoveredWire = nullptr;
        window.hoveredGroup = nullptr;
    }
}
void EraseTool::Draw(Window& window)
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

    if (!!window.hoveredGroup)
    {
        IRect rec = window.hoveredGroup->GetLabelBounds();
        window.hoveredGroup->Highlight(UIColor(UIColorID::UI_COLOR_ERROR));
        DrawLineEx({ (float)rec.x, (float)rec.y }, { (float)rec.x + (float)rec.h, (float)rec.Bottom() }, 3, UIColor(UIColorID::UI_COLOR_DESTRUCTIVE));
        DrawLineEx({ (float)rec.x, (float)rec.Bottom() }, { (float)rec.x + (float)rec.h, (float)rec.y }, 3, UIColor(UIColorID::UI_COLOR_DESTRUCTIVE));
    }

    window.CurrentTab().graph->DrawWires(UIColor(UIColorID::UI_COLOR_ACTIVE), UIColor(UIColorID::UI_COLOR_FOREGROUND3));

    if (!!window.hoveredWire)
    {
        window.hoveredWire->Draw(UIColor(UIColorID::UI_COLOR_ERROR));
        DrawCross(window.cursorPos, UIColor(UIColorID::UI_COLOR_DESTRUCTIVE));
    }
    else if (!!window.hoveredNode)
    {
        Color color;
        if ((IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)))
        {
            if (window.hoveredNode->IsSpecialErasable())
                color = UIColor(UIColorID::UI_COLOR_AVAILABLE);
            else if (window.hoveredNode->IsComplexBipassable() && (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)))
                color = UIColor(UIColorID::UI_COLOR_SPECIAL);
            else
                color = UIColor(UIColorID::UI_COLOR_DESTRUCTIVE);
        }
        else
            color = UIColor(UIColorID::UI_COLOR_ERROR);

        for (Wire* wire : window.hoveredNode->GetWires())
        {
            wire->Draw(color);
        }
    }

    window.CurrentTab().graph->DrawNodes(UIColor(UIColorID::UI_COLOR_ACTIVE), UIColor(UIColorID::UI_COLOR_FOREGROUND));

    if (!!window.hoveredNode)
    {
        window.hoveredNode->DrawStateless(UIColor(UIColorID::UI_COLOR_ERROR), UIColor(UIColorID::UI_COLOR_BACKGROUND));
        DrawCross(window.hoveredNode->GetPosition(), UIColor(UIColorID::UI_COLOR_DESTRUCTIVE));

        if ((IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)))
        {
            const char* text;
            Color color;
            if (window.hoveredNode->IsSpecialErasable())
            {
                color = UIColor(UIColorID::UI_COLOR_AVAILABLE);
                text = "Simple bipass";
            }
            else if (window.hoveredNode->IsComplexBipassable() && (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)))
            {
                color = UIColor(UIColorID::UI_COLOR_SPECIAL);
                text = "Complex bipass";
            }
            else
            {
                color = UIColor(UIColorID::UI_COLOR_DESTRUCTIVE);
                size_t iCount = window.hoveredNode->GetInputCount();
                size_t oCount = window.hoveredNode->GetOutputCount();

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
            window.DrawTooltipAtCursor_Shadowed(text, color);
        }
    }
}
void EraseTool::DrawProperties(Window& window)
{
    // Todo
}

// Interact

InteractTool::InteractTool() {}
InteractTool::~InteractTool() {}
ModeType InteractTool::GetModeType() const { return ModeType::Basic; }
Mode InteractTool::GetMode() const { return Mode::INTERACT; }
void InteractTool::Update(Window& window)
{
    window.hoveredNode = window.CurrentTab().graph->FindNodeAtPos(window.cursorPos);
    if (!!window.hoveredNode && !window.hoveredNode->IsOutputOnly())
        window.hoveredNode = nullptr;

    if (!!window.hoveredNode && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        window.hoveredNode->SetGate(window.hoveredNode->GetGate() == Gate::NOR ? Gate::OR : Gate::NOR);
}
void InteractTool::Draw(Window& window)
{
    window.CurrentTab().graph->DrawWires(UIColor(UIColorID::UI_COLOR_ACTIVE), UIColor(UIColorID::UI_COLOR_FOREGROUND3));
    window.CurrentTab().graph->DrawNodes(UIColor(UIColorID::UI_COLOR_ACTIVE), UIColor(UIColorID::UI_COLOR_FOREGROUND));

    for (const Node* node : window.CurrentTab().graph->GetStartNodes())
    {
        node->DrawStateless(UIColor(UIColorID::UI_COLOR_AVAILABLE), UIColor(UIColorID::UI_COLOR_BACKGROUND));
    }

    if (!!window.hoveredNode)
    {
        window.hoveredNode->DrawStateless(UIColor(UIColorID::UI_COLOR_CAUTION), UIColor(UIColorID::UI_COLOR_BACKGROUND));
    }
}
void InteractTool::DrawProperties(Window& window)
{
    // Node hover stats
    window.PushPropertySection_Node("Hovered interactable node", window.hoveredNode);
}

// Paste

PasteOverlay::PasteOverlay() {}
PasteOverlay::~PasteOverlay() {}
ModeType PasteOverlay::GetModeType() const { return ModeType::Overlay; }
Mode PasteOverlay::GetMode() const { return Mode::PASTE; }
void PasteOverlay::Update(Window& window)
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            window.CurrentTab().graph->SpawnBlueprint(window.clipboard, window.cursorPos);
        window.ClearSelection();
        window.ClearOverlayMode();
    }
}
void PasteOverlay::Draw(Window& window)
{
    window.CurrentTab().graph->DrawWires(UIColor(UIColorID::UI_COLOR_ACTIVE), UIColor(UIColorID::UI_COLOR_FOREGROUND3));
    window.CurrentTab().graph->DrawNodes(UIColor(UIColorID::UI_COLOR_ACTIVE), UIColor(UIColorID::UI_COLOR_FOREGROUND));

    window.clipboard->DrawSelectionPreview(window.cursorPos - IVec2(g_gridSize), ColorAlpha(UIColor(UIColorID::UI_COLOR_BACKGROUND2), 0.5f), UIColor(UIColorID::UI_COLOR_FOREGROUND2), UIColor(UIColorID::UI_COLOR_BACKGROUND2), UIColor(UIColorID::UI_COLOR_FOREGROUND3), window.pastePreviewLOD);
}
void PasteOverlay::DrawProperties(Window& window)
{
    // Todo
}

// Blueprint menu

BlueprintMenu::BlueprintMenu() :
    hovering(nullptr) {}
BlueprintMenu::~BlueprintMenu() {}
ModeType BlueprintMenu::GetModeType() const { return ModeType::Menu; }
Mode BlueprintMenu::GetMode() const { return Mode::BP_SELECT; }
void BlueprintMenu::Update(Window& window)
{
    constexpr int halfGrid = g_gridSize / 2;
    if (window.b_cursorMoved)
    {
        IVec2 pos(0, Button::g_width);
        int maxY = 0; // I know there must be a better algorithm, but this will at least be progress.
        hovering = nullptr;
        for (Blueprint* bp : window.CurrentTab().graph->GetBlueprints())
        {
            IRect rec = bp->GetSelectionPreviewRect(pos);
            if (rec.Right() > window.windowWidth)
            {
                pos = IVec2(0, maxY);
                rec = bp->GetSelectionPreviewRect(pos);
            }

            if (window.CursorInUIBounds(rec))
                hovering = bp;

            pos += rec.width;
            int recBottom = rec.y + rec.h;
            maxY = std::max(maxY, recBottom);
        }
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !!hovering)
    {
        window.clipboard = hovering;
        window.SetMode(Mode::PASTE);
    }
}
void BlueprintMenu::Draw(Window& window)
{
    constexpr int halfGrid = g_gridSize / 2;
    IVec2 pos(0, Button::g_width);
    int maxY = 0; // I know there must be a better algorithm, but this will at least be progress.
    ClearBackground(UIColor(UIColorID::UI_COLOR_BLUEPRINTS_BACKGROUND));
    for (int y = 0; y < window.windowHeight; y += g_gridSize)
    {
        DrawLine(0, y, window.windowWidth, y, UIColor(UIColorID::UI_COLOR_BACKGROUND1));
    }
    for (int x = 0; x < window.windowWidth; x += g_gridSize)
    {
        DrawLine(x, 0, x, window.windowHeight, UIColor(UIColorID::UI_COLOR_BACKGROUND1));
    }
    DrawRectangle(0, 0, window.windowWidth, Button::g_width, UIColor(UIColorID::UI_COLOR_BACKGROUND1));
    const int padding = Button::g_width / 2 - (window.FontSize() / 2);
    DrawText("Blueprints", padding, padding, window.FontSize(), UIColor(UIColorID::UI_COLOR_FOREGROUND));
    for (Blueprint* bp : window.CurrentTab().graph->GetBlueprints())
    {
        IRect rec = bp->GetSelectionPreviewRect(pos);
        if (rec.Right() > window.windowWidth)
        {
            pos = IVec2(0, maxY);
            rec = bp->GetSelectionPreviewRect(pos);
        }

        Color background;
        Color foreground;
        Color foregroundIO;
        if (!!hovering && bp == hovering) [[unlikely]]
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

        bp->DrawSelectionPreview(pos, background, foreground, foregroundIO, ColorAlpha(foreground, 0.25f), window.blueprintLOD);
        DrawRectangleLines(rec.x, rec.y, rec.w, rec.h, foreground);

        pos += rec.width;
        maxY = std::max(maxY, rec.Bottom());
    }
    if (!!hovering)
        window.DrawTooltipAtCursor_Shadowed(hovering->name.c_str(), UIColor(UIColorID::UI_COLOR_FOREGROUND));
}
void BlueprintMenu::DrawProperties(Window& window)
{
    // Todo
}
