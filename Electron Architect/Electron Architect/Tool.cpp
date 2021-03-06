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
#include "Tool.h"

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
    case Mode::SETTINGS: return "Settings (WIP)";
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
    dragging(false),
    dragMode(DragType::drag_create),
    previousWireStart(nullptr),
    currentWireStart(nullptr) {}
PenTool::~PenTool() {}
ModeType PenTool::GetModeType() const { return ModeType::Basic; }
Mode PenTool::GetMode() const { return Mode::PEN; }
void PenTool::Update(Window& window, bool allowHover)
{
    if (!allowHover)
    {
        window.hoveredWire = nullptr;
        window.hoveredNode = nullptr;
        window.hoveredGroup = nullptr;
    }
    else if (window.b_cursorMoved) // On move
    {
        window.hoveredWire = nullptr;
        window.hoveredNode = window.CurrentTab().graph->FindNodeAtPos(window.cursorPos);
        if (!window.hoveredNode)
            window.hoveredWire = window.CurrentTab().graph->FindWireAtPos(window.cursorPos);
    }
    
    // Click (start drag)
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) [[unlikely]]
    {
        dragging = true;
        Node* newNode = window.hoveredNode;

        // Not hovering a node - create a new one
        dragMode = (DragType)!!newNode;

        if (dragMode == DragType::drag_create)
        {
            newNode = window.CurrentTab().graph->CreateNode(window.cursorPos, window.gatePick, window.storedExtraParam);
            if (!!window.hoveredWire && !(IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)))
            {
                window.CurrentTab().graph->BisectWire(window.hoveredWire, newNode);
                window.hoveredWire = nullptr;
            }
        }

        // Do not create a new wire if already hovering the start node
        if (!!currentWireStart && newNode != currentWireStart)
        {
            Node* oldNode;
            if (!!previousWireStart && (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)))
                oldNode = previousWireStart;
            else
                oldNode = currentWireStart;

            if (oldNode != newNode)
            {
                previousWireStart = oldNode;

                std::pair<Node*, Node*> nodes;

                // Reverse wire direction when holding ctrl
                if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))
                    nodes = { newNode, oldNode };
                else
                    nodes = { oldNode, newNode };

                window.CurrentTab().graph->CreateWire(nodes.first, nodes.second, window.currentWireElbowConfig);
            }
        }
        currentWireStart = newNode;
    }
    // Stop dragging
    else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) [[unlikely]]
    {
        dragging = false;

        // Create connection
        if (dragMode == DragType::drag_connect && currentWireStart != window.hoveredNode)
        {
            if (window.hoveredNode)
            {
                std::pair<Node*, Node*> nodes;

                // Reverse wire direction when holding ctrl
                if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))
                    nodes = { window.hoveredNode, currentWireStart };
                else
                    nodes = { currentWireStart, window.hoveredNode };

                window.CurrentTab().graph->CreateWire(nodes.first, nodes.second, window.currentWireElbowConfig);
            }
            currentWireStart = nullptr; // Cancel wire
        }
    }
    // On move while dragging
    else if (dragging && window.b_cursorMoved && // Dragging
        dragMode == DragType::drag_create && // Should create
        !window.hoveredNode) // Won't create redundant
    {
        currentWireStart = nullptr;
        window.CurrentTab().graph->CreateNode(window.cursorPos, window.gatePick, window.storedExtraParam);
    }
    if (IsKeyPressed(KEY_R)) [[unlikely]]
    {
        if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
            --window.currentWireElbowConfig;
        else
            ++window.currentWireElbowConfig;
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) [[unlikely]]
    {
        previousWireStart = currentWireStart = nullptr;
    }
}
void PenTool::Draw(Window& window)
{
    window.CurrentTab().graph->DrawWires(UIColor(UIColorID::UI_COLOR_ACTIVE), UIColor(UIColorID::UI_COLOR_FOREGROUND3));

    // Draw node/wire preview
    Node* startNode = nullptr;
    {
        IVec2 end = window.cursorPos;
        if (!!currentWireStart)
        {
            if (!!previousWireStart && (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)))
                startNode = previousWireStart;
            else
                startNode = currentWireStart;

            IVec2 elbow;
            if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))
                elbow = Wire::GetLegalElbowPosition(end, startNode->GetPosition(), window.currentWireElbowConfig);
            else
                elbow = Wire::GetLegalElbowPosition(startNode->GetPosition(), end, window.currentWireElbowConfig);

            Wire::Draw(startNode->GetPosition(), elbow, end, UIColor(UIColorID::UI_COLOR_AVAILABLE));
        }
        if (!window.hoveredNode)
            Node::Draw(window.CurrentTab().camera.zoom, end, window.gatePick, UIColor(UIColorID::UI_COLOR_AVAILABLE), UIColor(UIColorID::UI_COLOR_BACKGROUND));
    }

    if (!!window.hoveredWire && !(IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)))
    {
        window.hoveredWire->Draw(UIColor(UIColorID::UI_COLOR_AVAILABLE));
    }
    else if (!!window.hoveredNode && !currentWireStart)
    {
        for (const Wire* wire : window.hoveredNode->GetInputsConst())
        {
            wire->Draw(UIColor(UIColorID::UI_COLOR_INPUT));
        }
        for (const Wire* wire : window.hoveredNode->GetOutputsConst())
        {
            wire->Draw(UIColor(UIColorID::UI_COLOR_OUTPUT));
        }
    }

    window.CurrentTab().graph->DrawNodes(window.CurrentTab().camera.zoom, UIColor(UIColorID::UI_COLOR_ACTIVE), UIColor(UIColorID::UI_COLOR_FOREGROUND));

    if (!!startNode && startNode != window.hoveredNode)
    {
        Color color;
        if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))
            color = UIColor(UIColorID::UI_COLOR_OUTPUT);
        else
            color = UIColor(UIColorID::UI_COLOR_INPUT);
        startNode->DrawStateless(window.CurrentTab().camera.zoom, color, UIColor(UIColorID::UI_COLOR_BACKGROUND));
    }
    if (!!window.hoveredWire)
    {
        window.hoveredWire->start->DrawStateless(window.CurrentTab().camera.zoom, UIColor(UIColorID::UI_COLOR_INPUT), UIColor(UIColorID::UI_COLOR_BACKGROUND));
        window.hoveredWire->end->DrawStateless(window.CurrentTab().camera.zoom, UIColor(UIColorID::UI_COLOR_OUTPUT), UIColor(UIColorID::UI_COLOR_BACKGROUND));
    }
    else if (!!window.hoveredNode)
    {
        if (!!currentWireStart && window.hoveredNode != currentWireStart)
        {
            Color color;
            if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))
                color = UIColor(UIColorID::UI_COLOR_INPUT);
            else
                color = UIColor(UIColorID::UI_COLOR_OUTPUT);
            window.hoveredNode->DrawStateless(window.CurrentTab().camera.zoom, color, UIColor(UIColorID::UI_COLOR_BACKGROUND));
        }
        else if (!currentWireStart)
            window.hoveredNode->DrawStateless(window.CurrentTab().camera.zoom, UIColor(UIColorID::UI_COLOR_AVAILABLE), UIColor(UIColorID::UI_COLOR_BACKGROUND));
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
    renamingGroup(false),
    groupCorner(),
    hoveringMergable(nullptr),
    nodeBeingDragged(nullptr),
    wireBeingDragged(nullptr) {}
EditTool::~EditTool() {}
ModeType EditTool::GetModeType() const { return ModeType::Basic; }
Mode EditTool::GetMode() const { return Mode::EDIT; }
void EditTool::Update(Window& window, bool allowHover)
{
    if (renamingGroup && allowHover)
    {
        _ASSERT_EXPR(!!window.hoveredGroup, L"Cannot rename null group");

        if (char pressed = GetCharPressed())
            window.hoveredGroup->GetLabelEditable() += pressed;

        if (IsKeyPressed(KEY_BACKSPACE))
            window.hoveredGroup->GetLabelEditable().pop_back();

        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE))
        {
            renamingGroup = false;
            window.b_cursorMoved = true;
        }
        return;
    }

    // Todo: fix bug with canceling multiple-drag (And update group dragging to match!!)

    if (!allowHover)
    {
        window.hoveredWire = nullptr;
        window.hoveredNode = nullptr;
        window.hoveredGroup = nullptr;
        groupCorner.group = nullptr;
    }
    else if (window.b_cursorMoved && !selectionWIP /* selectionWIP locks hover in place (as empty) */)
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
        // Drag group corner
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
            // Add to selection when holding ctrl
            if (!(IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)))
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
            window.CurrentTab().AddSelectionRec(IRect(0));
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

            if (selectionWIP)
                window.CurrentTab().AddSelectionRec(IRect(0));
        }
    }

    // Selection
    if (selectionWIP)
    {
        _ASSERT_EXPR(window.CurrentTab().SelectionRectExists(), L"Selection cannot be in progress without a rectangle");
        auto [minx, maxx] = std::minmax(window.cursorPos.x, selectionStart.x);
        auto [miny, maxy] = std::minmax(window.cursorPos.y, selectionStart.y);
        IVec2 min(minx, miny), max(maxx, maxy);
        window.CurrentTab().GetLastSelectionRec()->wh = max - (window.CurrentTab().GetLastSelectionRec()->xy = min);
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
                window.CurrentTab().ConfirmLastSelectionRec();
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
    else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    {
        if (!!window.hoveredNode)
        {
            window.hoveredNode->SetGate(window.gatePick);
            switch (window.hoveredNode->GetGate())
            {
            case Gate::RESISTOR:  window.hoveredNode->SetResistance(window.storedExtraParam); break;
            case Gate::LED:       window.hoveredNode->SetColorIndex(window.storedExtraParam); break;
            case Gate::CAPACITOR: window.hoveredNode->SetCapacity(window.storedExtraParam);   break;
            }
        }
        else if (!!window.hoveredGroup)
        {
            window.hoveredGroup->GetLabelEditable().clear();
            renamingGroup = true;
        }
    }

    // Wire bridge
    if (IsKeyPressed(KEY_SPACE) && window.CurrentTab().IsSelectionBridgeable()) [[unlikely]]
    {
        window.CurrentTab().BridgeSelection(window.currentWireElbowConfig);
        window.ClearSelection();
    }

    if (IsKeyPressed(KEY_R))
    {
        if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
            --window.currentWireElbowConfig;
        else
            ++window.currentWireElbowConfig;
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

        bool previewBridgeable = actuallyBridgeable; // Todo: define preview bridge

        auto DrawColoredOutlineRect = [](const IRect& rec, const Color& fill, const Color& outline)
        {
            DrawRectangleIRect(rec, fill);
            DrawRectangleLinesIRect(rec, outline);
        };

        if (previewBridgeable && !selectionWIP) [[unlikely]]
        {
            // Draw colored selection rectangle preview
            {
                const Color inputOutlineColor = UIColor(UIColorID::UI_COLOR_INPUT);
                const Color inputFillColor = ColorAlpha(inputOutlineColor, 0.5);
                const Color outputOutlineColor = UIColor(UIColorID::UI_COLOR_OUTPUT);
                const Color outputFillColor = ColorAlpha(outputOutlineColor, 0.5);

                switch (window.CurrentTab().cachedBridgeType)
                {
                default:
                case WireBridgeType::none:
                    _ASSERT_EXPR(false, L"Bad bridge type");
                    break;

                case WireBridgeType::one_to_many:
                    DrawColoredOutlineRect(window.CurrentTab().SelectionRecs()[0], inputFillColor, inputOutlineColor);
                    for (size_t i = 1; i < window.CurrentTab().SelectionRecs().size(); ++i)
                    {
                        DrawColoredOutlineRect(window.CurrentTab().SelectionRecs()[i], outputFillColor, outputOutlineColor);
                    }
                    break;

                case WireBridgeType::many_to_one:
                    for (size_t i = 0; i < window.CurrentTab().SelectionRecs().size() - 1; ++i)
                    {
                        DrawColoredOutlineRect(window.CurrentTab().SelectionRecs()[i], inputFillColor, inputOutlineColor);
                    }
                    DrawColoredOutlineRect(window.CurrentTab().SelectionRecs().back(), outputFillColor, outputOutlineColor);
                    break;

                case WireBridgeType::even:
                    const Color specialOutlineColor = UIColor(UIColorID::UI_COLOR_SPECIAL);
                    const Color specialFillColor = ColorAlpha(specialOutlineColor, 0.5);

                    DrawColoredOutlineRect(window.CurrentTab().SelectionRecs()[0], inputFillColor, inputOutlineColor);
                    for (size_t i = 1; i < window.CurrentTab().SelectionRecs().size() - 1; ++i)
                    {
                        DrawColoredOutlineRect(window.CurrentTab().SelectionRecs()[i], specialFillColor, specialOutlineColor);
                    }
                    DrawColoredOutlineRect(window.CurrentTab().SelectionRecs().back(), outputFillColor, outputOutlineColor);
                    break;
                }
            }

            if (actuallyBridgeable)
                window.CurrentTab().DrawBridgePreview(window.currentWireElbowConfig, UIColor(UIColorID::UI_COLOR_AVAILABLE));
        }
        else [[likely]]
        {
            const Color basicOutlineColor = UIColor(UIColorID::UI_COLOR_BACKGROUND2);
            const Color basicFillColor = ColorAlpha(UIColor(UIColorID::UI_COLOR_BACKGROUND1), 0.5);
            for (const IRect& rec : window.CurrentTab().SelectionRecs())
            {
                DrawColoredOutlineRect(rec, basicFillColor, basicOutlineColor);
            }
        }
    }
    window.CurrentTab().graph->DrawWires(UIColor(UIColorID::UI_COLOR_ACTIVE), UIColor(UIColorID::UI_COLOR_FOREGROUND3));

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
            Wire::DrawElbow(p, ColorAlpha(UIColor(UIColorID::UI_COLOR_AVAILABLE), 0.5f));
        }

        window.hoveredWire->Draw(UIColor(UIColorID::UI_COLOR_AVAILABLE));
        Color elbowColor;
        if (!!wireBeingDragged)
            elbowColor = UIColor(UIColorID::UI_COLOR_AVAILABLE);
        else
            elbowColor = UIColor(UIColorID::UI_COLOR_CAUTION);
        window.hoveredWire->DrawElbow(elbowColor);
    }

    window.CurrentTab().graph->DrawNodes(window.CurrentTab().camera.zoom, UIColor(UIColorID::UI_COLOR_ACTIVE), UIColor(UIColorID::UI_COLOR_FOREGROUND));

    if (!!window.hoveredNode)
    {
        window.hoveredNode->DrawStateless(window.CurrentTab().camera.zoom, UIColor(UIColorID::UI_COLOR_CAUTION), UIColor(UIColorID::UI_COLOR_BACKGROUND));
    }
    if (!!window.hoveredWire)
    {
        window.hoveredWire->start->DrawStateless(window.CurrentTab().camera.zoom, UIColor(UIColorID::UI_COLOR_INPUT), UIColor(UIColorID::UI_COLOR_BACKGROUND));
        window.hoveredWire->end->DrawStateless(window.CurrentTab().camera.zoom, UIColor(UIColorID::UI_COLOR_OUTPUT), UIColor(UIColorID::UI_COLOR_BACKGROUND));
    }

    if (!!nodeBeingDragged && hoveringMergable)
    {
        DrawCircleIV(nodeBeingDragged->GetPosition(), Node::g_nodeRadius * 2.0f, UIColor(UIColorID::UI_COLOR_SPECIAL));
        window.DrawTooltipAtCursor(
            "Hold [shift] to merge on release.\n"
            "Otherwise, nodes will only be swapped.", UIColor(UIColorID::UI_COLOR_SPECIAL));
    }


    // Selection
    for (Node* node : window.CurrentTab().selection)
    {
        node->DrawHighlight(window.CurrentTab().camera.zoom, UIColor(UIColorID::UI_COLOR_AVAILABLE));
        //DrawCircleIV(node->GetPosition(), node->g_nodeRadius + 3, UIColor(UIColorID::UI_COLOR_AVAILABLE));
    }
    for (Node* node : selectionPreviewNodes)
    {
        node->DrawHighlight(window.CurrentTab().camera.zoom, UIColor(UIColorID::UI_COLOR_AVAILABLE));
        //DrawCircleIV(node->GetPosition(), node->g_nodeRadius + 3, UIColor(UIColorID::UI_COLOR_AVAILABLE));
    }
}
void EditTool::DrawProperties(Window& window)
{
    // Selection stats
    if (window.SelectionExists() && window.CurrentTab().SelectionSize() > 1)
        window.PushPropertySection_Selection("Selection", window.CurrentTab().selection);
    else if (window.CurrentTab().SelectionSize() == 1)
    {
        _ASSERT_EXPR(!!window.CurrentTab().selection[0], L"Cannot show properties for null");
        window.PushPropertySection_Node("Selected node", window.CurrentTab().selection[0]);
    }
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
void EraseTool::Update(Window& window, bool allowHover)
{
    auto UpdateHovered = [&window]()
    {
        window.hoveredGroup = nullptr;
        window.hoveredWire = nullptr;
        window.hoveredNode = window.CurrentTab().graph->FindNodeAtPos(window.cursorPos);
        if (!window.hoveredNode)
        {
            window.hoveredWire = window.CurrentTab().graph->FindWireAtPos(window.cursorPos);
            if (!window.hoveredWire)
                window.hoveredGroup = window.CurrentTab().graph->FindGroupAtPos(window.cursorPos);
        }
    };

    if (!allowHover)
    {
        window.hoveredWire = nullptr;
        window.hoveredNode = nullptr;
        window.hoveredGroup = nullptr;
    }
    else if (window.b_cursorMoved)
        UpdateHovered();

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        bool shift = (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT));
        bool alt = (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT));

        // Note that shift has been made to block regular erasing. This is to prevent the continuous,
        // destructive erasing from ocurring when the user has bypassed a node.

        // Erase node
        if (!!window.hoveredNode)
        {
            // Special erase
            if (window.hoveredNode->IsSpecialErasable() && shift)
                window.CurrentTab().graph->BypassNode(window.hoveredNode);

            // Complex bipass
            else if (window.hoveredNode->IsComplexBipassable() && shift && alt)
                window.CurrentTab().graph->BypassNode_Complex(window.hoveredNode);

            // Regular erase
            else if (!shift)
                window.CurrentTab().graph->DestroyNode(window.hoveredNode);
        }

        // Erase wire
        else if (!!window.hoveredWire && !shift)
            window.CurrentTab().graph->DestroyWire(window.hoveredWire);

        // Erase group
        else if (!!window.hoveredGroup && !shift)
            window.CurrentTab().graph->DestroyGroup(window.hoveredGroup);

        if (allowHover)
            UpdateHovered();
        else
        {
            window.hoveredWire = nullptr;
            window.hoveredNode = nullptr;
            window.hoveredGroup = nullptr;
        }
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

    window.CurrentTab().graph->DrawNodes(window.CurrentTab().camera.zoom, UIColor(UIColorID::UI_COLOR_ACTIVE), UIColor(UIColorID::UI_COLOR_FOREGROUND));

    if (!!window.hoveredNode)
    {
        window.hoveredNode->DrawStateless(window.CurrentTab().camera.zoom, UIColor(UIColorID::UI_COLOR_ERROR), UIColor(UIColorID::UI_COLOR_BACKGROUND));
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
void InteractTool::Update(Window& window, bool allowHover)
{
    if (!allowHover)
    {
        window.hoveredWire = nullptr;
        window.hoveredNode = nullptr;
        window.hoveredGroup = nullptr;
    }
    else
        window.hoveredNode = window.CurrentTab().graph->FindNodeAtPos(window.cursorPos);

    if (!!window.hoveredNode && window.hoveredNode->IsInteractive() && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        window.hoveredNode->SetGate(window.hoveredNode->GetGate() == Gate::NOR ? Gate::OR : Gate::NOR);

    
}
void InteractTool::Draw(Window& window)
{
    window.CurrentTab().graph->DrawWires(UIColor(UIColorID::UI_COLOR_ACTIVE), UIColor(UIColorID::UI_COLOR_FOREGROUND3));
    window.CurrentTab().graph->DrawNodes(window.CurrentTab().camera.zoom, UIColor(UIColorID::UI_COLOR_ACTIVE), UIColor(UIColorID::UI_COLOR_FOREGROUND));

    // The set of all start nodes contains the set of all interactive nodes
    // The set of all interactive nodes does not contain all start nodes
    for (const Node* node : window.CurrentTab().graph->GetStartNodes())
    {
        if (node->IsInteractive())
            node->DrawStateless(window.CurrentTab().camera.zoom, UIColor(UIColorID::UI_COLOR_AVAILABLE), UIColor(UIColorID::UI_COLOR_BACKGROUND));
    }

    if (!!window.hoveredNode && window.hoveredNode->IsInteractive())
    {
        window.hoveredNode->DrawStateless(window.CurrentTab().camera.zoom, UIColor(UIColorID::UI_COLOR_CAUTION), UIColor(UIColorID::UI_COLOR_BACKGROUND));
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
void PasteOverlay::Update(Window& window, bool allowHover)
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && allowHover)
            window.CurrentTab().graph->SpawnBlueprint(window.clipboard, window.cursorPos);
        window.ClearSelection();
        window.ClearOverlayMode();
    }
}
void PasteOverlay::Draw(Window& window)
{
    window.CurrentTab().graph->DrawWires(UIColor(UIColorID::UI_COLOR_ACTIVE), UIColor(UIColorID::UI_COLOR_FOREGROUND3));
    window.CurrentTab().graph->DrawNodes(window.CurrentTab().camera.zoom, UIColor(UIColorID::UI_COLOR_ACTIVE), UIColor(UIColorID::UI_COLOR_FOREGROUND));

    window.clipboard->DrawSelectionPreview(window.CurrentTab().camera.zoom, window.cursorPos - IVec2(g_gridSize), ColorAlpha(UIColor(UIColorID::UI_COLOR_BACKGROUND2), 0.5f), UIColor(UIColorID::UI_COLOR_FOREGROUND2), UIColor(UIColorID::UI_COLOR_BACKGROUND2), UIColor(UIColorID::UI_COLOR_FOREGROUND3), window.pastePreviewLOD);
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
void BlueprintMenu::Update(Window& window, bool allowHover)
{
    // Clicks will always be allowed in menus.
    // Maybe that's not the best idea...

    constexpr int halfGrid = g_gridSize / 2;
    if (allowHover && !window.CursorInUIBounds(hoveredRec))
    {
        IVec2 pos(0, Button::g_width);
        int maxY = 0; // I know there must be a better algorithm, but this will at least be progress.
        hovering = nullptr;
        hoveredRec = IRect(0);
        for (Blueprint* bp : window.CurrentTab().graph->GetBlueprints())
        {
            IRect rec = bp->GetSelectionPreviewRect(pos);
            if (rec.Right() > window.windowWidth)
            {
                pos = IVec2(0, maxY);
                rec = bp->GetSelectionPreviewRect(pos);
            }

            if (window.CursorInUIBounds(rec))
            {
                hovering = bp;
                hoveredRec = rec;
                break;
            }

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
            foreground = UIColor(UIColorID::UI_COLOR_FOREGROUND2);
            foregroundIO = UIColor(UIColorID::UI_COLOR_FOREGROUND1);
        }
        else [[likely]]
        {
            background = UIColor(UIColorID::UI_COLOR_BACKGROUND1);
            foreground = UIColor(UIColorID::UI_COLOR_BACKGROUND2);
            foregroundIO = UIColor(UIColorID::UI_COLOR_FOREGROUND3);
        }

        bp->DrawSelectionPreview(window.CurrentTab().camera.zoom, pos, background, foreground, foregroundIO, ColorAlpha(foreground, 0.25f), window.blueprintLOD);
        DrawRectangleLines(rec.x, rec.y, rec.w, rec.h, foreground);

        pos += rec.width;
        maxY = std::max(maxY, rec.Bottom());
    }
    if (!!hovering)
        window.DrawTooltipAtCursor_Shadowed(hovering->name.c_str(), UIColor(UIColorID::UI_COLOR_FOREGROUND));
}
void BlueprintMenu::DrawProperties(Window& window)
{
    if (!!hovering)
    {
        window.PushPropertySubtitle("Hovered blueprint");
        window.PushProperty("Name", hovering->name);
        window.PushProperty_int("Width", hovering->extents.x / g_gridSize + 1);
        window.PushProperty_int("Height", hovering->extents.y / g_gridSize + 1);
        window.PushProperty_int("Wires", hovering->wires.size());
        window.PushPropertySubtitle("Wires");
        window.PushProperty_int("\tCount", hovering->wires.size());
        window.PushPropertySubtitle("Nodes");
        window.PushProperty_int("\tCount", hovering->nodes.size());
        window.PushPropertySubtitle("Exposed nodes", UIColor(UIColorID::UI_COLOR_FOREGROUND1)); // foregroundIO
        size_t exposed = std::count_if(hovering->nodes.begin(), hovering->nodes.end(), [](const NodeBP& node_bp) { return node_bp.b_io; });
        window.PushProperty_int("\tCount", exposed);
        for (const NodeBP& node_bp : hovering->nodes)
        {
            if (!node_bp.b_io)
                continue;
            if (!node_bp.name.empty())
                window.PushProperty("\tName", node_bp.name);
            else
                window.PushProperty("\tName", "[blank]");
        }
    }
}

SettingMenu::SettingMenu() {}
SettingMenu::~SettingMenu() {}
ModeType SettingMenu::GetModeType() const
{
    return ModeType::Menu;
}
Mode SettingMenu::GetMode() const
{
    return Mode::SETTINGS;
}
void SettingMenu::Update(Window& window, bool allowHover)
{

}
void SettingMenu::Draw(Window& window)
{
    DrawText("Settings menu [WIP]", 0, 0, window.FontSize(), MAGENTA);
}
void SettingMenu::DrawProperties(Window& window)
{

}
