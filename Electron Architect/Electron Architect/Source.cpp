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
#include "Graph.h"
#include "Tab.h"
#include "Buttons.h"
#include "UIColors.h"
#include "Tool.h"
#include "Window.h"

#include "program_icon.h"

int main()
{
    /******************************************
    *   Load textures, shaders, and meshes
    ******************************************/
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    Window window;

    Image icon{
        .data{PROGRAM_ICON_DATA},
        .width{PROGRAM_ICON_WIDTH},
        .height{PROGRAM_ICON_HEIGHT},
        .mipmaps{1},
        .format{PROGRAM_ICON_FORMAT}
    };

    SetWindowIcon(icon);

    window.CurrentTab().graph->Load("session.cg"); // Construct and load last session
    // Load blueprints
    {
        window.Log(LogType::attempt, "Loading blueprints");
        std::filesystem::path blueprints{ "blueprints" };
        std::filesystem::create_directories(blueprints);
        std::filesystem::directory_iterator directory{ blueprints };
        if (directory._At_end())
            window.Log(LogType::info, "No blueprints found");
        else
        {
            size_t count = 0;
            for (auto const& dir_entry : directory)
            {
                std::string filename = dir_entry.path().string();
                window.Log(LogType::info, "Located file \"" + filename + '\"');
                if (filename.substr(filename.size() - 3, filename.size()) == ".bp")
                    window.Log(LogType::attempt, "Loading blueprint \"" + filename + '\"');
                Blueprint bp;
                try
                {
                    LoadBlueprint(filename.c_str(), bp);
                    window.CurrentTab().graph->StoreBlueprint(&bp);
                    window.Log(LogType::success, "Blueprint loaded");
                    ++count;
                }
                catch (std::length_error e)
                {
                    window.Log(LogType::warning, "Blueprint file corrupt");
                }
            }
            window.Log(LogType::success, std::to_string(count) + " Blueprints loaded");
        }
    }

    double lastAutoSaveTime = 0;

    while (!WindowShouldClose())
    {
        /******************************************
        *   Simulate frame and update variables
        ******************************************/

        if ((GetTime() - lastAutoSaveTime) > 60.0)
        {
            window.CurrentTab().graph->Save("session.cg");
            lastAutoSaveTime = GetTime();
        }

        if (IsWindowResized())
            window.UpdateSize();

        window.IncrementTick();

        window.UpdateCursorPos();

        window.UpdateCamera();

        window.CheckHotkeys();

        // UI buttons
        if (window.GetModeType() == ModeType::Basic && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            for (const Button* const b : window.allButtons)
            {
                if (window.CursorInUIBounds(b->Bounds()))
                {
                    b->OnClick();
                    goto EVAL; // Skip button sim this frame
                }
            }
        }

        // Input
        window.UpdateTool();

    EVAL:
        window.cursorPosPrev = window.cursorPos;
        if (window.CurrentTab().graph->IsOrderDirty())
            window.tickThisFrame = !(window.tickFrame = 0);
        if (window.tickThisFrame)
            window.CurrentTab().graph->Evaluate();

        /******************************************
        *   Draw the frame
        ******************************************/

        BeginDrawing(); {

            bool inMenu = window.GetModeType() == ModeType::Menu;

            ClearBackground(UIColor(UIColorID::UI_COLOR_BACKGROUND));

            if (!inMenu)
            {
                window.CurrentTab().Set2DMode(true);

                window.DrawGrid();

                window.CurrentTab().graph->DrawGroups();
            }

            // Draw
            window.DrawTool();

            window.CurrentTab().Set2DMode(false); // Just in case

            // Panels

            // Tool pane
            if (!inMenu)
                window.DrawToolPane();

            // Console panel
            if (window.consoleOn)
            {
                window.CleanConsolePane();
                window.DrawConsoleOutput();
            }

            // Properties pane
            if (window.propertiesOn)
            {
                // Initialize properties panel
                window.CleanPropertiesPane();

                // Cursor stats
                if (
                   !(window.CursorInUIBounds(window.toolPaneRec) ||
                    (window.CursorInUIBounds(window.propertiesPaneRec) && window.propertiesOn) ||
                    (window.CursorInUIBounds(window.consolePaneRec) && window.consoleOn) ||
                    (inMenu)))
                {
                    window.PushPropertySubtitle("Cursor");
                    window.PushProperty_int("X", window.cursorPos.x / g_gridSize);
                    window.PushProperty_int("Y", window.cursorPos.y / g_gridSize);
                    window.PushPropertySpacer();
                }

                if (!inMenu)
                {
                    window.PushPropertySubtitle("Mode");
                    const char* modeName;
                    switch (window.GetBaseMode())
                    {
                    case Mode::PEN:      modeName = "Draw";     break;
                    case Mode::EDIT:     modeName = "Edit";     break;
                    case Mode::ERASE:    modeName = "Erase";    break;
                    case Mode::INTERACT: modeName = "Interact"; break;
                    default: modeName = "ERROR"; break;
                    }
                    window.PushProperty("Name", modeName);
                    window.PushProperty_longStr("Description", window.ButtonFromMode(window.GetBaseMode())->description);
                    window.PushPropertySpacer();
                }

                // Show element properties in pen/edit mode
                if (!inMenu &&
                    window.GetBaseMode() == Mode::PEN ||
                    window.GetBaseMode() == Mode::EDIT)
                {
                    window.PushPropertySubtitle("Element");
                    window.PushProperty("Name", GateName(window.gatePick));
                    switch (window.gatePick)
                    {
                    case Gate::RESISTOR:  window.PushProperty_uint("Resistance", window.storedExtraParam); break;
                    case Gate::CAPACITOR: window.PushProperty_uint("Capacity",   window.storedExtraParam); break;
                    case Gate::LED:       window.PushProperty("Color", Node::GetColorName(window.storedExtraParam)); break;
                    default: break;
                    }
                    window.PushProperty_longStr("Description", window.ButtonFromGate(window.gatePick)->description);
                    window.PushPropertySpacer();
                }

                window.DrawToolProperties();

                // Node tooltip
                if (!inMenu && !!window.hoveredNode && window.hoveredNode->HasName())
                    DrawTextShadowedIV(
                        window.hoveredNode->GetName(),
                        window.cursorUIPos + IVec2(16),
                        window.FontSize(),
                        UIColor(UIColorID::UI_COLOR_FOREGROUND),
                        UIColor(UIColorID::UI_COLOR_BACKGROUND));
            }

        } EndDrawing();
    }

    /******************************************
    *   Unload and free memory
    ******************************************/

    window.SaveConfig();

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
* -Refactor Graph to have a bulk destroy function to make bulk deletion more efficient
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
