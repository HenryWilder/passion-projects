#pragma once
#include <vector>
#include <raylib.h>
#include <type_traits>

constexpr int g_gridSize = 8;

enum class Gate : char;
class Node;
struct Wire;
class Blueprint;

enum class Mode;
struct ModeHandler;
struct Tool;

struct ProgramData
{
    ProgramData(int windowWidth, int windowHeight);
    ~ProgramData();

    static constexpr IRect buttonBounds[] = {
        IRect(0, 0, 16), // Mode
        IRect(16, 0, 16), // Gate
        IRect(32, 0, 16), // Parameter
        IRect(48, 0, 16), // Blueprints
        IRect(64, 0, 16), // Clipboard
    };
    static constexpr IRect ButtonBound_Mode() { return buttonBounds[0]; }
    static constexpr IRect ButtonBound_Gate() { return buttonBounds[1]; }
    static constexpr IRect ButtonBound_Parameter() { return buttonBounds[2]; }
    static constexpr IRect ButtonBound_Blueprints() { return buttonBounds[3]; }
    static constexpr IRect ButtonBound_Clipboard() { return buttonBounds[4]; }

private:
    static Texture2D clipboardIcon;
    static Texture2D modeIcons;
    static Texture2D gateIcons16x;
    static Texture2D gateIcons32x;
public:

    int windowWidth;
    int windowHeight;

    ModeHandler* currentMode_object;
    Tool* basicMode_object;

    IVec2 cursorUIPos = IVec2::Zero();
    IVec2 cursorPos = IVec2::Zero();
    IVec2 cursorPosPrev = IVec2::Zero(); // For checking if there was movement
    bool b_cursorMoved = false;

    uint8_t framesPerTick = 6; // Number of frames in a tick
    uint8_t tickFrame = framesPerTick - 1; // Evaluate on 0
    bool tickThisFrame;

    Gate gatePick = (Gate)0;
    Gate lastGate = (Gate)0;
    uint8_t storedExtraParam = 0;

    Camera2D camera{ .offset{ 0,0 }, .target{ 0,0 }, .rotation{ 0 }, .zoom{ 1 } };
    bool b_twoDee = false;

    const char* deviceParameterTextFmt = "";

    Node* hoveredNode = nullptr;
    Wire* hoveredWire = nullptr;

    Blueprint* clipboard = nullptr;
    std::vector<Node*> selection;

public:

    static Texture2D GetClipboardIcon();
    static void DrawModeIcon(Mode mode, IVec2 pos, Color tint);
    static void DrawGateIcon16x(Gate gate, IVec2 pos, Color tint);
    static void DrawGateIcon32x(Gate gate, IVec2 pos, Color tint);

    static bool ModeIsMenu(Mode mode);
    bool ModeIsMenu() const;
    static bool ModeIsOverlay(Mode mode);
    bool ModeIsOverlay() const;
    bool ModeIsBasic() const;

    Mode GetCurrentMode(); // Mode enum of currentMode_object
    Mode GetBaseMode(); // Mode enum of basicMode_object

    template<class T> requires std::is_base_of_v<ModeHandler, T>
    T* CurrentModeAs()
    {
        T* modeObj = dynamic_cast<T*>(currentMode_object);
        _ASSERT_EXPR(!!modeObj, L"Type does not cast to specified mode");
        return modeObj;
    }
    template<class T> requires std::is_base_of_v<Tool, T>
    T* BaseModeAs()
    {
        T* modeObj = dynamic_cast<T*>(basicMode_object);
        _ASSERT_EXPR(!!modeObj, L"Type does not cast to specified mode");
        return modeObj;
    }
    ModeHandler* BaseModeAsPolymorphic();

    void SetMode(Mode newMode);
    void SetGate(Gate newGate);

    void ClearOverlayMode();

    static const char* GetModeTooltipName(Mode mode);
    static const char* GetModeTooltipDescription(Mode mode);
    static const char* GetGateTooltipName(Gate gate);
    static const char* GetGateTooltipDescription(Gate gate);

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

    // Inclusive
    bool CursorInRangeX(int xMin, int xMax) const;
    // Inclusive
    bool CursorInUIRangeX(int xMin, int xMax) const;

    // Inclusive
    bool CursorInRangeY(int yMin, int yMax) const;
    // Inclusive
    bool CursorInUIRangeY(int yMin, int yMax) const;

    bool CursorInBounds(IRect bounds) const;
    bool CursorInUIBounds(IRect uiBounds) const;

    template<int gridSize = g_gridSize> void DrawGrid() const
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
    void DrawGrid(int gridSize) const;

    IRect GetSelectionBounds(const std::vector<Node*>& vec) const;
    IRect GetSelectionBounds() const;

    static Color ResistanceBandColor(uint8_t index);
    Color ExtraParamColor() const;

    void SetMode2D(bool value);

    void DrawTooltipAtCursor(const char* text, Color color);
};
extern Texture2D clipboardIcon;
extern Texture2D modeIcons;
extern Texture2D gateIcons16x;
extern Texture2D gateIcons32x;