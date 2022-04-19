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
};

struct Tool_Pen : ModeHandler
{
    IVec2 dragStart;
    ElbowConfig currentWireElbowConfig;
    Node* previousWireStart;
    Node* currentWireStart;
    static constexpr size_t bulkNodeCount = 8;
    Node* nodesMadeByDragging[bulkNodeCount];

    Tool_Pen();
    ~Tool_Pen();
    void Update() override;
    void Draw() override;
};

struct Tool_Edit : ModeHandler
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
};

struct Tool_Erase : ModeHandler
{
    Tool_Erase();
    ~Tool_Erase();

    void Update() override;
    void Draw() override;
};

struct Tool_Interact : ModeHandler
{
    Tool_Interact();
    ~Tool_Interact();

    void Update() override;
    void Draw() override;
};

struct Overlay_Button : ModeHandler
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
};

struct Overlay_Paste : ModeHandler
{
    Overlay_Paste();
    ~Overlay_Paste();

    void Update() override;
    void Draw() override;
};

struct Menu_Icon : ModeHandler
{
    BlueprintIcon* object;
    IVec2 pos; // Width and height are fixed
    IRect sheetRec;
    BlueprintIconID_t iconID;
    uint8_t iconCount;
    int draggingIcon; // -1 for none/not dragging

    Menu_Icon();
    ~Menu_Icon();

    void Update() override;
    void Draw() override;
};

// todo
struct Menu_Select : ModeHandler
{
    int hovering; // -1 for none

    Menu_Select();
    ~Menu_Select();

    void Update() override;
    void Draw() override;
};