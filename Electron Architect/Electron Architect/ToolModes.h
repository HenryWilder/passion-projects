#pragma once
#include "IVec.h"

#include "Gate_Enum.h"
class Node;

enum class ElbowConfig : uint8_t;
struct Wire;

struct ProgramData;

enum class Mode
{
    PEN,
    EDIT,
    ERASE,
    INTERACT,

    BUTTON,
    PASTE,

    BP_ICON,
    BP_SELECT,
};

// Polymorphic
struct ModeHandler
{
    static ProgramData& data;
    virtual void Update() = 0;
    virtual void Draw() = 0;
    constexpr virtual Mode GetMode() = 0;
};

// Basic mode/tool
struct Tool : ModeHandler
{
    virtual void Update() = 0;
    virtual void Draw() = 0;
    constexpr virtual Mode GetMode() = 0;
};
// Overlay mode (independent of tool)
struct Overlay : ModeHandler
{
    virtual void Update() = 0;
    virtual void Draw() = 0;
    constexpr virtual Mode GetMode() = 0;
};
// Menu mode (independent of tool (but replaces overlay) + overrides normal rendering)
struct Menu : Overlay
{
    virtual void Update() = 0;
    virtual void Draw() = 0;
    constexpr virtual Mode GetMode() = 0;
};

struct Tool_Pen : public Tool
{
    IVec2 dragStart{};
    ElbowConfig currentWireElbowConfig{};
    Node* previousWireStart = nullptr;
    Node* currentWireStart = nullptr;
    static constexpr size_t bulkNodeCount = 8;
    Node* nodesMadeByDragging[bulkNodeCount]{};
    bool bulkNodesBeingMade = false;

    Tool_Pen();
    ~Tool_Pen();
private:
    void CreateBulkNodes();
    void UpdateBulkNodes();
    void CancelBulkNodes();

    Node* CreateNode();
    void BisectWireWithNode(Node* node);
    void FinishWire(Node* wireEnd);

    void UpdateHover();

    void CycleElbow();
    void CancelWire();
public:
    void Update() override;

private:
    void DrawCurrentWire();
    void DrawHoveredWire();
    void DrawHoveredNodeWires();
    void DrawHoveredNode();
public:
    void Draw() override;

    constexpr Mode GetMode() override { return Mode::PEN; }
};

struct Tool_Edit : public Tool
{
    IVec2 fallbackPos{0};
    bool selectionWIP = false;
    IVec2 selectionStart{0};
    IRect selectionRec{0};
    bool draggingGroup = false;
    Group* hoveredGroup = nullptr;
    Node* hoveringMergable = nullptr;
    Node* nodeBeingDragged = nullptr;
    Wire* wireBeingDragged = nullptr;

    Tool_Edit();
    ~Tool_Edit();

    void MakeGroupFromSelection();
    bool IsSelectionRectValid() const;
    void ClearSelection();

    void TryGrouping();

    void Update() override;
    void Draw() override;
    constexpr Mode GetMode() override { return Mode::EDIT; }
};

struct Tool_Erase : public Tool
{
    Tool_Erase();
    ~Tool_Erase();

    void Update() override;
    void Draw() override;
    constexpr Mode GetMode() override { return Mode::ERASE; }
};

struct Tool_Interact : public Tool
{
    Tool_Interact();
    ~Tool_Interact();

    void Update() override;
    void Draw() override;
    constexpr Mode GetMode() override { return Mode::INTERACT; }
};


struct Overlay_Button : public ModeHandler
{
    static constexpr Mode dropdownModeOrder[] = {
        Mode::PEN,
        Mode::EDIT,
        Mode::ERASE,
        Mode::INTERACT,
    };
    static constexpr Gate dropdownGateOrder[] = {
        g_GateOrder[0],
        g_GateOrder[1],
        g_GateOrder[2],
        g_GateOrder[3],

        g_GateOrder[4],
        g_GateOrder[5],
        g_GateOrder[6],
        g_GateOrder[7],
    };
    static constexpr IRect dropdownBounds[] = {
        IRect( 0, 16, 16, 16 * (_countof(dropdownModeOrder) - 1)), // Mode
        IRect(16, 16, 16, 16 * (_countof(dropdownGateOrder) - 1)), // Gate
        IRect(32, 16, 16, 16 * (_countof(Node::g_resistanceBands) - 1)), // Parameter
    };

    int dropdownActive; // Set by constructor

    Overlay_Button();
    ~Overlay_Button();

    void Update() override;
    void Draw() override;
    constexpr Mode GetMode() override { return Mode::BUTTON; }
};

struct Overlay_Paste : public ModeHandler
{
    Overlay_Paste();
    ~Overlay_Paste();

    void Update() override;
    void Draw() override;
    constexpr Mode GetMode() override { return Mode::PASTE; }
};

struct Menu_Icon : public ModeHandler
{
    IVec2 pos; // Width and height are fixed
    IRect sheetRec;
    BlueprintIconID_t iconID;
    uint8_t iconCount;
    int draggingIcon = -1; // -1 for none/not dragging
    BlueprintIcon* object; // Set by constructor

    Menu_Icon();
    ~Menu_Icon();

    void SaveBlueprint();

    void Update() override;
    void Draw() override;
    constexpr Mode GetMode() override { return Mode::BP_ICON; }
};

// todo
struct Menu_Select : public ModeHandler
{
    int hovering = -1; // -1 for none

    Menu_Select();
    ~Menu_Select();

    void Update() override;
    void Draw() override;
    constexpr Mode GetMode() override { return Mode::BP_SELECT; }
};