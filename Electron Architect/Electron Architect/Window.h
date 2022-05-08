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

    const char* deviceParameterTextFmt = "";

    Node* hoveredNode = nullptr;
    Wire* hoveredWire = nullptr;
    Group* hoveredGroup = nullptr;

    Blueprint* clipboard = nullptr;

    int propertyNumber; // For the "PushProperty" functions
    IRect propertiesPaneRec;

    std::string consoleOutput[6];
    IRect consolePaneRec;

    std::vector<IconButton> modeButtons;
    const IconButton* ButtonFromMode(Mode mode) const;
    std::vector<IconButton> gateButtons;
    const IconButton* ButtonFromGate(Gate gate) const;
    IconButton blueprintsButton;
    IconButton clipboardButton;

private:

    Tool* base;
    Tool* overlay; // Menu modes are stored in here

public:

    int FontSize() const;

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
        if (!tabs.empty())
        {
            IVec2 extents((int)((float)windowWidth / CurrentTab().camera.zoom), (int)((float)windowHeight / CurrentTab().camera.zoom));
            IRect bounds(IVec2(CurrentTab().camera.target), extents);

            constexpr float gridSpaceFrac = 1.0f / gridSize;
            // "If the fraction of a screen pixel in a grid space equals or exceeds the fraction of a screen pixel in a world pixel"
            if (CurrentTab().camera.zoom <= gridSpaceFrac)
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

    void CleanConsolePane();
    void DrawConsoleOutput();
    // Push to the console and log file
    void Log(const char* output);
    void LogMessage(const char* output);
    void LogAttempt(const char* output);
    void LogSuccess(const char* output);
    void LogError(const char* output);
    void ClearLog();

    void CleanPropertiesPane();
    void PushProperty(const char* name, const char* value);
    void PushProperty_int(const char* name, int value);
    void PushProperty_uint(const char* name, size_t value);
    void PushProperty_ptr(const char* name, void* value);
    void PushProperty_str(const char* name, const std::string& value);
    void PushProperty_longStr(const char* name, const char* value);
    void PushProperty_bool(const char* name, bool value);
    void PushPropertyTitle(const char* title);
    void PushPropertySubtitle(const char* title, Color color = UIColor(UIColorID::UI_COLOR_FOREGROUND));
    void PushPropertySpacer();
    void PushPropertySection_Node(const char* name, Node* value);
    void PushPropertySection_Wire(const char* name, Wire* value);
    void PushPropertySection_Selection(const char* name, const std::vector<Node*>& value);
    void PushPropertySection_Group(const char* name, Group* value);
    void DrawToolProperties();
};
