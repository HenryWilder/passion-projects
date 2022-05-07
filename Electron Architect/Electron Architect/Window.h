#pragma once
#include <raylib.h>
#include <vector>
#include "IVec.h"

// Reusable address so clipboard doesn't have to delete
extern Blueprint g_clipboardBP;

struct Window
{
    Window(int windowWidth, int windowHeight);
    ~Window();

private:
    Texture2D blueprintIcon16x;
    Texture2D blueprintIcon32x;
    Texture2D clipboardIcon16x;
    Texture2D clipboardIcon32x;
    Texture2D modeIcons16x;
    Texture2D modeIcons32x;
    Texture2D gateIcons16x;
    Texture2D gateIcons32x;
public:

    int windowWidth;
    int windowHeight;

    size_t activeTab;
    std::vector<Tab*> tabs;
    Tab& CurrentTab();

    uint8_t blueprintLOD = 0;
    uint8_t clipboardPreviewLOD = 0;
    uint8_t pastePreviewLOD = 0;
    int uiScale = 1;

    IVec2 cursorUIPos = IVec2::Zero();
    IVec2 cursorPos = IVec2::Zero();
    IVec2 cursorPosPrev = IVec2::Zero(); // For checking if there was movement
    bool b_cursorMoved = false;

    uint8_t framesPerTick = 6; // Number of frames in a tick
    uint8_t tickFrame = framesPerTick - 1; // Evaluate on 0
    bool tickThisFrame;

    Gate gatePick = Gate::OR;
    Gate lastGate = Gate::OR;
    uint8_t storedExtraParam = 0;

    Camera2D camera{ .offset{ 0,0 }, .target{ 0,0 }, .rotation{ 0 }, .zoom{ 1 } };

    const char* deviceParameterTextFmt = "";

    Node* hoveredNode = nullptr;
    Wire* hoveredWire = nullptr;
    Group* hoveredGroup = nullptr;

    Blueprint* clipboard = nullptr;
    std::vector<Node*> selection;

    std::vector<IconButton> modeButtons
    {
        IconButton(
            IVec2(0,0),
            "Mode: Draw [b]",
            "@TODO",
            [this]() { SetMode(Mode::PEN); },
            IVec2(0,0),
            &modeIcons16x),

        IconButton(
            IVec2(0,1),
            "Mode: Edit [v]",
            "@TODO",
            [this]() { SetMode(Mode::EDIT); },
            IVec2(1,0),
            &modeIcons16x),

        IconButton(
            IVec2(0,2),
            "Mode: Erase [x]",
            "@TODO",
            [this]() { SetMode(Mode::ERASE); },
            IVec2(0,1),
            &modeIcons16x),

        IconButton(
            IVec2(0,3),
            "Mode: Interact [f]",
            "@TODO",
            [this]() { SetMode(Mode::INTERACT); },
            IVec2(1,1),
            &modeIcons16x),
    };
    std::vector<IconButton> gateButtons
    {
        IconButton(
            IVec2(1,0),
            "Gate: Or [1]",
            "@TODO",
            [this]() { SetGate(Gate::OR); },
            IVec2(0,0),
            &gateIcons16x),

        IconButton(
            IVec2(1,1),
            "Gate: And [2]",
            "@TODO",
            [this]() { SetGate(Gate::AND); },
            IVec2(1,0),
            &gateIcons16x),

        IconButton(
            IVec2(1,2),
            "Gate: Nor [3]",
            "@TODO",
            [this]() { SetGate(Gate::NOR); },
            IVec2(0,1),
            &gateIcons16x),

        IconButton(
            IVec2(1,3),
            "Gate: Xor [4]",
            "@TODO",
            [this]() { SetGate(Gate::XOR); },
            IVec2(1,1),
            &gateIcons16x),

        IconButton(
            IVec2(1,4),
            "Element: Resistor [5]",
            "@TODO",
            [this]() { SetGate(Gate::RESISTOR); },
            IVec2(0,2),
            &gateIcons16x),

        IconButton(
            IVec2(1,5),
            "Element: Capacitor [6]",
            "@TODO",
            [this]() { SetGate(Gate::CAPACITOR); },
            IVec2(1,2),
            &gateIcons16x),

        IconButton(
            IVec2(1,6),
            "Element: LED [7]",
            "@TODO",
            [this]() { SetGate(Gate::LED); },
            IVec2(0,3),
            &gateIcons16x),

        IconButton(
            IVec2(1,7),
            "Element: Delay [8]",
            "@TODO",
            [this]() { SetGate(Gate::DELAY); },
            IVec2(1,3),
            &gateIcons16x),

        IconButton(
            IVec2(1,8),
            "Element: Battery [9]",
            "@TODO",
            [this]() { SetGate(Gate::BATTERY); },
            IVec2(0,4),
            &gateIcons16x),
    };
    IconButton blueprintsButton =
        IconButton(
            IVec2(2, 0),
            "Blueprints",
            "@TODO",
            [this]() { SetMode(Mode::BP_SELECT); },
            IVec2::Zero(),
            &blueprintIcon16x);

    IconButton clipboardButton =
        IconButton(
            IVec2(2, 1),
            "Clipboard (ctrl+c to copy, ctrl+v to paste)",
            "@TODO",
            [this]() { if (this->IsClipboardValid()) SetMode(Mode::PASTE); },
            IVec2::Zero(),
            &clipboardIcon16x);

#if _DEBUG
private:
#endif

    Tool* base = nullptr;
    Tool* overlay = nullptr; // Menu modes are stored in here

public:

    int FontSize() const;

    void DrawUIIcon(Texture2D iconSheet, IVec2 iconColRow, IVec2 pos, Color tint) const;

    Mode GetBaseMode();
    Mode GetMode();
    ModeType GetModeType();

    void SetMode(Mode newMode);
    void SetGate(Gate newGate);

    void ClearOverlayMode();

    static const char* GetModeTooltipName(Mode mode)
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
    static const char* GetModeTooltipDescription(Mode mode)
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
                "Left click a node without any inputs (such nodes will render in \"available_color\" (blue by default)) to toggle it between outputting true and false.";

        default:
            _ASSERT_EXPR(false, L"Missing tooltip description for selected mode");
            return "";
        }
    };
    static const char* GetGateTooltipName(Gate gate)
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
            return "Element: Resistor [5]";
        case Gate::CAPACITOR:
            return "Element: Capacitor [6]";
        case Gate::LED:
            return "Element: LED [7]";
        case Gate::DELAY:
            return "Element: Delay [8]";
        case Gate::BATTERY:
            return "Element: Battery [9]";

        default:
            _ASSERT_EXPR(false, L"Missing tooltip for selected gate");
            return "";
        }
    };
    static const char* GetGateTooltipDescription(Gate gate)
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
        case Gate::BATTERY:
            return
                "Always outputs true, regardless of inputs.";

        default:
            _ASSERT_EXPR(false, L"Missing tooltip description for selected gate");
            return "";
        }
    };

    void IncrementTick();

    void UpdateCursorPos();

    // Zoom/pan
    void UpdateCamera();

    void CopySelectionToClipboard();

    void MakeGroupFromSelection();

    bool IsSelectionRectValid() const;

    void SaveBlueprint();

    bool IsClipboardValid() const;
    void ClearClipboard();
    bool SelectionExists() const;
    void ClearSelection();
    void DestroySelection();

    void CheckHotkeys();

    IVec2 GetCursorDelta() const;

    bool CursorInBounds(IRect bounds) const;
    bool CursorInUIBounds(IRect uiBounds) const;

    template<int gridSize = g_gridSize>
    void DrawGrid() const
    {
        // Grid
        {
            IVec2 extents((int)((float)windowWidth / camera.zoom), (int)((float)windowHeight / camera.zoom));
            IRect bounds(IVec2(camera.target), extents);

            constexpr float gridSpaceFrac = 1.0f / gridSize;
            // "If the fraction of a screen pixel in a grid space equals or exceeds the fraction of a screen pixel in a world pixel"
            if (camera.zoom <= gridSpaceFrac)
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
            constexpr int halfgrid = gridSize / 2;
            DrawLine(bounds.x, -halfgrid, bounds.x + bounds.w, -halfgrid, UIColor(UIColorID::UI_COLOR_BACKGROUND2));
            DrawLine(-halfgrid, bounds.y, -halfgrid, bounds.y + bounds.h, UIColor(UIColorID::UI_COLOR_BACKGROUND2));
        }
    }
    void DrawGrid(int gridSize) const;

    IRect GetSelectionBounds(const std::vector<Node*>& vec) const;
    IRect GetSelectionBounds() const;

    static Color ResistanceBandColor(uint8_t index);
    Color ExtraParamColor() const;

    void DrawTooltipAtCursor(const char* text, Color color);

    void DrawTooltipAtCursor_Shadowed(const char* text, Color color);

    void ReloadConfig();

    void UpdateTool();
    void DrawTool();
};
