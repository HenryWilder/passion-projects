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

    Window data(1280, 720);

    data.CurrentTab().Load("session.cg"); // Construct and load last session
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
                data.CurrentTab().StoreBlueprint(&bp);
            }
            catch (std::length_error e)
            {
                printf("File corrupt. Encountered \"%s\" error\n", e.what());
            }
        }
    }

    while (!WindowShouldClose())
    {
        /******************************************
        *   Simulate frame and update variables
        ******************************************/

        data.IncrementTick();

        data.UpdateCursorPos();

        data.UpdateCamera();

        data.CheckHotkeys();

        // UI buttons
        if (data.GetModeType() != ModeType::Basic && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            if (data.CursorInUIBounds(IRect(Button::g_width, 0, Button::g_width, Button::g_width * _countof(data.sidebarGateOrder))))
            {
                // todo
            }
            else if (data.CursorInUIBounds(Window::ButtonBound_Blueprints()))
            {
                data.SetMode(Mode::BP_SELECT);
                goto EVAL; // Skip button sim this frame
            }
            else if (data.CursorInUIBounds(Window::ButtonBound_Clipboard()))
            {
                if (data.IsClipboardValid())
                    data.SetMode(Mode::PASTE);
                goto EVAL; // Skip button sim this frame
            }
        }

        // Input
        data.UpdateTool();

    EVAL:
        data.cursorPosPrev = data.cursorPos;
        if (data.CurrentTab().IsOrderDirty())
            data.tickThisFrame = !(data.tickFrame = 0);
        if (data.tickThisFrame)
            data.CurrentTab().Evaluate();

        /******************************************
        *   Draw the frame
        ******************************************/

        BeginDrawing(); {

            ClearBackground(UIColor(UIColorID::UI_COLOR_BACKGROUND));

            if (data.GetModeType() != ModeType::Menu)
            {
                BeginMode2D(data.camera);

                data.DrawGrid();

                data.CurrentTab().DrawGroups();
            }
                

            // Draw
            data.DrawTool();

            if (data.GetModeType() != ModeType::Menu)
            {
                EndMode2D();

                // UI

                // Sidebars
                DrawRectangleIRect(IRect(Button::g_width * 2, data.windowHeight), UIColor(UIColorID::UI_COLOR_BACKGROUND1));
                //DrawLine(Button::g_width * 2 + 1, 0, Button::g_width * 2 + 1, data.windowHeight, UIColor(UIColorID::UI_COLOR_BACKGROUND2));
                //DrawRectangleIRect(Window::ButtonBound_Parameter(), data.ExtraParamColor());

                const IVec2 tooltipNameOffset = IVec2(Button::g_width) + IVec2(data.FontSize() / 2, data.FontSize() / 8);
                const IVec2 tooltipSeprOffset = tooltipNameOffset + Height(data.FontSize() + data.FontSize() / 2);
                const IVec2 tooltipDescOffset = tooltipNameOffset + Height(data.FontSize() + data.FontSize());

                // Mode
                if (data.CursorInUIBounds(Window::ButtonBound_Mode()))
                {
                    DrawRectangleIRect(Window::ButtonBound_Mode(), UIColor(UIColorID::UI_COLOR_AVAILABLE));
                    // Tooltip
                    const char* name = data.GetModeTooltipName(data.baseMode);
                    DrawTextIV(name, Window::ButtonBound_Mode().xy + tooltipNameOffset, data.FontSize(), UIColor(UIColorID::UI_COLOR_FOREGROUND));
                    Width separatorWidth(MeasureText(name, data.FontSize()));
                    DrawLineIV(Window::ButtonBound_Mode().xy + tooltipSeprOffset, separatorWidth, UIColor(UIColorID::UI_COLOR_FOREGROUND)); // Separator
                    DrawTextIV(data.GetModeTooltipDescription(data.baseMode), Window::ButtonBound_Mode().xy + tooltipDescOffset, data.FontSize(), UIColor(UIColorID::UI_COLOR_FOREGROUND));
                }
                // Gate
                else if (data.CursorInUIBounds(Window::ButtonBound_Gate()))
                {
                    DrawRectangleIRect(Window::ButtonBound_Gate(), UIColor(UIColorID::UI_COLOR_AVAILABLE));
                    // Tooltip
                    const char* name = data.GetGateTooltipName(data.gatePick);
                    DrawTextIV(name, Window::ButtonBound_Gate().xy + tooltipNameOffset, data.FontSize(), UIColor(UIColorID::UI_COLOR_FOREGROUND));
                    Width separatorWidth(MeasureText(name, data.FontSize()));
                    DrawLineIV(Window::ButtonBound_Gate().xy + tooltipSeprOffset, separatorWidth, UIColor(UIColorID::UI_COLOR_FOREGROUND));
                    DrawTextIV(data.GetGateTooltipDescription(data.gatePick), Window::ButtonBound_Gate().xy + tooltipDescOffset, data.FontSize(), UIColor(UIColorID::UI_COLOR_FOREGROUND));
                }
                // Extra param
                else if (data.CursorInUIBounds(Window::ButtonBound_Parameter()))
                {
                    DrawRectangleIRect(Window::ButtonBound_Parameter(), UIColor(UIColorID::UI_COLOR_AVAILABLE));
                    DrawRectangleIRect(ShrinkIRect(Window::ButtonBound_Parameter(), 2), data.ExtraParamColor());
                    // Tooltip
                    const char* text;
                    if (data.gatePick == Gate::LED)
                        text = TextFormat(data.deviceParameterTextFmt, Node::GetColorName(data.storedExtraParam));
                    else
                        text = TextFormat(data.deviceParameterTextFmt, data.storedExtraParam);
                    DrawTextIV(text, Window::ButtonBound_Parameter().xy + tooltipNameOffset, data.FontSize(), UIColor(UIColorID::UI_COLOR_FOREGROUND));
                }
                // Blueprints
                else if (data.CursorInUIBounds(Window::ButtonBound_Blueprints()))
                {
                    DrawRectangleIRect(Window::ButtonBound_Blueprints(), UIColor(UIColorID::UI_COLOR_AVAILABLE));
                    // Tooltip
                    DrawTextIV("Blueprints", Window::ButtonBound_Blueprints().xy + tooltipNameOffset, data.FontSize(), UIColor(UIColorID::UI_COLOR_FOREGROUND));
                }
                // Clipboard
                else if (data.CursorInUIBounds(Window::ButtonBound_Clipboard()))
                {
                    DrawRectangleIRect(Window::ButtonBound_Clipboard(), UIColor(UIColorID::UI_COLOR_AVAILABLE));
                    // Tooltip
                    DrawTextIV("Clipboard (ctrl+c to copy, ctrl+v to paste)", Window::ButtonBound_Clipboard().xy + tooltipNameOffset, data.FontSize(), UIColor(UIColorID::UI_COLOR_FOREGROUND));
                    const IVec2 clipboardPreviewOffset = tooltipNameOffset + Height(Button::g_width);
                    // Clipboard preview
                    if (data.IsClipboardValid())
                        data.clipboard->DrawSelectionPreview(
                            Window::ButtonBound_Clipboard().xy + clipboardPreviewOffset,
                            UIColor(UIColorID::UI_COLOR_BACKGROUND1),
                            UIColor(UIColorID::UI_COLOR_FOREGROUND3),
                            UIColor(UIColorID::UI_COLOR_BACKGROUND2),
                            ColorAlpha(UIColor(UIColorID::UI_COLOR_FOREGROUND3), 0.25f),
                            data.clipboardPreviewLOD);
                }

                data.DrawModeIcon(data.baseMode, Window::ButtonBound_Mode().xy, UIColor(UIColorID::UI_COLOR_FOREGROUND));
                data.DrawGateIcon(data.gatePick, Window::ButtonBound_Gate().xy, UIColor(UIColorID::UI_COLOR_FOREGROUND));
                
                for (IVec2 offset = IVec2(-1); offset.y <= 1; ++offset.y)
                {
                    for (offset.x = -1; offset.x <= 1; ++offset.x)
                    {
                        DrawTextIV(TextFormat("%i", data.storedExtraParam), Window::ButtonBound_Parameter().xy + IVec2(2, 1) + offset, data.FontSize(), UIColor(UIColorID::UI_COLOR_BACKGROUND));
                    }
                }
                DrawTextIV(TextFormat("%i", data.storedExtraParam), Window::ButtonBound_Parameter().xy + IVec2(2,1), data.FontSize(), UIColor(UIColorID::UI_COLOR_FOREGROUND));
                DrawTextureIV(data.GetBlueprintIcon(), Window::ButtonBound_Blueprints().xy, UIColor(UIColorID::UI_COLOR_FOREGROUND));
                DrawTextureIV(data.GetClipboardIcon(), Window::ButtonBound_Clipboard().xy, data.IsClipboardValid() ? UIColor(UIColorID::UI_COLOR_FOREGROUND) : ColorAlpha(UIColor(UIColorID::UI_COLOR_FOREGROUND), 0.25f));
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
