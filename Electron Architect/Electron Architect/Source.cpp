#include <raylib.h>
#include "HUtility.h"
#include "IVec.h"
#include "Node.h"
#include "Wire.h"
#include "Blueprint.h"
#include "Group.h"
#include "NodeWorld.h"

enum class Mode
{
    PEN,
    EDIT,
    ERASE,
    INTERACT,

    GATE,
    BUTTON,
    PASTE,

    BP_ICON,
    BP_SELECT,
};

struct ProgramData
{
    ProgramData(int windowWidth, int windowHeight) : windowWidth(windowWidth), windowHeight(windowHeight)
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
    ~ProgramData()
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

    static constexpr Mode dropdownModeOrder[] =
    {
        Mode::PEN,
        Mode::EDIT,
        Mode::ERASE,
        Mode::INTERACT,
    };
    static constexpr Gate dropdownGateOrder[] =
    {
        Gate::OR,
        Gate::AND,
        Gate::NOR,
        Gate::XOR,

        Gate::RESISTOR,
        Gate::CAPACITOR,
        Gate::LED,
        Gate::DELAY,
    };
    static constexpr IRect dropdownBounds[] =
    {
        IRect(0, 16, 16, 16 * (_countof(dropdownModeOrder) - 1)), // Mode
        IRect(16, 16, 16, 16 * (_countof(dropdownGateOrder) - 1)), // Gate
        IRect(32, 16, 16, 16 * (_countof(Node::g_resistanceBands) - 1)), // Parameter
    };
    static constexpr Gate radialGateOrder[] =
    {
        Gate::XOR,
        Gate::AND,
        Gate::OR,
        Gate::NOR,
    };

private:
    static Texture2D clipboardIcon;
    static Texture2D modeIcons;
    static Texture2D gateIcons16x;
    static Texture2D gateIcons32x;
public:

    int windowWidth;
    int windowHeight;

    Mode mode = Mode::PEN;
    Mode baseMode = Mode::PEN;

    IVec2 cursorUIPos = IVec2::Zero();
    IVec2 cursorPos = IVec2::Zero();
    IVec2 cursorPosPrev = IVec2::Zero(); // For checking if there was movement
    bool b_cursorMoved = false;

    uint8_t framesPerTick = 3; // Number of frames in a tick
    uint8_t tickFrame = framesPerTick - 1; // Evaluate on 0
    bool tickThisFrame;

    Gate gatePick = Gate::OR;
    Gate lastGate = Gate::OR;
    uint8_t storedExtraParam = 0;

    Camera2D camera{ .offset{ 0,0 }, .target{ 0,0 }, .rotation{ 0 }, .zoom{ 1 } };

    const char* deviceParameterTextFmt = "";

    Node* hoveredNode = nullptr;
    Wire* hoveredWire = nullptr;

    Blueprint* clipboard = nullptr;
    std::vector<Node*> selection;

#if _DEBUG
private:
#endif
    // Base mode
    union BaseModeData
    {
        struct PenModeData
        {
            Node* currentWireStart;
            ElbowConfig currentWireElbowConfig;
        } pen;

        struct EditModeData
        {
            IVec2 fallbackPos;
            bool selectionWIP;
            IVec2 selectionStart;
            IRect selectionRec;
            bool draggingGroup;
            Group* hoveredGroup;
            Node* nodeBeingDragged;
            Wire* wireBeingDragged;
        } edit;

        struct EraseModeData
        {
        } erase;

        struct InteractModeData
        {
        } interact;
    } base;

    // Overlay mode - doesn't reset the base mode
    union OverlayModeData
    {
        struct GateModeData
        {
            IVec2 radialMenuCenter;
            uint8_t overlappedSection;
        } gate;

        struct ButtonModeData
        {
            int dropdownActive;
        } button;

        struct PasteModeData
        {
        } paste;

        struct BP_IconModeData
        {
            BlueprintIcon* object;
            IVec2 pos; // Width and height are fixed
            IRect sheetRec;
            BlueprintIconID_t iconID;
            uint8_t iconCount;
            int draggingIcon; // -1 for none/not dragging
        } bp_icon;

        struct BP_SelectModeData
        {
            int hovering; // -1 for none
        } bp_select;
    } overlay;

public: // Accessors for unions

#if _DEBUG

    /**********
    * Basic
    **********/

    // Pen
    Node*& Pen_CurrentWireStart()               { _ASSERT_EXPR(baseMode == Mode::PEN, "Tried to access member of different mode"); return base.pen.currentWireStart; }
    ElbowConfig& Pen_CurrentWireElbowConfig()   { _ASSERT_EXPR(baseMode == Mode::PEN, "Tried to access member of different mode"); return base.pen.currentWireElbowConfig; }

    // Edit
    IVec2& Edit_FallbackPos()                   { _ASSERT_EXPR(baseMode == Mode::EDIT, "Tried to access member of different mode"); return base.edit.fallbackPos; }
    bool& Edit_SelectionWIP()                   { _ASSERT_EXPR(baseMode == Mode::EDIT, "Tried to access member of different mode"); return base.edit.selectionWIP; }
    IVec2& Edit_SelectionStart()                { _ASSERT_EXPR(baseMode == Mode::EDIT, "Tried to access member of different mode"); return base.edit.selectionStart; }
    IRect& Edit_SelectionRec()                  { _ASSERT_EXPR(baseMode == Mode::EDIT, "Tried to access member of different mode"); return base.edit.selectionRec; }
    bool& Edit_DraggingGroup()                  { _ASSERT_EXPR(baseMode == Mode::EDIT, "Tried to access member of different mode"); return base.edit.draggingGroup; }
    Group*& Edit_HoveredGroup()                 { _ASSERT_EXPR(baseMode == Mode::EDIT, "Tried to access member of different mode"); return base.edit.hoveredGroup; }
    Node*& Edit_NodeBeingDragged()              { _ASSERT_EXPR(baseMode == Mode::EDIT, "Tried to access member of different mode"); return base.edit.nodeBeingDragged; }
    Wire*& Edit_WireBeingDragged()              { _ASSERT_EXPR(baseMode == Mode::EDIT, "Tried to access member of different mode"); return base.edit.wireBeingDragged; }

    /**********
    * Overlay
    **********/

    // Gate
    IVec2& Gate_RadialMenuCenter()              { _ASSERT_EXPR(mode == Mode::GATE, "Tried to access member of different mode"); return overlay.gate.radialMenuCenter; }
    uint8_t& Gate_OverlappedSection()           { _ASSERT_EXPR(mode == Mode::GATE, "Tried to access member of different mode"); return overlay.gate.overlappedSection; }

    // Button
    int& Button_DropdownActive()                { _ASSERT_EXPR(mode == Mode::GATE, "Tried to access member of different mode"); return overlay.button.dropdownActive; }

    // BP_Icon
    BlueprintIcon*& BPIcon_Object()             { _ASSERT_EXPR(mode == Mode::BP_ICON, "Tried to access member of different mode"); return overlay.bp_icon.object; }
    // Width and height are fixed
    IVec2& BPIcon_Pos()                         { _ASSERT_EXPR(mode == Mode::BP_ICON, "Tried to access member of different mode"); return overlay.bp_icon.pos; }
    IRect& BPIcon_SheetRec()                    { _ASSERT_EXPR(mode == Mode::BP_ICON, "Tried to access member of different mode"); return overlay.bp_icon.sheetRec; }
    BlueprintIconID_t& BPIcon_IconID()          { _ASSERT_EXPR(mode == Mode::BP_ICON, "Tried to access member of different mode"); return overlay.bp_icon.iconID; }
    uint8_t& BPIcon_IconCount()                 { _ASSERT_EXPR(mode == Mode::BP_ICON, "Tried to access member of different mode"); return overlay.bp_icon.iconCount; }
    // -1 for none/not dragging
    int& BPIcon_DraggingIcon()                  { _ASSERT_EXPR(mode == Mode::BP_ICON, "Tried to access member of different mode"); return overlay.bp_icon.draggingIcon; }

    // Select
    int& BPSelect_Hovering()                    { _ASSERT_EXPR(mode == Mode::BP_SELECT, "Tried to access member of different mode"); return overlay.bp_select.hovering; }

#else // Release

    /**********
    * Basic
    **********/

    // Pen
#define Pen_CurrentWireStart() base.pen.currentWireStart
#define Pen_CurrentWireElbowConfig() base.pen.currentWireElbowConfig

    // Edit
#define Edit_FallbackPos() base.edit.fallbackPos
#define Edit_SelectionWIP() base.edit.selectionWIP
#define Edit_SelectionStart() base.edit.selectionStart
#define Edit_SelectionRec() base.edit.selectionRec
#define Edit_DraggingGroup() base.edit.draggingGroup
#define Edit_HoveredGroup() base.edit.hoveredGroup
#define Edit_NodeBeingDragged() base.edit.nodeBeingDragged
#define Edit_WireBeingDragged() base.edit.wireBeingDragged

    /**********
    * Overlay
    **********/

    // Gate
#define Gate_RadialMenuCenter() overlay.gate.radialMenuCenter
#define Gate_OverlappedSection() overlay.gate.overlappedSection

    // Button
#define Button_DropdownActive() overlay.button.dropdownActive

    // BP_Icon
#define BPIcon_Object() overlay.bp_icon.object
    // Width and height are fixed
#define BPIcon_Pos() overlay.bp_icon.pos
#define BPIcon_SheetRec() overlay.bp_icon.sheetRec
#define BPIcon_IconID() overlay.bp_icon.iconID
#define BPIcon_IconCount() overlay.bp_icon.iconCount
    // -1 for none/not dragging
#define BPIcon_DraggingIcon() overlay.bp_icon.draggingIcon

    // Select
#define BPSelect_Hovering() overlay.bp_select.hovering

#endif

public:

    static Texture2D GetClipboardIcon()
    {
        return clipboardIcon;
    }
    static void DrawModeIcon(Mode mode, IVec2 pos, Color tint)
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
    static void DrawGateIcon16x(Gate gate, IVec2 pos, Color tint)
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
    };
    static void DrawGateIcon32x(Gate gate, IVec2 pos, Color tint)
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

    static bool ModeIsMenu(Mode mode)
    {
        // Modes which disable use of basic UI and drawing of certain UI elements
        return mode == Mode::BP_ICON || mode == Mode::BP_SELECT;
    }
    inline bool ModeIsMenu() const
    {
        return ModeIsMenu(this->mode);
    }
    static bool ModeIsOverlay(Mode mode)
    {
        // Modes which can be active simultaneously with a non-overlay mode
        return mode == Mode::GATE || mode == Mode::BUTTON || mode == Mode::PASTE || ModeIsMenu(mode);
    }
    inline bool ModeIsOverlay() const
    {
        return ModeIsOverlay(this->mode);
    }
    bool ModeIsBasic() const
    {
        return this->mode == this->baseMode;
    }

    void SetMode(Mode newMode)
    {
        if (mode == Mode::BP_ICON)
        {
            delete overlay.bp_icon.object;
            overlay.bp_icon.object = nullptr;
        }

        b_cursorMoved = true;
        mode = newMode;

        if (!ModeIsOverlay(newMode))
        {
            baseMode = newMode;
            memset(&base, 0, sizeof(overlay));
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
    };
    void SetGate(Gate newGate)
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
    };

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
                "Left click a node without any inputs (such nodes will render in BLUE) to toggle it between outputting true and false.\n"
                "NOTE: Framerate is intentionally lowered from 120 to 24 while in this mode for ease of inspection.";

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

        default:
            _ASSERT_EXPR(false, L"Missing tooltip description for selected gate");
            return "";
        }
    };

    void IncrementTick()
    {
        ++tickFrame;
        tickFrame %= framesPerTick;
        tickThisFrame = tickFrame == 0;
    }

    void UpdateCursorPos()
    {
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
    }

    // Zoom/pan
    void UpdateCamera()
    {
        if (!ModeIsMenu())
        {
            if (GetMouseWheelMove() > 0 && camera.zoom < 2.0f)
                camera.zoom *= 2;
            else if (GetMouseWheelMove() < 0 && camera.zoom > 0.125f)
                camera.zoom /= 2;

            camera.target.x += (float)(IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT)) * g_gridSize;
            camera.target.y += (float)(IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP)) * g_gridSize;
        }
    }

    void CopySelectionToClipboard()
    {
        if (clipboard != nullptr)
            delete clipboard;

        if (selection.empty())
            clipboard = nullptr;
        else
            clipboard = new Blueprint(selection);
    }

    void MakeGroupFromSelection()
    {
        NodeWorld::Get().CreateGroup(edit.selectionRec);
        edit.selectionRec = IRect(0, 0, 0, 0);
        selection.clear();
    }

    bool IsSelectionRectValid() const
    {
        return mode == Mode::EDIT && !edit.selectionWIP && !(edit.selectionRec.w == 0 || edit.selectionRec.h == 0);
    }

    void SaveBlueprint()
    {
        SetMode(Mode::BP_ICON);
        bp_icon.object = new BlueprintIcon;
        bp_icon.pos = cursorPos - IVec2(BlueprintIcon::g_size / 2, BlueprintIcon::g_size / 2);
        bp_icon.sheetRec.xy = bp_icon.pos + IVec2(BlueprintIcon::g_size * 2, BlueprintIcon::g_size * 2);
        bp_icon.sheetRec.wh = BlueprintIcon::GetSheetSize_Px();
    }

    bool IsClipboardValid() const
    {
        return !!clipboard;
    }
    void ClearClipboard()
    {
        delete clipboard;
        clipboard = nullptr;
    }
    bool SelectionExists() const
    {
        return !selection.empty();
    }
    void ClearSelection()
    {
        selection.clear();
        if (mode == Mode::EDIT)
            edit.selectionRec = IRect(0);
    }
    void DestroySelection()
    {
        // TODO: Refactor NodeWorld to have a bulk destroy function to make this process more efficient
        for (Node* node : selection)
        {
            NodeWorld::Get().DestroyNode(node);
        }
        ClearSelection();
    }

    void CheckHotkeys()
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
            if      (IsKeyPressed(KEY_ONE))   SetGate(Gate::OR);
            else if (IsKeyPressed(KEY_TWO))   SetGate(Gate::AND);
            else if (IsKeyPressed(KEY_THREE)) SetGate(Gate::NOR);
            else if (IsKeyPressed(KEY_FOUR))  SetGate(Gate::XOR);
            else if (IsKeyPressed(KEY_FIVE))  SetGate(Gate::RESISTOR);
            else if (IsKeyPressed(KEY_SIX))   SetGate(Gate::CAPACITOR);
            else if (IsKeyPressed(KEY_SEVEN)) SetGate(Gate::LED);
            else if (IsKeyPressed(KEY_EIGHT)) SetGate(Gate::DELAY);
        }

        // Mode hotkeys
        if      (IsKeyPressed(KEY_B)) SetMode(Mode::PEN);
        else if (IsKeyPressed(KEY_V)) SetMode(Mode::EDIT);
        else if (IsKeyPressed(KEY_G)) SetMode(Mode::GATE);
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

    // Inclusive
    inline bool CursorInRangeX(int xMin, int xMax) const
    {
        return cursorPos.x >= xMin && cursorPos.x <= xMax;
    }
    // Inclusive
    inline bool CursorInUIRangeX(int xMin, int xMax) const
    {
        return cursorUIPos.x >= xMin && cursorUIPos.x <= xMax;
    }

    // Inclusive
    inline bool CursorInRangeY(int yMin, int yMax) const
    {
        return cursorPos.y >= yMin && cursorPos.y <= yMax;
    }
    // Inclusive
    inline bool CursorInUIRangeY(int yMin, int yMax) const
    {
        return cursorUIPos.y >= yMin && cursorUIPos.y <= yMax;
    }

    inline bool CursorInBounds(IRect bounds) const
    {
        return InBoundingBox(bounds, cursorPos);
    }
    inline bool CursorInUIBounds(IRect uiBounds) const
    {
        return InBoundingBox(uiBounds, cursorUIPos);
    }

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
            constexpr int halfgrid = gridSize / 2;
            DrawLine(bounds.x, -halfgrid, bounds.x + bounds.w, -halfgrid, LIFELESSNEBULA);
            DrawLine(-halfgrid, bounds.y, -halfgrid, bounds.y + bounds.h, LIFELESSNEBULA);
        }
    }
    void DrawGrid(int gridSize) const
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
};

void Update_Pen(ProgramData& data)
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
void Update_Edit(ProgramData& data)
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
        if (!data.selection.empty() && !!data.hoveredNode) // There is a selection, and a node has been pressed
        {
            data.edit.nodeBeingDragged = data.hoveredNode;
            data.edit.wireBeingDragged = nullptr;

            int minx = INT_MAX;
            int miny = INT_MAX;
            int maxx = INT_MIN;
            int maxy = INT_MIN;

            for (Node* node : data.selection)
            {
                if (node->GetX() < minx) minx = node->GetX();
                else if (node->GetX() > maxx) maxx = node->GetX();
                if (node->GetY() < miny) miny = node->GetY();
                else if (node->GetY() > maxy) maxy = node->GetY();
            }

            data.edit.selectionRec.w = maxx - (data.edit.selectionRec.x = minx);
            data.edit.selectionRec.h = maxy - (data.edit.selectionRec.y = miny);
        }
        else
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

            data.edit.fallbackPos = cursorPos;
            if (data.edit.selectionWIP = !(data.edit.nodeBeingDragged || data.edit.wireBeingDragged || data.edit.draggingGroup))
                data.edit.selectionStart = cursorPos;
        }
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
        // Multiple selection
        if (!data.selection.empty())
        {
            const IVec2 offset = cursorPos - cursorPosPrev;
            for (Node* node : data.selection)
            {
                node->SetPosition_Temporary(node->GetPosition() + offset);
            }
            data.edit.selectionRec.position += offset;
        }
        else
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
                data.edit.selectionRec = IRect(0, 0, 0, 0);
            }
        }
        // Finalize
        else
        {
            if (!!data.edit.nodeBeingDragged)
            {
                data.edit.nodeBeingDragged->SetPosition(data.edit.fallbackPos);
                data.hoveredNode = NodeWorld::Get().FindNodeAtPos(cursorPos);
                if (!!data.hoveredNode && data.edit.nodeBeingDragged != data.hoveredNode)
                    NodeWorld::Get().SwapNodes(data.edit.nodeBeingDragged, data.hoveredNode);
                else
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
void Update_Erase(ProgramData& data)
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
void Update_Interact(ProgramData& data)
{
    data.hoveredNode = NodeWorld::Get().FindNodeAtPos(cursorPos);
    if (!!data.hoveredNode && !data.hoveredNode->IsOutputOnly())
        data.hoveredNode = nullptr;

    if (!!data.hoveredNode && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        data.hoveredNode->SetGate(data.hoveredNode->GetGate() == Gate::NOR ? Gate::OR : Gate::NOR);
}
void Update_Overlay_Gate(ProgramData& data)
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
            SetGate(radialGateOrder[data.gate.overlappedSection]);

        mode = baseMode;
        SetMousePosition(data.gate.radialMenuCenter.x, data.gate.radialMenuCenter.y);
        cursorUIPos = data.gate.radialMenuCenter;
    }
}
void Update_Overlay_Button(ProgramData& data)
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
                        SetGate(g);
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
void Update_Overlay_Paste(ProgramData& data)
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            NodeWorld::Get().SpawnBlueprint(data.clipboard, cursorPos);
        data.selection.clear();
        SetMode(baseMode);
    }
}
void Update_Menu_Icon(ProgramData& data)
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
// todo
void Update_Menu_Select(ProgramData& data)
{
    
}


void Draw_Pen(ProgramData& data)
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
void Draw_Edit(ProgramData& data)
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
}
void Draw_Erase(ProgramData& data)
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
        DrawCross(data.cursorPos, DESTRUCTIVERED);
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
void Draw_Interact(ProgramData& data)
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
void Draw_Overlay_Gate(ProgramData& data)
{
    if (data.baseMode == Mode::PEN)
    {
        NodeWorld::Get().DrawWires();

        if (!!data.Pen_CurrentWireStart())
        {
            IVec2 start = data.Pen_CurrentWireStart()->GetPosition();
            IVec2 elbow;
            IVec2 end = data.Gate_RadialMenuCenter();
            elbow = Wire::GetLegalElbowPosition(start, end, data.Pen_CurrentWireElbowConfig());
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
        IRect(+menuIconOffset,      +menuIconOffset,      32, 32),
        IRect(+menuIconOffset,      -menuIconOffset - 32, 32, 32),
        IRect(-menuIconOffset - 32, -menuIconOffset - 32, 32, 32),
        IRect(-menuIconOffset - 32, +menuIconOffset,      32, 32),
    };
    int x = data.Gate_RadialMenuCenter().x;
    int y = data.Gate_RadialMenuCenter().y;
    constexpr Color menuBackground = SPACEGRAY;
    DrawCircleIV(data.Gate_RadialMenuCenter(), static_cast<float>(menuRadius + 4), menuBackground);

    for (int i = 0; i < 4; ++i)
    {
        Color colorA;
        Color colorB;
        int radius;

        if (i == data.Gate_OverlappedSection()) // Active
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
        ProgramData::DrawGateIcon32x(ProgramData::radialGateOrder[i], rec, colorB);

        EndScissorMode();
    }
}
void Draw_Overlay_Button(ProgramData& data)
{
    NodeWorld::Get().DrawWires();
    NodeWorld::Get().DrawNodes();

    EndMode2D();

    IRect rec = data.dropdownBounds[data.Button_DropdownActive()];
    DrawRectangleIRect(rec, SPACEGRAY);
    rec.h = 16;

    switch (data.Button_DropdownActive())
    {
    case 0: // Mode
    {
        for (Mode m : ProgramData::dropdownModeOrder)
        {
            if (m == data.baseMode)
                continue;
            Color color;
            if (InBoundingBox(rec, data.cursorUIPos))
            {
                color = WHITE;
                DrawText(ProgramData::GetModeTooltipName(m), 20, 17 + rec.y, 8, WHITE);
            }
            else
                color = DEADCABLE;
            ProgramData::DrawModeIcon(m, rec.xy, color);
            rec.y += 16;
        }
    }
    break;

    case 1: // Gate
    {
        for (Gate g : ProgramData::dropdownGateOrder)
        {
            if (g == data.gatePick)
                continue;
            Color color;
            if (InBoundingBox(rec, data.cursorUIPos))
            {
                color = WHITE;
                DrawText(ProgramData::GetGateTooltipName(g), 20 + 16, 17 + rec.y, 8, WHITE);
            }
            else
                color = DEADCABLE;
            ProgramData::DrawGateIcon16x(g, rec.xy, color);
            rec.y += 16;
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
                DrawRectangleIRect(rec, WIPBLUE);
                DrawRectangleIRect(ExpandIRect(rec, -2), color);
                const char* text;
                if (data.gatePick == Gate::LED)
                    text = TextFormat(data.deviceParameterTextFmt, Node::GetColorName(v));
                else
                    text = TextFormat(data.deviceParameterTextFmt, v);
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
void Draw_Overlay_Paste(ProgramData& data)
{
    NodeWorld::Get().DrawWires();
    NodeWorld::Get().DrawNodes();

    data.clipboard->DrawPreview(data.cursorPos, ColorAlpha(LIFELESSNEBULA, 0.5f), HAUNTINGWHITE);
}
void Draw_Menu_Icon(ProgramData& data)
{
    _ASSERT_EXPR(!!data.BPIcon_Object(), L"Blueprint icon object not initialized");
    _ASSERT_EXPR(data.IsClipboardValid(), L"Bad entry into Mode::BP_ICON");

    data.BPIcon_Object()->DrawBackground(data.BPIcon_Pos(), SPACEGRAY);
    data.BPIcon_Object()->Draw(data.BPIcon_Pos(), WHITE);

    for (size_t i = 0; i < 4; ++i)
    {
        for (int i = 0; i < 4; ++i)
        {
            if (data.BPIcon_Object()->combo[i].id == NULL)
                continue;

            IRect bounds(data.BPIcon_Pos(), BlueprintIcon::g_size);
            bounds.xy = bounds.xy + data.BPIcon_Object()->combo[i].Pos();
            if (data.CursorInUIBounds(bounds))
                DrawRectangleIRect(bounds, ColorAlpha(WIPBLUE, 0.25f));
        }
    }
    if (data.BPIcon_DraggingIcon() != -1)
        BlueprintIcon::DrawBPIcon(data.BPIcon_IconID(), data.BPIcon_Pos() + data.BPIcon_Object()->combo[data.BPIcon_DraggingIcon()].Pos(), WIPBLUE);

    BlueprintIcon::DrawSheet(data.BPIcon_SheetRec().xy, SPACEGRAY, WHITE);
}
// todo
void Draw_Menu_Select(ProgramData& data)
{
    DrawText("[ @TODO make blueprint selection screen ]\nPress Esc to return to circuit graph.", 4, 4, 8, WHITE);
}

int main()
{
    /******************************************
    *   Load textures, shaders, and meshes
    ******************************************/

    ProgramData data(1280, 720);

    NodeWorld::Get().Load("session.cg"); // Construct and load last session

    while (!WindowShouldClose())
    {
        /******************************************
        *   Simulate frame and update variables
        ******************************************/

        data.IncrementTick();

        data.UpdateCursorPos();

        data.UpdateCamera();

        data.CheckHotkeys();

        // UI buttons
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !data.ModeIsMenu() && (data.cursorUIPos.y <= 16 && data.cursorUIPos.x <= (16 * 3)) && (data.mode == Mode::BUTTON ? data.Button_DropdownActive() != (data.cursorUIPos.x / 16) : true))
            {
                data.SetMode(Mode::BUTTON);
                goto EVAL; // Skip button sim this frame
            }
            else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !data.ModeIsMenu() && (data.cursorUIPos.y <= 16 && data.cursorUIPos.x >= (16 * 3) && data.cursorUIPos.x <= (16 * 4)))
            {
                data.SetMode(Mode::BP_SELECT);
            }
        }

        // Input
        switch (data.mode)
        {
            ASSERT_SPECIALIZATION(L"Simulation phase");
            
        // Basic
        case Mode::PEN:         Update_Pen(data);               break;
        case Mode::EDIT:        Update_Edit(data);              break;
        case Mode::ERASE:       Update_Erase(data);             break;
        case Mode::INTERACT:    Update_Interact(data);          break;

        // Overlay
        case Mode::GATE:        Update_Overlay_Gate(data);      break;
        case Mode::BUTTON:      Update_Overlay_Button(data);    break;
        case Mode::PASTE:       Update_Overlay_Paste(data);     break;
        case Mode::BP_ICON:     Update_Menu_Icon(data);         break;
        case Mode::BP_SELECT:   Update_Menu_Select(data);       break;
        }

    EVAL:
        data.cursorPosPrev = data.cursorPos;
        if (data.tickThisFrame)
            NodeWorld::Get().Evaluate();

        /******************************************
        *   Draw the frame
        ******************************************/

        BeginDrawing(); {

            ClearBackground(BLACK);

            if (!data.ModeIsMenu())
            {
                BeginMode2D(data.camera);

                data.DrawGrid();

                NodeWorld::Get().DrawGroups();
            }
                

            // Draw
            switch (data.mode)
            {
                ASSERT_SPECIALIZATION(L"Basic draw phase");

                // Basic
            case Mode::PEN:         Draw_Pen(data);             break;
            case Mode::EDIT:        Draw_Edit(data);            break;
            case Mode::ERASE:       Draw_Erase(data);           break;
            case Mode::INTERACT:    Draw_Interact(data);        break;

                // Overlay
            case Mode::GATE:        Draw_Overlay_Gate(data);    break;
            case Mode::BUTTON:      Draw_Overlay_Button(data);  break;
            case Mode::PASTE:       Draw_Overlay_Paste(data);   break;

                // Menu
            case Mode::BP_ICON:     Draw_Menu_Icon(data);       break;
            case Mode::BP_SELECT:   Draw_Menu_Select(data);     break;
            }

            if (!data.ModeIsMenu())
            {
                EndMode2D();

                // UI

                DrawRectangleIRect(IRect(32, 16), SPACEGRAY);
                DrawRectangleIRect(IRect(64, 16), SPACEGRAY);
                if (!!data.clipboard)
                {
                    constexpr IRect clipboardRec(16 * 4, 0, 16);
                    DrawRectangleIRect(clipboardRec, SPACEGRAY);
                    DrawTextureIV(data.GetClipboardIcon(), clipboardRec.xy, WHITE);
                }

                _ASSERT_EXPR(data.storedExtraParam < _countof(Node::g_resistanceBands), L"Stored parameter out of bounds");
                DrawRectangleIRect(IRect(32, 0, 16), Node::g_resistanceBands[data.storedExtraParam]);

                // Buttons
                if (data.cursorUIPos.y <= 16)
                {
                    // Mode
                    if (data.cursorUIPos.x <= 16)
                    {
                        constexpr IRect rec(0, 0, 16);
                        DrawRectangleIRect(rec, WIPBLUE);
                        const char* name = data.GetModeTooltipName(data.baseMode);
                        DrawText(name, 20, 17, 8, WHITE);
                        DrawLine(20, 17 + 12, 20 + MeasureText(name, 8), 17 + 12, WHITE);
                        DrawText(data.GetModeTooltipDescription(data.baseMode), 20, 17 + 16, 8, WHITE);
                    }
                    // Gate
                    else if (data.cursorUIPos.x <= 32)
                    {
                        constexpr IRect rec(16, 0, 16);
                        DrawRectangleIRect(rec, WIPBLUE);
                        const char* name = data.GetGateTooltipName(data.gatePick);
                        DrawText(name, 36, 17, 8, WHITE);
                        DrawLine(36, 17 + 12, 36 + MeasureText(name, 8), 17 + 12, WHITE);
                        DrawText(data.GetGateTooltipDescription(data.gatePick), 36, 17 + 16, 8, WHITE);
                    }
                    // Extra param
                    else if (data.cursorUIPos.x <= 48)
                    {
                        constexpr IRect rec(32, 0, 16);
                        DrawRectangleIRect(rec, WIPBLUE);
                        _ASSERT_EXPR(data.storedExtraParam < _countof(Node::g_resistanceBands), L"Stored parameter out of bounds");
                        DrawRectangleIRect(ShrinkIRect(rec, 2), Node::g_resistanceBands[data.storedExtraParam]);
                        const char* text;
                        if (data.gatePick == Gate::LED)
                            text = TextFormat(data.deviceParameterTextFmt, Node::GetColorName(data.storedExtraParam));
                        else
                            text = TextFormat(data.deviceParameterTextFmt, data.storedExtraParam);
                        DrawText(text, 52, 17, 8, WHITE);
                    }
                    // Blueprints
                    else if (data.cursorUIPos.x <= 64)
                    {
                        constexpr IRect rec(48, 0, 16);
                        DrawRectangleIRect(rec, WIPBLUE);
                    }
                }

                data.DrawModeIcon(data.baseMode, IVec2(0), WHITE);
                data.DrawGateIcon16x(data.gatePick, IVec2(16,0), WHITE);
            }

        } EndDrawing();
    }

    /******************************************
    *   Unload and free memory
    ******************************************/

	return 0;
}

/* Todo
* Requirements for v1.0.0
* -Improve groups
* -Fix wonky zooming
* -More explanation of controls
* -Undo/redo
* -Blueprint pallet
* -Prefab blueprints for things like timers, counters, and latches
* -Blueprint pallet icons (User-made combination of 4 premade icons. See Factorio for inspiration)
* -File menu (https://en.cppreference.com/w/cpp/filesystem/directory_iterator)
* -File thumbnails (based on file contents)
* -Menu screen (Open to file menu with "new" at the top)
*
* Refactors
* -Refactor NodeWorld to have a bulk destroy function to make bulk deletion more efficient
* -Refactor buttons to be classes/structs instead of freeform
* 
* Beyond v1.0.0
* -Parallel node drawing with pen (multiple nodes created with parallel wires)
* -Special erase (keep wires, erase node)
* -Multiple wire stacking on single node
* -Hotkeys for output-only gate state toggles (Like the Reason on-screen piano (Yes, this is different from interact mode))
*
* Optional
* -Multiple color pallets
* -Log files for debug/crashes
*/
