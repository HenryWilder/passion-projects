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
#include "Window.h"

Blueprint g_clipboardBP;

Window::Window(int windowWidth, int windowHeight) : windowWidth(windowWidth), windowHeight(windowHeight),
propertiesPaneRec(windowWidth - 256, 0, 256, windowHeight),
modeButtons{
    IconButton(
        IVec2(0, 3),
        "Mode: Draw [b]",
        "Left click to create a new node and start a wire from it, or to start a wire from an existing node.\n"
        "Left click again to connect the wire to a new node or an existing one, and start a new wire from there.\n"
        "Right click while creating a wire to cancel it.",
        [this]() { SetMode(Mode::PEN); },
        IVec2(0, 0),
        &modeIcons16x),

        IconButton(
            IVec2(0, 4),
            "Mode: Edit [v]",
            "Left click and drag nodes to move them around.\n"
            "Left click and drag wire elbows to snap them to a preferred angle.\n"
            "Right click nodes to apply the currently selected gate/settings to them.",
            [this]() { SetMode(Mode::EDIT); },
            IVec2(1, 0),
            &modeIcons16x),

        IconButton(
            IVec2(0, 5),
            "Mode: Erase [x]",
            "Left click a node to erase it and all wires directly connected to it (collateral will render in MAGENTA).\n"
            "Left click a wire to erase only that wire, disconnecting the nodes without erasing them.",
            [this]() { SetMode(Mode::ERASE); },
            IVec2(0, 1),
            &modeIcons16x),

        IconButton(
            IVec2(0, 6),
            "Mode: Interact [f]",
            "Left click a node without any inputs (such nodes will render in \"available_color\" (blue by default)) to toggle it between outputting true and false.",
            [this]() { SetMode(Mode::INTERACT); },
            IVec2(1, 1),
            &modeIcons16x),
},
gateButtons{
    IconButton(
        IVec2(0,8),
        "Gate: Or [1]",
        "Outputs true if any input is true,\n"
        "Outputs false otherwise.",
        [this]() { SetGate(Gate::OR); },
        IVec2(0,0),
        &gateIcons16x),

    IconButton(
        IVec2(0,9),
        "Gate: And [2]",
        "Outputs true if all inputs are true,\n"
        "Outputs false otherwise.",
        [this]() { SetGate(Gate::AND); },
        IVec2(1,0),
        &gateIcons16x),

    IconButton(
        IVec2(0,10),
        "Gate: Nor [3]",
        "Outputs false if any input is true.\n"
        "Outputs true otherwise.",
        [this]() { SetGate(Gate::NOR); },
        IVec2(0,1),
        &gateIcons16x),

    IconButton(
        IVec2(0,11),
        "Gate: Xor [4]",
        "Outputs true if exactly 1 input is true,\n"
        "Outputs false otherwise.\n"
        "Order of inputs does not matter.",
        [this]() { SetGate(Gate::XOR); },
        IVec2(1,1),
        &gateIcons16x),

    IconButton(
        IVec2(0,12),
        "Element: Resistor [5]",
        "Outputs true if greater than [resistance] inputs are true,\n"
        "Outputs false otherwise.\n"
        "Order of inputs does not matter.",
        [this]() { SetGate(Gate::RESISTOR); },
        IVec2(0,2),
        &gateIcons16x),

    IconButton(
        IVec2(0,13),
        "Element: Capacitor [6]",
        "Stores charge while any input is true.\n"
        "Stops charging once charge equals [capacity].\n"
        "Drains charge while no input is true.\n"
        "Outputs true while charge is greater than zero,\n"
        "Outputs true while any input is true,\n"
        "Outputs false otherwise.",
        [this]() { SetGate(Gate::CAPACITOR); },
        IVec2(1,2),
        &gateIcons16x),

    IconButton(
        IVec2(0,14),
        "Element: LED [7]",
        "Treats I/O the same as an OR gate.\n"
        "Lights up with the selected color when powered.",
        [this]() { SetGate(Gate::LED); },
        IVec2(0,3),
        &gateIcons16x),

    IconButton(
        IVec2(0,15),
        "Element: Delay [8]",
         "Treats I/O the same as an OR gate.\n"
        "Outputs with a 1-tick delay.\n"
        "Sequntial delay devices are recommended for delay greater than 1 tick.",
        [this]() { SetGate(Gate::DELAY); },
        IVec2(1,3),
        &gateIcons16x),

    IconButton(
        IVec2(0,16),
        "Element: Battery [9]",
        "Always outputs true, regardless of inputs.",
        [this]() { SetGate(Gate::BATTERY); },
        IVec2(0,4),
        &gateIcons16x),
},
blueprintsButton(
    IVec2(0, 0),
    "Blueprints",
    "@TODO",
    [this]() { SetMode(Mode::BP_SELECT); },
    IVec2::Zero(),
    &blueprintIcon16x),
clipboardButton(
        IVec2(0, 1),
        "Clipboard (ctrl+c to copy, ctrl+v to paste)",
        "@TODO",
        [this]() { if (this->IsClipboardValid()) SetMode(Mode::PASTE); },
        IVec2::Zero(),
        &clipboardIcon16x)
{
    InitWindow(windowWidth, windowHeight, "Electron Architect");
    SetExitKey(0);
    SetTargetFPS(60);

    blueprintIcon16x = LoadTexture("icon_blueprints16x.png");
    blueprintIcon32x = LoadTexture("icon_blueprints32x.png");
    clipboardIcon16x = LoadTexture("icon_clipboard16x.png");
    clipboardIcon32x = LoadTexture("icon_clipboard32x.png");
    modeIcons16x = LoadTexture("icons_mode16x.png");
    modeIcons32x = LoadTexture("icons_mode32x.png");
    gateIcons16x = LoadTexture("icons_gate16x.png");
    gateIcons32x = LoadTexture("icons_gate32x.png");

    activeTab = 0;
    tabs.push_back(new Tab);

    base = nullptr;
    overlay = nullptr;

    SetMode(Mode::PEN);
    SetGate(Gate::OR);
    ReloadConfig();
}

Window::~Window()
{
    for (const Tab* tab : tabs)
    {
        tab->Save("session.cg");
        tab->Export("render.svg");
        delete tab;
    }

    if (!!base) [[likely]]
        delete base;
    if (!!overlay)
        delete overlay;

    UnloadTexture(blueprintIcon16x);
    UnloadTexture(blueprintIcon32x);
    UnloadTexture(clipboardIcon16x);
    UnloadTexture(clipboardIcon32x);
    UnloadTexture(modeIcons16x);
    UnloadTexture(modeIcons32x);
    UnloadTexture(gateIcons16x);
    UnloadTexture(gateIcons32x);

    CloseWindow();
}

Tab& Window::CurrentTab()
{
    _ASSERT_EXPR(activeTab < tabs.size(), L"Subscript out of range");
    _ASSERT_EXPR(!!tabs[activeTab], L"Current tab is nullptr");
    return *tabs[activeTab];
}

const IconButton& Window::ButtonFromMode(Mode mode) const
{
    switch (mode)
    {
    default: throw(std::exception("Missing specialization"));
    case Mode::PEN:         return modeButtons[0];
    case Mode::EDIT:        return modeButtons[1];
    case Mode::ERASE:       return modeButtons[2];
    case Mode::INTERACT:    return modeButtons[3];
    case Mode::BUTTON:      return modeButtons[4];
    case Mode::PASTE:       return modeButtons[5];
    case Mode::BP_SELECT:   return modeButtons[6];
    }
}
const IconButton& Window::ButtonFromGate(Gate gate) const
{
    switch (gate)
    {
    default: throw(std::exception("Missing specialization"));
    case Gate::OR:          return gateButtons[0];
    case Gate::AND:         return gateButtons[1];
    case Gate::NOR:         return gateButtons[2];
    case Gate::XOR:         return gateButtons[3];
    case Gate::RESISTOR:    return gateButtons[4];
    case Gate::CAPACITOR:   return gateButtons[5];
    case Gate::LED:         return gateButtons[6];
    case Gate::DELAY:       return gateButtons[7];
    case Gate::BATTERY:     return gateButtons[8];
    }
}

int Window::FontSize() const
{
    switch (uiScale)
    {
    default:
    case 1: return 8;
    case 2: return 20;
    }
}

void Window::DrawUIIcon(Texture2D iconSheet, IVec2 iconColRow, IVec2 pos, Color tint) const
{
    switch (uiScale)
    {
    case 1: DrawIcon<16>(iconSheet, iconColRow, pos, tint); break;
    case 2: DrawIcon<32>(iconSheet, iconColRow, pos, tint); break;
    }
}

Mode Window::GetBaseMode()
{
    _ASSERT_EXPR(!!base, L"Base was not initialized");
    return base->GetMode();
}
Mode Window::GetMode()
{
    if (!!overlay)
        return overlay->GetMode();
    return GetBaseMode();
}
ModeType Window::GetModeType()
{
    if (!!overlay)
        return overlay->GetModeType();
    _ASSERT_EXPR(!!base, L"Base was not initialized");
    return base->GetModeType();
}

void Window::SetMode(Mode newMode)
{
    b_cursorMoved = true;

    if (!!overlay ? newMode != overlay->GetMode() : true)
    {
        if (!!overlay)
            delete overlay;

        if (TypeOfMode(newMode) != ModeType::Basic)
        {
            switch (newMode)
            {
            default:
                _ASSERT_EXPR(false, L"Missing specialization for creating mode");
                overlay = nullptr;
                break;

            case Mode::PEN:         overlay = new PenTool; break;
            case Mode::EDIT:        overlay = new EditTool; break;
            case Mode::ERASE:       overlay = new EraseTool; break;
            case Mode::INTERACT:    overlay = new InteractTool; break;

            case Mode::PASTE:       overlay = new PasteOverlay; break;

            case Mode::BP_SELECT:   overlay = new BlueprintMenu; break;
            }
        }
        else // Mode is basic; no overlay
            overlay = nullptr;
    }

    if (TypeOfMode(newMode) == ModeType::Basic &&
        (!!base ? newMode != base->GetMode() : true))
    {
        if (!!base) [[likely]] // There should only be one point in the program where base is nullptr. Namely, the start.
            delete base;

        switch (newMode)
        {
        default:
            _ASSERT_EXPR(false, L"Missing specialization for creating mode");
            base = nullptr;
            break;

        case Mode::PEN:         base = new PenTool; break;
        case Mode::EDIT:        base = new EditTool; break;
        case Mode::ERASE:       base = new EraseTool; break;
        case Mode::INTERACT:    base = new InteractTool; break;

        case Mode::PASTE:       base = new PasteOverlay; break;

        case Mode::BP_SELECT:   base = new BlueprintMenu; break;
        }
    }
    _ASSERT_EXPR(!!base, L"Base was not initialized");
}

void Window::SetGate(Gate newGate)
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

void Window::ClearOverlayMode()
{
    _ASSERT_EXPR(!!base, L"Base mode was not initialized or got nullified");
    SetMode(base->GetMode());
}

void Window::IncrementTick()
{
    ++tickFrame;
    tickFrame %= framesPerTick;
    tickThisFrame = tickFrame == 0;
}

void Window::UpdateCursorPos()
{
    cursorUIPos = IVec2(GetMouseX(), GetMouseY());

    cursorPos = IVec2(
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
}

// Zoom/pan

void Window::UpdateCamera()
{
    if (GetModeType() != ModeType::Menu)
    {
        if (GetMouseWheelMove() > 0 && camera.zoom < 2.0f)
            camera.zoom *= 2;

        else if (GetMouseWheelMove() < 0 && camera.zoom > 0.125f)
            camera.zoom /= 2;

        camera.target.x += (float)(((int)IsKeyDown(KEY_RIGHT) - (int)IsKeyDown(KEY_LEFT)) * g_gridSize);
        camera.target.y += (float)(((int)IsKeyDown(KEY_DOWN)  - (int)IsKeyDown(KEY_UP))   * g_gridSize);
    }
}

void Window::CopySelectionToClipboard()
{
    if (selection.empty())
        clipboard = nullptr;
    else
    {
        g_clipboardBP = Blueprint(selection);
        clipboard = &g_clipboardBP;
    }
}

void Window::MakeGroupFromSelection()
{
    _ASSERT_EXPR(base->GetMode() == Mode::EDIT, L"Group from selection shouldn't be possible outside of edit mode");
    EditTool* edit = dynamic_cast<EditTool*>(base);
    CurrentTab().CreateGroup(edit->selectionRec, UIColor(UIColorID::UI_COLOR_AVAILABLE));
    edit->selectionRec = IRect(0);
    selection.clear();
}

bool Window::IsSelectionRectValid() const
{
    EditTool* edit = dynamic_cast<EditTool*>(base);
    return !!edit && !edit->selectionWIP && edit->selectionRec.w > 0 && edit->selectionRec.h > 0;
}

void Window::SaveBlueprint()
{
    if (!IsClipboardValid())
        return;
    CurrentTab().StoreBlueprint(clipboard);
    clipboard->Save();
}

bool Window::IsClipboardValid() const
{
    return !!clipboard;
}

void Window::ClearClipboard()
{
    clipboard = nullptr;
}

bool Window::SelectionExists() const
{
    return !selection.empty();
}

void Window::ClearSelection()
{
    selection.clear();
    if (EditTool* edit = dynamic_cast<EditTool*>(base))
        edit->selectionRec = IRect(0);
}

void Window::DestroySelection()
{
    CurrentTab().DestroyNodes(selection);
    ClearSelection();
}

void Window::CheckHotkeys()
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
        if (IsKeyPressed(KEY_C) && GetMode() == Mode::EDIT)
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
            if (GetMode() == Mode::PASTE)
                SaveBlueprint();

            // Save file
            else CurrentTab().Save(TextFormat("%s.cg", CurrentTab().GetName()));
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
    if (GetModeType() == ModeType::Basic)
    {
        if      (IsKeyPressed(KEY_ONE))   SetGate(Gate::OR);
        else if (IsKeyPressed(KEY_TWO))   SetGate(Gate::AND);
        else if (IsKeyPressed(KEY_THREE)) SetGate(Gate::NOR);
        else if (IsKeyPressed(KEY_FOUR))  SetGate(Gate::XOR);
        else if (IsKeyPressed(KEY_FIVE))  SetGate(Gate::RESISTOR);
        else if (IsKeyPressed(KEY_SIX))   SetGate(Gate::CAPACITOR);
        else if (IsKeyPressed(KEY_SEVEN)) SetGate(Gate::LED);
        else if (IsKeyPressed(KEY_EIGHT)) SetGate(Gate::DELAY);
        else if (IsKeyPressed(KEY_NINE))  SetGate(Gate::BATTERY);
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
        if (GetModeType() != ModeType::Basic)
            ClearOverlayMode();

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
    {
        DestroySelection();
        hoveredNode = nullptr;
        hoveredWire = nullptr;
    }
}

IVec2 Window::GetCursorDelta() const
{
    return cursorPos - cursorPosPrev;
}

bool Window::CursorInBounds(IRect bounds) const
{
    return InBoundingBox(bounds, cursorPos);
}

bool Window::CursorInUIBounds(IRect uiBounds) const
{
    return InBoundingBox(uiBounds, cursorUIPos);
}

void Window::DrawGrid(int gridSize) const
{
    // Grid
    {
        IVec2 extents((int)((float)windowWidth / camera.zoom), (int)((float)windowHeight / camera.zoom));
        IRect bounds(IVec2(camera.target), extents);

        // "If the number of world pixels compacted into a single screen pixel equal or exceed the pixels between gridlines"
        if ((int)(1.0f / camera.zoom) >= gridSize)
        {
            DrawRectangleIRect(bounds, UIColor(UIColorID::UI_COLOR_BACKGROUND1));
        }
        else
        {
            for (int y = bounds.y; y < bounds.y + bounds.h; y += gridSize)
            {
                DrawLine(bounds.x, y, bounds.x + bounds.w, y, UIColor(UIColorID::UI_COLOR_BACKGROUND1));
            }
            for (int x = bounds.x; x < bounds.x + bounds.w; x += gridSize)
            {
                DrawLine(x, bounds.y, x, bounds.y + bounds.h, UIColor(UIColorID::UI_COLOR_BACKGROUND1));
            }
        }
        int halfgrid = gridSize / 2;
        DrawLine(bounds.x, -halfgrid, bounds.x + bounds.w, -halfgrid, UIColor(UIColorID::UI_COLOR_BACKGROUND2));
        DrawLine(-halfgrid, bounds.y, -halfgrid, bounds.y + bounds.h, UIColor(UIColorID::UI_COLOR_BACKGROUND2));
    }
}

IRect Window::GetSelectionBounds(const std::vector<Node*>& vec) const
{
    IVec2 min = vec[0]->GetPosition();
    IVec2 max = vec[0]->GetPosition();
    for (Node* node : vec)
    {
        if (node->GetX() < min.x) min.x = node->GetX();
        if (node->GetY() < min.y) min.y = node->GetY();
        if (node->GetX() > max.x) max.x = node->GetX();
        if (node->GetY() > max.y) max.y = node->GetY();
    }
    return IRect(min.x, min.y, max.x - min.x, max.y - min.y);
}

IRect Window::GetSelectionBounds() const
{
    return GetSelectionBounds(selection);
}

Color Window::ResistanceBandColor(uint8_t index)
{
    _ASSERT_EXPR(index < _countof(Node::g_resistanceBands), L"Stored parameter out of bounds");
    return Node::g_resistanceBands[index];
}

Color Window::ExtraParamColor() const
{
    return ResistanceBandColor(storedExtraParam);
}

void Window::DrawTooltipAtCursor(const char* text, Color color)
{
    EndMode2D();
    DrawTextIV(text, cursorUIPos + IVec2(16), 8, color);
    BeginMode2D(camera);
}

void Window::DrawTooltipAtCursor_Shadowed(const char* text, Color color)
{
    EndMode2D();
    IVec2 offset;
    for (offset.y = 16 - 1; offset.y <= 16 + 1; ++offset.y)
    {
        for (offset.x = 16 - 1; offset.x <= 16 + 1; ++offset.x)
        {
            if (offset == IVec2(16)) [[unlikely]]
                continue;
            DrawTextIV(text, cursorUIPos + offset, 8, UIColor(UIColorID::UI_COLOR_BACKGROUND));
        }
    }
    DrawTextIV(text, cursorUIPos + IVec2(16), 8, color);
    BeginMode2D(camera);
}

Color ConfigStrToColor(const std::string& str)
{
    size_t separators[2]{ 0,0 };
    separators[0] = str.find('|');
    if (separators[0] == str.npos)
        return MAGENTA;
    separators[1] = str.find('|', separators[0] + 1);
    if (separators[1] == str.npos)
        return MAGENTA;
    try
    {
        int r = std::stoi(str.substr(0, separators[0]));
        int g = std::stoi(str.substr(separators[0] + 1, separators[1] - (separators[0] + 1)));
        int b = std::stoi(str.substr(separators[1] + 1));
        return { (uint8_t)r, (uint8_t)g, (uint8_t)b, 255u };
    }
    catch (...)
    {
        return MAGENTA;
    }
}

const char* ConfigColorToString(Color color)
{
    return TextFormat("%u|%u|%u", color.r, color.g, color.b);
}

void Window::ReloadConfig()
{
    std::ifstream file("config.ini");

    UIColor(UIColorID::UI_COLOR_BACKGROUND) = BLACK;
    UIColor(UIColorID::UI_COLOR_BACKGROUND1) = ui_color::SPACEGRAY;
    UIColor(UIColorID::UI_COLOR_BACKGROUND2) = ui_color::LIFELESSNEBULA;
    UIColor(UIColorID::UI_COLOR_BACKGROUND3) = ui_color::GLEEFULDUST;

    UIColor(UIColorID::UI_COLOR_FOREGROUND3) = ui_color::DEADCABLE;
    UIColor(UIColorID::UI_COLOR_FOREGROUND2) = ui_color::HAUNTINGWHITE;
    UIColor(UIColorID::UI_COLOR_FOREGROUND1) = ui_color::INTERFERENCEGRAY;
    UIColor(UIColorID::UI_COLOR_FOREGROUND) = WHITE;

    UIColor(UIColorID::UI_COLOR_INPUT) = ui_color::INPUTLAVENDER;
    UIColor(UIColorID::UI_COLOR_OUTPUT) = ui_color::OUTPUTAPRICOT;

    UIColor(UIColorID::UI_COLOR_AVAILABLE) = ui_color::WIPBLUE;
    UIColor(UIColorID::UI_COLOR_INTERACT) = YELLOW;

    UIColor(UIColorID::UI_COLOR_ACTIVE) = ui_color::REDSTONE;

    UIColor(UIColorID::UI_COLOR_ERROR) = MAGENTA;
    UIColor(UIColorID::UI_COLOR_DESTRUCTIVE) = ui_color::DESTRUCTIVERED;
    UIColor(UIColorID::UI_COLOR_SPECIAL) = VIOLET;
    UIColor(UIColorID::UI_COLOR_CAUTION) = ui_color::CAUTIONYELLOW;

    UIColor(UIColorID::UI_COLOR_BLUEPRINTS_BACKGROUND) = { 10,15,30, 255 };

    if (!file.is_open()) // In case of deletion
    {
        file.close();

        std::ofstream replacement("config.ini");

        replacement <<
            "[Colors]"
            "\nbackground_color=" << ConfigColorToString(UIColor(UIColorID::UI_COLOR_BACKGROUND)) <<
            "\nbackground1_color=" << ConfigColorToString(UIColor(UIColorID::UI_COLOR_BACKGROUND1)) <<
            "\nbackground2_color=" << ConfigColorToString(UIColor(UIColorID::UI_COLOR_BACKGROUND2)) <<
            "\nbackground3_color=" << ConfigColorToString(UIColor(UIColorID::UI_COLOR_BACKGROUND3)) <<
            "\nforeground3_color=" << ConfigColorToString(UIColor(UIColorID::UI_COLOR_FOREGROUND3)) <<
            "\nforeground2_color=" << ConfigColorToString(UIColor(UIColorID::UI_COLOR_FOREGROUND2)) <<
            "\nforeground1_color=" << ConfigColorToString(UIColor(UIColorID::UI_COLOR_FOREGROUND1)) <<
            "\nforeground_color=" << ConfigColorToString(UIColor(UIColorID::UI_COLOR_FOREGROUND)) <<
            "\ninput_color=" << ConfigColorToString(UIColor(UIColorID::UI_COLOR_INPUT)) <<
            "\noutput_color=" << ConfigColorToString(UIColor(UIColorID::UI_COLOR_OUTPUT)) <<
            "\navailable_color=" << ConfigColorToString(UIColor(UIColorID::UI_COLOR_AVAILABLE)) <<
            "\ninteract_color=" << ConfigColorToString(UIColor(UIColorID::UI_COLOR_INTERACT)) <<
            "\nactive_color=" << ConfigColorToString(UIColor(UIColorID::UI_COLOR_ACTIVE)) <<
            "\nerror_color=" << ConfigColorToString(UIColor(UIColorID::UI_COLOR_ERROR)) <<
            "\ndestruction_color=" << ConfigColorToString(UIColor(UIColorID::UI_COLOR_DESTRUCTIVE)) <<
            "\naugment_color=" << ConfigColorToString(UIColor(UIColorID::UI_COLOR_SPECIAL)) <<
            "\ncaution_color=" << ConfigColorToString(UIColor(UIColorID::UI_COLOR_CAUTION)) <<
            "\nblueprint_background_color=" << ConfigColorToString(UIColor(UIColorID::UI_COLOR_BLUEPRINTS_BACKGROUND)) <<
            "\n\n[LOD]"
            "\nblueprint_menu_lod=1"
            "\nclipboard_preview_lod=0"
            "\npaste_preview_lod=4"
            "\nframes_per_tick=6"
            "\nui_scale=1";

        replacement.close();

        file.open("config.ini");
    }
    while (!file.eof())
    {
        std::string line;
        std::getline(file, line);
        if (line[0] == '[') // Comment
            continue;

        std::string attribute = line.substr(0, line.find('='));
        std::string value = line.substr(line.find('=') + 1);

        // Colors
        if (attribute == "background_color")   UIColor(UIColorID::UI_COLOR_BACKGROUND) = ConfigStrToColor(value);
        else if (attribute == "background1_color")  UIColor(UIColorID::UI_COLOR_BACKGROUND1) = ConfigStrToColor(value);
        else if (attribute == "background2_color")  UIColor(UIColorID::UI_COLOR_BACKGROUND2) = ConfigStrToColor(value);
        else if (attribute == "background3_color")  UIColor(UIColorID::UI_COLOR_BACKGROUND3) = ConfigStrToColor(value);
        else if (attribute == "foreground3_color")  UIColor(UIColorID::UI_COLOR_FOREGROUND3) = ConfigStrToColor(value);
        else if (attribute == "foreground2_color")  UIColor(UIColorID::UI_COLOR_FOREGROUND2) = ConfigStrToColor(value);
        else if (attribute == "foreground1_color")  UIColor(UIColorID::UI_COLOR_FOREGROUND1) = ConfigStrToColor(value);
        else if (attribute == "foreground_color")   UIColor(UIColorID::UI_COLOR_FOREGROUND) = ConfigStrToColor(value);
        else if (attribute == "input_color")        UIColor(UIColorID::UI_COLOR_INPUT) = ConfigStrToColor(value);
        else if (attribute == "output_color")       UIColor(UIColorID::UI_COLOR_OUTPUT) = ConfigStrToColor(value);
        else if (attribute == "available_color")    UIColor(UIColorID::UI_COLOR_AVAILABLE) = ConfigStrToColor(value);
        else if (attribute == "interact_color")     UIColor(UIColorID::UI_COLOR_INTERACT) = ConfigStrToColor(value);
        else if (attribute == "active_color")       UIColor(UIColorID::UI_COLOR_ACTIVE) = ConfigStrToColor(value);
        else if (attribute == "error_color")        UIColor(UIColorID::UI_COLOR_ERROR) = ConfigStrToColor(value);
        else if (attribute == "destruction_color")  UIColor(UIColorID::UI_COLOR_DESTRUCTIVE) = ConfigStrToColor(value);
        else if (attribute == "caution_color")      UIColor(UIColorID::UI_COLOR_CAUTION) = ConfigStrToColor(value);

        // Integers
        else if (attribute == "blueprint_menu_lod")     blueprintLOD = std::stoi(value);
        else if (attribute == "clipboard_preview_lod")  clipboardPreviewLOD = std::stoi(value);
        else if (attribute == "paste_preview_lod")      pastePreviewLOD = std::stoi(value);
        else if (attribute == "paste_preview_lod")      pastePreviewLOD = std::stoi(value);
        else if (attribute == "frames_per_tick")        framesPerTick = std::stoi(value);
        else if (attribute == "ui_scale")               uiScale = std::stoi(value);
    }

    if (uiScale >= 2)
        uiScale = 2;
    if (uiScale <= 1)
        uiScale = 1;

    IconButton::g_width = 16 * uiScale;
    switch (uiScale)
    {
    default:
        break;
    case 1:
        for (IconButton& b : modeButtons)
        {
            b.textureSheet = &modeIcons16x;
        }
        for (IconButton& b : gateButtons)
        {
            b.textureSheet = &gateIcons16x;
        }
        blueprintsButton.textureSheet = &blueprintIcon16x;
        clipboardButton.textureSheet = &clipboardIcon16x;
        break;
    case 2:
        for (IconButton& b : modeButtons)
        {
            b.textureSheet = &modeIcons32x;
        }
        for (IconButton& b : gateButtons)
        {
            b.textureSheet = &gateIcons32x;
        }
        blueprintsButton.textureSheet = &blueprintIcon32x;
        clipboardButton.textureSheet = &clipboardIcon32x;
        break;
    }

    file.close();
}

void Window::UpdateTool()
{
    if (!!overlay)
        overlay->Update(*this);
    else
        base->Update(*this);
}
void Window::DrawTool()
{
    if (!!overlay)
        overlay->Draw(*this);
    else
        base->Draw(*this);
}
void Window::PushProperty(const char* name, const char* value)
{
    const int propHeight = FontSize() * 2;
    const IVec2 padding(FontSize() / 2);
    IRect box(propertiesPaneRec.x, propHeight * propertyNumber, propertiesPaneRec.w, propHeight);
    DrawRectangleLines(box.x, box.y, box.w, box.h, UIColor(UIColorID::UI_COLOR_BACKGROUND2));
    const int propertiesPaneMiddle = propertiesPaneRec.w / 3;
    const int propertiesPaneMiddleAbs = propertiesPaneRec.x + propertiesPaneMiddle;
    DrawLine(propertiesPaneMiddleAbs, box.y, propertiesPaneMiddleAbs, box.Bottom(), UIColor(UIColorID::UI_COLOR_BACKGROUND2));
    DrawTextIV(name, box.xy + padding, FontSize(), UIColor(UIColorID::UI_COLOR_FOREGROUND));
    DrawTextIV(value, box.xy + Width(propertiesPaneMiddle) + padding, FontSize(), UIColor(UIColorID::UI_COLOR_FOREGROUND));
    propertyNumber++;
}
void Window::PushProperty_int(const char* name, int value)
{
    PushProperty(name, TextFormat("%i", value));
}
void Window::PushProperty_uint(const char* name, int value)
{
    PushProperty(name, TextFormat("%u", value));
}
void Window::PushProperty_ptr(const char* name, void* value)
{
    PushProperty(name, TextFormat("%p", value));
}
void Window::PushProperty_str(const char* name, const std::string& value)
{
    PushProperty(name, value.c_str());
}
void Window::PushProperty_bool(const char* name, bool value)
{
    PushProperty(name, value ? "true" : "false");
}
void Window::PushPropertyTitle(const char* title)
{
    const int propHeight = FontSize() * 2;
    const IVec2 padding(FontSize() / 2);
    IRect box(propertiesPaneRec.x, propHeight * propertyNumber, propertiesPaneRec.w, propHeight);
    DrawRectangleIRect(ShrinkIRect(box), UIColor(UIColorID::UI_COLOR_BACKGROUND2));
    DrawTextIV(title, box.xy + padding, FontSize(), UIColor(UIColorID::UI_COLOR_FOREGROUND));
    propertyNumber++;
}
void Window::PushPropertySubtitle(const char* title, Color color)
{
    const int propHeight = FontSize() * 2;
    const IVec2 padding(FontSize() / 2);
    IVec2 pos(propertiesPaneRec.x, propHeight * propertyNumber);
    DrawTextIV(title, pos + padding, FontSize(), color);
    propertyNumber++;
}
void Window::PushPropertySection_Node(const char* name, Node* value)
{
    if (!!value)
    {
        PushPropertySubtitle(name);
        PushProperty_ptr("Pointer", value);
        PushProperty_uint("Serial", CurrentTab().NodeID(value));
        PushProperty("Type", GateName(value->GetGate()));
        switch (value->GetGate())
        {
        case Gate::RESISTOR:  PushProperty_uint("Resistance", value->GetResistance());  break;
        case Gate::CAPACITOR: PushProperty_uint("Capacity", value->GetCapacity());      break;
        case Gate::LED:       PushProperty_uint("Color index", value->GetColorIndex()); break;
        default: break;
        }
        PushPropertySubtitle("Inputs", UIColor(UIColorID::UI_COLOR_INPUT));
        PushProperty_uint("Count", value->GetInputCount());
        if (value->GetInputCount() == 0)
            PushProperty("Note", "Available for interaction");

        for (const Wire* wire : value->GetInputsConst())
        {
            PushProperty_ptr("\tPointer", wire->start);
            PushProperty("\tState", StateName(wire->GetState()));
        }
        PushPropertySubtitle("Outputs", UIColor(UIColorID::UI_COLOR_OUTPUT));
        PushProperty("State", StateName(value->GetState()));
        PushProperty_uint("Count", value->GetOutputCount());
        for (const Wire* wire : value->GetOutputsConst())
        {
            PushProperty_ptr("\tPointer", wire->end);
        }
    }
}
void Window::PushPropertySection_Wire(const char* name, Wire* value)
{
    if (!!value)
    {
        PushPropertySubtitle(name);
        PushProperty_ptr("Pointer", value);
        PushProperty("Joint config", ElbowConfigName(value->elbowConfig));
        PushProperty("State", StateName(value->GetState()));
        PushPropertySubtitle("Input", UIColor(UIColorID::UI_COLOR_INPUT));
        PushProperty_uint("Serial", CurrentTab().NodeID(value->start));
        PushProperty_ptr("Pointer", value->start);
        PushPropertySubtitle("Output", UIColor(UIColorID::UI_COLOR_OUTPUT));
        PushProperty_uint("Serial", CurrentTab().NodeID(value->end));
        PushProperty_ptr("Pointer", value->end);
    }
}
void Window::PushPropertySection_Selection(const char* name, const std::vector<Node*>& value)
{
    PushPropertySubtitle(name);

    unsigned ORs = 0;
    unsigned ANDs = 0;
    unsigned NORs = 0;
    unsigned XORs = 0;
    unsigned RESs = 0;
    unsigned CAPs = 0;
    unsigned LEDs = 0;
    unsigned DELs = 0;
    unsigned BATs = 0;

    for (Node* node : value)
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

    PushProperty_uint("Total", value.size());
    if (ORs)  PushProperty_uint("OR gates", ORs);
    if (ANDs) PushProperty_uint("AND gates", ANDs);
    if (NORs) PushProperty_uint("NOR gates", NORs);
    if (XORs) PushProperty_uint("XOR gates", XORs);
    if (RESs) PushProperty_uint("Resistors", RESs);
    if (CAPs) PushProperty_uint("Capacitors", CAPs);
    if (LEDs) PushProperty_uint("LEDs", LEDs);
    if (DELs) PushProperty_uint("Delays", DELs);
    if (BATs) PushProperty_uint("Batteries", BATs);
}
void Window::PushPropertySection_Group(const char* name, Group* value)
{
    if (!!value)
    {
        PushPropertySubtitle(name);
        PushProperty_ptr("Pointer", value);
        PushProperty_str("Label", value->GetLabel());
    }
}
void Window::DrawToolProperties()
{
    EndMode2D(); // In case
    DrawRectangleIRect(propertiesPaneRec, UIColor(UIColorID::UI_COLOR_BACKGROUND1));
    propertyNumber = 0;
    PushPropertyTitle("Properties");
    if (!!overlay)
        overlay->DrawProperties(*this);
    else
        base->DrawProperties(*this);
}
