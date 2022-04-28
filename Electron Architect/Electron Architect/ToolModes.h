#pragma once
#include "IVec.h"

#include "Gate_Enum.h"
class Node;

enum class ElbowConfig : uint8_t;
struct Wire;

class Group;

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
    virtual void Update() = 0;
    virtual void Draw() = 0;
    constexpr virtual Mode GetMode() = 0;
    virtual ModeHandler* As(Mode mode) = 0;
};

// Basic mode/tool
struct Tool : ModeHandler
{
    virtual void Update() = 0;
    virtual void Draw() = 0;
    constexpr virtual Mode GetMode() = 0;
    virtual Tool* As(Mode mode) = 0;
};
// Overlay mode (independent of tool)
struct Overlay : ModeHandler
{
    virtual void Update() = 0;
    virtual void Draw() = 0;
    constexpr virtual Mode GetMode() = 0;
    virtual Overlay* As(Mode mode) = 0;
};
// Menu mode (independent of tool (but replaces overlay) + overrides normal rendering)
struct Menu : Overlay
{
    virtual void Update() = 0;
    virtual void Draw() = 0;
    constexpr virtual Mode GetMode() = 0;
    virtual Menu* As(Mode mode) = 0;
};

struct Tool_Pen : public Tool
{
    IVec2 dragStart;
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
    void Update() final;

private:
    void DrawCurrentWire();
    void DrawHoveredWire();
    void DrawHoveredNodeWires();
    void DrawHoveredNode();
public:
    void Draw() final;

    constexpr Mode GetMode() final { return Mode::PEN; }

    Tool_Pen* As(Mode mode) final;
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

    void Update() final;
    void Draw() final;
    constexpr Mode GetMode() final { return Mode::EDIT; }
    Tool_Edit* As(Mode mode) final;
};

struct Tool_Erase : public Tool
{
    Tool_Erase();
    ~Tool_Erase();

    void Update() final;
    void Draw() final;
    constexpr Mode GetMode() final { return Mode::ERASE; }
    Tool_Erase* As(Mode mode) final;
};

struct Tool_Interact : public Tool
{
    Tool_Interact();
    ~Tool_Interact();

    void Update() final;
    void Draw() final;
    constexpr Mode GetMode() final { return Mode::INTERACT; }
    Tool_Interact* As(Mode mode) final;
};


struct Overlay_Button : public ModeHandler
{
    static const Mode dropdownModeOrder[];
    static const Gate dropdownGateOrder[];
    static const IRect dropdownBounds[];

    int dropdownActive; // Set by constructor

    Overlay_Button();
    ~Overlay_Button();

    void Update() final;
    void Draw() final;
    constexpr Mode GetMode() final { return Mode::BUTTON; }
    Overlay_Button* As(Mode mode) final;
};

struct Overlay_Paste : public ModeHandler
{
    Overlay_Paste();
    ~Overlay_Paste();

    void Update() final;
    void Draw() final;
    constexpr Mode GetMode() final { return Mode::PASTE; }
    Overlay_Paste* As(Mode mode) final;
};

struct BlueprintIcon;
struct Menu_Icon : public ModeHandler
{
    IVec2 pos; // Width and height are fixed
    IRect sheetRec;
    uint16_t iconID; // BlueprintIconID_t
    uint8_t iconCount;
    int draggingIcon = -1; // -1 for none/not dragging
    BlueprintIcon* object; // Set by constructor

    Menu_Icon();
    ~Menu_Icon();

    void SaveBlueprint();

    void Update() final;
    void Draw() final;
    constexpr Mode GetMode() final { return Mode::BP_ICON; }
    Menu_Icon* As(Mode mode) final;
};

// Todo: finish Menu_Select
struct Menu_Select : public ModeHandler
{
    int hovering = -1; // -1 for none

    Menu_Select();
    ~Menu_Select();

    void Update() final;
    void Draw() final;
    constexpr Mode GetMode() final { return Mode::BP_SELECT; }
    Menu_Select* As(Mode mode) final;
};