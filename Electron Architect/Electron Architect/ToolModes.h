#pragma once
#include "IVec.h"

enum class Gate : char
{
    OR = '|',
    AND = '&',
    NOR = '!',
    XOR = '^',

    RESISTOR = '~',
    CAPACITOR = '=',
    LED = '@',
    DELAY = ';',
};
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

    GATE,
    BUTTON,
    PASTE,

    BP_ICON,
    BP_SELECT,
};

struct ModeHandler
{
    static ProgramData& data;
    virtual void Update() = 0;
    virtual void Draw() = 0;
    constexpr virtual Mode GetMode() = 0;
};

// A mode specific mode
struct Tool : ModeHandler {};
struct Overlay : ModeHandler {};
struct Menu : Overlay {};

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
    IVec2 fallbackPos;
    bool selectionWIP;
    IVec2 selectionStart;
    IRect selectionRec;
    bool draggingGroup;
    Group* hoveredGroup;
    Node* hoveringMergable;
    Node* nodeBeingDragged;
    Wire* wireBeingDragged;

    Tool_Edit();
    ~Tool_Edit();

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
        Gate::OR,
        Gate::AND,
        Gate::NOR,
        Gate::XOR,

        Gate::RESISTOR,
        Gate::CAPACITOR,
        Gate::LED,
        Gate::DELAY,
    };
    static constexpr IRect dropdownBounds[] = {
        IRect( 0, 16, 16, 16 * (_countof(dropdownModeOrder) - 1)), // Mode
        IRect(16, 16, 16, 16 * (_countof(dropdownGateOrder) - 1)), // Gate
        IRect(32, 16, 16, 16 * (_countof(Node::g_resistanceBands) - 1)), // Parameter
    };

    int dropdownActive;

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
    int draggingIcon; // -1 for none/not dragging
    BlueprintIcon* object;

    Menu_Icon();
    ~Menu_Icon();

    void Update() override;
    void Draw() override;
    constexpr Mode GetMode() override { return Mode::BP_ICON; }
};

// todo
struct Menu_Select : public ModeHandler
{
    int hovering; // -1 for none

    Menu_Select();
    ~Menu_Select();

    void Update() override;
    void Draw() override;
    constexpr Mode GetMode() override { return Mode::BP_SELECT; }
};