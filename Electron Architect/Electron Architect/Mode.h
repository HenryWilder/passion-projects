#pragma once
#include <raylib.h>
#include "IVec.h"

struct Window;
enum class ElbowConfig : uint8_t;
class Node;
struct Wire;
class Group;
struct GroupCorner;


enum class ModeType
{
    Basic,
    Overlay,
    Menu,
};
enum class Mode
{
    // Basic
    PEN,
    EDIT,
    ERASE,
    INTERACT,

    // Overlay
    BUTTON,
    PASTE,

    // Menu
    BP_SELECT,
};

const std::string GateName(Gate gate);
const std::string ModeName(Mode mode);
const std::string StateName(bool state);
const std::string ElbowConfigName(ElbowConfig elbow);

constexpr ModeType TypeOfMode(Mode mode)
{
    switch (mode)
    {
    default:
        _ASSERT_EXPR(false, L"Missing mode type specialization");
        return ModeType::Basic;

    case Mode::PEN:
    case Mode::EDIT:
    case Mode::ERASE:
    case Mode::INTERACT:
        return ModeType::Basic;

    case Mode::BUTTON:
    case Mode::PASTE:
        return ModeType::Overlay;

    case Mode::BP_SELECT:
        return ModeType::Menu;
    }
}

struct Tool
{
    virtual ModeType GetModeType() const = 0;
    virtual Mode GetMode() const = 0;

    virtual void Update(Window& window) = 0;
    virtual void Draw(Window& window) = 0;
    virtual void DrawProperties(Window& window) = 0;
};

struct PenTool : public Tool
{
    PenTool();
    ~PenTool();

    ModeType GetModeType() const final;
    Mode GetMode() const final;

    void Update(Window& window) final;
    void Draw(Window& window) final;
    void DrawProperties(Window& window) final;

    IVec2 dragStart;
    bool dragging;
    enum class DragType : bool { drag_create = false, drag_connect = true } dragMode;
    Node* previousWireStart;
    Node* currentWireStart;
};

struct EditTool : public Tool
{
    EditTool();
    ~EditTool();

    ModeType GetModeType() const final;
    Mode GetMode() const final;

    void Update(Window& window) final;
    void Draw(Window& window) final;
    void DrawProperties(Window& window) final;

    IVec2 fallbackPos;
    bool selectionWIP;
    IVec2 selectionStart;
    bool draggingGroup;
    bool draggingGroupCorner;
    bool renamingGroup;
    GroupCorner groupCorner;
    Node* hoveringMergable;
    Node* nodeBeingDragged;
    Wire* wireBeingDragged;
};

struct EraseTool : public Tool
{
    EraseTool();
    ~EraseTool();

    ModeType GetModeType() const final;
    Mode GetMode() const final;

    void Update(Window& window) final;
    void Draw(Window& window) final;
    void DrawProperties(Window& window) final;
};

struct InteractTool : public Tool
{
    InteractTool();
    ~InteractTool();

    ModeType GetModeType() const final;
    Mode GetMode() const final;

    void Update(Window& window) final;
    void Draw(Window& window) final;
    void DrawProperties(Window& window) final;
};

struct PasteOverlay : public Tool
{
    PasteOverlay();
    ~PasteOverlay();

    ModeType GetModeType() const final;
    Mode GetMode() const final;

    void Update(Window& window) final;
    void Draw(Window& window) final;
    void DrawProperties(Window& window) final;
};

struct BlueprintMenu : public Tool
{
    BlueprintMenu();
    ~BlueprintMenu();

    ModeType GetModeType() const final;
    Mode GetMode() const final;

    void Update(Window& window) final;
    void Draw(Window& window) final;
    void DrawProperties(Window& window) final;

    Blueprint* hovering;
};
