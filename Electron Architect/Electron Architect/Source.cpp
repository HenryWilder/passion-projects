#include <raylib.h>
#include <fstream>
#include <filesystem>
#include <functional>
#include "HUtility.h"
#include "IVec.h"
#include "Node.h"
#include "Wire.h"
#include "Blueprint.h"
#include "Group.h"
#include "Tab.h"
#include "Buttons.h"
#include "UIColors.h"
#include "Mode.h"
#include "Window.h"

Blueprint g_clipboardBP; // Reusable address so clipboard doesn't have to delete

int main()
{
    /******************************************
    *   Load textures, shaders, and meshes
    ******************************************/

    Window window(1280, 720);

    window.CurrentTab().Load("session.cg"); // Construct and load last session
    // Load blueprints
    {
        std::filesystem::path blueprints{ "blueprints" };
        std::filesystem::create_directories(blueprints);
        for (auto const& dir_entry : std::filesystem::directory_iterator{ blueprints })
        {
            std::string filename = dir_entry.path().string();
            if (filename.substr(filename.size() - 3, filename.size()) == ".bp")
            printf("Loading blueprint \"%s\"\n", filename.c_str());
            Blueprint bp;
            try
            {
                LoadBlueprint(filename.c_str(), bp);
                window.CurrentTab().StoreBlueprint(&bp);
            }
            catch (std::length_error e)
            {
                printf("File corrupt. Encountered \"%s\" error\n", e.what());
            }
        }
    }

    std::vector<const Button&> allButtons;
    allButtons.reserve();

    while (!WindowShouldClose())
    {
        /******************************************
        *   Simulate frame and update variables
        ******************************************/

        window.IncrementTick();

        window.UpdateCursorPos();

        window.UpdateCamera();

        window.CheckHotkeys();

        // UI buttons
        if (window.GetModeType() == ModeType::Basic && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            for (const Button& b : window.modeButtons)
            {
                if (window.CursorInUIBounds(b.Bounds()))
                {
                    b.OnClick();
                    goto EVAL; // Skip button sim this frame
                }
            }
            for (const Button& b : window.gateButtons)
            {
                if (window.CursorInUIBounds(b.Bounds()))
                {
                    b.OnClick();
                    goto EVAL; // Skip button sim this frame
                }
            }
            if (window.CursorInUIBounds(window.blueprintsButton.Bounds()))
            {
                window.blueprintsButton.OnClick();
                goto EVAL; // Skip button sim this frame
            }
            if (window.CursorInUIBounds(window.clipboardButton.Bounds()))
            {
                window.clipboardButton.OnClick();
                goto EVAL; // Skip button sim this frame
            }
        }

        // Input
        window.UpdateTool();

    EVAL:
        window.cursorPosPrev = window.cursorPos;
        if (window.CurrentTab().IsOrderDirty())
            window.tickThisFrame = !(window.tickFrame = 0);
        if (window.tickThisFrame)
            window.CurrentTab().Evaluate();

        /******************************************
        *   Draw the frame
        ******************************************/

        BeginDrawing(); {

            ClearBackground(UIColor(UIColorID::UI_COLOR_BACKGROUND));

            if (window.GetModeType() != ModeType::Menu)
            {
                BeginMode2D(window.camera);

                window.DrawGrid();

                window.CurrentTab().DrawGroups();
            }

            // Draw
            window.DrawTool();

            if (window.GetModeType() != ModeType::Menu)
            {
                EndMode2D(); // Just in case

                // UI

                // Sidebars
                for ()

                DrawRectangleIRect(IRect(Button::g_width * 2, window.windowHeight), UIColor(UIColorID::UI_COLOR_BACKGROUND1));
                //DrawLine(Button::g_width * 2 + 1, 0, Button::g_width * 2 + 1, window.windowHeight, UIColor(UIColorID::UI_COLOR_BACKGROUND2));
                //DrawRectangleIRect(Window::ButtonBound_Parameter(), window.ExtraParamColor());

                const IVec2 tooltipNameOffset = IVec2(Button::g_width) + IVec2(window.FontSize() / 2, window.FontSize() / 8);
                const IVec2 tooltipSeprOffset = tooltipNameOffset + Height(window.FontSize() + window.FontSize() / 2);
                const IVec2 tooltipDescOffset = tooltipNameOffset + Height(window.FontSize() + window.FontSize());

                // Mode
                if (window.CursorInUIBounds(Window::ButtonBound_Mode()))
                {
                    DrawRectangleIRect(Window::ButtonBound_Mode(), UIColor(UIColorID::UI_COLOR_AVAILABLE));
                    // Tooltip
                    const char* name = window.GetModeTooltipName(window.baseMode);
                    DrawTextIV(name, Window::ButtonBound_Mode().xy + tooltipNameOffset, window.FontSize(), UIColor(UIColorID::UI_COLOR_FOREGROUND));
                    Width separatorWidth(MeasureText(name, window.FontSize()));
                    DrawLineIV(Window::ButtonBound_Mode().xy + tooltipSeprOffset, separatorWidth, UIColor(UIColorID::UI_COLOR_FOREGROUND)); // Separator
                    DrawTextIV(window.GetModeTooltipDescription(window.baseMode), Window::ButtonBound_Mode().xy + tooltipDescOffset, window.FontSize(), UIColor(UIColorID::UI_COLOR_FOREGROUND));
                }
                // Gate
                else if (window.CursorInUIBounds(Window::ButtonBound_Gate()))
                {
                    DrawRectangleIRect(Window::ButtonBound_Gate(), UIColor(UIColorID::UI_COLOR_AVAILABLE));
                    // Tooltip
                    const char* name = window.GetGateTooltipName(window.gatePick);
                    DrawTextIV(name, Window::ButtonBound_Gate().xy + tooltipNameOffset, window.FontSize(), UIColor(UIColorID::UI_COLOR_FOREGROUND));
                    Width separatorWidth(MeasureText(name, window.FontSize()));
                    DrawLineIV(Window::ButtonBound_Gate().xy + tooltipSeprOffset, separatorWidth, UIColor(UIColorID::UI_COLOR_FOREGROUND));
                    DrawTextIV(window.GetGateTooltipDescription(window.gatePick), Window::ButtonBound_Gate().xy + tooltipDescOffset, window.FontSize(), UIColor(UIColorID::UI_COLOR_FOREGROUND));
                }
                // Extra param
                else if (window.CursorInUIBounds(Window::ButtonBound_Parameter()))
                {
                    DrawRectangleIRect(Window::ButtonBound_Parameter(), UIColor(UIColorID::UI_COLOR_AVAILABLE));
                    DrawRectangleIRect(ShrinkIRect(Window::ButtonBound_Parameter(), 2), window.ExtraParamColor());
                    // Tooltip
                    const char* text;
                    if (window.gatePick == Gate::LED)
                        text = TextFormat(window.deviceParameterTextFmt, Node::GetColorName(window.storedExtraParam));
                    else
                        text = TextFormat(window.deviceParameterTextFmt, window.storedExtraParam);
                    DrawTextIV(text, Window::ButtonBound_Parameter().xy + tooltipNameOffset, window.FontSize(), UIColor(UIColorID::UI_COLOR_FOREGROUND));
                }
                // Blueprints
                else if (window.CursorInUIBounds(Window::ButtonBound_Blueprints()))
                {
                    DrawRectangleIRect(Window::ButtonBound_Blueprints(), UIColor(UIColorID::UI_COLOR_AVAILABLE));
                    // Tooltip
                    DrawTextIV("Blueprints", Window::ButtonBound_Blueprints().xy + tooltipNameOffset, window.FontSize(), UIColor(UIColorID::UI_COLOR_FOREGROUND));
                }
                // Clipboard
                else if (window.CursorInUIBounds(Window::ButtonBound_Clipboard()))
                {
                    DrawRectangleIRect(Window::ButtonBound_Clipboard(), UIColor(UIColorID::UI_COLOR_AVAILABLE));
                    // Tooltip
                    DrawTextIV("Clipboard (ctrl+c to copy, ctrl+v to paste)", Window::ButtonBound_Clipboard().xy + tooltipNameOffset, window.FontSize(), UIColor(UIColorID::UI_COLOR_FOREGROUND));
                    const IVec2 clipboardPreviewOffset = tooltipNameOffset + Height(Button::g_width);
                    // Clipboard preview
                    if (window.IsClipboardValid())
                        window.clipboard->DrawSelectionPreview(
                            Window::ButtonBound_Clipboard().xy + clipboardPreviewOffset,
                            UIColor(UIColorID::UI_COLOR_BACKGROUND1),
                            UIColor(UIColorID::UI_COLOR_FOREGROUND3),
                            UIColor(UIColorID::UI_COLOR_BACKGROUND2),
                            ColorAlpha(UIColor(UIColorID::UI_COLOR_FOREGROUND3), 0.25f),
                            window.clipboardPreviewLOD);
                }

                window.DrawModeIcon(window.baseMode, Window::ButtonBound_Mode().xy, UIColor(UIColorID::UI_COLOR_FOREGROUND));
                window.DrawGateIcon(window.gatePick, Window::ButtonBound_Gate().xy, UIColor(UIColorID::UI_COLOR_FOREGROUND));
                
                for (IVec2 offset = IVec2(-1); offset.y <= 1; ++offset.y)
                {
                    for (offset.x = -1; offset.x <= 1; ++offset.x)
                    {
                        DrawTextIV(TextFormat("%i", window.storedExtraParam), Window::ButtonBound_Parameter().xy + IVec2(2, 1) + offset, window.FontSize(), UIColor(UIColorID::UI_COLOR_BACKGROUND));
                    }
                }
                DrawTextIV(TextFormat("%i", window.storedExtraParam), Window::ButtonBound_Parameter().xy + IVec2(2,1), window.FontSize(), UIColor(UIColorID::UI_COLOR_FOREGROUND));
                DrawTextureIV(window.GetBlueprintIcon(), Window::ButtonBound_Blueprints().xy, UIColor(UIColorID::UI_COLOR_FOREGROUND));
                DrawTextureIV(window.GetClipboardIcon(), Window::ButtonBound_Clipboard().xy, window.IsClipboardValid() ? UIColor(UIColorID::UI_COLOR_FOREGROUND) : ColorAlpha(UIColor(UIColorID::UI_COLOR_FOREGROUND), 0.25f));
            }

        } EndDrawing();
    }

    /******************************************
    *   Unload and free memory
    ******************************************/

	return 0;
}

/* Todo
* Requirements for v1.0.0
* -Improve groups
* -Fix wonky zooming
* -More explanation of controls
* -Undo/redo
* -Blueprint pallet
* -Prefab blueprints for things like timers, counters, and latches
* -Blueprint pallet icons (User-made combination of 4 premade icons. See Factorio for inspiration)
* -File menu (https://en.cppreference.com/w/cpp/filesystem/directory_iterator)
* -File thumbnails (based on file contents)
* -Menu screen (Open to file menu with "new" at the top)
*
* Refactors
* -Refactor Tab to have a bulk destroy function to make bulk deletion more efficient
* -Refactor buttons to be classes/structs instead of freeform
* 
* Beyond v1.0.0
* -Parallel node drawing with pen when dragging - up to 8 at a time
* -Parallel node connection of selected nodes being shift-clicked (simmilar to "bridge" in 3D programs)
* -Shift-click pen should create intermediate nodes at wire elbow and connect with those instead of the parent node
* (Maybe? It might be more computationally expensive. Perhaps there could be an "intermediate/repeater node" which takes only one input and gives one output and has no eval?)
* -Multiple wire stacking on single node
* -Hotkeys for output-only gate state toggles (Like the Reason on-screen piano (Yes, this is different from interact mode))
*
* Optional
* -Multiple color pallets
* -Log files for debug/crashes
*/
