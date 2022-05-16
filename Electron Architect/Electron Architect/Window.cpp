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
#include "Tool.h"
#include "Window.h"

#include "icon_blueprints16x.h"
#include "icon_blueprints32x.h"
#include "icon_clipboard16x.h"
#include "icon_clipboard32x.h"
#include "icons_gate16x.h"
#include "icons_gate32x.h"
#include "icons_mode16x.h"
#include "icons_mode32x.h"

#define MEMORY_IMAGE(name) CLITERAL(Image){(name##_DATA),(name##_WIDTH),(name##_HEIGHT),1,(name##_FORMAT)}

Blueprint g_clipboardBP;

void DrawTextShadowedIV(const std::string& text, IVec2 pos, int fontSize, Color color, Color shadow)
{
    const char* str = text.c_str();
    for (int y = -1; y <= 1; ++y)
    {
        for (int x = -1; x <= 1; ++x)
        {
            DrawTextIV(str, pos + IVec2(x, y), fontSize, shadow);
        }
    }
    DrawTextIV(str, pos, fontSize, color);
}

Window::Window() :
    windowWidth(1280),
    windowHeight(720),
    consoleOutput{ "", "", "", "", "", "", },
    modeButtons{
        IconButton(
            IVec2(),
            "Mode: Draw [b]",
            "[left click] to create a node and wire.\n"
            "  if a node exists, a wire will be created from it.\n"
            "[left click] again to connect to another node\n"
            "  if no node exists, one will be made.\n"
            "[right click] to stop drawing.\n"
            "Hold [shift] while creating a wire for parallel.\n"
            "Hold [ctrl] while creating a wire to reverse it.\n"
            "Press [r] to cycle through wire joints.",
            [this]() { SetMode(Mode::PEN); },
            IVec2(0, 0),
            &modeIcons16x),

        IconButton(
            IVec2(),
            "Mode: Edit [v]",
            "Drag with [left click] to select.\n"
            "Drag nodes with [left click].\n"
            "Hold [ctrl] to make multiple selections.\n"
            "Press [ctrl]+[g] to make a group.\n"
            "[right click] a group to rename it.\n"
            "Drag wire joints with [left click].\n"
            "  Wire joints snap to 45 degree angles.\n"
            "[right click] a node to apply current gate.\n"
            "Press [space] to bridge nodes with wires\n"
            "  Must have exactly two selections\n"
            "  One selection must contain exactly 1 node\n"
            "  OR both rectangles must have equal nodes.\n"
            "  Selections will change colors if valid.\n"
            "  Bridge wires are connected in the order:\n"
            "    left to right\n"
            "    if horizontally same, top to bottom\n"
            "Press [r] to cycle through bridge joints.",
            [this]() { SetMode(Mode::EDIT); },
            IVec2(1, 0),
            &modeIcons16x),

        IconButton(
            IVec2(),
            "Mode: Erase [x]",
            "[Left click] a node, wire, or group to erase it.\n"
            "[shift]+[left click] to bypass the node without\n"
            "  erasing the wires connected to it.",
            [this]() { SetMode(Mode::ERASE); },
            IVec2(0, 1),
            &modeIcons16x),

        IconButton(
            IVec2(),
            "Mode: Interact [f]",
            "[left click] an interactable node to toggle it on/off.\n"
            "  A node is interactable if it has no inputs.",
            [this]() { SetMode(Mode::INTERACT); },
            IVec2(1, 1),
            & modeIcons16x),
    },
    gateButtons{
        IconButton(
            IVec2(),
            "Gate: Or [1]",
            "Outputs true if any input is true,\n"
            "Outputs false otherwise.",
            [this]() { SetGate(Gate::OR); },
            IVec2(0,0),
            &gateIcons16x),

        IconButton(
            IVec2(),
            "Gate: And [2]",
            "Outputs true if all inputs are true,\n"
            "Outputs false otherwise.",
            [this]() { SetGate(Gate::AND); },
            IVec2(1,0),
            &gateIcons16x),

        IconButton(
            IVec2(),
            "Gate: Nor [3]",
            "Outputs false if any input is true.\n"
            "Outputs true otherwise.",
            [this]() { SetGate(Gate::NOR); },
            IVec2(0,1),
            &gateIcons16x),

        IconButton(
            IVec2(),
            "Gate: Xor [4]",
            "Outputs true if exactly 1 input is true,\n"
            "Outputs false otherwise.\n"
            "Order of inputs does not matter.",
            [this]() { SetGate(Gate::XOR); },
            IVec2(1,1),
            &gateIcons16x),

        IconButton(
            IVec2(),
            "Element: Resistor [5]",
            "Outputs true if > resistance inputs are true,\n"
            "Outputs false otherwise.\n"
            "Order of inputs does not matter.",
            [this]() { SetGate(Gate::RESISTOR); },
            IVec2(0,2),
            &gateIcons16x),

        IconButton(
            IVec2(),
            "Element: Capacitor [6]",
            "Stores charge while any input is true.\n"
            "Stops charging once charge = capacity.\n"
            "Drains charge while no input is true.\n"
            "Outputs true while charge > zero,\n"
            "Outputs true while any input is true,\n"
            "Outputs false otherwise.",
            [this]() { SetGate(Gate::CAPACITOR); },
            IVec2(1,2),
            &gateIcons16x),

        IconButton(
            IVec2(),
            "Element: LED [7]",
            "Treats I/O the same as an OR gate.\n"
            "Lights up with the selected color when powered.",
            [this]() { SetGate(Gate::LED); },
            IVec2(0,3),
            &gateIcons16x),

        IconButton(
            IVec2(),
            "Element: Delay [8]",
             "Treats I/O the same as an OR gate.\n"
            "Outputs with a 1-tick delay.\n"
            "Sequntial delay devices are recommended\n"
            "for delay greater than 1 tick.",
            [this]() { SetGate(Gate::DELAY); },
            IVec2(1,3),
            &gateIcons16x),

        IconButton(
            IVec2(),
            "Element: Battery [9]",
            "Always outputs true, regardless of inputs.\n"
            "Batteries will always be start nodes - \n"
            "  that is, batteries will always be treated as\n"
            "  entrypoints for graph traversal.",
            [this]() { SetGate(Gate::BATTERY); },
            IVec2(0,4),
            &gateIcons16x),
    },
    paramButtons{
        ColorButton(
            IVec2(),
            "Parameter: 0 [ctrl+0]/[numpad 0]",
            "@TODO",
            [this]() { storedExtraParam = 0; },
Node::g_resistanceBands[0],
"0"),

ColorButton(
    IVec2(),
    "Parameter: 1 [ctrl+1]/[numpad 1]",
    "@TODO",
    [this]() { storedExtraParam = 1; },
    Node::g_resistanceBands[1],
    "1"),

    ColorButton(
        IVec2(),
        "Parameter: 2 [ctrl+2]/[numpad 2]",
        "@TODO",
        [this]() { storedExtraParam = 2; },
        Node::g_resistanceBands[2],
        "2"),

    ColorButton(
        IVec2(),
        "Parameter: 3 [ctrl+3]/[numpad 3]",
        "@TODO",
        [this]() { storedExtraParam = 3; },
        Node::g_resistanceBands[3],
        "3"),

    ColorButton(
        IVec2(),
        "Parameter: 4 [ctrl+4]/[numpad 4]",
        "@TODO",
        [this]() { storedExtraParam = 4; },
        Node::g_resistanceBands[4],
        "4"),

ColorButton(
    IVec2(),
    "Parameter: 5 [ctrl+5]/[numpad 5]",
    "@TODO",
    [this]() { storedExtraParam = 5; },
    Node::g_resistanceBands[5],
    "5"),

    ColorButton(
        IVec2(),
        "Parameter: 6 [ctrl+6]/[numpad 6]",
        "@TODO",
        [this]() { storedExtraParam = 6; },
        Node::g_resistanceBands[6],
        "6"),

    ColorButton(
        IVec2(),
        "Parameter: 7 [ctrl+7]/[numpad 7]",
        "@TODO",
        [this]() { storedExtraParam = 7; },
        Node::g_resistanceBands[7],
        "7"),

    ColorButton(
        IVec2(),
        "Parameter: 8 [ctrl+8]/[numpad 8]",
        "@TODO",
        [this]() { storedExtraParam = 8; },
        Node::g_resistanceBands[8],
        "8"),

    ColorButton(
        IVec2(),
        "Parameter: 9 [ctrl+9]/[numpad 9]",
        "@TODO",
        [this]() { storedExtraParam = 9; },
        Node::g_resistanceBands[9],
        "9"),
    },
    blueprintsButton(
        IVec2(),
        "Blueprints",
        "@TODO",
        [this]() { SetMode(Mode::BP_SELECT); },
        IVec2::Zero(),
        & blueprintIcon16x
    ),
    clipboardButton(
        IVec2(),
        "Clipboard (ctrl+c to copy, ctrl+v to paste)",
        "@TODO",
        [this]() { if (this->IsClipboardValid()) SetMode(Mode::PASTE); },
        IVec2::Zero(),
        & clipboardIcon16x
    ),
    toolPaneSizeButton(
        IVec2(0),
        "Toggle toolpane size",
        "@TODO",
        [this]() { ToggleToolPaneSize(); },
        "+",
        1
    ),
    propertiesToggleButton(
        IVec2(),
        "Toggle the properties pane",
        "@TODO",
        [this]() { ToggleProperties(); },
        "P",
        1
        ),
    consoleToggleButton(
        IVec2(),
        "Toggle the console pane",
        "@TODO",
        [this]() { ToggleConsole(); },
        "C",
        1
        ),
    allButtons{
        &modeButtons[0],
        &modeButtons[1],
        &modeButtons[2],
        &modeButtons[3],
        &gateButtons[0],
        &gateButtons[1],
        &gateButtons[2],
        &gateButtons[3],
        &gateButtons[4],
        &gateButtons[5],
        &gateButtons[6],
        &gateButtons[7],
        &gateButtons[8],
        &paramButtons[0],
        &paramButtons[1],
        &paramButtons[2],
        &paramButtons[3],
        &paramButtons[4],
        &paramButtons[5],
        &paramButtons[6],
        &paramButtons[7],
        &paramButtons[8],
        &paramButtons[9],
        &blueprintsButton,
        &clipboardButton,
        &toolPaneSizeButton,
        &propertiesToggleButton,
        &consoleToggleButton,
    }
{
    ClearLog();
    InitWindow(windowWidth, windowHeight, "Electron Architect");
    SetExitKey(0);
    SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));

    blueprintIcon16x = LoadTextureFromImage(MEMORY_IMAGE(ICON_BLUEPRINTS16X));
    blueprintIcon32x = LoadTextureFromImage(MEMORY_IMAGE(ICON_BLUEPRINTS32X));
    clipboardIcon16x = LoadTextureFromImage(MEMORY_IMAGE(ICON_CLIPBOARD16X));
    clipboardIcon32x = LoadTextureFromImage(MEMORY_IMAGE(ICON_CLIPBOARD32X));
    modeIcons16x = LoadTextureFromImage(MEMORY_IMAGE(ICONS_MODE16X));
    modeIcons32x = LoadTextureFromImage(MEMORY_IMAGE(ICONS_MODE32X));
    gateIcons16x = LoadTextureFromImage(MEMORY_IMAGE(ICONS_GATE16X));
    gateIcons32x = LoadTextureFromImage(MEMORY_IMAGE(ICONS_GATE32X));

    activeTab = 0;
    tabs.push_back(new Tab(this, "Unnamed graph"));

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
        tab->graph->Save("session.cg");
        tab->graph->Export("render.svg");
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
const Tab& Window::CurrentTab() const
{
    _ASSERT_EXPR(activeTab < tabs.size(), L"Subscript out of range");
    _ASSERT_EXPR(!!tabs[activeTab], L"Current tab is nullptr");
    return *tabs[activeTab];
}

const IconButton* Window::ButtonFromMode(Mode mode) const
{
    switch (mode)
    {
    default:
        // Only basic modes need buttons
        _ASSERT_EXPR(TypeOfMode(mode) != ModeType::Basic, "Missing specialization");
        return nullptr;

    case Mode::PEN:         return &modeButtons[0];
    case Mode::EDIT:        return &modeButtons[1];
    case Mode::ERASE:       return &modeButtons[2];
    case Mode::INTERACT:    return &modeButtons[3];
    }
}
const IconButton* Window::ButtonFromGate(Gate gate) const
{
    switch (gate)
    {
    default: 
        _ASSERT_EXPR(false, "Missing specialization");
        return nullptr;

    case Gate::OR:          return &gateButtons[0];
    case Gate::AND:         return &gateButtons[1];
    case Gate::NOR:         return &gateButtons[2];
    case Gate::XOR:         return &gateButtons[3];
    case Gate::RESISTOR:    return &gateButtons[4];
    case Gate::CAPACITOR:   return &gateButtons[5];
    case Gate::LED:         return &gateButtons[6];
    case Gate::DELAY:       return &gateButtons[7];
    case Gate::BATTERY:     return &gateButtons[8];
    }
}
const ColorButton* Window::ButtonFromParameter(uint8_t param) const
{
    return &paramButtons[param];
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
IVec2 Window::FontPadding() const
{
    switch (uiScale)
    {
    default:
    case 1: return IVec2(4);
    case 2: return IVec2(10);
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
    Log(LogType::attempt, "Changing mode from { base: " +
        (base ? ModeName(base->GetMode()) : "null") + "; overlay: " +
        (overlay ? ModeName(overlay->GetMode()) : "null") + " } to " + ModeName(newMode));

    b_cursorMoved = true;

    if (!!overlay ? newMode != overlay->GetMode() : TypeOfMode(newMode) != ModeType::Basic)
    {
        if (!!overlay)
        {
            delete overlay;
            Log(LogType::info, "Deleted overlay mode");
        }

        if (TypeOfMode(newMode) != ModeType::Basic)
        {
            switch (newMode)
            {
            default:
                Log(LogType::warning, "Missing overlay mode construct for mode. Defaulting to null");
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
        {
            overlay = nullptr;
            Log(LogType::info, "Annulled overlay mode");
        }
    }

    if (TypeOfMode(newMode) == ModeType::Basic &&
        (!!base ? newMode != base->GetMode() : true))
    {
        if (!!base) [[likely]] // There should only be one point in the program where base is nullptr. Namely, the start.
        {
            delete base;
            Log(LogType::info, "Deleted base mode");
        }

        switch (newMode)
        {
        default:
            Log(LogType::warning, "Missing base mode construct for mode. Defaulting to null");
            base = nullptr;
            break;

        case Mode::PEN:         base = new PenTool;       break;
        case Mode::EDIT:        base = new EditTool;      break;
        case Mode::ERASE:       base = new EraseTool;     break;
        case Mode::INTERACT:    base = new InteractTool;  break;

        case Mode::PASTE:       base = new PasteOverlay;  break;

        case Mode::BP_SELECT:   base = new BlueprintMenu; break;
        }
    }
    if (!base)
    {
        Log(LogType::error, "Base mode is null");
        exit(1);
    }
    Log(LogType::success, "Mode changed to { " +
        (base ? ModeName(base->GetMode()) : "null") + ";" +
        (overlay ? ModeName(overlay->GetMode()) : "null") + " }");
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
    Log(LogType::info, "Changed gate from " + GateName(gatePick) + " to " + GateName(newGate));
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
    Log(LogType::info, "Clearing overlay mode");
    if (!base)
    {
        Log(LogType::error, "Base mode was not initialized or got nullified");
        exit(1);
    }
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

    if (!tabs.empty())
        cursorPos = IVec2(GetScreenToWorld2D(GetMousePosition(), CurrentTab().camera));
    else
        cursorPos = cursorUIPos;

    // Snap
    cursorPos /= g_gridSize;
    cursorPos *= g_gridSize;
    {
        constexpr int halfgrid = g_gridSize / 2;
        if (cursorPos.x < 0)
            cursorPos.x -= halfgrid;
        else
            cursorPos.x += halfgrid;

        if (cursorPos.y < 0)
            cursorPos.y -= halfgrid;
        else
            cursorPos.y += halfgrid;
    }

    b_cursorMoved = cursorPosPrev != cursorPos;
}

// Zoom/pan

void Window::UpdateCamera()
{
    if (GetModeType() != ModeType::Menu)
        CurrentTab().UpdateCamera();
}

void Window::UpdateSize()
{
    windowWidth = GetRenderWidth();
    windowHeight = GetRenderHeight();
    Log(LogType::info, "Window width is now " + std::to_string(windowWidth));
    Log(LogType::info, "Window height is now " + std::to_string(windowHeight));
    ReloadPanes();
}

void Window::CopySelectionToClipboard()
{
    // Todo: actually copy a csv to the user's clipboard
    Log(LogType::info, "Copied selection to clipboard");
    if (!CurrentTab().selection.empty()) // Copy selection
    {
        g_clipboardBP = Blueprint(CurrentTab().selection);
        clipboard = &g_clipboardBP;
    }
    else // Clear selection
        clipboard = nullptr;
}

void Window::MakeGroupFromSelection()
{
    if (!CurrentTab().SelectionRectExists())
        return;

    for (const IRect& rec : CurrentTab().SelectionRecs())
    {
        CurrentTab().graph->CreateGroup(rec, UIColor(UIColorID::UI_COLOR_AVAILABLE));
    }
    CurrentTab().ClearSelection();
}

bool Window::IsSelectionRectValid() const
{
    EditTool* edit = dynamic_cast<EditTool*>(base);
    return !!edit && !edit->selectionWIP && CurrentTab().SelectionRectExists();
}

void Window::SaveBlueprint()
{
    if (!IsClipboardValid())
        return;
    CurrentTab().graph->StoreBlueprint(clipboard);
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
    return CurrentTab().SelectionExists();
}

void Window::ClearSelection()
{
    CurrentTab().ClearSelection();
}

void Window::DestroySelection()
{
    CurrentTab().graph->DestroyNodes(CurrentTab().selection);
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

        // Parameter hotkeys
        if (GetModeType() == ModeType::Basic)
        {
            if      (IsKeyPressed(KEY_ZERO))  storedExtraParam = 0;
            else if (IsKeyPressed(KEY_ONE))   storedExtraParam = 1;
            else if (IsKeyPressed(KEY_TWO))   storedExtraParam = 2;
            else if (IsKeyPressed(KEY_THREE)) storedExtraParam = 3;
            else if (IsKeyPressed(KEY_FOUR))  storedExtraParam = 4;
            else if (IsKeyPressed(KEY_FIVE))  storedExtraParam = 5;
            else if (IsKeyPressed(KEY_SIX))   storedExtraParam = 6;
            else if (IsKeyPressed(KEY_SEVEN)) storedExtraParam = 7;
            else if (IsKeyPressed(KEY_EIGHT)) storedExtraParam = 8;
            else if (IsKeyPressed(KEY_NINE))  storedExtraParam = 9;
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
            else CurrentTab().graph->Save(TextFormat("%s.cg", CurrentTab().graph->GetName()));
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

        if      (IsKeyPressed(KEY_KP_0)) storedExtraParam = 0;
        else if (IsKeyPressed(KEY_KP_1)) storedExtraParam = 1;
        else if (IsKeyPressed(KEY_KP_2)) storedExtraParam = 2;
        else if (IsKeyPressed(KEY_KP_3)) storedExtraParam = 3;
        else if (IsKeyPressed(KEY_KP_4)) storedExtraParam = 4;
        else if (IsKeyPressed(KEY_KP_5)) storedExtraParam = 5;
        else if (IsKeyPressed(KEY_KP_6)) storedExtraParam = 6;
        else if (IsKeyPressed(KEY_KP_7)) storedExtraParam = 7;
        else if (IsKeyPressed(KEY_KP_8)) storedExtraParam = 8;
        else if (IsKeyPressed(KEY_KP_9)) storedExtraParam = 9;
    }

    // Mode hotkeys
    if      (IsKeyPressed(KEY_B)) SetMode(Mode::PEN);
    else if (IsKeyPressed(KEY_V)) SetMode(Mode::EDIT);
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

    // Help
    if (IsKeyPressed(KEY_F1))
    {
        // Todo
    }
    // Rename
    else if (IsKeyPressed(KEY_F2))
    {
        // Todo
    }
    // Search
    else if (IsKeyPressed(KEY_F3))
    {
        // Todo
    }
    // Nothing yet
    else if (IsKeyPressed(KEY_F4))
    {
        // Todo
    }
    // Reload graph
    else if (IsKeyPressed(KEY_F5))
    {
        CurrentTab().graph->Sort();
    }
    // Nothing yet
    else if (IsKeyPressed(KEY_F6))
    {
        // Todo
    }
    // Nothing yet
    else if (IsKeyPressed(KEY_F7))
    {
        // Todo
    }
    // Nothing yet
    else if (IsKeyPressed(KEY_F8))
    {
        // Todo
    }
    // Nothing yet
    else if (IsKeyPressed(KEY_F9))
    {
        // Todo
    }
    // Toggle ribbon
    else if (IsKeyPressed(KEY_F10))
    {
        // Todo
    }
    // Fullscreen
    else if (IsKeyPressed(KEY_F11))
    {
        if (!IsWindowFullscreen())
            SetWindowSize(GetMonitorWidth(GetCurrentMonitor()), GetMonitorHeight(GetCurrentMonitor()));
        else
            SetWindowSize(1280, 720);
        ToggleFullscreen();
        if (!IsWindowFullscreen())
            MaximizeWindow();
        UpdateSize();
    }
    // Save
    //else if (IsKeyPressed(KEY_F12))
    //{
    //    // Export SVG
    //    if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) &&
    //        (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)))
    //    {
    //        CurrentTab().graph->Export(CurrentTab().graph->GetName() + ".svg");
    //    }
    //    // Save
    //    else
    //    {
    //        CurrentTab().graph->Save(CurrentTab().graph->GetName() + ".cg");
    //    }
    //}
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
    if (!tabs.empty())
    {
        Vector2 start = GetScreenToWorld2D({ 0,0 }, CurrentTab().camera);
        start.x = std::floor(start.x / (float)gridSize) * (float)gridSize;
        start.y = std::floor(start.y / (float)gridSize) * (float)gridSize;
        Vector2 end = GetScreenToWorld2D({ (float)windowWidth, (float)windowHeight }, CurrentTab().camera);
        end.x = std::ceil(end.x / (float)gridSize) * (float)gridSize;
        end.y = std::ceil(end.y / (float)gridSize) * (float)gridSize;
        Rectangle bounds = { start.x, start.y, end.x - start.x, end.y - start.y };

        // "If the number of world pixels compacted into a single screen pixel equal or exceed the pixels between gridlines"
        if ((int)(1.0f / CurrentTab().camera.zoom) >= gridSize)
        {
            DrawRectangleRec(bounds, UIColor(UIColorID::UI_COLOR_BACKGROUND1));
        }
        else
        {
            for (int y = start.y; y < end.y; y += gridSize)
            {
                DrawLine(start.x, y, end.x, y, UIColor(UIColorID::UI_COLOR_BACKGROUND1));
            }
            for (int x = start.x; x < end.x; x += gridSize)
            {
                DrawLine(x, start.y, x, end.y, UIColor(UIColorID::UI_COLOR_BACKGROUND1));
            }
        }
        int halfgrid = gridSize / 2;
        DrawLine(start.x, -halfgrid, end.x, -halfgrid, UIColor(UIColorID::UI_COLOR_BACKGROUND2));
        DrawLine(-halfgrid, start.y, -halfgrid, end.y, UIColor(UIColorID::UI_COLOR_BACKGROUND2));
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
    if (!tabs.empty())
        return GetSelectionBounds(CurrentTab().selection);
    else
        return IRect(0);
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

void Window::DrawTooltipAtCursor(const std::string& text, Color color)
{
    CurrentTab().Set2DMode(false);
    DrawTextIV(text.c_str(), cursorUIPos + IVec2(16), FontSize(), color);
    if (!tabs.empty())
        CurrentTab().Set2DMode(true);
}

void Window::DrawTooltipAtCursor_Shadowed(const std::string& text, Color color)
{
    CurrentTab().Set2DMode(false);
    DrawTextShadowedIV(text, cursorUIPos + IVec2(16), FontSize(), color, UIColor(UIColorID::UI_COLOR_BACKGROUND));
    if (!tabs.empty())
        CurrentTab().Set2DMode(true);
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

IRect ConfigStrToIRect(const std::string& str)
{
    size_t separators[3]{ 0,0,0 };
    separators[0] = str.find('|');
    if (separators[0] == str.npos)
        return IRect(0);
    separators[1] = str.find('|', separators[0] + 1);
    if (separators[1] == str.npos)
        return IRect(0);
    separators[2] = str.find('|', separators[1] + 1);
    if (separators[2] == str.npos)
        return IRect(0);
    try
    {
        int x = std::stoi(str.substr(0, separators[0]));
        int y = std::stoi(str.substr(separators[0] + 1, separators[1] - (separators[0] + 1)));
        int w = std::stoi(str.substr(separators[1] + 1, separators[2] - (separators[1] + 1)));
        int h = std::stoi(str.substr(separators[2] + 1));
        return IRect(x, y, w, h);
    }
    catch (...)
    {
        return IRect(0);
    }
}

const char* ConfigIRectToString(IRect rec)
{
    return TextFormat("%i|%i|%i|%i", rec.x, rec.y, rec.w, rec.h);
}

void Window::ReloadPanes()
{
    propertiesPaneRec.y = 0;
    propertiesPaneRec.h = windowHeight;
    propertiesPaneRec.w = 256 * uiScale;
    propertiesPaneRec.x = windowWidth - propertiesPaneRec.w;

    consolePaneRec.x = 0;
    consolePaneRec.h = FontSize() * 7 * 2;
    consolePaneRec.y = windowHeight - consolePaneRec.h;
    if (propertiesOn)
        consolePaneRec.w = windowWidth - propertiesPaneRec.w;
    else
        consolePaneRec.w = windowWidth;
        
    toolPaneRec.xy = IVec2(0);
    toolPaneRec.w = (toolPaneSizeState ? 3 * Button::g_width : Button::g_width);
    if (consoleOn)
        toolPaneRec.h = windowHeight - consolePaneRec.h;
    else
        toolPaneRec.h = windowHeight;

    ReloadToolPane();
}

IVec2 Window::WindowExtents() const
{
    return IVec2(windowWidth, windowHeight);
}

void Window::SaveConfig() const
{
    std::ofstream config("config.ini", std::ios_base::trunc);

    config <<
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
        "\n\n[Performance]"
        "\nblueprint_menu_lod=" << (int)blueprintLOD <<
        "\nclipboard_preview_lod=" << (int)clipboardPreviewLOD <<
        "\npaste_preview_lod=" << (int)pastePreviewLOD <<
        "\nframes_per_tick=" << (int)framesPerTick <<
        "\n\n[Preferences]"
        "\nwindow_position_size=" << GetWindowPosition().x << '|' << GetWindowPosition().y << '|' << GetRenderWidth() << '|' << GetRenderHeight() <<
        "\nui_scale=" << uiScale <<
        "\ntoolpane_expanded=" << toolPaneSizeState <<
        "\nshow_console=" << consoleOn <<
        "\nshow_properties=" << propertiesOn <<
        "\nmin_log_level=" << (int)minLogLevel <<
        "\nselection_preview=" << selectionPreview;
        
    config.close();
}
void Window::ReloadConfig()
{
    // Defaults
    {
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

        SetWindowPosition(320, 180);
        SetWindowSize(1280,720);
        windowWidth = 1280;
        windowHeight = 720;

        blueprintLOD = 0;
        clipboardPreviewLOD = 0;
        pastePreviewLOD = 0;
        framesPerTick = 6;
        uiScale = 1;
        toolPaneSizeState = 1;
        consoleOn = 1;
        propertiesOn = 1;
        minLogLevel = LogType::warning;
        selectionPreview = false;
    }

    std::ifstream file("config.ini");
    if (!file.is_open()) // In case of deletion
    {
        file.close();

        SaveConfig();

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
        if      (attribute == "background_color")   UIColor(UIColorID::UI_COLOR_BACKGROUND)  = ConfigStrToColor(value);
        else if (attribute == "background1_color")  UIColor(UIColorID::UI_COLOR_BACKGROUND1) = ConfigStrToColor(value);
        else if (attribute == "background2_color")  UIColor(UIColorID::UI_COLOR_BACKGROUND2) = ConfigStrToColor(value);
        else if (attribute == "background3_color")  UIColor(UIColorID::UI_COLOR_BACKGROUND3) = ConfigStrToColor(value);
        else if (attribute == "foreground3_color")  UIColor(UIColorID::UI_COLOR_FOREGROUND3) = ConfigStrToColor(value);
        else if (attribute == "foreground2_color")  UIColor(UIColorID::UI_COLOR_FOREGROUND2) = ConfigStrToColor(value);
        else if (attribute == "foreground1_color")  UIColor(UIColorID::UI_COLOR_FOREGROUND1) = ConfigStrToColor(value);
        else if (attribute == "foreground_color")   UIColor(UIColorID::UI_COLOR_FOREGROUND)  = ConfigStrToColor(value);
        else if (attribute == "input_color")        UIColor(UIColorID::UI_COLOR_INPUT)       = ConfigStrToColor(value);
        else if (attribute == "output_color")       UIColor(UIColorID::UI_COLOR_OUTPUT)      = ConfigStrToColor(value);
        else if (attribute == "available_color")    UIColor(UIColorID::UI_COLOR_AVAILABLE)   = ConfigStrToColor(value);
        else if (attribute == "interact_color")     UIColor(UIColorID::UI_COLOR_INTERACT)    = ConfigStrToColor(value);
        else if (attribute == "active_color")       UIColor(UIColorID::UI_COLOR_ACTIVE)      = ConfigStrToColor(value);
        else if (attribute == "error_color")        UIColor(UIColorID::UI_COLOR_ERROR)       = ConfigStrToColor(value);
        else if (attribute == "destruction_color")  UIColor(UIColorID::UI_COLOR_DESTRUCTIVE) = ConfigStrToColor(value);
        else if (attribute == "caution_color")      UIColor(UIColorID::UI_COLOR_CAUTION)     = ConfigStrToColor(value);
        else if (attribute == "blueprint_background_color") UIColor(UIColorID::UI_COLOR_BLUEPRINTS_BACKGROUND) = ConfigStrToColor(value);

        // Integers
        else if (attribute == "blueprint_menu_lod")     blueprintLOD        = std::stoi(value);
        else if (attribute == "clipboard_preview_lod")  clipboardPreviewLOD = std::stoi(value);
        else if (attribute == "paste_preview_lod")      pastePreviewLOD     = std::stoi(value);
        else if (attribute == "frames_per_tick")        framesPerTick       = std::stoi(value);
        else if (attribute == "window_position_size")
        {
            IRect windowRec = ConfigStrToIRect(value);
            SetWindowPosition(windowRec.x, windowRec.y);
            SetWindowSize(windowRec.w, windowRec.h);
        }
        else if (attribute == "ui_scale")               uiScale             = std::stoi(value);
        else if (attribute == "toolpane_expanded")      toolPaneSizeState   = std::stoi(value);
        else if (attribute == "show_console")           consoleOn           = std::stoi(value);
        else if (attribute == "show_properties")        propertiesOn        = std::stoi(value);
        else if (attribute == "min_log_level")          minLogLevel = LogType(std::min(std::max(0, std::stoi(value)), 4));
        else if (attribute == "selection_preview")      selectionPreview    = !!std::stoi(value);
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

    ReloadPanes();

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
void Window::PushProperty(const std::string& name, const std::string& value)
{
    const int propHeight = FontSize() * 2;
    IRect box(propertiesPaneRec.x, propHeight * propertyNumber, propertiesPaneRec.w, propHeight);
    DrawRectangleLinesIRect(box, UIColor(UIColorID::UI_COLOR_BACKGROUND2));
    const int propertiesPaneMiddle = propertiesPaneRec.w / 3;
    const int propertiesPaneMiddleAbs = propertiesPaneRec.x + propertiesPaneMiddle;
    DrawLine(propertiesPaneMiddleAbs, box.y, propertiesPaneMiddleAbs, box.Bottom(), UIColor(UIColorID::UI_COLOR_BACKGROUND2));
    DrawTextIV(name.c_str(), box.xy + FontPadding(), FontSize(), UIColor(UIColorID::UI_COLOR_FOREGROUND));
    DrawTextIV(value.c_str(), box.xy + Width(propertiesPaneMiddle) + FontPadding(), FontSize(), UIColor(UIColorID::UI_COLOR_FOREGROUND));
    propertyNumber++;
}
void Window::PushProperty_longStr(const std::string& name, const std::string& value)
{
    const int propHeight = FontSize() * 2;

    IRect box1(propertiesPaneRec.x, propHeight * propertyNumber, propertiesPaneRec.w, propHeight);
    DrawRectangleLinesIRect(box1, UIColor(UIColorID::UI_COLOR_BACKGROUND2));
    DrawTextIV(name.c_str(), box1.xy + FontPadding(), FontSize(), UIColor(UIColorID::UI_COLOR_FOREGROUND));
    propertyNumber++;

    // Size to text
    const int propHeight1 = FontSize() * 2 - 1;
    int lineCount = (int)std::count(value.begin(), value.end(), '\n') + 1;
    IRect box2(propertiesPaneRec.x, propHeight * propertyNumber, propertiesPaneRec.w, propHeight1 * lineCount);
    DrawRectangleLinesIRect(box2, UIColor(UIColorID::UI_COLOR_BACKGROUND2));
    DrawTextIV(value.c_str(), box2.xy + FontPadding(), FontSize(), UIColor(UIColorID::UI_COLOR_FOREGROUND));
    propertyNumber += lineCount;
}
void Window::PushPropertyTitle(const std::string& title)
{
    const int propHeight = FontSize() * 2;
    IRect box(propertiesPaneRec.x, propHeight * propertyNumber, propertiesPaneRec.w, propHeight);
    DrawRectangleIRect(ShrinkIRect(box), UIColor(UIColorID::UI_COLOR_BACKGROUND2));
    DrawTextIV(title.c_str(), box.xy + FontPadding(), FontSize(), UIColor(UIColorID::UI_COLOR_FOREGROUND));
    propertyNumber++;
}
void Window::PushPropertySubtitle(const std::string& title, Color color)
{
    const int propHeight = FontSize() * 2;
    IVec2 pos(propertiesPaneRec.x, propHeight * propertyNumber);
    DrawTextIV(title.c_str(), pos + FontPadding(), FontSize(), color);
    propertyNumber++;
}
void Window::PushPropertySpacer()
{
    propertyNumber++;
}
void Window::PushPropertySection_Node(const std::string& name, Node* value)
{
    if (!!value)
    {
        PushPropertySubtitle(name);
        if (value->HasName())
            PushProperty_str("Name", value->GetName());
        PushProperty_ptr("Pointer", value);
        PushProperty_uint("Serial", CurrentTab().graph->NodeID(value));
        PushProperty("Type", GateName(value->GetGate()));
        PushProperty_bool("Passthrough", value->IsPassthrough());
        PushProperty_bool("Interactable", value->IsOutputOnly());
        PushProperty_bool("Start node", value->IsOutputOnly() || value->GetGate() == Gate::BATTERY);
        switch (value->GetGate())
        {
        case Gate::RESISTOR:  PushProperty_uint("Resistance", value->GetResistance());  break;
        case Gate::CAPACITOR: PushProperty_uint("Capacity", value->GetCapacity());      break;
        case Gate::LED:       PushProperty_uint("Color index", value->GetColorIndex()); break;
        default: break;
        }
        PushPropertySubtitle("Inputs", UIColor(UIColorID::UI_COLOR_INPUT));
        PushProperty_uint("Count", value->GetInputCount());

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
        PushPropertySpacer();
    }
}
void Window::PushPropertySection_Wire(const std::string& name, Wire* value)
{
    if (!!value)
    {
        PushPropertySubtitle(name);
        PushProperty_ptr("Pointer", value);
        PushProperty("Joint config", ElbowConfigName(value->elbowConfig));
        PushProperty("State", StateName(value->GetState()));
        PushPropertySubtitle("Input", UIColor(UIColorID::UI_COLOR_INPUT));
        PushProperty_uint("Serial", CurrentTab().graph->NodeID(value->start));
        PushProperty_ptr("Pointer", value->start);
        PushPropertySubtitle("Output", UIColor(UIColorID::UI_COLOR_OUTPUT));
        PushProperty_uint("Serial", CurrentTab().graph->NodeID(value->end));
        PushProperty_ptr("Pointer", value->end);
        PushPropertySpacer();
    }
}
void Window::PushPropertySection_Selection(const std::string& name, const std::vector<Node*>& value)
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

    PushPropertySpacer();
}
void Window::PushPropertySection_Group(const std::string& name, Group* value)
{
    if (!!value)
    {
        PushPropertySubtitle(name);
        PushProperty_ptr("Pointer", value);
        PushProperty_str("Label", value->GetLabel());
        PushPropertySpacer();
    }
}
void Window::DrawClipboardPreview() const
{
    clipboard->DrawSelectionPreview(
        clipboardButton.Bounds().BR() + FontPadding(),
        UIColor(UIColorID::UI_COLOR_BACKGROUND1),
        UIColor(UIColorID::UI_COLOR_FOREGROUND3),
        UIColor(UIColorID::UI_COLOR_BACKGROUND2),
        ColorAlpha(UIColor(UIColorID::UI_COLOR_FOREGROUND3), 0.25f),
        clipboardPreviewLOD);
}
void Window::ReloadToolPane()
{
    if (toolPaneSizeState)
    {
        toolPaneSizeButton.relativePos      = IVec2(0, 0);
        propertiesToggleButton.relativePos  = IVec2(1, 0);
        consoleToggleButton.relativePos     = IVec2(2, 0);

        blueprintsButton.relativePos        = IVec2(0, 1);
        clipboardButton.relativePos         = IVec2(0, 2);
        modeButtons[0].relativePos          = IVec2(1, 1);
        modeButtons[1].relativePos          = IVec2(1, 2);
        modeButtons[2].relativePos          = IVec2(2, 1);
        modeButtons[3].relativePos          = IVec2(2, 2);

        gateButtons[0].relativePos          = IVec2(0, 4);
        gateButtons[1].relativePos          = IVec2(1, 4);
        gateButtons[2].relativePos          = IVec2(2, 4);
        gateButtons[3].relativePos          = IVec2(0, 5);
        gateButtons[4].relativePos          = IVec2(1, 5);
        gateButtons[5].relativePos          = IVec2(2, 5);
        gateButtons[6].relativePos          = IVec2(0, 6);
        gateButtons[7].relativePos          = IVec2(1, 6);
        gateButtons[8].relativePos          = IVec2(2, 6);

        paramButtons[0].relativePos         = IVec2(1,11);
        paramButtons[1].relativePos         = IVec2(0,10);
        paramButtons[2].relativePos         = IVec2(1,10);
        paramButtons[3].relativePos         = IVec2(2,10);
        paramButtons[4].relativePos         = IVec2(0, 9);
        paramButtons[5].relativePos         = IVec2(1, 9);
        paramButtons[6].relativePos         = IVec2(2, 9);
        paramButtons[7].relativePos         = IVec2(0, 8);
        paramButtons[8].relativePos         = IVec2(1, 8);
        paramButtons[9].relativePos         = IVec2(2, 8);

        toolPaneSizeButton.buttonText = "-";
    }
    else
    {
        int i = 0;

        toolPaneSizeButton.relativePos      = IVec2(0, i++);
        propertiesToggleButton.relativePos  = IVec2(0, i++);
        consoleToggleButton.relativePos     = IVec2(0, i++);


        blueprintsButton.relativePos = IVec2(0, i++);
        clipboardButton.relativePos  = IVec2(0, i++);

        for (int j = 0; j < _countof(modeButtons); ++j)
        {
            modeButtons[j].relativePos = IVec2(0, i++);
        }

        i++;

        for (int j = 0; j < _countof(gateButtons); ++j)
        {
            gateButtons[j].relativePos = IVec2(0, i++);
        }

        i++;

        for (int j = 0; j < _countof(paramButtons); ++j)
        {
            paramButtons[j].relativePos = IVec2(0, i++);
        }

        toolPaneSizeButton.buttonText = "+";
    }
}
void Window::ToggleToolPaneSize()
{
    toolPaneSizeState = !toolPaneSizeState;
    ReloadPanes();
}

void Window::ToggleProperties()
{
    if (propertiesOn = !propertiesOn)
        consolePaneRec.w = windowWidth - propertiesPaneRec.w;
    else
        consolePaneRec.w = windowWidth;
}

void Window::ToggleConsole()
{
    if (consoleOn = !consoleOn)
        toolPaneRec.h = windowHeight - consolePaneRec.h;
    else
        toolPaneRec.h = windowHeight;
}

void Window::DrawToolPane()
{
    DrawRectangleIRect(toolPaneRec, UIColor(UIColorID::UI_COLOR_BACKGROUND1));
    DrawRectangleLinesIRect(ExpandIRect(toolPaneRec), UIColor(UIColorID::UI_COLOR_BACKGROUND2));

    // Background
    for (const Button* const b : allButtons)
    {
        if (CursorInUIBounds(b->Bounds())) [[unlikely]]
            DrawRectangleIRect(b->Bounds(), UIColor(UIColorID::UI_COLOR_AVAILABLE));
    }

    for (const Button* const b : allButtons)
    {
        bool shouldHighlight =
            (b == ButtonFromMode(GetBaseMode())) ||
            (b == ButtonFromGate(gatePick)) ||
            (b == ButtonFromParameter(storedExtraParam) ||
            (b == &clipboardButton && IsClipboardValid()));

        Color color;
        if (shouldHighlight || CursorInUIBounds(b->Bounds())) [[unlikely]]
            color = UIColor(UIColorID::UI_COLOR_FOREGROUND);
        else [[likely]]
            color = UIColor(UIColorID::UI_COLOR_FOREGROUND2);

        // Icon buttons
        if (const IconButton* ib = dynamic_cast<const IconButton*>(b))
            DrawUIIcon(*ib->textureSheet, ib->textureSheetPos, ib->Bounds().xy, color);

        // Text buttons
        else if (const TextButton* tb = dynamic_cast<const TextButton*>(b))
        {
            IVec2 textCenter = tb->Bounds().xy + Height(FontPadding().y) + Width((tb->Bounds().w - MeasureText(tb->buttonText, FontSize())) / 2);
            Color background;
            if (CursorInUIBounds(b->Bounds())) [[unlikely]]
                background = UIColor(UIColorID::UI_COLOR_AVAILABLE);
            else [[likely]]
                background = UIColor(UIColorID::UI_COLOR_BACKGROUND1);
            DrawRectangleIRect(tb->Bounds(), background);
            DrawTextIV(tb->buttonText, textCenter, FontSize(), color);
        }

        // Color buttons
        else if (const ColorButton* cb = dynamic_cast<const ColorButton*>(b))
        {
            IRect rec;
            if (shouldHighlight)
                rec = cb->Bounds();
            else
                rec = ShrinkIRect(cb->Bounds(), 2);
            DrawRectangleIRect(rec, cb->color);
            IVec2 textCenter = cb->Bounds().xy + Height(FontPadding().y) + Width((Button::g_width - MeasureText(cb->buttonText, FontSize())) / 2);
            DrawTextShadowedIV(cb->buttonText, textCenter, FontSize(), color, UIColor(UIColorID::UI_COLOR_BACKGROUND1));
        }
    }

    // Tooltips
    const Button* hoveredButton = nullptr;
    for (const Button* const b : allButtons)
    {
        if (CursorInUIBounds(b->Bounds())) [[unlikely]]
        {
            hoveredButton = b;
            break;
        }
    }
    if (!!hoveredButton) [[unlikely]]
    {
        DrawTextShadowedIV(hoveredButton->tooltip, hoveredButton->Bounds().TR() + FontPadding(), FontSize(),
        UIColor(UIColorID::UI_COLOR_FOREGROUND), UIColor(UIColorID::UI_COLOR_BACKGROUND));

        // Clipboard preview
        if (hoveredButton == &clipboardButton && IsClipboardValid()) [[unlikely]]
            DrawClipboardPreview();
    }
}
void Window::CleanConsolePane()
{
    CurrentTab().Set2DMode(false); // In case
    DrawRectangleIRect(consolePaneRec, UIColor(UIColorID::UI_COLOR_BACKGROUND1));
    const int titleHeight = FontSize() * 2;
    IRect box(consolePaneRec.x, consolePaneRec.y, consolePaneRec.w, titleHeight);
    DrawRectangleIRect(ShrinkIRect(box), UIColor(UIColorID::UI_COLOR_BACKGROUND2));
    DrawRectangleLinesIRect(consolePaneRec, UIColor(UIColorID::UI_COLOR_BACKGROUND2));
    DrawTextIV("Console", box.xy + FontPadding(), FontSize(), UIColor(UIColorID::UI_COLOR_FOREGROUND));
}
void Window::DrawConsoleOutput()
{
    for (int i = 0; i < _countof(consoleOutput); ++i)
    {
        if (consoleOutput[i].size() < 2) // ""
            continue;

        Color color;
        switch (consoleOutput[i][1])
        {
        case 'I': color = UIColor(UIColorID::UI_COLOR_FOREGROUND);  break;
        case 'A': color = UIColor(UIColorID::UI_COLOR_SPECIAL);     break;
        case 'S': color = UIColor(UIColorID::UI_COLOR_FOREGROUND1); break;
        case 'W': color = UIColor(UIColorID::UI_COLOR_CAUTION);     break;
        case 'E': color = UIColor(UIColorID::UI_COLOR_DESTRUCTIVE); break;
        default:  color = UIColor(UIColorID::UI_COLOR_ERROR);       break; // Malformed
        }
        DrawTextIV(
            consoleOutput[i].c_str(),
            consolePaneRec.xy + Height(FontSize() * 2 * (i + 1)) + FontPadding(),
            FontSize(), color);
    }
}
std::string LogTypeStr(LogType type)
{
    switch (type)
    {
    case LogType::info:     return "[INFO] ";
    case LogType::attempt:  return "[ATTEMPT] ";
    case LogType::success:  return "[SUCCESS] ";
    case LogType::warning:  return "[WARNING] ";
    case LogType::error:    return "[ERROR] ";
    default:                return "[UNKNOWN] ";
    }
}
void Window::Log(LogType type, const std::string& output)
{
    if ((int)type < (int)minLogLevel)
        return;

    double logTime = GetTime();
    consoleOutput[0] =
        LogTypeStr(type) + output +
        " - t+" + std::to_string(logTime) +
        "(" + std::to_string((logTime - timeOfLastLog) * 1000) + "ms)";

    std::ofstream logfile("session.log", std::ios_base::app);
    logfile << consoleOutput[0] << '\n';
    logfile.close();

    for (int i = 1; i < _countof(consoleOutput); ++i)
    {
        consoleOutput[i - 1].swap(consoleOutput[i]); // [0] gets sequentially traded forward without copying
    }
    timeOfLastLog = logTime;
}
void Window::ClearLog()
{
    timeOfLastLog = GetTime();
    std::ofstream logfile("session.log", std::ios_base::trunc);
    logfile.close();
    Log(LogType::info, "Start of log");
}
void Window::CleanPropertiesPane()
{
    CurrentTab().Set2DMode(false);  // In case
    DrawRectangleIRect(propertiesPaneRec, UIColor(UIColorID::UI_COLOR_BACKGROUND1));
    DrawRectangleLinesIRect(ExpandIRect(propertiesPaneRec), UIColor(UIColorID::UI_COLOR_BACKGROUND2));
    propertyNumber = 0;
    PushPropertyTitle("Properties");
}
void Window::DrawToolProperties()
{
    if (!!overlay)
        overlay->DrawProperties(*this);
    else
        base->DrawProperties(*this);
}
