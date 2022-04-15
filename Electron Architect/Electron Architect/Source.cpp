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
        BP_SELECT,
    } mode, baseMode;

    auto ModeIsMenu = [](Mode mode) {
        // Modes which disable use of basic UI and drawing of certain UI elements
        return mode == Mode::BP_ICON || mode == Mode::BP_SELECT;
    };

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
        case Gate::LED:       offset = IVec2(0, 3); break;
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
        case Gate::LED:       offset = IVec2(0, 3); break;
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

            struct {
                int hovering; // -1 for none
            } bp_select;
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
        Gate::LED,
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

    auto GetModeTooltipName = [](Mode mode)
    {
        switch (mode)
        {
        case Mode::PEN:
            return "Mode: Draw [b]";
        case Mode::EDIT:
            return "Mode: Edit [v]";
        case Mode::ERASE:
            return "Mode: Erase [x]";
        case Mode::INTERACT:
            return "Mode: Interact [f]";
        default:
            _ASSERT_EXPR(false, L"Missing tooltip for selected mode");
            return "";
        }
    };
    auto GetModeTooltipDescription = [](Mode mode)
    {
        switch (mode)
        {
        case Mode::PEN:
            return
                "Left click to create a new node and start a wire from it, or to start a wire from an existing node.\n"
                "Left click again to connect the wire to a new node or an existing one, and start a new wire from there.\n"
                "Right click while creating a wire to cancel it.";
        case Mode::EDIT:
            return
                "Left click and drag nodes to move them around.\n"
                "Left click and drag wire elbows to snap them to a preferred angle.\n"
                "Right click nodes to apply the currently selected gate/settings to them.";
        case Mode::ERASE:
            return
                "Left click a node to erase it and all wires directly connected to it (collateral will render in MAGENTA).\n"
                "Left click a wire to erase only that wire, disconnecting the nodes without erasing them.";
        case Mode::INTERACT:
            return
                "Left click a node without any inputs (such nodes will render in BLUE) to toggle it between outputting true and false.\n"
                "NOTE: Framerate is intentionally lowered from 120 to 24 while in this mode for ease of inspection.";

        default:
            _ASSERT_EXPR(false, L"Missing tooltip description for selected mode");
            return "";
        }
    };
    auto GetGateTooltipName = [](Gate gate)
    {
        switch (gate)
        {
        case Gate::OR:
            return "Gate: Or [1]";
        case Gate::AND:
            return "Gate: And [2]";
        case Gate::NOR:
            return "Gate: Nor [3]";
        case Gate::XOR:
            return "Gate: Xor [4]";
        case Gate::RESISTOR:
            return "Device: Resistor [5]";
        case Gate::CAPACITOR:
            return "Device: Capacitor [6]";
        case Gate::LED:
            return "Device: LED [7]";

        default:
            _ASSERT_EXPR(false, L"Missing tooltip for selected gate");
            return "";
        }
    };
    auto GetGateTooltipDescription = [](Gate gate)
    {
        switch (gate)
        {
        case Gate::OR:
            return
                "Outputs true if any input is true,\n"
                "Outputs false otherwise.";
        case Gate::AND:
            return
                "Outputs true if all inputs are true,\n"
                "Outputs false otherwise.";
        case Gate::NOR:
            return
                "Outputs false if any input is true.\n"
                "Outputs true otherwise.";
        case Gate::XOR:
            return
                "Outputs true if exactly 1 input is true,\n"
                "Outputs false otherwise.\n"
                "Order of inputs does not matter.";
        case Gate::RESISTOR:
            return
                "Outputs true if greater than [resistance] inputs are true,\n"
                "Outputs false otherwise.\n"
                "Order of inputs does not matter.";
        case Gate::CAPACITOR:
            return
                "Stores charge while any input is true.\n"
                "Stops charging once charge equals [capacity].\n"
                "Drains charge while no input is true.\n"
                "Outputs true while charge is greater than zero,\n"
                "Outputs true while any input is true,\n"
                "Outputs false otherwise.";
        case Gate::LED:
            return
                "Treats I/O the same as an OR gate.\n"
                "Lights up with the selected color when powered.";

        default:
            _ASSERT_EXPR(false, L"Missing tooltip description for selected gate");
            return "";
        }
    };

    IVec2 cursorPosPrev = IVec2::Zero(); // For checking if there was movement
    bool b_cursorMoved = false;

    auto SetMode = [targetFPS, &baseMode, &mode, &data, &cursorPosPrev, &b_cursorMoved](Mode newMode)
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

        b_cursorMoved = true;
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

        case Mode::BP_SELECT:
            data.bp_select.hovering = -1;
            break;
        }
    };

    mode = Mode::PEN;
    SetMode(Mode::PEN);

    NodeWorld::Get().Load("session.cg"); // Construct and load last session

    Camera2D camera;
    camera.offset = { 0,0 };
    camera.target = { 0,0 };
    camera.rotation = 0;
    camera.zoom = 1;
                
    while (!WindowShouldClose())
    {
        /******************************************
        *   Simulate frame and update variables
        ******************************************/

        IVec2 cursorUIPos(GetMouseX(), GetMouseY());

        IVec2 cursorPos(
            (int)(GetMouseX() / camera.zoom) + (int)camera.target.x,
            (int)(GetMouseY() / camera.zoom) + (int)camera.target.y
        );
        cursorPos /= g_gridSize;
        cursorPos *= g_gridSize;
        {
            constexpr int halfgrid = g_gridSize / 2;
            if (cursorPos.x < 0) cursorPos.x -= halfgrid;
            else                 cursorPos.x += halfgrid;
            if (cursorPos.y < 0) cursorPos.y -= halfgrid;
            else                 cursorPos.y += halfgrid;
        }

        b_cursorMoved = cursorPosPrev != cursorPos;

        // Zoom/pan
        if (!ModeIsMenu(mode))
        {
            if (GetMouseWheelMove() > 0 && camera.zoom < 2.0f)
                camera.zoom *= 2;
            else if (GetMouseWheelMove() < 0 && camera.zoom > 0.125f)
                camera.zoom /= 2;
            camera.target.x += (float)(IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT)) * g_gridSize;
            camera.target.y += (float)(IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP)) * g_gridSize;
        }

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
                else if (IsKeyPressed(KEY_SEVEN))
                    data.gatePick = Gate::LED;
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
                    data.gate.radialMenuCenter = cursorUIPos;
                }
                else if (IsKeyPressed(KEY_X))
                {
                    SetMode(Mode::ERASE);
                }
                else if (IsKeyPressed(KEY_F))
                {
                    SetMode(Mode::INTERACT);
                }
            }
        }

        // UI buttons
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !ModeIsMenu(mode) && (cursorUIPos.y <= 16 && cursorUIPos.x <= (16 * 3)) &&
                (mode == Mode::BUTTON ? data.button.dropdownActive != (cursorUIPos.x / 16) : true))
            {
                SetMode(Mode::BUTTON);
                data.button.dropdownActive = cursorUIPos.x / 16;
                goto EVAL; // Skip button sim this frame
            }
            else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !ModeIsMenu(mode) && (cursorUIPos.y <= 16 && cursorUIPos.x >= (16 * 3) && cursorUIPos.x <= (16 * 4)))
            {
                SetMode(Mode::BP_SELECT);
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
                else if (data.hoveredNode->GetGate() == Gate::LED)
                    data.hoveredNode->SetColorIndex(data.storedExtendedParam);
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
                if (cursorUIPos.x < data.gate.radialMenuCenter.x)
                {
                    if (cursorUIPos.y < data.gate.radialMenuCenter.y)
                        data.gate.overlappedSection = 2;
                    else // cursorPos.y > data.gate.radialMenuCenter.y
                        data.gate.overlappedSection = 3;
                }
                else // cursorPos.x > data.gate.radialMenuCenter.x
                {
                    if (cursorUIPos.y < data.gate.radialMenuCenter.y)
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
                cursorUIPos = data.gate.radialMenuCenter;
            }
        }
        break;

        case Mode::BUTTON:
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                IRect rec = dropdownBounds[data.button.dropdownActive];
                if (InBoundingBox(rec, cursorUIPos))
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

                            if (InBoundingBox(rec, cursorUIPos))
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

                            if (InBoundingBox(rec, cursorUIPos))
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

                            if (InBoundingBox(rec, cursorUIPos))
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
                    NodeWorld::Get().SpawnBlueprint(data.clipboard, cursorPos);
                data.selection.clear();
                SetMode(baseMode);
            }
        }
        break;

        case Mode::BP_ICON:
        {
            if (b_cursorMoved && data.bp_icon.draggingIcon == -1)
            {
                data.bp_icon.iconID = BlueprintIcon::GetIconAtColRow(BlueprintIcon::PixelToColRow(data.bp_icon.sheetRec.xy, cursorUIPos));
            }

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                if (InBoundingBox(data.bp_icon.sheetRec, cursorUIPos) && data.bp_icon.iconCount < 4 && !!data.bp_icon.iconID)
                {
                    cursorUIPos = data.bp_icon.pos;
                    SetMousePosition(cursorUIPos.x + BlueprintIcon::g_size / 2, cursorUIPos.y + BlueprintIcon::g_size / 2);
                    data.bp_icon.object->combo[data.bp_icon.iconCount] = { data.bp_icon.iconID, 0,0 };
                    data.bp_icon.draggingIcon = data.bp_icon.iconCount;
                    data.bp_icon.iconCount++;
                }
                else if (InBoundingBox(IRect(data.bp_icon.pos.x, data.bp_icon.pos.y, BlueprintIcon::g_size * 2, BlueprintIcon::g_size * 2), cursorUIPos))
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
                        if (InBoundingBox(bounds, cursorUIPos))
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
                IVec2 colRow = (cursorUIPos - data.bp_icon.pos - centerOffset) / centerOffset;
                colRow.x = std::min(std::max(colRow.x, 0), 2);
                colRow.y = std::min(std::max(colRow.y, 0), 2);
                data.bp_icon.object->combo[data.bp_icon.draggingIcon].x = colRow.x;
                data.bp_icon.object->combo[data.bp_icon.draggingIcon].y = colRow.y;
            }
        }
        break;

        case Mode::BP_SELECT:
        {
            // todo
        }
        break;

        default:
            _ASSERT_EXPR(false, L"Missing sim phase specialization for selected mode");
            break;
        }

    EVAL:
        cursorPosPrev = cursorPos;
        NodeWorld::Get().Evaluate();

        /******************************************
        *   Draw the frame
        ******************************************/

        // todo: refactor to not do unnecesary tests every frame
        const char* deviceParameterTextFmt;
        if (data.gatePick == Gate::RESISTOR)
            deviceParameterTextFmt = "Resistance: %i inputs";
        else if (data.gatePick == Gate::LED)
            deviceParameterTextFmt = "Color: %s";
        else if (data.gatePick == Gate::CAPACITOR)
            deviceParameterTextFmt = "Capacity: %i ticks";
        else
            deviceParameterTextFmt = "Component parameter: %i";

        constexpr const char* colorName[]
        {
            "black",
            "brown",
            "red",
            "orange",
            "yellow",
            "green",
            "blue",
            "violet",
            "gray",
            "white",
        };

        BeginDrawing(); {

            ClearBackground(BLACK);

            if (ModeIsMenu(mode))
            {
                switch (mode)
                {
                case Mode::BP_ICON:
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

                            IRect bounds(data.bp_icon.pos, BlueprintIcon::g_size);
                            bounds.xy = bounds.xy + data.bp_icon.object->combo[i].Pos();
                            if (InBoundingBox(bounds, cursorUIPos))
                            {
                                DrawRectangleIRect(bounds, ColorAlpha(WIPBLUE, 0.25f));
                            }
                        }
                    }
                    if (data.bp_icon.draggingIcon != -1)
                        BlueprintIcon::DrawBPIcon(data.bp_icon.iconID, data.bp_icon.pos + data.bp_icon.object->combo[data.bp_icon.draggingIcon].Pos(), WIPBLUE);

                    BlueprintIcon::DrawSheet(data.bp_icon.sheetRec.xy, SPACEGRAY, WHITE);
                }
                break;
                case Mode::BP_SELECT:
                {
                    // todo
                    DrawText("[ @TODO make blueprint selection screen ]\nPress Esc to return to circuit graph.", 4,4,8,WHITE);
                }
                break;
                default:
                    _ASSERT_EXPR(false, L"No specialization for selected menu mode");
                    break;
                }
            }
            else
            {
                BeginMode2D(camera);

                // Grid
                {
                    IVec2 extents((int)((float)windowWidth / camera.zoom), (int)((float)windowHeight / camera.zoom));
                    IRect bounds(IVec2(camera.target), extents);

                    if (camera.zoom > 0.125)
                    {
                        for (int y = bounds.y; y < bounds.y + bounds.h; y += g_gridSize)
                        {
                            DrawLine(bounds.x, y, bounds.x + bounds.w, y, SPACEGRAY);
                        }
                        for (int x = bounds.x; x < bounds.x + bounds.w; x += g_gridSize)
                        {
                            DrawLine(x, bounds.y, x, bounds.y + bounds.h, SPACEGRAY);
                        }
                    }
                    else
                    {
                        DrawRectangleIRect(bounds, SPACEGRAY);
                    }
                    DrawLine(bounds.x, 0, bounds.x + bounds.w, 0, LIFELESSNEBULA);
                    DrawLine(0, bounds.y, 0, bounds.y + bounds.h, LIFELESSNEBULA);
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

                    EndMode2D();

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

                    EndMode2D();

                    IRect rec = dropdownBounds[data.button.dropdownActive];
                    DrawRectangleIRect(rec, SPACEGRAY);
                    rec.h = 16;

                    switch (data.button.dropdownActive)
                    {
                    case 0: // Mode
                    {
                        for (Mode m : dropdownModeOrder)
                        {
                            if (m == baseMode)
                                continue;
                            Color color;
                            if (InBoundingBox(rec, cursorUIPos))
                            {
                                color = WHITE;
                                DrawText(GetModeTooltipName(m), 20, 17 + rec.y, 8, WHITE);
                            }
                            else
                                color = DEADCABLE;
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
                            Color color;
                            if (InBoundingBox(rec, cursorUIPos))
                            {
                                color = WHITE;
                                DrawText(GetGateTooltipName(g), 20 + 16, 17 + rec.y, 8, WHITE);
                            }
                            else
                                color = DEADCABLE;
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
                            if (InBoundingBox(rec, cursorUIPos))
                            {
                                DrawRectangleIRect(rec, WIPBLUE);
                                DrawRectangleIRect(ExpandIRect(rec, -2), color);
                                const char* text;
                                if (data.gatePick == Gate::LED)
                                    text = TextFormat(deviceParameterTextFmt, colorName[v]);
                                else
                                    text = TextFormat(deviceParameterTextFmt, v);
                                DrawText(text, 20 + 32, 17 + rec.y, 8, WHITE);
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

                    data.clipboard->DrawPreview(cursorPos, ColorAlpha(LIFELESSNEBULA, 0.5f), HAUNTINGWHITE);
                }
                break;

                case Mode::BP_ICON:
                case Mode::BP_SELECT:
                    _ASSERT_EXPR(false, L"How did we get here?");
                    break;

                default:
                    _ASSERT_EXPR(false, L"Missing draw phase specialization for selected mode");
                    break;
                }

                EndMode2D();

                // Global UI


                DrawRectangleIRect(IRect(32, 16), SPACEGRAY);
                DrawRectangleIRect(IRect(64, 16), SPACEGRAY);
                if (!!data.clipboard)
                {
                    constexpr IRect clipboardRec(16 * 4, 0, 16);
                    DrawRectangleIRect(clipboardRec, SPACEGRAY);
                    DrawTextureIV(clipboardIcon, clipboardRec.xy, WHITE);
                }

                _ASSERT_EXPR(data.storedExtendedParam < _countof(Node::g_resistanceBands), L"Stored parameter out of bounds");
                DrawRectangleIRect(IRect(32, 0, 16), Node::g_resistanceBands[data.storedExtendedParam]);

                // Buttons
                if (cursorUIPos.y <= 16)
                {
                    // Mode
                    if (cursorUIPos.x <= 16)
                    {
                        constexpr IRect rec(0, 0, 16);
                        DrawRectangleIRect(rec, WIPBLUE);
                        const char* name = GetModeTooltipName(baseMode);
                        DrawText(name, 20, 17, 8, WHITE);
                        DrawLine(20, 17 + 12, 20 + MeasureText(name, 8), 17 + 12, WHITE);
                        DrawText(GetModeTooltipDescription(baseMode), 20, 17 + 16, 8, WHITE);
                    }
                    // Gate
                    else if (cursorUIPos.x <= 32)
                    {
                        constexpr IRect rec(16, 0, 16);
                        DrawRectangleIRect(rec, WIPBLUE);
                        const char* name = GetGateTooltipName(data.gatePick);
                        DrawText(name, 36, 17, 8, WHITE);
                        DrawLine(36, 17 + 12, 36 + MeasureText(name, 8), 17 + 12, WHITE);
                        DrawText(GetGateTooltipDescription(data.gatePick), 36, 17 + 16, 8, WHITE);
                    }
                    // Extra param
                    else if (cursorUIPos.x <= 48)
                    {
                        constexpr IRect rec(32, 0, 16);
                        DrawRectangleIRect(rec, WIPBLUE);
                        _ASSERT_EXPR(data.storedExtendedParam < _countof(Node::g_resistanceBands), L"Stored parameter out of bounds");
                        DrawRectangleIRect(ShrinkIRect(rec, 2), Node::g_resistanceBands[data.storedExtendedParam]);
                        const char* text;
                        if (data.gatePick == Gate::LED)
                            text = TextFormat(deviceParameterTextFmt, colorName[data.storedExtendedParam]);
                        else
                            text = TextFormat(deviceParameterTextFmt, data.storedExtendedParam);
                        DrawText(text, 52, 17, 8, WHITE);
                    }
                    // Blueprints
                    else if (cursorUIPos.x <= 64)
                    {
                        constexpr IRect rec(48, 0, 16);
                        DrawRectangleIRect(rec, WIPBLUE);
                    }
                }

                DrawModeIcon(baseMode, IVec2(0), WHITE);
                DrawGateIcon16x(data.gatePick, IVec2(16,0), WHITE);
            }

        } EndDrawing();
    }

    /******************************************
    *   Unload and free memory
    ******************************************/

    if (data.clipboard != nullptr)
        delete data.clipboard;

    NodeWorld::Get().Save("session.cg");
    NodeWorld::Get().Export("render.svg");
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
* -Improve groups
* -Add "buffer" (delay) gate
* -Change overlapping nodes to swap gates instead of merging
* -More explanation of controls
* -Blueprint pallet
* -Prefab blueprints for things like timers, counters, and latches
* -Blueprint pallet icons (User-made combination of 4 premade icons. See Factorio for inspiration)
* -Save/load
* -Save file menu (https://en.cppreference.com/w/cpp/filesystem/directory_iterator)
* -Save file thumbnails (based on file contents)
* -Menu screen (Open to file menu with "new" at the top)
*
* Quality of Life
* -Special erase (keep wires, erase node)
* -Hotkeys for output-only gate state toggles (Like the Reason on-screen piano)
*
* Stretch goals
* -Multiple color pallets
* -Log files for debug/crashes
*/
