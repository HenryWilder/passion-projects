#pragma once
#include <raylib.h>
#include <vector>
#include "IVec.h"
#include "UIColors.h"
#include "Buttons.h"

class Group;
class Graph;
struct Tab;
struct Tool;
enum class Mode;
enum class ModeType;

// Reusable address so clipboard doesn't have to delete
extern Blueprint g_clipboardBP;

void DrawTextShadowedIV(const std::string& text, IVec2 pos, int fontSize, Color color, Color shadow);

enum class LogType
{
    // Just FYI
    info = 0,
    // Upcoming might not be successful
    attempt = 1,
    // Successful
    success = 2,
    // Unsuccessful, can still continue
    warning = 3,
    // Cannot continue
    error = 4,
};

struct Window
{
    Window();
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
    const Tab& CurrentTab() const;

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
    ElbowConfig currentWireElbowConfig = ElbowConfig::horizontal;

    std::string deviceParameterTextFmt;

    Node* hoveredNode = nullptr;
    Wire* hoveredWire = nullptr;
    Group* hoveredGroup = nullptr;

    Blueprint* clipboard = nullptr;

    int propertyNumber; // For the "PushProperty" functions
    IRect propertiesPaneRec;

    double timeOfLastLog = 0.0;
    LogType minLogLevel = LogType::info; // Allows user to surpress logs below this level
    std::string consoleOutput[6];
    IRect consolePaneRec;

    IconButton modeButtons[4];
    const IconButton* ButtonFromMode(Mode mode) const;
    IconButton gateButtons[9];
    const IconButton* ButtonFromGate(Gate gate) const;
    ColorButton paramButtons[10];
    const ColorButton* ButtonFromParameter(uint8_t param) const;
    IconButton blueprintsButton;
    IconButton clipboardButton;
    TextButton toolPaneSizeButton;
    TextButton propertiesToggleButton;
    TextButton consoleToggleButton;
    Button* const allButtons[28];
    IRect toolPaneRec;
    bool toolPaneSizeState;
    bool consoleOn;
    bool propertiesOn;

private:

    Tool* base;
    Tool* overlay; // Menu modes are stored in here

public:

    int FontSize() const;
    IVec2 FontPadding() const;

    void DrawUIIcon(Texture2D iconSheet, IVec2 iconColRow, IVec2 pos, Color tint) const;

    Mode GetBaseMode();
    Mode GetMode();
    ModeType GetModeType();

    void SetMode(Mode newMode);
    void SetGate(Gate newGate);

    void ClearOverlayMode();

    void IncrementTick();

    void UpdateCursorPos();

    // Zoom/pan
    void UpdateCamera();

    void UpdateSize();

    void CopySelectionToClipboard();

    void MakeGroupFromSelection();

    bool IsSelectionRectValid() const;

    void SaveBlueprint();

    void DrawClipboardPreview() const;
    bool IsClipboardValid() const;
    void ClearClipboard();
    bool SelectionExists() const;
    void ClearSelection();
    void DestroySelection();

    void CheckHotkeys();

    IVec2 GetCursorDelta() const;

    bool CursorInBounds(IRect bounds) const;
    bool CursorInUIBounds(IRect uiBounds) const;

    void DrawGrid(int gridSize = g_gridSize) const;

    IRect GetSelectionBounds(const std::vector<Node*>& vec) const;
    IRect GetSelectionBounds() const;

    static Color ResistanceBandColor(uint8_t index);
    Color ExtraParamColor() const;

    void DrawTooltipAtCursor(const std::string& text, Color color);

    void DrawTooltipAtCursor_Shadowed(const std::string& text, Color color);

    void ReloadPanes();
    void SaveConfig() const;
    void ReloadConfig();

    void UpdateTool();
    void DrawTool();

    void ReloadToolPane();
    void ToggleToolPaneSize();
    void DrawToolPane();

    void ToggleProperties();
    void ToggleConsole();

    void CleanConsolePane();
    void DrawConsoleOutput();
    void Log(LogType type, const std::string& output);
    void ClearLog();

    void CleanPropertiesPane();
    void PushProperty(const std::string& name, const std::string& value);
    inline void PushProperty_int(const std::string& name, int value)
    {
        PushProperty(name, TextFormat("%i", value));
    }
    inline void PushProperty_uint(const std::string& name, size_t value)
    {
        PushProperty(name, TextFormat("%u", value));
    }
    inline void PushProperty_ptr(const std::string& name, void* value)
    {
        PushProperty(name, TextFormat("0x%p", value));
    }
    inline void PushProperty_str(const std::string& name, const std::string& value)
    {
        PushProperty(name, value);
    }
    void PushProperty_longStr(const std::string& name, const std::string& value);
    inline void PushProperty_bool(const std::string& name, bool value)
    {
        PushProperty(name, value ? "true" : "false");
    }
    void PushPropertyTitle(const std::string& title);
    void PushPropertySubtitle(const std::string& title, Color color = UIColor(UIColorID::UI_COLOR_FOREGROUND));
    void PushPropertySpacer();
    void PushPropertySection_Node(const std::string& name, Node* value);
    void PushPropertySection_Wire(const std::string& name, Wire* value);
    void PushPropertySection_Selection(const std::string& name, const std::vector<Node*>& value);
    void PushPropertySection_Group(const std::string& name, Group* value);
    void DrawToolProperties();
};
