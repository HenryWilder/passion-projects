#pragma once
#include <vector>
#include <raylib.h>
#include <type_traits>
#include "HUtility.h"
#include "IVec.h"

#include "Gate_Enum.h"
class Node;
struct Wire;
struct Blueprint;

enum class Mode;
struct ModeHandler;
struct Tool;

const enum class ButtonID : uint8_t
{
    Mode,
    Gate,
    Parameter,
    Blueprints,
    Clipboard,

    count // Not assigned to an actual button
};

namespace data
{
    void Init(int windowWidth, int windowHeight);
    void DeInit();

    constexpr IRect ButtonBounds(int index) { return IRect(16 * (int)index, 0, 16); }
    constexpr IRect ButtonBounds(ButtonID index) { return IRect(16 * (int)index, 0, 16); }

#pragma region Members

    namespace iconTextures
    {
        extern Texture2D clipboardIcon;
        extern Texture2D modeIcons;
        extern Texture2D gateIcons16x;
        extern Texture2D gateIcons32x;
    }

    extern int windowWidth;
    extern int windowHeight;

    extern ModeHandler* currentMode_object;
    extern Tool* basicMode_object;

    extern IVec2 cursorUIPos;
    extern IVec2 cursorPos;
    extern IVec2 cursorPosPrev; // For checking if there was movement
    extern bool b_cursorMoved;

    extern uint8_t framesPerTick; // Number of frames in a tick
    extern uint8_t tickFrame; // Evaluate on 0
    extern bool tickThisFrame;

    extern Gate gatePick;
    extern Gate lastGate;
    extern uint8_t storedExtraParam;

    extern Camera2D camera;
    extern bool b_twoDee;

    extern const char* deviceParameterTextFmt;

    extern Node* hoveredNode;
    extern Wire* hoveredWire;

    extern Blueprint* clipboard;
    extern std::vector<Node*> selection;

#pragma endregion

    Texture2D GetClipboardIcon();
    void DrawModeIcon(Mode mode, IVec2 pos, Color tint);
    void DrawGateIcon16x(Gate gate, IVec2 pos, Color tint);
    void DrawGateIcon32x(Gate gate, IVec2 pos, Color tint);

    bool ModeIsMenu(Mode mode);
    bool ModeIsMenu();
    bool ModeIsOverlay(Mode mode);
    bool ModeIsOverlay();
    bool ModeIsBasic();

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

    const char* GetModeTooltipName(Mode mode);
    const char* GetModeTooltipDescription(Mode mode);
    const char* GetGateTooltipName(Gate gate);
    const char* GetGateTooltipDescription(Gate gate);

    void IncrementTick();

    void UpdateCursorPos();

    // Zoom/pan
    void UpdateCamera();

    void CopySelectionToClipboard();

    bool IsClipboardValid();
    void ClearClipboard();
    bool SelectionExists();
    void DestroySelection();

    void CheckHotkeys();

    IVec2 GetCursorDelta();

    // Inclusive
    bool CursorInRangeX(int xMin, int xMax);
    // Inclusive
    bool CursorInUIRangeX(int xMin, int xMax);

    // Inclusive
    bool CursorInRangeY(int yMin, int yMax);
    // Inclusive
    bool CursorInUIRangeY(int yMin, int yMax);

    bool CursorInBounds(IRect bounds);
    bool CursorInUIBounds(IRect uiBounds);

    template<int gridSize = g_gridSize>
    void DrawGrid()
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
    void DrawGrid(int gridSize);

    IRect GetSelectionBounds(const std::vector<Node*>& vec);
    IRect GetSelectionBounds();

    Color ResistanceBandColor(uint8_t index);
    Color ExtraParamColor();

    void SetMode2D(bool value);

    void DrawTooltipAtCursor(const char* text, Color color);

    void SaveClipboardBlueprint();
};
