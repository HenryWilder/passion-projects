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

ProgramData::ProgramData(int windowWidth, int windowHeight) : windowWidth(windowWidth), windowHeight(windowHeight)
{
    InitWindow(windowWidth, windowHeight, "Electron Architect");
    SetExitKey(0);
    SetTargetFPS(60);

    BlueprintIcon::Load("icons_blueprint.png");

    clipboardIcon = LoadTexture("icon_clipboard.png");
    modeIcons = LoadTexture("icons_mode.png");
    gateIcons16x = LoadTexture("icons_gate16x.png");
    gateIcons32x = LoadTexture("icons_gate32x.png");

    SetMode(Mode::PEN);
    SetGate(Gate::OR);
}

ProgramData::~ProgramData()
{
    if (clipboard != nullptr)
        delete clipboard;

    NodeWorld::Get().Save("session.cg");
    NodeWorld::Get().Export("render.svg");
    BlueprintIcon::Unload();
    UnloadTexture(gateIcons32x);
    UnloadTexture(gateIcons16x);
    UnloadTexture(modeIcons);
    UnloadTexture(clipboardIcon);

    CloseWindow();
}

Texture2D ProgramData::GetClipboardIcon()
{
    return clipboardIcon;
}

void ProgramData::DrawModeIcon(Mode mode, IVec2 pos, Color tint)
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
}

void ProgramData::DrawGateIcon16x(Gate gate, IVec2 pos, Color tint)
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
    case Gate::DELAY:     offset = IVec2(1, 3); break;
    default: return;
    }
    DrawIcon<16>(gateIcons16x, offset, pos, tint);
}

void ProgramData::DrawGateIcon32x(Gate gate, IVec2 pos, Color tint)
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
}

bool ProgramData::ModeIsMenu(Mode mode)
{
    // Modes which disable use of basic UI and drawing of certain UI elements
    return mode == Mode::BP_ICON || mode == Mode::BP_SELECT;
}

bool ProgramData::ModeIsMenu() const
{
    return ModeIsMenu(this->mode);
}

bool ProgramData::ModeIsOverlay(Mode mode)
{
    // Modes which can be active simultaneously with a non-overlay mode
    return mode == Mode::GATE || mode == Mode::BUTTON || mode == Mode::PASTE || ModeIsMenu(mode);
}

bool ProgramData::ModeIsOverlay() const
{
    return ModeIsOverlay(this->mode);
}

bool ProgramData::ModeIsBasic() const
{
    return this->mode == this->baseMode;
}

void ProgramData::SetMode(Mode newMode)
{
    if (mode == Mode::BP_ICON)
    {
        ASSERT_CONDITION(!!BPIcon_Object(), L"Object must be initialized at the start of the mode")
        {
            delete BPIcon_Object();
            BPIcon_Object() = nullptr;
        }
    }

    b_cursorMoved = true;
    mode = newMode;

    if (!ModeIsOverlay(newMode))
    {
        baseMode = newMode;
        memset(&base, 0, sizeof(base));
    }
    memset(&overlay, 0, sizeof(overlay));

    // Initialize any non-zero values
    switch (newMode)
    {
    case Mode::GATE:
        Gate_RadialMenuCenter() = cursorUIPos;
        break;

    case Mode::BP_ICON:
        BPIcon_DraggingIcon() = -1;
        break;

    case Mode::BP_SELECT:
        BPSelect_Hovering() = -1;
        break;

    case Mode::BUTTON:
        Button_DropdownActive() = cursorUIPos.x / 16;
        break;
    }
}

void ProgramData::SetGate(Gate newGate)
{
    constexpr const char* parameterTextFmtOptions[] =
    {
        "Component parameter: %i",
        "Resistance: %i inputs",
        "Capacity: %i ticks",
        "Color: %s"
    };
    gatePick = newGate;
    switch (newGate)
    {
    default:
        deviceParameterTextFmt = parameterTextFmtOptions[0];
        break;

    case Gate::RESISTOR:
        deviceParameterTextFmt = parameterTextFmtOptions[1];
        break;
    case Gate::CAPACITOR:
        deviceParameterTextFmt = parameterTextFmtOptions[2];
        break;
    case Gate::LED:
        deviceParameterTextFmt = parameterTextFmtOptions[3];
        break;
    }
}

void ProgramData::ClearOverlayMode()
{
    SetMode(baseMode);
}

const char* ProgramData::GetModeTooltipName(Mode mode)
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
}

const char* ProgramData::GetModeTooltipDescription(Mode mode)
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
}

const char* ProgramData::GetGateTooltipName(Gate gate)
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
    case Gate::DELAY:
        return "Device: Delay [8]";

    default:
        _ASSERT_EXPR(false, L"Missing tooltip for selected gate");
        return "";
    }
}

const char* ProgramData::GetGateTooltipDescription(Gate gate)
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
    case Gate::DELAY:
        return
            "Treats I/O the same as an OR gate.\n"
            "Outputs with a 1-tick delay.\n"
            "Sequntial delay devices are recommended for delay greater than 1 tick.";

    default:
        _ASSERT_EXPR(false, L"Missing tooltip description for selected gate");
        return "";
    }
}

void ProgramData::IncrementTick()
{
    ++tickFrame;
    tickFrame %= framesPerTick;
    tickThisFrame = tickFrame == 0;
}

void ProgramData::UpdateCursorPos()
{
    cursorUIPos = IVec2(GetMouseX(), GetMouseY());

    cursorPos = Snap(IVec2(
        (int)(GetMouseX() / camera.zoom) + (int)camera.target.x,
        (int)(GetMouseY() / camera.zoom) + (int)camera.target.y
    ), g_gridSize);
    {
        constexpr int halfgrid = g_gridSize / 2;
        if (cursorPos.x < 0) cursorPos.x -= halfgrid;
        else                 cursorPos.x += halfgrid;
        if (cursorPos.y < 0) cursorPos.y -= halfgrid;
        else                 cursorPos.y += halfgrid;
    }

    b_cursorMoved = cursorPosPrev != cursorPos;
}

// Zoom/pan

void ProgramData::UpdateCamera()
{
    if (!ModeIsMenu())
    {
        if (GetMouseWheelMove() > 0 && camera.zoom < 2.0f)
        {
            camera.zoom *= 2;
        }
        else if (GetMouseWheelMove() < 0 && camera.zoom > 0.125f)
        {
            camera.zoom /= 2;
        }

        camera.target.x += (float)(IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT)) * g_gridSize;
        camera.target.y += (float)(IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP)) * g_gridSize;
    }
}

void ProgramData::CopySelectionToClipboard()
{
    if (clipboard != nullptr)
        delete clipboard;

    if (selection.empty())
        clipboard = nullptr;
    else
        clipboard = new Blueprint(selection);
}

void ProgramData::MakeGroupFromSelection()
{
    NodeWorld::Get().CreateGroup(Edit_SelectionRec());
    Edit_SelectionRec() = IRect(0);
    selection.clear();
}

bool ProgramData::IsSelectionRectValid() const
{
    return mode == Mode::EDIT && !Edit_SelectionWIP() && !(Edit_SelectionRec().w == 0 || Edit_SelectionRec().h == 0);
}

void ProgramData::SaveBlueprint()
{
    SetMode(Mode::BP_ICON);
    BPIcon_Object() = new BlueprintIcon;
    BPIcon_Pos() = cursorPos - IVec2(BlueprintIcon::g_size / 2, BlueprintIcon::g_size / 2);
    BPIcon_SheetRec().xy = BPIcon_Pos() + IVec2(BlueprintIcon::g_size * 2, BlueprintIcon::g_size * 2);
    BPIcon_SheetRec().wh = BlueprintIcon::GetSheetSize_Px();
}

bool ProgramData::IsClipboardValid() const
{
    return !!clipboard;
}

void ProgramData::ClearClipboard()
{
    delete clipboard;
    clipboard = nullptr;
}

bool ProgramData::SelectionExists() const
{
    return !selection.empty();
}

void ProgramData::ClearSelection()
{
    selection.clear();
    if (mode == Mode::EDIT)
        Edit_SelectionRec() = IRect(0);
}

void ProgramData::DestroySelection()
{
    NodeWorld::Get().DestroyNodes(selection);
    ClearSelection();
}

void ProgramData::CheckHotkeys()
{
    // KEY COMBOS BEFORE INDIVIDUAL KEYS!

    // Ctrl
    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))
    {

        // Ctrl-Alt
        if (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT))
        {
            // Ctrl-Alt-Shift
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
            {
                return; // Don't miscommunicate to the user!!
            }
            return; // Don't miscommunicate to the user!!
        }

        // Ctrl-Shift
        else if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
        {
            return; // Don't miscommunicate to the user!!
        }


        // Copy
        if (IsKeyPressed(KEY_C) && mode == Mode::EDIT)
            CopySelectionToClipboard();

        // Paste
        if (IsKeyPressed(KEY_V) && IsClipboardValid())
            SetMode(Mode::PASTE);

        // Group
        if (IsKeyPressed(KEY_G) && IsSelectionRectValid())
            MakeGroupFromSelection();

        // Save
        if (IsKeyPressed(KEY_S))
        {
            // Save blueprint
            if (mode == Mode::PASTE)
                SaveBlueprint();

            // Save file
            else NodeWorld::Get().Save("save.cg");
        }

        return; // Don't miscommunicate to the user!!
    }

    // Shift
    else if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
    {
        // Shift-Alt
        if (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT))
        {
            return; // Don't miscommunicate to the user!!
        }
        return; // Don't miscommunicate to the user!!
    }

    // Alt
    else if (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT))
    {
        return; // Don't miscommunicate to the user!!
    }

    // Gate hotkeys
    if (ModeIsBasic() || mode == Mode::GATE)
    {
        if (IsKeyPressed(KEY_ONE))   SetGate(Gate::OR);
        else if (IsKeyPressed(KEY_TWO))   SetGate(Gate::AND);
        else if (IsKeyPressed(KEY_THREE)) SetGate(Gate::NOR);
        else if (IsKeyPressed(KEY_FOUR))  SetGate(Gate::XOR);
        else if (IsKeyPressed(KEY_FIVE))  SetGate(Gate::RESISTOR);
        else if (IsKeyPressed(KEY_SIX))   SetGate(Gate::CAPACITOR);
        else if (IsKeyPressed(KEY_SEVEN)) SetGate(Gate::LED);
        else if (IsKeyPressed(KEY_EIGHT)) SetGate(Gate::DELAY);
    }

    // Mode hotkeys
    if (IsKeyPressed(KEY_B)) SetMode(Mode::PEN);
    else if (IsKeyPressed(KEY_V)) SetMode(Mode::EDIT);
    //else if (IsKeyPressed(KEY_G)) SetMode(Mode::GATE); // TODO: Gate mode being depricated
    else if (IsKeyPressed(KEY_X)) SetMode(Mode::ERASE);
    else if (IsKeyPressed(KEY_F)) SetMode(Mode::INTERACT);

    // Escape in stages
    if (IsKeyPressed(KEY_ESCAPE))
    {
        // In order of priority!

        // Exit overlay mode
        if (!ModeIsBasic())
            SetMode(baseMode);

        // Clear selection
        else if (SelectionExists())
            ClearSelection();

        // Clear clipboard
        else if (IsClipboardValid())
            ClearClipboard();

        // Reset to default mode
        else SetMode(Mode::PEN);
    }

    // Selection delete
    if ((IsKeyPressed(KEY_DELETE) || IsKeyPressed(KEY_BACKSPACE)) && SelectionExists())
        DestroySelection();
}

IVec2 ProgramData::GetCursorDelta() const
{
    return cursorPos - cursorPosPrev;
}

// Inclusive

bool ProgramData::CursorInRangeX(int xMin, int xMax) const
{
    return cursorPos.x >= xMin && cursorPos.x <= xMax;
}

// Inclusive

bool ProgramData::CursorInUIRangeX(int xMin, int xMax) const
{
    return cursorUIPos.x >= xMin && cursorUIPos.x <= xMax;
}

// Inclusive

bool ProgramData::CursorInRangeY(int yMin, int yMax) const
{
    return cursorPos.y >= yMin && cursorPos.y <= yMax;
}

// Inclusive

bool ProgramData::CursorInUIRangeY(int yMin, int yMax) const
{
    return cursorUIPos.y >= yMin && cursorUIPos.y <= yMax;
}

bool ProgramData::CursorInBounds(IRect bounds) const
{
    return InBoundingBox(bounds, cursorPos);
}

bool ProgramData::CursorInUIBounds(IRect uiBounds) const
{
    return InBoundingBox(uiBounds, cursorUIPos);
}

void ProgramData::DrawGrid(int gridSize) const
{
    // Grid
    {
        IVec2 extents((int)((float)windowWidth / camera.zoom), (int)((float)windowHeight / camera.zoom));
        IRect bounds(IVec2(camera.target), extents);

        // "If the number of world pixels compacted into a single screen pixel equal or exceed the pixels between gridlines"
        if ((int)(1.0f / camera.zoom) >= gridSize)
        {
            DrawRectangleIRect(bounds, SPACEGRAY);
        }
        else
        {
            for (int y = bounds.y; y < bounds.y + bounds.h; y += gridSize)
            {
                DrawLine(bounds.x, y, bounds.x + bounds.w, y, SPACEGRAY);
            }
            for (int x = bounds.x; x < bounds.x + bounds.w; x += gridSize)
            {
                DrawLine(x, bounds.y, x, bounds.y + bounds.h, SPACEGRAY);
            }
        }
        int halfgrid = gridSize / 2;
        DrawLine(bounds.x, -halfgrid, bounds.x + bounds.w, -halfgrid, LIFELESSNEBULA);
        DrawLine(-halfgrid, bounds.y, -halfgrid, bounds.y + bounds.h, LIFELESSNEBULA);
    }
}

IRect ProgramData::GetSelectionBounds(const std::vector<Node*>& vec) const
{
    IRect bounds = IRect::Abused();
    for (Node* node : vec)
    {
        if (node->GetX() < bounds.minx) bounds.minx = node->GetX();
        else if (node->GetX() > bounds.maxx) bounds.maxx = node->GetX();
        if (node->GetY() < bounds.miny) bounds.miny = node->GetY();
        else if (node->GetY() > bounds.maxy) bounds.maxy = node->GetY();
    }
    bounds.DeAbuse();
    return bounds;
}

IRect ProgramData::GetSelectionBounds() const
{
    return GetSelectionBounds(selection);
}

Color ProgramData::ResistanceBandColor(uint8_t index)
{
    _ASSERT_EXPR(index < _countof(Node::g_resistanceBands), L"Stored parameter out of bounds");
    return Node::g_resistanceBands[index];
}

Color ProgramData::ExtraParamColor() const
{
    return ResistanceBandColor(storedExtraParam);
}

void ProgramData::SetMode2D(bool value)
{
    if (b_twoDee == value)
        return;

    if (b_twoDee = value) // 2D mode
        BeginMode2D(camera);
    else // UI mode
        EndMode2D();
}

void ProgramData::DrawTooltipAtCursor(const char* text, Color color)
{
    auto wat_do = [&]() { DrawTextIV(text, cursorUIPos + IVec2(16), 8, color); };
    // Force UI mode
    if (b_twoDee)
    {
        EndMode2D();
        wat_do();
        BeginMode2D(camera);
    }
    else
        wat_do();
}

Texture2D ProgramData::clipboardIcon;
Texture2D ProgramData::modeIcons;
Texture2D ProgramData::gateIcons16x;
Texture2D ProgramData::gateIcons32x;
