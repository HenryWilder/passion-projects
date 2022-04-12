#include <raylib.h>
#include "HUtility.h"
#include "IVec.h"
#include "Node.h"
#include "Wire.h"
#include "Blueprint.h"
#include "Group.h"
#include "NodeWorld.h"

int main()
{
    int windowWidth = 1280;
    int windowHeight = 720;
    InitWindow(windowWidth, windowHeight, "Electron Architect");
    SetExitKey(0);
    constexpr int targetFPS = 120;
    SetTargetFPS(120);

    /******************************************
    *   Load textures, shaders, and meshes
    ******************************************/

    enum class Mode {
        PEN,
        EDIT,
        ERASE,
        INTERACT,

        GATE,
        BUTTON,
        PASTE,
        BP_ICON,
    } mode, baseMode;

    Texture2D clipboardIcon = LoadTexture("icon_clipboard.png");

    Texture2D modeIcons = LoadTexture("icons_mode.png");
    auto DrawModeIcon = [&modeIcons](Mode mode, IVec2 pos, Color tint)
    {
        IVec2 offset;
        switch (mode)
        {
        case Mode::PEN:      offset = IVec2(0, 0); break;
        case Mode::EDIT:     offset = IVec2(1, 0); break;
        case Mode::ERASE:    offset = IVec2(0, 1); break;
        case Mode::INTERACT: offset = IVec2(1, 1); break;
        default: return;
        }
        DrawIcon<16>(modeIcons, offset, pos, tint);
    };

    Texture2D gateIcons16x = LoadTexture("icons_gate16x.png");
    auto DrawGateIcon16x = [&gateIcons16x](Gate gate, IVec2 pos, Color tint)
    {
        IVec2 offset;
        switch (gate)
        {
        case Gate::OR:  offset = IVec2(0, 0); break;
        case Gate::AND: offset = IVec2(1, 0); break;
        case Gate::NOR: offset = IVec2(0, 1); break;
        case Gate::XOR: offset = IVec2(1, 1); break;

        case Gate::RESISTOR:  offset = IVec2(0, 2); break;
        case Gate::CAPACITOR: offset = IVec2(1, 2); break;
        default: return;
        }
        DrawIcon<16>(gateIcons16x, offset, pos, tint);
    };

    Texture2D gateIcons32x = LoadTexture("icons_gate32x.png");
    auto DrawGateIcon32x = [&gateIcons32x](Gate gate, IVec2 pos, Color tint)
    {
        IVec2 offset;
        switch (gate)
        {
        case Gate::OR:  offset = IVec2(0, 0); break;
        case Gate::AND: offset = IVec2(1, 0); break;
        case Gate::NOR: offset = IVec2(0, 1); break;
        case Gate::XOR: offset = IVec2(1, 1); break;

        case Gate::RESISTOR:  offset = IVec2(0, 2); break;
        case Gate::CAPACITOR: offset = IVec2(1, 2); break;
        default: return;
        }
        DrawIcon<32>(gateIcons32x, offset, pos, tint);
    };

    BlueprintIcon::Load("icons_blueprint.png");

    struct {
        Gate gatePick = Gate::OR;
        Gate lastGate = Gate::OR;
        union { // Alternate names for the same value
            uint8_t storedResistance;
            uint8_t storedCapacity;
            uint8_t storedExtendedParam = 0;
        };
        Node* hoveredNode = nullptr;
        Wire* hoveredWire = nullptr;
        Blueprint* clipboard = nullptr;
        std::vector<Node*> selection;

        union // Base mode
        {
            struct {
                Node* currentWireStart;
                ElbowConfig currentWireElbowConfig;
            } pen;

            struct {
                IVec2 fallbackPos;
                bool selectionWIP;
                IVec2 selectionStart;
                IRect selectionRec;
                bool draggingGroup;
                Group* hoveredGroup;
                Node* nodeBeingDragged;
                Wire* wireBeingDragged;
            } edit;

            struct {
            } erase;

            struct {
            } interact = {};
        };
        union // Overlay mode - doesn't affect other modes
        {
            struct {
                IVec2 radialMenuCenter;
                uint8_t overlappedSection;
            } gate;

            struct {
                int dropdownActive;
            } button;

            struct {
            } paste = {};

            struct {
                BlueprintIcon* object;
                IVec2 pos; // Width and height are fixed
                IRect sheetRec;
                BlueprintIconID_t iconID;
                uint8_t iconCount;
                int draggingIcon; // -1 for none/not dragging
            } bp_icon;
        };
    } data;

    constexpr Mode dropdownModeOrder[] = {
        Mode::PEN,
        Mode::EDIT,
        Mode::ERASE,
        Mode::INTERACT,
    };
    constexpr Gate dropdownGateOrder[] = {
        Gate::OR,
        Gate::AND,
        Gate::NOR,
        Gate::XOR,

        Gate::RESISTOR,
        Gate::CAPACITOR,
    };
    constexpr IRect dropdownBounds[] = {
        IRect( 0, 16, 16, 16 * (_countof(dropdownModeOrder) - 1)), // Mode
        IRect(16, 16, 16, 16 * (_countof(dropdownGateOrder) - 1)), // Gate
        IRect(32, 16, 16, 16 * (_countof(Node::g_resistanceBands) - 1)), // Parameter
    };

    constexpr Gate radialGateOrder[] = {
        Gate::XOR,
        Gate::AND,
        Gate::OR,
        Gate::NOR,
    };

    IVec2 cursorPosPrev = IVec2::Zero(); // For checking if there was movement

    auto SetMode = [targetFPS, &baseMode, &mode, &data, &cursorPosPrev](Mode newMode)
    {
        if (mode == Mode::BP_ICON)
        {
            delete data.bp_icon.object;
            data.bp_icon.object = nullptr;
        }
        else if (newMode != mode)
        {
            if (mode == Mode::INTERACT)
            {
                SetTargetFPS(targetFPS);
            }
            if (newMode == Mode::INTERACT)
            {
                SetTargetFPS(24);
            }
        }

        cursorPosPrev = IVec2(-1,-1);
        mode = newMode;

        switch (newMode)
        {
        case Mode::PEN:
            baseMode = Mode::PEN;
            data.pen.currentWireStart = nullptr;
            data.pen.currentWireElbowConfig = (ElbowConfig)0;
            break;

        case Mode::EDIT:
            baseMode = Mode::EDIT;
            data.edit.fallbackPos = IVec2::Zero();
            data.edit.selectionWIP = false;
            data.edit.selectionStart = IVec2::Zero();
            data.edit.selectionRec = IRect(0,0,0,0);
            data.edit.draggingGroup = false;
            data.edit.hoveredGroup = nullptr;
            data.edit.nodeBeingDragged = nullptr;
            data.edit.wireBeingDragged = nullptr;
            break;

        case Mode::ERASE:
            baseMode = Mode::ERASE;
            break;

        case Mode::INTERACT:
            baseMode = Mode::INTERACT;
            break;

        case Mode::GATE:
            data.gate.radialMenuCenter = IVec2::Zero();
            data.gate.overlappedSection = 0;
            break;

        case Mode::BUTTON:
            data.button.dropdownActive = 0;
            break;

        case Mode::PASTE:
            break;

        case Mode::BP_ICON:
            data.bp_icon.object = nullptr;
            data.bp_icon.pos = IVec2::Zero();
            data.bp_icon.sheetRec = IRect(0,0,0,0);
            data.bp_icon.iconID = NULL;
            data.bp_icon.iconCount = 0;
            data.bp_icon.draggingIcon = -1;
            break;
        }
    };

    mode = Mode::PEN;
    SetMode(Mode::PEN);

    NodeWorld::Get().Load("session.cg"); // Construct and load last session
            
    while (!WindowShouldClose())
    {
        /******************************************
        *   Simulate frame and update variables
        ******************************************/

        IVec2 cursorPos{
            GetMouseX() / g_gridSize,
            GetMouseY() / g_gridSize
        };
        cursorPos = cursorPos * g_gridSize + IVec2(g_gridSize / 2);

        bool b_cursorMoved = cursorPosPrev != cursorPos;

        // Hotkeys
        {
            if (mode == baseMode || mode == Mode::GATE)
            {
                if (IsKeyPressed(KEY_ONE))
                    data.gatePick = Gate::OR;
                else if (IsKeyPressed(KEY_TWO))
                    data.gatePick = Gate::AND;
                else if (IsKeyPressed(KEY_THREE))
                    data.gatePick = Gate::NOR;
                else if (IsKeyPressed(KEY_FOUR))
                    data.gatePick = Gate::XOR;
                else if (IsKeyPressed(KEY_FIVE))
                    data.gatePick = Gate::RESISTOR;
                else if (IsKeyPressed(KEY_SIX))
                    data.gatePick = Gate::CAPACITOR;
            }

            // KEY COMBOS BEFORE INDIVIDUAL KEYS!
            
            // Ctrl
            if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))
            {
                // Copy
                if (IsKeyPressed(KEY_C) && mode == Mode::EDIT)
                {
                    if (data.clipboard != nullptr)
                        delete data.clipboard;

                    if (data.selection.empty())
                        data.clipboard = nullptr;
                    else
                        data.clipboard = new Blueprint(data.selection);
                }
                // Paste
                else if (IsKeyPressed(KEY_V) && !!data.clipboard)
                {
                    SetMode(Mode::PASTE);
                }
                // Group
                else if (IsKeyPressed(KEY_G) && mode == Mode::EDIT && !data.edit.selectionWIP &&
                    !(data.edit.selectionRec.w == 0 || data.edit.selectionRec.h == 0))
                {
                    NodeWorld::Get().CreateGroup(data.edit.selectionRec);
                    data.edit.selectionRec = IRect(0,0,0,0);
                    data.selection.clear();
                }
                // Save
                else if (IsKeyPressed(KEY_S))
                {
                    // Save blueprint
                    if (mode == Mode::PASTE)
                    {
                        SetMode(Mode::BP_ICON);
                        data.bp_icon.object = new BlueprintIcon;
                        data.bp_icon.pos = cursorPos - IVec2(BlueprintIcon::g_size / 2, BlueprintIcon::g_size / 2);
                        data.bp_icon.sheetRec.xy = data.bp_icon.pos + IVec2(BlueprintIcon::g_size * 2, BlueprintIcon::g_size * 2);
                        data.bp_icon.sheetRec.wh = BlueprintIcon::GetSheetSize_Px();
                    }
                    // Save file
                    else
                    {
                        NodeWorld::Get().Save("save.cg");
                    }
                }
            }
            else
            {
                if (IsKeyPressed(KEY_ESCAPE))
                {
                    if (mode != baseMode)
                        SetMode(baseMode);
                    else if (!!data.clipboard)
                    {
                        delete data.clipboard;
                        data.clipboard = nullptr;
                    }
                }
                else if (IsKeyPressed(KEY_B))
                {
                    SetMode(Mode::PEN);
                }
                else if (IsKeyPressed(KEY_V))
                {
                    SetMode(Mode::EDIT);
                }
                else if (IsKeyPressed(KEY_G))
                {
                    SetMode(Mode::GATE);
                    data.gate.radialMenuCenter = cursorPos;
                }
                else if (IsKeyPressed(KEY_X))
                {
                    SetMode(Mode::ERASE);
                }
                else if (IsKeyPressed(KEY_F))
                {
                    SetMode(Mode::INTERACT);
                }
                else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && mode != Mode::BP_ICON && (cursorPos.y <= 16 && cursorPos.x <= 48) &&
                    (mode == Mode::BUTTON ? data.button.dropdownActive != (cursorPos.x / 16) : true))
                {
                    SetMode(Mode::BUTTON);
                    data.button.dropdownActive = cursorPos.x / 16;
                    goto EVAL;
                }
            }
        }

        // Input
        switch (mode)
        {

        // Basic

        case Mode::PEN:
        {
            if (b_cursorMoved)
            {
                data.hoveredWire = nullptr;
                data.hoveredNode = NodeWorld::Get().FindNodeAtPos(cursorPos);
                if (!data.hoveredNode)
                    data.hoveredWire = NodeWorld::Get().FindWireAtPos(cursorPos);
            }

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                Node* newNode = data.hoveredNode;
                if (!newNode)
                {
                    newNode = NodeWorld::Get().CreateNode(cursorPos, data.gatePick, data.storedExtendedParam);
                    if (!!data.hoveredWire)
                    {
                        NodeWorld::Get().BisectWire(data.hoveredWire, newNode);
                        data.hoveredWire = nullptr;
                    }
                }
                // Do not create a new node/wire if already hovering the start node
                if (!!data.pen.currentWireStart && newNode != data.pen.currentWireStart)
                {
                    Wire* wire = NodeWorld::Get().CreateWire(data.pen.currentWireStart, newNode);
                    wire->elbowConfig = data.pen.currentWireElbowConfig;
                    wire->UpdateElbowToLegal();
                }
                data.pen.currentWireStart = newNode;
            }
            else if (IsKeyPressed(KEY_R))
            {
                if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
                    --data.pen.currentWireElbowConfig;
                else
                    ++data.pen.currentWireElbowConfig;
            }
            else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
            {
                data.pen.currentWireStart = nullptr;
            }
        }
        break;

        case Mode::EDIT:
        {
            if (b_cursorMoved)
            {
                if (!data.edit.nodeBeingDragged &&
                    !data.edit.wireBeingDragged &&
                    !data.edit.draggingGroup)
                {
                    data.edit.hoveredGroup = nullptr;
                    data.hoveredWire = nullptr;
                    data.hoveredNode = NodeWorld::Get().FindNodeAtPos(cursorPos);
                    if (!data.hoveredNode)
                    {
                        data.hoveredWire = NodeWorld::Get().FindWireElbowAtPos(cursorPos);
                        if (!data.hoveredWire)
                        {
                            data.edit.hoveredGroup = NodeWorld::Get().FindGroupAtPos(cursorPos);
                        }
                    }
                }
            }

            // Press
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                data.selection.clear();
                data.edit.nodeBeingDragged = data.hoveredNode;
                data.edit.wireBeingDragged = data.hoveredWire;

                // selectionStart being used as an offset here
                if (data.edit.draggingGroup = !!data.edit.hoveredGroup)
                {
                    NodeWorld::Get().FindNodesInGroup(data.selection, data.edit.hoveredGroup);
                    data.edit.selectionStart = (cursorPos - (data.edit.fallbackPos = data.edit.hoveredGroup->GetPosition()));
                }

                if (data.edit.selectionWIP = !(data.edit.nodeBeingDragged || data.edit.wireBeingDragged || data.edit.draggingGroup))
                    data.edit.selectionStart = data.edit.fallbackPos = cursorPos;
            }

            // Selection
            if (data.edit.selectionWIP)
            {
                auto [minx, maxx] = std::minmax(cursorPos.x, data.edit.selectionStart.x);
                auto [miny, maxy] = std::minmax(cursorPos.y, data.edit.selectionStart.y);
                data.edit.selectionRec.w = maxx - (data.edit.selectionRec.x = minx);
                data.edit.selectionRec.h = maxy - (data.edit.selectionRec.y = miny);
            }
            // Node
            else if (!!data.edit.nodeBeingDragged)
            {
                data.edit.nodeBeingDragged->SetPosition_Temporary(cursorPos);
            }
            // Wire
            else if (!!data.edit.wireBeingDragged)
            {
                data.edit.wireBeingDragged->SnapElbowToLegal(cursorPos);
            }
            // Group
            else if (data.edit.draggingGroup)
            {
                data.edit.hoveredGroup->SetPosition(cursorPos - data.edit.selectionStart);
                for (Node* node : data.selection)
                {
                    IVec2 offset = cursorPos - cursorPosPrev;
                    node->SetPosition_Temporary(node->GetPosition() + offset);
                }
            }

            // Release
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) || (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)))
            {
                // Cancel
                if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
                {
                    if (!!data.edit.nodeBeingDragged)
                    {
                        data.edit.nodeBeingDragged->SetPosition(data.edit.fallbackPos);
                    }
                    else if (data.edit.draggingGroup)
                    {
                        data.edit.hoveredGroup->SetPosition(data.edit.fallbackPos);
                        for (Node* node : data.selection)
                        {
                            IVec2 offset = (data.edit.fallbackPos + data.edit.selectionStart) - cursorPos;
                            node->SetPosition_Temporary(node->GetPosition() + offset);
                        }
                    }
                    else if (data.edit.selectionWIP)
                    {
                        data.edit.selectionRec = IRect(0,0,0,0);
                    }
                }
                // Finalize
                else
                {
                    if (!!data.edit.nodeBeingDragged)
                    {
                        data.hoveredNode = NodeWorld::Get().FindNodeAtPos(cursorPos);
                        if (!!data.hoveredNode && data.edit.nodeBeingDragged != data.hoveredNode)
                            data.hoveredNode = data.edit.nodeBeingDragged = NodeWorld::Get().MergeNodes(data.edit.nodeBeingDragged, data.hoveredNode);

                        data.edit.nodeBeingDragged->SetPosition(cursorPos);
                    }
                    else if (data.edit.draggingGroup)
                    {
                        for (Node* node : data.selection)
                        {
                            node->SetPosition(node->GetPosition());
                        }
                    }
                    else if (data.edit.selectionWIP)
                    {
                        NodeWorld::Get().FindNodesInRect(data.selection, data.edit.selectionRec);
                    }
                }
                if (data.edit.draggingGroup)
                    data.selection.clear();
                data.edit.nodeBeingDragged = nullptr;
                data.edit.selectionWIP = false;
                data.edit.draggingGroup = false;
                data.edit.wireBeingDragged = nullptr;
            }
            // Right click
            else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && !!data.hoveredNode)
            {
                data.hoveredNode->SetGate(data.gatePick);
                if (data.hoveredNode->GetGate() == Gate::RESISTOR)
                    data.hoveredNode->SetResistance(data.storedResistance);
                else if (data.hoveredNode->GetGate() == Gate::CAPACITOR)
                    data.hoveredNode->SetCapacity(data.storedCapacity);
            }
        }
        break;

        case Mode::ERASE:
        {
            if (b_cursorMoved)
            {
                data.hoveredWire = nullptr;
                data.hoveredNode = NodeWorld::Get().FindNodeAtPos(cursorPos);
                if (!data.hoveredNode)
                    data.hoveredWire = NodeWorld::Get().FindWireAtPos(cursorPos);
            }

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                if (!!data.hoveredNode)
                    NodeWorld::Get().DestroyNode(data.hoveredNode);
                else if (!!data.hoveredWire)
                    NodeWorld::Get().DestroyWire(data.hoveredWire);

                data.hoveredNode = nullptr;
                data.hoveredWire = nullptr;
            }
        }
        break;

        case Mode::INTERACT:
        {
            data.hoveredNode = NodeWorld::Get().FindNodeAtPos(cursorPos);
            if (!!data.hoveredNode && !data.hoveredNode->IsOutputOnly())
                data.hoveredNode = nullptr;

            if (!!data.hoveredNode && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                data.hoveredNode->SetGate(data.hoveredNode->GetGate() == Gate::NOR ? Gate::OR : Gate::NOR);
        }
        break;


        // Overlay

        case Mode::GATE:
        {
            if (b_cursorMoved)
            {
                if (cursorPos.x < data.gate.radialMenuCenter.x)
                {
                    if (cursorPos.y < data.gate.radialMenuCenter.y)
                        data.gate.overlappedSection = 2;
                    else // cursorPos.y > data.gate.radialMenuCenter.y
                        data.gate.overlappedSection = 3;
                }
                else // cursorPos.x > data.gate.radialMenuCenter.x
                {
                    if (cursorPos.y < data.gate.radialMenuCenter.y)
                        data.gate.overlappedSection = 1;
                    else // cursorPos.y > data.gate.radialMenuCenter.y
                        data.gate.overlappedSection = 0;
                }
            }

            bool leftMouse = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
            if (leftMouse || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
            {
                if (leftMouse)
                    data.gatePick = radialGateOrder[data.gate.overlappedSection];

                mode = baseMode;
                SetMousePosition(data.gate.radialMenuCenter.x, data.gate.radialMenuCenter.y);
                cursorPos = data.gate.radialMenuCenter;
            }
        }
        break;

        case Mode::BUTTON:
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                IRect rec = dropdownBounds[data.button.dropdownActive];
                if (InBoundingBox(rec, cursorPos))
                {
                    rec.h = 16;

                    switch (data.button.dropdownActive)
                    {
                    case 0: // Mode
                    {
                        for (Mode m : dropdownModeOrder)
                        {
                            if (m == baseMode)
                                continue;

                            if (InBoundingBox(rec, cursorPos))
                            {
                                SetMode(m);
                                break;
                            }

                            rec.y += 16;
                        }
                    }
                    break;

                    case 1: // Gate
                    {
                        for (Gate g : dropdownGateOrder)
                        {
                            if (g == data.gatePick)
                                continue;

                            if (InBoundingBox(rec, cursorPos))
                            {
                                data.gatePick = g;
                                break;
                            }

                            rec.y += 16;
                        }

                        SetMode(baseMode);
                    }
                    break;

                    case 2: // Resistance
                    {
                        for (uint8_t v = 0; v < 10; ++v)
                        {
                            if (v == data.storedExtendedParam)
                                continue;

                            if (InBoundingBox(rec, cursorPos))
                            {
                                data.storedExtendedParam = v;
                                break;
                            }

                            rec.y += 16;
                        }

                        SetMode(baseMode);
                    }
                    break;
                    }
                }
                else
                    SetMode(baseMode);
            }
            else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
            {
                SetMode(baseMode);
            }
        }
        break;

        case Mode::PASTE:
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
            {
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                {
                    IVec2 pos = cursorPos - data.clipboard->extents / 2;
                    pos /= g_gridSize;
                    pos *= g_gridSize;
                    pos = pos + IVec2(g_gridSize / 2);
                    NodeWorld::Get().SpawnBlueprint(data.clipboard, pos);
                }
                data.selection.clear();
                SetMode(baseMode);
            }
        }
        break;

        case Mode::BP_ICON:
        {
            if (b_cursorMoved && data.bp_icon.draggingIcon == -1)
            {
                data.bp_icon.iconID = BlueprintIcon::GetIconAtColRow(BlueprintIcon::PixelToColRow(data.bp_icon.sheetRec.xy, cursorPos));
            }

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                if (InBoundingBox(data.bp_icon.sheetRec, cursorPos) && data.bp_icon.iconCount < 4 && !!data.bp_icon.iconID)
                {
                    cursorPos = data.bp_icon.pos;
                    SetMousePosition(cursorPos.x + BlueprintIcon::g_size / 2, cursorPos.y + BlueprintIcon::g_size / 2);
                    data.bp_icon.object->combo[data.bp_icon.iconCount] = { data.bp_icon.iconID, 0,0 };
                    data.bp_icon.draggingIcon = data.bp_icon.iconCount;
                    data.bp_icon.iconCount++;
                }
                else if (InBoundingBox(IRect(data.bp_icon.pos.x, data.bp_icon.pos.y, BlueprintIcon::g_size * 2, BlueprintIcon::g_size * 2), cursorPos))
                {
                    data.bp_icon.draggingIcon = -1;
                    for (decltype(data.bp_icon.draggingIcon) i = 0; i < data.bp_icon.iconCount; ++i)
                    {
                        if (data.bp_icon.object->combo[i].id == NULL)
                            continue;

                        IRect bounds(
                            data.bp_icon.pos.x,
                            data.bp_icon.pos.y,
                            BlueprintIcon::g_size,
                            BlueprintIcon::g_size
                        );
                        bounds.xy = bounds.xy + data.bp_icon.object->combo[i].Pos();
                        if (InBoundingBox(bounds, cursorPos))
                        {
                            data.bp_icon.draggingIcon = i;
				            data.bp_icon.iconID = data.bp_icon.object->combo[i].id;
                            break;
                        }
                    }
                }
            }
            if ((IsMouseButtonReleased(MOUSE_LEFT_BUTTON) || IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) && data.bp_icon.draggingIcon != -1)
            {
                if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
                {
                    if (data.bp_icon.draggingIcon < 3)
                    {
                        memcpy(
                            data.bp_icon.object->combo + data.bp_icon.draggingIcon,
                            data.bp_icon.object->combo + data.bp_icon.draggingIcon + 1,
                            sizeof(IconPos) * (4ull - (size_t)data.bp_icon.draggingIcon));
                    }
                    data.bp_icon.object->combo[3] = { NULL, 0,0 };
                    data.bp_icon.iconCount--;
                }
                data.bp_icon.draggingIcon = -1;
            }
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && !!data.bp_icon.iconID)
            {
                constexpr IVec2 centerOffset = IVec2(BlueprintIcon::g_size / 2);
                IVec2 colRow = (cursorPos - data.bp_icon.pos - centerOffset) / centerOffset;
                colRow.x = std::min(std::max(colRow.x, 0), 2);
                colRow.y = std::min(std::max(colRow.y, 0), 2);
                data.bp_icon.object->combo[data.bp_icon.draggingIcon].x = colRow.x;
                data.bp_icon.object->combo[data.bp_icon.draggingIcon].y = colRow.y;
            }
        }
        break;
        }

    EVAL:
        cursorPosPrev = cursorPos;
        NodeWorld::Get().Evaluate();

        /******************************************
        *   Draw the frame
        ******************************************/

        BeginDrawing(); {

            ClearBackground(BLACK);

            if (mode == Mode::BP_ICON)
            {
                _ASSERT_EXPR(!!data.bp_icon.object, L"Blueprint icon object not initialized");
                _ASSERT_EXPR(!!data.clipboard, L"Bad entry into Mode::BP_ICON");

                data.bp_icon.object->DrawBackground(data.bp_icon.pos, SPACEGRAY);
                data.bp_icon.object->Draw(data.bp_icon.pos, WHITE);

                for (size_t i = 0; i < 4; ++i)
                {
                    for (decltype(data.bp_icon.draggingIcon) i = 0; i < 4; ++i)
                    {
                        if (data.bp_icon.object->combo[i].id == NULL)
                            continue;

                        IRect bounds(
                            data.bp_icon.pos.x,
                            data.bp_icon.pos.y,
                            BlueprintIcon::g_size,
                            BlueprintIcon::g_size
                        );
                        bounds.xy = bounds.xy + data.bp_icon.object->combo[i].Pos();
                        if (InBoundingBox(bounds, cursorPos))
                        {
                            DrawRectangleIRect(bounds, ColorAlpha(WIPBLUE, 0.25f));
                        }
                    }
                }
                if (data.bp_icon.draggingIcon != -1)
                    BlueprintIcon::DrawBPIcon(data.bp_icon.iconID, data.bp_icon.pos + data.bp_icon.object->combo[data.bp_icon.draggingIcon].Pos(), WIPBLUE);

                BlueprintIcon::DrawSheet(data.bp_icon.sheetRec.xy, SPACEGRAY, WHITE);
            }
            else
            {
                // Grid
                for (int y = 0; y < windowHeight; y += g_gridSize)
                {
                    DrawLine(0, y, windowWidth, y, SPACEGRAY);
                }
                for (int x = 0; x < windowWidth; x += g_gridSize)
                {
                    DrawLine(x, 0, x, windowHeight, SPACEGRAY);
                }

                NodeWorld::Get().DrawGroups();

                // Draw
                switch (mode)
                {

                // Base mode

                case Mode::PEN:
                {
                    NodeWorld::Get().DrawWires();

                    if (!!data.pen.currentWireStart)
                    {
                        IVec2 start = data.pen.currentWireStart->GetPosition();
                        IVec2 elbow;
                        IVec2 end = cursorPos;
                        elbow = Wire::GetLegalElbowPosition(start, end, data.pen.currentWireElbowConfig);
                        Wire::Draw(start, elbow, end, WIPBLUE);
                        Node::Draw(end, data.gatePick, WIPBLUE);
                    }

                    if (!!data.hoveredWire)
                    {
                        data.hoveredWire->Draw(GOLD);
                    }

                    if (!!data.hoveredNode)
                    {
                        for (const Wire* wire : data.hoveredNode->GetWires())
                        {
                            Color color;
                            if (wire->start == data.hoveredNode)
                                color = OUTPUTAPRICOT; // Output
                            else
                                color = INPUTLAVENDER; // Input

                            wire->Draw(color);
                        }
                    }

                    NodeWorld::Get().DrawNodes();

                    if (!!data.hoveredNode)
                    {
                        data.hoveredNode->Draw(WIPBLUE);
                    }
                }
                break;

                case Mode::EDIT:
                {
                    if (!!data.edit.hoveredGroup)
                        data.edit.hoveredGroup->Highlight(INTERFERENCEGRAY);

                    DrawRectangleIRect(data.edit.selectionRec, ColorAlpha(SPACEGRAY, 0.5));
                    DrawRectangleLines(data.edit.selectionRec.x, data.edit.selectionRec.y, data.edit.selectionRec.w, data.edit.selectionRec.h, LIFELESSNEBULA);

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
                        if (!!data.edit.wireBeingDragged)
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
                }
                break;

                case Mode::ERASE:
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
                        DrawCross(cursorPos, DESTRUCTIVERED);
                    }
                    else if (!!data.hoveredNode)
                    {
                        for (Wire* wire : data.hoveredNode->GetWires())
                        {
                            wire->Draw(MAGENTA);
                        }
                    }

                    NodeWorld::Get().DrawNodes();

                    if (!!data.hoveredNode)
                    {
                        data.hoveredNode->Draw(BLACK);
                        DrawCross(data.hoveredNode->GetPosition(), DESTRUCTIVERED);
                    }
                }
                break;

                case Mode::INTERACT:
                {
                    NodeWorld::Get().DrawWires();
                    NodeWorld::Get().DrawNodes();

                    for (const Node* node : NodeWorld::Get().GetStartNodes())
                    {
                        node->Draw(WIPBLUE);
                    }

                    if (!!data.hoveredNode)
                    {
                        data.hoveredNode->Draw(CAUTIONYELLOW);
                    }
                }
                break;


                // Overlay mode

                case Mode::GATE:
                {
                    if (baseMode == Mode::PEN)
                    {
                        NodeWorld::Get().DrawWires();

                        if (!!data.pen.currentWireStart)
                        {
                            IVec2 start = data.pen.currentWireStart->GetPosition();
                            IVec2 elbow;
                            IVec2 end = data.gate.radialMenuCenter;
                            elbow = Wire::GetLegalElbowPosition(start, end, data.pen.currentWireElbowConfig);
                            Wire::Draw(start, elbow, end, WIPBLUE);
                            Node::Draw(end, data.gatePick, WIPBLUE);
                        }

                        NodeWorld::Get().DrawNodes();
                    }
                    else
                    {
                        NodeWorld::Get().DrawWires();
                        NodeWorld::Get().DrawNodes();
                    }

                    constexpr int menuRadius = 64;
                    constexpr int menuIconOffset = 12;
                    constexpr IVec2 menuOff[4] = {
                        IVec2(0, 0),
                        IVec2(0,-1),
                        IVec2(-1,-1),
                        IVec2(-1, 0),
                    };
                    constexpr IRect iconDest[4] = {
                        IRect(menuIconOffset,       menuIconOffset,      32, 32),
                        IRect(menuIconOffset,      -menuIconOffset - 32, 32, 32),
                        IRect(-menuIconOffset - 32, -menuIconOffset - 32, 32, 32),
                        IRect(-menuIconOffset - 32,  menuIconOffset,      32, 32),
                    };
                    int x = data.gate.radialMenuCenter.x;
                    int y = data.gate.radialMenuCenter.y;
                    constexpr Color menuBackground = SPACEGRAY;
                    DrawCircleIV(data.gate.radialMenuCenter, static_cast<float>(menuRadius + 4), menuBackground);

                    for (int i = 0; i < 4; ++i)
                    {
                        Color colorA;
                        Color colorB;
                        int radius;

                        if (i == data.gate.overlappedSection) // Active
                        {
                            colorA = GLEEFULDUST;
                            colorB = INTERFERENCEGRAY;
                            radius = menuRadius + 8;
                        }
                        else // Inactive
                        {
                            colorA = LIFELESSNEBULA;
                            colorB = HAUNTINGWHITE;
                            radius = menuRadius;
                        }
                        BeginScissorMode(x + (menuOff[i].x * 8 + 4) + menuOff[i].x * radius, y + (menuOff[i].y * 8 + 4) + menuOff[i].y * radius, radius, radius);

                        float startAngle = static_cast<float>(i * 90);
                        DrawCircleSector({ static_cast<float>(x), static_cast<float>(y) }, static_cast<float>(radius), startAngle, startAngle + 90.0f, 8, colorA);
                        IVec2 rec = iconDest[i].xy;
                        rec.x += x;
                        rec.y += y;
                        DrawGateIcon32x(radialGateOrder[i], rec, colorB);

                        EndScissorMode();
                    }
                }
                break;

                case Mode::BUTTON:
                {
                    NodeWorld::Get().DrawWires();
                    NodeWorld::Get().DrawNodes();

                    IRect rec = dropdownBounds[data.button.dropdownActive];
                    DrawRectangleIRect(dropdownBounds[data.button.dropdownActive], SPACEGRAY);
                    rec.h = 16;

                    switch (data.button.dropdownActive)
                    {
                    case 0: // Mode
                    {
                        for (Mode m : dropdownModeOrder)
                        {
                            if (m == baseMode)
                                continue;
                            Color color = InBoundingBox(rec, cursorPos) ? WHITE : DEADCABLE;
                            DrawModeIcon(m, rec.xy, color);
                            rec.y += 16;
                        }
                    }
                    break;

                    case 1: // Gate
                    {
                        for (Gate g : dropdownGateOrder)
                        {
                            if (g == data.gatePick)
                                continue;
                            Color color = InBoundingBox(rec, cursorPos) ? WHITE : GRAY;
                            DrawGateIcon16x(g, rec.xy, color);
                            rec.y += 16;
                        }
                    }
                    break;

                    case 2: // Resistance
                    {
                        for (uint8_t v = 0; v < 10; ++v)
                        {
                            _ASSERT_EXPR(v < _countof(Node::g_resistanceBands), L"Resistance out of bounds");
                            if (v == data.storedExtendedParam)
                                continue;
                            Color color = Node::g_resistanceBands[v];
                            if (InBoundingBox(rec, cursorPos))
                            {
                                DrawRectangleIRect(rec, WIPBLUE);
                                IRect smaller = rec;
                                smaller.x += 2;
                                smaller.y += 2;
                                smaller.w -= 4;
                                smaller.h -= 4;
                                DrawRectangleIRect(smaller, color);
                            }
                            else
                                DrawRectangleIRect(rec, color);
                            rec.y += 16;
                        }
                    }
                    break;
                    }
                }
                break;

                case Mode::PASTE:
                {
                    NodeWorld::Get().DrawWires();
                    NodeWorld::Get().DrawNodes();

                    IVec2 pos = cursorPos - data.clipboard->extents / 2;
                    pos *= g_gridSize;
                    pos /= g_gridSize;
                    pos = pos + IVec2(g_gridSize / 2, g_gridSize / 2);
                    data.clipboard->DrawPreview(pos, ColorAlpha(LIFELESSNEBULA, 0.5f), HAUNTINGWHITE);
                }
                break;

                case Mode::BP_ICON:
                {
                    _ASSERT_EXPR(false, L"Henry made a mistake.");
                }
                break;
                }

                // Global UI

                // Buttons
                if (cursorPos.y <= 16)
                {
                    if (cursorPos.x <= 16)
                    {
                        const char* text;
                        switch (baseMode)
                        {
                        case Mode::PEN:   text = "Mode: Draw";        break;
                        case Mode::EDIT:  text = "Mode: Edit";        break;
                        case Mode::GATE:  text = "Mode: Gate select"; break;
                        case Mode::ERASE: text = "Mode: Erase";       break;
                        default: _ASSERT_EXPR(false, L"Missing tooltip for selected mode");
                                          text = "";                  break;
                        }
                        DrawText(text, 20, 17, 8, WHITE);
                        DrawRectangleIRect(IRect(0, 0, 16), SPACEGRAY);
                    }
                    else if (cursorPos.x <= 32)
                    {
                        const char* text;
                        switch (data.gatePick)
                        {
                        case Gate::OR:        text = "Gate: | (or)";         break;
                        case Gate::AND:       text = "Gate: & (and)";        break;
                        case Gate::NOR:       text = "Gate: ! (nor)";        break;
                        case Gate::XOR:       text = "Gate: ^ (xor)";        break;
                        case Gate::RESISTOR:  text = "Component: Resistor";  break;
                        case Gate::CAPACITOR: text = "Component: Capacitor"; break;
                        default: _ASSERT_EXPR(false, L"Missing tooltip for selected gate");
                                              text = "";                     break;
                        }
                        DrawText(text, 36, 17, 8, WHITE);
                        DrawRectangleIRect((IRect(16, 0, 16)), SPACEGRAY);
                    }
                    else if (cursorPos.x <= 48)
                    {
                        const char* text;
                        if (data.gatePick == Gate::RESISTOR)
                            text = "Resistance: %i inputs";
                        else if (data.gatePick == Gate::CAPACITOR)
                            text = "Capacity : %i ticks";
                        else
                            text = "Component parameter: %i";
                        DrawText(TextFormat(text, data.storedExtendedParam), 52, 17, 8, WHITE);
                        _ASSERT_EXPR(data.storedExtendedParam < _countof(Node::g_resistanceBands), L"Stored parameter out of bounds");
                        Color color = Node::g_resistanceBands[data.storedExtendedParam];
                        DrawRectangleIRect(IRect(32, 0, 16), WIPBLUE);
                        DrawRectangleIRect(IRect(34, 2, 12), Node::g_resistanceBands[data.storedExtendedParam]);
                    }
                }

                IRect rec(16);
                DrawRectangleIRect(rec, SPACEGRAY);
                DrawModeIcon(baseMode, rec.xy, WHITE);
                rec.x += 16;
                DrawRectangleIRect(rec, SPACEGRAY);
                DrawGateIcon16x(data.gatePick, rec.xy, WHITE);
                rec.x += 16;
                if (!(cursorPos.y <= 16 && cursorPos.x > 32 && cursorPos.x <= 48))
                {
                    _ASSERT_EXPR(data.storedExtendedParam < _countof(Node::g_resistanceBands), L"Stored parameter out of bounds");
                    DrawRectangleIRect(rec, Node::g_resistanceBands[data.storedExtendedParam]);
                }
                if (!!data.clipboard)
                {
                    rec.x += 16;
                    DrawRectangleIRect(rec, SPACEGRAY);
                    DrawTextureIV(clipboardIcon, rec.xy, WHITE);
                }
            }

        } EndDrawing();
    }

    /******************************************
    *   Unload and free memory
    ******************************************/

    if (data.clipboard != nullptr)
        delete data.clipboard;

    NodeWorld::Get().Save("session.cg");
    BlueprintIcon::Unload();
    UnloadTexture(gateIcons32x);
    UnloadTexture(gateIcons16x);
    UnloadTexture(modeIcons);
    UnloadTexture(clipboardIcon);

    CloseWindow();

	return 0;
}

/* Todo
* Major
* -Selection move-together and delete-together
* -Blueprint pallet
* -Blueprint pallet icons (User-made combination of 4 premade icons. See Factorio for inspiration)
* -Save/load (https://en.cppreference.com/w/cpp/filesystem/directory_iterator)
* -Save file menu
* -Save file thumbnails (based on cropped screenshot)
* -Menu screen (Open to file menu with "new" at the top)
*
* Quality of Life
* -Special erase (keep wires, erase node)
* -Hotkeys for output-only gate state toggles (Like the Reason on-screen piano)
*
* Stretch goals
* -Multiple color pallets
* -Step-by-step evaluation option
* -Log files for debug/crashes
* -Export as SVG
*/
