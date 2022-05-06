#include <raylib.h>
#include <fstream>
#include <filesystem>
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

    BUTTON,
    PASTE,

    BP_SELECT,
};

Blueprint g_clipboardBP; // Reusable address so clipboard doesn't have to delete

struct ProgramData
{
    ProgramData(int windowWidth, int windowHeight) : windowWidth(windowWidth), windowHeight(windowHeight)
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

        SetMode(Mode::PEN);
        SetGate(Gate::OR);
        ReloadConfig();
    }
    ~ProgramData()
    {
        NodeWorld::Get().Save("session.cg");
        NodeWorld::Get().Export("render.svg");

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
    int ButtonWidth() const
    {
        return 16 * uiScale;
    }
    int FontSize() const
    {
        switch (uiScale)
        {
        default:
        case 1: return 8;
        case 2: return 20;
        }
    }
    static IRect buttonBounds[5];
    static IRect ButtonBound_Mode()
    {
        return buttonBounds[0];
    }
    static IRect ButtonBound_Gate()
    {
        return buttonBounds[1];
    }
    static IRect ButtonBound_Parameter()
    {
        return buttonBounds[2];
    }
    static IRect ButtonBound_Blueprints()
    {
        return buttonBounds[3];
    }
    static IRect ButtonBound_Clipboard()
    {
        return buttonBounds[4];
    }
    static IRect dropdownBounds[3];

private:
    static Texture2D blueprintIcon16x;
    static Texture2D blueprintIcon32x;
    static Texture2D clipboardIcon16x;
    static Texture2D clipboardIcon32x;
    static Texture2D modeIcons16x;
    static Texture2D modeIcons32x;
    static Texture2D gateIcons16x;
    static Texture2D gateIcons32x;
public:

    int windowWidth;
    int windowHeight;

    uint8_t blueprintLOD = 0;
    uint8_t clipboardPreviewLOD = 0;
    uint8_t pastePreviewLOD = 0;
    int uiScale = 1;

    Mode mode = Mode::PEN;
    Mode baseMode = Mode::PEN;

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

#if _DEBUG
private:
#endif
    // Base mode
    union BaseModeData
    {
        struct PenModeData
        {
            IVec2 dragStart;
            ElbowConfig currentWireElbowConfig;
            Node* previousWireStart;
            Node* currentWireStart;
        } pen;

        struct EditModeData
        {
            IVec2 fallbackPos;
            bool selectionWIP;
            IVec2 selectionStart;
            IRect selectionRec;
            bool draggingGroup;
            bool draggingGroupCorner;
            GroupCorner groupCorner;
            Node* hoveringMergable;
            Node* nodeBeingDragged;
            Wire* wireBeingDragged;
        } edit;

        struct EraseModeData
        {
        } erase;

        struct InteractModeData
        {
        } interact;

        BaseModeData() { memset(this, 0, sizeof(BaseModeData)); }
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
            IVec2 pos; // Width and height are fixed
            IRect sheetRec;
            uint16_t iconID;
            uint8_t iconCount;
            int draggingIcon; // -1 for none/not dragging
        } bp_icon;

        struct BP_SelectModeData
        {
            Blueprint* hovering;
        } bp_select;

        OverlayModeData() { memset(this, 0, sizeof(OverlayModeData)); }
    } overlay;

#pragma region Accessors
public: // Accessors for unions
#define ACCESSOR(name, assertion, bind) \
    inline       decltype(bind)& name       { _ASSERT_EXPR(assertion, L"Tried to access member of different mode"); return bind; } \
    inline const decltype(bind)& name const { _ASSERT_EXPR(assertion, L"Tried to access member of different mode"); return bind; }

    /**********
    * Basic
    **********/

    // Pen
    ACCESSOR(Pen_DragStart(),               baseMode == Mode::PEN,      base.pen.dragStart)
    ACCESSOR(Pen_CurrentWireElbowConfig(),  baseMode == Mode::PEN,      base.pen.currentWireElbowConfig)
    ACCESSOR(Pen_PreviousWireStart(),       baseMode == Mode::PEN,      base.pen.previousWireStart)
    ACCESSOR(Pen_CurrentWireStart(),        baseMode == Mode::PEN,      base.pen.currentWireStart)

    // Edit
    ACCESSOR(Edit_FallbackPos(),            baseMode == Mode::EDIT,     base.edit.fallbackPos)
    ACCESSOR(Edit_SelectionWIP(),           baseMode == Mode::EDIT,     base.edit.selectionWIP)
    ACCESSOR(Edit_SelectionStart(),         baseMode == Mode::EDIT,     base.edit.selectionStart)
    ACCESSOR(Edit_SelectionRec(),           baseMode == Mode::EDIT,     base.edit.selectionRec)
    ACCESSOR(Edit_DraggingGroup(),          baseMode == Mode::EDIT,     base.edit.draggingGroup)
    ACCESSOR(Edit_DraggingGroupCorner(),    baseMode == Mode::EDIT,     base.edit.draggingGroupCorner)
    ACCESSOR(Edit_GroupCorner(),            baseMode == Mode::EDIT,     base.edit.groupCorner)
    ACCESSOR(Edit_HoveringMergable(),       baseMode == Mode::EDIT,     base.edit.hoveringMergable)
    ACCESSOR(Edit_NodeBeingDragged(),       baseMode == Mode::EDIT,     base.edit.nodeBeingDragged)
    ACCESSOR(Edit_WireBeingDragged(),       baseMode == Mode::EDIT,     base.edit.wireBeingDragged)

    /**********
    * Overlay
    **********/

    // Button
    ACCESSOR(Button_DropdownActive(),       mode == Mode::BUTTON,       overlay.button.dropdownActive)

    // Select
    ACCESSOR(BPSelect_Hovering(),           mode == Mode::BP_SELECT,    overlay.bp_select.hovering)

#undef ACCESSOR
#pragma endregion

public:

    Texture2D GetBlueprintIcon()
    {
        switch (uiScale)
        {
        default:
        case 1: return blueprintIcon16x;
        case 2: return blueprintIcon32x;
        }
    }
    Texture2D GetClipboardIcon()
    {
        switch (uiScale)
        {
        default:
        case 1: return clipboardIcon16x;
        case 2: return clipboardIcon32x;
        }
    }
    void DrawModeIcon(Mode mode, IVec2 pos, Color tint)
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
        switch (uiScale)
        {
        case 1: DrawIcon<16>(modeIcons16x, offset, pos, tint); break;
        case 2: DrawIcon<32>(modeIcons32x, offset, pos, tint); break;
        default: break;
        }
    };
    void DrawGateIcon(Gate gate, IVec2 pos, Color tint)
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
        switch (uiScale)
        {
        case 1: DrawIcon<16>(gateIcons16x, offset, pos, tint); break;
        case 2: DrawIcon<32>(gateIcons32x, offset, pos, tint); break;
        default: break;
        }
    }

    static bool ModeIsMenu(Mode mode)
    {
        // Modes which disable use of basic UI and drawing of certain UI elements
        return mode == Mode::BP_SELECT;
    }
    inline bool ModeIsMenu() const
    {
        return ModeIsMenu(this->mode);
    }
    static bool ModeIsOverlay(Mode mode)
    {
        // Modes which can be active simultaneously with a non-overlay mode
        return mode == Mode::BUTTON || mode == Mode::PASTE || ModeIsMenu(mode);
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
        case Mode::BUTTON:
            Button_DropdownActive() = cursorUIPos.x / ButtonWidth();
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

    inline void ClearOverlayMode()
    {
        SetMode(baseMode);
    }

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
    void UpdateCamera()
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

    void CopySelectionToClipboard()
    {
        if (selection.empty())
            clipboard = nullptr;
        else
        {
            g_clipboardBP = Blueprint(selection);
            clipboard = &g_clipboardBP;
        }
    }

    void MakeGroupFromSelection()
    {
        NodeWorld::Get().CreateGroup(Edit_SelectionRec());
        Edit_SelectionRec() = IRect(0);
        selection.clear();
    }

    bool IsSelectionRectValid() const
    {
        return mode == Mode::EDIT && !Edit_SelectionWIP() && Edit_SelectionRec().w > 0 && Edit_SelectionRec().h > 0;
    }

    void SaveBlueprint()
    {
        if (!IsClipboardValid())
            return;
        NodeWorld::Get().StoreBlueprint(clipboard);
        clipboard->Save();
    }

    bool IsClipboardValid() const
    {
        return !!clipboard;
    }
    void ClearClipboard()
    {
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
            Edit_SelectionRec() = IRect(0);
    }
    void DestroySelection()
    {
        NodeWorld::Get().DestroyNodes(selection);
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
        if (ModeIsBasic())
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

    inline IVec2 GetCursorDelta() const
    {
        return cursorPos - cursorPosPrev;
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

    IRect GetSelectionBounds(const std::vector<Node*>& vec) const
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
    IRect GetSelectionBounds() const
    {
        return GetSelectionBounds(selection);
    }

    inline static Color ResistanceBandColor(uint8_t index)
    {
        _ASSERT_EXPR(index < _countof(Node::g_resistanceBands), L"Stored parameter out of bounds");
        return Node::g_resistanceBands[index];
    }
    inline Color ExtraParamColor() const
    {
        return ResistanceBandColor(storedExtraParam);
    }

    void DrawTooltipAtCursor(const char* text, Color color)
    {
        EndMode2D();
        DrawTextIV(text, cursorUIPos + IVec2(16), 8, color);
        BeginMode2D(camera);
    }

    void DrawTooltipAtCursor_Shadowed(const char* text, Color color)
    {
        EndMode2D();
        IVec2 offset;
        for (offset.y = 16 - 1; offset.y <= 16 + 1; ++offset.y)
        {
            for (offset.x = 16 - 1; offset.x <= 16 + 1; ++offset.x)
            {
                if (offset == IVec2(16)) [[unlikely]]
                    continue;
                DrawTextIV(text, cursorUIPos + offset, 8, BLACK);
            }
        }
        DrawTextIV(text, cursorUIPos + IVec2(16), 8, color);
        BeginMode2D(camera);
    }

    void ReloadConfig()
    {
        std::ifstream file("config.ini");
        if (!file.is_open()) // In case of deletion
        {
            file.close();

            std::ofstream replacement("config.ini");

            replacement << R"txt([Colors]
background_color=28|26|41|255
[LOD]
blueprint_menu_lod=1
clipboard_preview_lod=0
paste_preview_lod=4
frames_per_tick=6)txt";

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
            if (attribute == "background_color")
            {
                // Todo
            }
            else if (attribute == "blueprint_menu_lod")
            {
                blueprintLOD = std::stoi(value);
            }
            else if (attribute == "clipboard_preview_lod")
            {
                clipboardPreviewLOD = std::stoi(value);
            }
            else if (attribute == "paste_preview_lod")
            {
                pastePreviewLOD = std::stoi(value);
            }
            else if (attribute == "paste_preview_lod")
            {
                pastePreviewLOD = std::stoi(value);
            }
            else if (attribute == "frames_per_tick")
            {
                framesPerTick = std::stoi(value);
            }
            else if (attribute == "ui_scale")
            {
                uiScale = std::stoi(value);

                if (uiScale >= 2)
                    uiScale = 2;
                if (uiScale <= 1)
                    uiScale = 1;

                const int width = 16 * uiScale;
                for (int i = 0; i < _countof(buttonBounds); ++i)
                {
                    buttonBounds[i] = IRect(i * width, 0, width);
                }

                constexpr int heights[] =
                {
                    _countof(dropdownModeOrder) - 1,
                    _countof(dropdownGateOrder) - 1,
                    _countof(Node::g_resistanceBands) - 1,
                };
                for (int i = 0; i < _countof(dropdownBounds); ++i)
                {
                    dropdownBounds[i].x = i * width;
                    dropdownBounds[i].y = width;
                    dropdownBounds[i].w = width;
                    dropdownBounds[i].h = width * heights[i];
                }
            }
        }
        file.close();
    }
};
IRect ProgramData::buttonBounds[5];
IRect ProgramData::dropdownBounds[3];
Texture2D ProgramData::blueprintIcon16x;
Texture2D ProgramData::blueprintIcon32x;
Texture2D ProgramData::clipboardIcon16x;
Texture2D ProgramData::clipboardIcon32x;
Texture2D ProgramData::modeIcons16x;
Texture2D ProgramData::modeIcons32x;
Texture2D ProgramData::gateIcons16x;
Texture2D ProgramData::gateIcons32x;

void Update_Pen(ProgramData& data)
{
    if (data.b_cursorMoved) // On move
    {
        data.hoveredWire = nullptr;
        data.hoveredNode = NodeWorld::Get().FindNodeAtPos(data.cursorPos);
        if (!data.hoveredNode)
            data.hoveredWire = NodeWorld::Get().FindWireAtPos(data.cursorPos);
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        Node* newNode = data.hoveredNode;
        if (!newNode)
        {
            newNode = NodeWorld::Get().CreateNode(data.cursorPos, data.gatePick, data.storedExtraParam);
            if (!!data.hoveredWire)
            {
                NodeWorld::Get().BisectWire(data.hoveredWire, newNode);
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
                Wire* wire = NodeWorld::Get().CreateWire(oldNode, newNode);
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
void Draw_Pen(ProgramData& data)
{
    NodeWorld::Get().DrawWires();

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

    EndMode2D();

    // Stats
    {
        int i = 0;
        // Cursor stats
        DrawText(TextFormat("y: %i", data.cursorPos.y / g_gridSize), 2, data.windowHeight - (++i * 12), 8, WHITE);
        DrawText(TextFormat("x: %i", data.cursorPos.x / g_gridSize), 2, data.windowHeight - (++i * 12), 8, WHITE);
        
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
                DrawText(stateName, 2, data.windowHeight - (++i * 12), 8, HAUNTINGWHITE);
            }
            DrawText(TextFormat("\toutputs: %i", data.hoveredNode->GetOutputCount()), 2, data.windowHeight - (++i * 12), 8, HAUNTINGWHITE);
            DrawText(TextFormat("\tinputs: %i", data.hoveredNode->GetInputCount()), 2, data.windowHeight - (++i * 12), 8, HAUNTINGWHITE);
            DrawText(TextFormat("\ttype: %s", gateName), 2, data.windowHeight - (++i * 12), 8, HAUNTINGWHITE);
            DrawText(TextFormat("\tserial: %u", NodeWorld::Get().NodeID(data.hoveredNode)), 2, data.windowHeight - (++i * 12), 8, HAUNTINGWHITE);
            DrawText(TextFormat("\tptr: %p", data.hoveredNode), 2, data.windowHeight - (++i * 12), 8, HAUNTINGWHITE);
            DrawText("hovered node", 2, data.windowHeight - (++i * 12), 8, WHITE);
        }
        // Wire hover stats
        else if (!!data.hoveredWire)
        {
            DrawText(TextFormat("\t\tptr: %p", data.hoveredWire->end), 2, data.windowHeight - (++i * 12), 8, HAUNTINGWHITE);
            DrawText(TextFormat("\t\tserial: %u", NodeWorld::Get().NodeID(data.hoveredWire->end)), 2, data.windowHeight - (++i * 12), 8, HAUNTINGWHITE);
            DrawText("\toutput", 2, data.windowHeight - (++i * 12), 8, INTERFERENCEGRAY);
            DrawText(TextFormat("\t\tptr: %p", data.hoveredWire->start), 2, data.windowHeight - (++i * 12), 8, HAUNTINGWHITE);
            DrawText(TextFormat("\t\tserial: %u", NodeWorld::Get().NodeID(data.hoveredWire->start)), 2, data.windowHeight - (++i * 12), 8, HAUNTINGWHITE);
            DrawText("\tinput", 2, data.windowHeight - (++i * 12), 8, INTERFERENCEGRAY);
            // State
            {
                const char* stateName;
                if (data.hoveredWire->GetState())
                    stateName = "\tactive";
                else
                    stateName = "\tinactive";
                DrawText(stateName, 2, data.windowHeight - (++i * 12), 8, HAUNTINGWHITE);
            }
            DrawText(TextFormat("\tptr: %p", data.hoveredWire), 2, data.windowHeight - (++i * 12), 8, HAUNTINGWHITE);
            DrawText("hovered wire", 2, data.windowHeight - (++i * 12), 8, WHITE);
        }
    }
}

void Update_Edit(ProgramData& data)
{
    // Todo: fix bug with canceling multiple-drag (And update group dragging to match!!)

    if (data.b_cursorMoved)
    {
        if (!data.Edit_NodeBeingDragged() &&
            !data.Edit_WireBeingDragged() &&
            !data.Edit_DraggingGroup() &&
            !data.Edit_DraggingGroupCorner())
        {
            data.hoveredGroup = nullptr;
            data.hoveredWire = nullptr;
            data.hoveredNode = NodeWorld::Get().FindNodeAtPos(data.cursorPos);
            if (!data.hoveredNode)
            {
                data.hoveredWire = NodeWorld::Get().FindWireElbowAtPos(data.cursorPos);
                if (!data.hoveredWire)
                {
                    data.hoveredGroup = NodeWorld::Get().FindGroupAtPos(data.cursorPos);
                    if (!data.hoveredGroup)
                    {
                        data.Edit_GroupCorner() = NodeWorld::Get().FindGroupCornerAtPos(data.cursorPos);
                    }
                }
            }
        }
        else if (data.Edit_NodeBeingDragged())
        {
            data.Edit_HoveringMergable() = NodeWorld::Get().FindNodeAtPos(data.cursorPos); // This will come before updating the position of the dragged node
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
        else
        {
            data.selection.clear();
            data.Edit_NodeBeingDragged() = data.hoveredNode;
            data.Edit_WireBeingDragged() = data.hoveredWire;

            // selectionStart being used as an offset here
            if (data.Edit_DraggingGroup() = !!data.hoveredGroup)
            {
                NodeWorld::Get().FindNodesInGroup(data.selection, data.hoveredGroup);
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
        IRect captureBounds =  data.Edit_GroupCorner().group->GetCaptureBounds();
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
                        data.hoveredNode = NodeWorld::Get().MergeNodes(data.Edit_HoveringMergable(), data.Edit_NodeBeingDragged());
                        data.Edit_HoveringMergable() = data.Edit_NodeBeingDragged() = nullptr;
                    }
                    else
                    {
                        NodeWorld::Get().SwapNodes(data.Edit_HoveringMergable(), data.Edit_NodeBeingDragged());
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
                    NodeWorld::Get().FindNodesInRect(data.selection, data.Edit_SelectionRec());
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
void Draw_Edit(ProgramData& data)
{
    if (!!data.hoveredGroup)
    {
        data.hoveredGroup->Highlight(INTERFERENCEGRAY);
    }
    else if (data.Edit_GroupCorner().Valid())
    {
        Color color;
        if (data.Edit_DraggingGroupCorner())
            color = INTERFERENCEGRAY;
        else
            color = data.Edit_GroupCorner().group->GetColor();
        DrawRectangleIRect(data.Edit_GroupCorner().GetCollisionRect(), color);
    }

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

    if (!!data.Edit_NodeBeingDragged() && data.Edit_HoveringMergable())
    {
        DrawCircleIV(data.Edit_NodeBeingDragged()->GetPosition(), Node::g_nodeRadius * 2.0f, VIOLET);
        data.DrawTooltipAtCursor(
            "Hold [shift] to merge on release.\n"
            "Otherwise, nodes will only be swapped.", VIOLET);
    }

    EndMode2D();
    
    // Stats
    {
        int i = 0;
        // Cursor stats
        DrawText(TextFormat("y: %i", data.cursorPos.y / g_gridSize), 2, data.windowHeight - (++i * 12), 8, WHITE);
        DrawText(TextFormat("x: %i", data.cursorPos.x / g_gridSize), 2, data.windowHeight - (++i * 12), 8, WHITE);

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
                }
            }

            if (DELs) DrawText(TextFormat("\t\tdelay: %i", DELs), 2, data.windowHeight - (++i * 12), 8, HAUNTINGWHITE);
            if (LEDs) DrawText(TextFormat("\t\tLED: %i", LEDs), 2, data.windowHeight - (++i * 12), 8, HAUNTINGWHITE);
            if (CAPs) DrawText(TextFormat("\t\tcapacitor: %i", CAPs), 2, data.windowHeight - (++i * 12), 8, HAUNTINGWHITE);
            if (RESs) DrawText(TextFormat("\t\tresistor: %i", RESs), 2, data.windowHeight - (++i * 12), 8, HAUNTINGWHITE);
            if (XORs) DrawText(TextFormat("\t\txor: %i", XORs), 2, data.windowHeight - (++i * 12), 8, HAUNTINGWHITE);
            if (NORs) DrawText(TextFormat("\t\tnor: %i", NORs), 2, data.windowHeight - (++i * 12), 8, HAUNTINGWHITE);
            if (ANDs) DrawText(TextFormat("\t\tand: %i", ANDs), 2, data.windowHeight - (++i * 12), 8, HAUNTINGWHITE);
            if (ORs)  DrawText(TextFormat("\t\tor: %i", ORs), 2, data.windowHeight - (++i * 12), 8, HAUNTINGWHITE);
            DrawText(TextFormat("\ttotal: %i", data.selection.size()), 2, data.windowHeight - (++i * 12), 8, INTERFERENCEGRAY);
            DrawText("selection", 2, data.windowHeight - (++i * 12), 8, WHITE);
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
                DrawText(stateName, 2, data.windowHeight - (++i * 12), 8, HAUNTINGWHITE);
            }
            DrawText(TextFormat("\toutputs: %i", data.hoveredNode->GetOutputCount()), 2, data.windowHeight - (++i * 12), 8, HAUNTINGWHITE);
            DrawText(TextFormat("\tinputs: %i", data.hoveredNode->GetInputCount()), 2, data.windowHeight - (++i * 12), 8, HAUNTINGWHITE);
            DrawText(TextFormat("\ttype: %s", gateName), 2, data.windowHeight - (++i * 12), 8, HAUNTINGWHITE);
            DrawText(TextFormat("\tserial: %u", NodeWorld::Get().NodeID(data.hoveredNode)), 2, data.windowHeight - (++i * 12), 8, HAUNTINGWHITE);
            DrawText(TextFormat("\tptr: %p", data.hoveredNode), 2, data.windowHeight - (++i * 12), 8, HAUNTINGWHITE);
            DrawText("hovered node", 2, data.windowHeight - (++i * 12), 8, WHITE);
        }
        // Joint hover stats
        else if (!!data.hoveredWire)
        {
            const char* configurationName;
            switch (data.hoveredWire->elbowConfig)
            {
            case ElbowConfig::horizontal: configurationName = "horizontal"; break;
            case ElbowConfig::diagonalA: configurationName = "diagonal - input closer"; break;
            case ElbowConfig::vertical: configurationName = "vertical"; break;
            case ElbowConfig::diagonalB: configurationName = "diagonal - output closer"; break;
            default: configurationName = "ERROR"; break;
            }
            DrawText(TextFormat("\tconfiguration: %s", configurationName), 2, data.windowHeight - (++i * 12), 8, HAUNTINGWHITE);
            DrawText(TextFormat("\twire ptr: %p", data.hoveredWire), 2, data.windowHeight - (++i * 12), 8, HAUNTINGWHITE);
            DrawText("hovered wire-joint", 2, data.windowHeight - (++i * 12), 8, WHITE);
        }
    }
}

void Update_Erase(ProgramData& data)
{
    if (data.b_cursorMoved)
    {
        data.hoveredWire = nullptr;
        data.hoveredNode = NodeWorld::Get().FindNodeAtPos(data.cursorPos);
        if (!data.hoveredNode)
        {
            data.hoveredWire = NodeWorld::Get().FindWireAtPos(data.cursorPos);
            if (!data.hoveredWire)
                data.hoveredGroup = NodeWorld::Get().FindGroupAtPos(data.cursorPos);
        }
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if (!!data.hoveredNode)
        {
            // Special erase
            if ((IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) &&
                data.hoveredNode->IsSpecialErasable())
                NodeWorld::Get().BypassNode(data.hoveredNode);
            // Complex bipass
            else if (
                (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) &&
                (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) &&
                data.hoveredNode->IsComplexBipassable())
                NodeWorld::Get().BypassNode_Complex(data.hoveredNode);
            else
                NodeWorld::Get().DestroyNode(data.hoveredNode);
        }
        else if (!!data.hoveredWire)
            NodeWorld::Get().DestroyWire(data.hoveredWire);
        else if (!!data.hoveredGroup)
            NodeWorld::Get().DestroyGroup(data.hoveredGroup);

        data.hoveredNode = nullptr;
        data.hoveredWire = nullptr;
        data.hoveredGroup = nullptr;
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

    if (!!data.hoveredGroup)
    {
        IRect rec = data.hoveredGroup->GetLabelBounds();
        data.hoveredGroup->Highlight(MAGENTA);
        DrawLineEx({ (float)rec.x, (float)rec.y }, { (float)rec.x + (float)rec.h, (float)rec.Bottom() }, 3, DESTRUCTIVERED);
        DrawLineEx({ (float)rec.x, (float)rec.Bottom() }, { (float)rec.x + (float)rec.h, (float)rec.y }, 3, DESTRUCTIVERED);
    }

    NodeWorld::Get().DrawWires();

    if (!!data.hoveredWire)
    {
        data.hoveredWire->Draw(MAGENTA);
        DrawCross(data.cursorPos, DESTRUCTIVERED);
    }
    else if (!!data.hoveredNode)
    {
        Color color;
        if ((IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)))
        {
            if (data.hoveredNode->IsSpecialErasable())
                color = WIPBLUE;
            else if (data.hoveredNode->IsComplexBipassable() && (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)))
                color = VIOLET;
            else
                color = DESTRUCTIVERED;
        }
        else
            color = MAGENTA;

        for (Wire* wire : data.hoveredNode->GetWires())
        {
            wire->Draw(color);
        }
    }

    NodeWorld::Get().DrawNodes();

    if (!!data.hoveredNode)
    {
        data.hoveredNode->Draw(BLACK);
        DrawCross(data.hoveredNode->GetPosition(), DESTRUCTIVERED);

        if ((IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)))
        {
            const char* text;
            Color color;
            if (data.hoveredNode->IsSpecialErasable())
            {
                color = WIPBLUE;
                text = "Simple bipass";
            }
            else if (data.hoveredNode->IsComplexBipassable() && (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)))
            {
                color = VIOLET;
                text = "Complex bipass";
            }
            else
            {
                color = DESTRUCTIVERED;
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

void Update_Interact(ProgramData& data)
{
    data.hoveredNode = NodeWorld::Get().FindNodeAtPos(data.cursorPos);
    if (!!data.hoveredNode && !data.hoveredNode->IsOutputOnly())
        data.hoveredNode = nullptr;

    if (!!data.hoveredNode && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        data.hoveredNode->SetGate(data.hoveredNode->GetGate() == Gate::NOR ? Gate::OR : Gate::NOR);
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

void Update_Overlay_Button(ProgramData& data)
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        IRect rec = ProgramData::dropdownBounds[data.Button_DropdownActive()];
        if (data.CursorInUIBounds(rec))
        {
            rec.h = data.ButtonWidth();

            switch (data.Button_DropdownActive())
            {
            case 0: // Mode
            {
                for (Mode m : ProgramData::dropdownModeOrder)
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
                for (Gate g : ProgramData::dropdownGateOrder)
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
void Draw_Overlay_Button(ProgramData& data)
{
    NodeWorld::Get().DrawWires();
    NodeWorld::Get().DrawNodes();

    EndMode2D();

    IRect rec = data.dropdownBounds[data.Button_DropdownActive()];
    DrawRectangleIRect(rec, SPACEGRAY);
    rec.h = data.ButtonWidth();

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
                DrawText(ProgramData::GetModeTooltipName(m), data.ButtonWidth() + 4, data.ButtonWidth() + 1 + rec.y, 8, WHITE);
            }
            else
                color = DEADCABLE;
            data.DrawModeIcon(m, rec.xy, color);
            rec.y += data.ButtonWidth();
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
                DrawText(ProgramData::GetGateTooltipName(g), data.ButtonWidth() * 2 + 4, data.ButtonWidth() + 1 + rec.y, 8, WHITE);
            }
            else
                color = DEADCABLE;
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
                DrawRectangleIRect(rec, WIPBLUE);
                DrawRectangleIRect(ExpandIRect(rec, -2), color);
                const char* text;
                if (data.gatePick == Gate::LED)
                    text = TextFormat(data.deviceParameterTextFmt, Node::GetColorName(v));
                else
                    text = TextFormat(data.deviceParameterTextFmt, v);
                DrawText(text, data.ButtonWidth() * 3 + 4, data.ButtonWidth() + 1 + rec.y, 8, WHITE);
            }
            else
                DrawRectangleIRect(rec, color);
            rec.y += data.ButtonWidth();
        }
    }
    break;
    }
}

void Update_Overlay_Paste(ProgramData& data)
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            NodeWorld::Get().SpawnBlueprint(data.clipboard, data.cursorPos);
        data.ClearSelection();
        data.ClearOverlayMode();
    }
}
void Draw_Overlay_Paste(ProgramData& data)
{
    NodeWorld::Get().DrawWires();
    NodeWorld::Get().DrawNodes();

    data.clipboard->DrawSelectionPreview(data.cursorPos - IVec2(g_gridSize), ColorAlpha(LIFELESSNEBULA, 0.5f), HAUNTINGWHITE, LIFELESSNEBULA, DEADCABLE, data.pastePreviewLOD);
}

void Update_Menu_Select(ProgramData& data)
{
    constexpr int halfGrid = g_gridSize / 2;
    if (data.b_cursorMoved)
    {
        IVec2 pos(0, 16);
        int maxY = 0; // I know there must be a better algorithm, but this will at least be progress.
        data.BPSelect_Hovering() = nullptr;
        for (Blueprint* bp : NodeWorld::Get().GetBlueprints())
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
void Draw_Menu_Select(ProgramData& data)
{
    constexpr int halfGrid = g_gridSize / 2;
    IVec2 pos(0,16);
    int maxY = 0; // I know there must be a better algorithm, but this will at least be progress.
    ClearBackground({ 10,15,30, 255 });
    for (int y = 0; y < data.windowHeight; y += g_gridSize)
    {
        DrawLine(0, y, data.windowWidth, y, SPACEGRAY);
    }
    for (int x = 0; x < data.windowWidth; x += g_gridSize)
    {
        DrawLine(x, 0, x, data.windowHeight, SPACEGRAY);
    }
    DrawRectangle(0,0,data.windowWidth, 16, SPACEGRAY);
    DrawText("Blueprints (WIP)", 4,4, 8, WHITE);
    for (Blueprint* bp : NodeWorld::Get().GetBlueprints())
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
            background = WIPBLUE;
            foreground = INTERFERENCEGRAY;
            foregroundIO = HAUNTINGWHITE;
        }
        else [[likely]]
        {
            background = SPACEGRAY;
            foreground = DEADCABLE;
            foregroundIO = LIFELESSNEBULA;
        }

        bp->DrawSelectionPreview(pos, background, foreground, foregroundIO, ColorAlpha(foreground, 0.25f), data.blueprintLOD);
        DrawRectangleLines(rec.x, rec.y, rec.w, rec.h, foreground);

        pos += rec.width;
        maxY = std::max(maxY, rec.Bottom());
    }
    if (!!data.BPSelect_Hovering())
        data.DrawTooltipAtCursor(data.BPSelect_Hovering()->name.c_str(), WHITE);
}

int main()
{
    /******************************************
    *   Load textures, shaders, and meshes
    ******************************************/

    ProgramData data(1280, 720);

    NodeWorld::Get().Load("session.cg"); // Construct and load last session
    // Load blueprints
    {
        std::filesystem::path blueprints{ "blueprints" };
        std::filesystem::create_directories(blueprints);
        for (auto const& dir_entry : std::filesystem::directory_iterator{ blueprints })
        {
            std::string filename = dir_entry.path().string();
            if (filename.substr(filename.size() - 3, filename.size()) == ".bp")
            printf("Loading blueprint \"%s\"\n", filename.c_str());
            Blueprint bp;
            try
            {
                LoadBlueprint(filename.c_str(), bp);
                NodeWorld::Get().StoreBlueprint(&bp);
            }
            catch (std::length_error e)
            {
                printf("File corrupt. Encountered \"%s\" error\n", e.what());
            }
        }
    }

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
        if (!data.ModeIsMenu() && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            if ((data.CursorInUIBounds(ProgramData::ButtonBound_Mode()) ||
                 data.CursorInUIBounds(ProgramData::ButtonBound_Gate()) ||
                 data.CursorInUIBounds(ProgramData::ButtonBound_Parameter())) &&
                (data.mode == Mode::BUTTON ?
                    data.Button_DropdownActive() != (data.cursorUIPos.x / data.ButtonWidth()) :
                    true))
            {
                data.SetMode(Mode::BUTTON);
                goto EVAL; // Skip button sim this frame
            }
            else if (data.CursorInUIBounds(ProgramData::ButtonBound_Blueprints()))
            {
                data.SetMode(Mode::BP_SELECT);
                goto EVAL; // Skip button sim this frame
            }
            else if (data.CursorInUIBounds(ProgramData::ButtonBound_Clipboard()))
            {
                if (data.IsClipboardValid())
                    data.SetMode(Mode::PASTE);
                goto EVAL; // Skip button sim this frame
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
        case Mode::BUTTON:      Update_Overlay_Button(data);    break;
        case Mode::PASTE:       Update_Overlay_Paste(data);     break;
        case Mode::BP_SELECT:   Update_Menu_Select(data);       break;
        }

    EVAL:
        data.cursorPosPrev = data.cursorPos;
        if (NodeWorld::Get().IsOrderDirty())
            data.tickThisFrame = !(data.tickFrame = 0);
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
            case Mode::BUTTON:      Draw_Overlay_Button(data);  break;
            case Mode::PASTE:       Draw_Overlay_Paste(data);   break;

                // Menu
            case Mode::BP_SELECT:   Draw_Menu_Select(data);     break;
            }

            if (!data.ModeIsMenu())
            {
                EndMode2D();

                // UI

                // Buttons
                const IRect WithClipboard = ProgramData::buttonBounds[0] + Width(data.ButtonWidth()) * (_countof(ProgramData::buttonBounds) - 1);
                const IRect WithoutClipboard = WithClipboard - Width(ProgramData::ButtonBound_Clipboard());
                DrawRectangleIRect(data.IsClipboardValid() ? WithClipboard : WithoutClipboard, SPACEGRAY);
                DrawRectangleIRect(ProgramData::ButtonBound_Parameter(), data.ExtraParamColor());

                const IVec2 tooltipNameOffset = IVec2(data.ButtonWidth()) + IVec2(4,1);
                const IVec2 tooltipSeprOffset = tooltipNameOffset + Height(8 + 8 / 2);
                const IVec2 tooltipDescOffset = tooltipNameOffset + Height(8 + 8);

                // Mode
                if (data.CursorInUIBounds(ProgramData::ButtonBound_Mode()))
                {
                    DrawRectangleIRect(ProgramData::ButtonBound_Mode(), WIPBLUE);
                    // Tooltip
                    const char* name = data.GetModeTooltipName(data.baseMode);
                    DrawTextIV(name, ProgramData::ButtonBound_Mode().xy + tooltipNameOffset, 8, WHITE);
                    Width separatorWidth(MeasureText(name, 8));
                    DrawLineIV(ProgramData::ButtonBound_Mode().xy + tooltipSeprOffset, separatorWidth, WHITE); // Separator
                    DrawTextIV(data.GetModeTooltipDescription(data.baseMode), ProgramData::ButtonBound_Mode().xy + tooltipDescOffset, 8, WHITE);
                }
                // Gate
                else if (data.CursorInUIBounds(ProgramData::ButtonBound_Gate()))
                {
                    DrawRectangleIRect(ProgramData::ButtonBound_Gate(), WIPBLUE);
                    // Tooltip
                    const char* name = data.GetGateTooltipName(data.gatePick);
                    DrawTextIV(name, ProgramData::ButtonBound_Gate().xy + tooltipNameOffset, 8, WHITE);
                    Width separatorWidth(MeasureText(name, 8));
                    DrawLineIV(ProgramData::ButtonBound_Gate().xy + tooltipSeprOffset, separatorWidth, WHITE);
                    DrawTextIV(data.GetGateTooltipDescription(data.gatePick), ProgramData::ButtonBound_Gate().xy + tooltipDescOffset, 8, WHITE);
                }
                // Extra param
                else if (data.CursorInUIBounds(ProgramData::ButtonBound_Parameter()))
                {
                    DrawRectangleIRect(ProgramData::ButtonBound_Parameter(), WIPBLUE);
                    DrawRectangleIRect(ShrinkIRect(ProgramData::ButtonBound_Parameter(), 2), data.ExtraParamColor());
                    // Tooltip
                    const char* text;
                    if (data.gatePick == Gate::LED)
                        text = TextFormat(data.deviceParameterTextFmt, Node::GetColorName(data.storedExtraParam));
                    else
                        text = TextFormat(data.deviceParameterTextFmt, data.storedExtraParam);
                    DrawTextIV(text, ProgramData::ButtonBound_Parameter().xy + tooltipNameOffset, 8, WHITE);
                }
                // Blueprints
                else if (data.CursorInUIBounds(ProgramData::ButtonBound_Blueprints()))
                {
                    DrawRectangleIRect(ProgramData::ButtonBound_Blueprints(), WIPBLUE);
                    // Tooltip
                    DrawTextIV("Blueprints", ProgramData::ButtonBound_Blueprints().xy + tooltipNameOffset, 8, WHITE);
                }
                // Clipboard
                else if (data.CursorInUIBounds(ProgramData::ButtonBound_Clipboard()))
                {
                    DrawRectangleIRect(ProgramData::ButtonBound_Clipboard(), WIPBLUE);
                    // Tooltip
                    DrawTextIV("Clipboard (ctrl+c to copy, ctrl+v to paste)", ProgramData::ButtonBound_Clipboard().xy + tooltipNameOffset, 8, WHITE);
                    const IVec2 clipboardPreviewOffset = tooltipNameOffset + Height(data.ButtonWidth());
                    // Clipboard preview
                    if (data.IsClipboardValid())
                        data.clipboard->DrawSelectionPreview(
                            ProgramData::ButtonBound_Clipboard().xy + clipboardPreviewOffset,
                            SPACEGRAY,
                            DEADCABLE,
                            LIFELESSNEBULA,
                            ColorAlpha(DEADCABLE, 0.25f),
                            data.clipboardPreviewLOD);
                }

                data.DrawModeIcon(data.baseMode, ProgramData::ButtonBound_Mode().xy, WHITE);
                data.DrawGateIcon(data.gatePick, ProgramData::ButtonBound_Gate().xy, WHITE);
                
                for (IVec2 offset = IVec2(-1); offset.y <= 1; ++offset.y)
                {
                    for (offset.x = -1; offset.x <= 1; ++offset.x)
                    {
                        DrawTextIV(TextFormat("%i", data.storedExtraParam), ProgramData::ButtonBound_Parameter().xy + IVec2(2, 1) + offset, data.FontSize(), BLACK);
                    }
                }
                DrawTextIV(TextFormat("%i", data.storedExtraParam), ProgramData::ButtonBound_Parameter().xy + IVec2(2,1), data.FontSize(), WHITE);
                DrawTextureIV(data.GetBlueprintIcon(), ProgramData::ButtonBound_Blueprints().xy, WHITE);
                DrawTextureIV(data.GetClipboardIcon(), ProgramData::ButtonBound_Clipboard().xy, data.IsClipboardValid() ? WHITE : ColorAlpha(WHITE, 0.25f));
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
* -Parallel node drawing with pen when dragging - up to 8 at a time
* -Parallel node connection of selected nodes being shift-clicked (simmilar to "bridge" in 3D programs)
* -Shift-click pen should create intermediate nodes at wire elbow and connect with those instead of the parent node
* (Maybe? It might be more computationally expensive. Perhaps there could be an "intermediate/repeater node" which takes only one input and gives one output and has no eval?)
* -Multiple wire stacking on single node
* -Hotkeys for output-only gate state toggles (Like the Reason on-screen piano (Yes, this is different from interact mode))
*
* Optional
* -Multiple color pallets
* -Log files for debug/crashes
*/
