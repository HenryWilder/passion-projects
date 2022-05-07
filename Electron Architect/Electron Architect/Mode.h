#pragma once
#include <raylib.h>
#include "IVec.h"

struct Window;
enum class ElbowConfig : uint8_t;
class Node;
struct Wire;
struct Group;
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

    virtual void Update(Window& data) = 0;
    virtual void Draw(Window& data) = 0;
};

struct PenTool : public Tool
{
    PenTool();
    ~PenTool();

    ModeType GetModeType() const final;
    Mode GetMode() const final;

    void Update(Window& data) final;
    void Draw(Window& data) final;

    IVec2 dragStart;
    ElbowConfig currentWireElbowConfig;
    Node* previousWireStart;
    Node* currentWireStart;
};

struct EditTool : public Tool
{
    EditTool();
    ~EditTool();

    ModeType GetModeType() const final;
    Mode GetMode() const final;

    void Update(Window& data) final;
    void Draw(Window& data) final;

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
};

struct EraseTool : public Tool
{
    EraseTool();
    ~EraseTool();

    ModeType GetModeType() const final;
    Mode GetMode() const final;

    void Update(Window& data) final;
    void Draw(Window& data) final;
};

struct InteractTool : public Tool
{
    InteractTool();
    ~InteractTool();

    ModeType GetModeType() const final;
    Mode GetMode() const final;

    void Update(Window& data) final;
    void Draw(Window& data) final;
};

struct ButtonOverlay : public Tool
{
    ButtonOverlay();
    ~ButtonOverlay();

    ModeType GetModeType() const final;
    Mode GetMode() const final;

    void Update(Window& data) final;
    void Draw(Window& data) final;

    int dropdownActive;
};

struct PasteOverlay : public Tool
{
    PasteOverlay();
    ~PasteOverlay();

    ModeType GetModeType() const final;
    Mode GetMode() const final;

    void Update(Window& data) final;
    void Draw(Window& data) final;
};

struct BlueprintMenu : public Tool
{
    BlueprintMenu();
    ~BlueprintMenu();

    ModeType GetModeType() const final;
    Mode GetMode() const final;

    void Update(Window& data) final;
    void Draw(Window& data) final;

    Blueprint* hovering;
};

Tool* NewToolOfMode(Mode mode)
{
    switch (mode)
    {
        ASSERT_SPECIALIZATION(L"Mode generation");

    case Mode::PEN:         return new PenTool;         break;
    case Mode::EDIT:        return new EditTool;        break;
    case Mode::ERASE:       return new EraseTool;       break;
    case Mode::INTERACT:    return new InteractTool;    break;

    case Mode::BUTTON:      return new ButtonOverlay;   break;
    case Mode::PASTE:       return new PasteOverlay;    break;

    case Mode::BP_SELECT:   return new BlueprintMenu;   break;
    }
}
