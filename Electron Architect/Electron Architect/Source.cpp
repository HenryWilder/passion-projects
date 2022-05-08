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
#include "Mode.h"
#include "Window.h"

int main()
{
    /******************************************
    *   Load textures, shaders, and meshes
    ******************************************/

    Window window(1280, 720);

    window.CurrentTab().graph->Load("session.cg"); // Construct and load last session
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
                window.CurrentTab().graph->StoreBlueprint(&bp);
            }
            catch (std::length_error e)
            {
                printf("File corrupt. Encountered \"%s\" error\n", e.what());
            }
        }
    }

    std::vector<Button*> allButtons;
    {
        allButtons.reserve(window.modeButtons.size() + window.gateButtons.size() + 2);
        for (Button& b : window.modeButtons)
        {
            allButtons.push_back(&b);
        }
        for (Button& b : window.gateButtons)
        {
            allButtons.push_back(&b);
        }
        allButtons.push_back(&window.blueprintsButton);
        allButtons.push_back(&window.clipboardButton);
    }

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
            for (const Button* const b : allButtons)
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

            ClearBackground(UIColor(UIColorID::UI_COLOR_BACKGROUND));

            if (window.GetModeType() != ModeType::Menu)
            {
                BeginMode2D(window.CurrentTab().camera);

                window.DrawGrid();

                window.CurrentTab().graph->DrawGroups();

                //DrawRectangleIRect(IRect(window.cursorPos - IVec2(g_gridSize / 2) - Width(1), g_gridSize + 1), UIColor(UIColorID::UI_COLOR_AVAILABLE));
            }

            // Draw
            window.DrawTool();

            if (window.GetModeType() != ModeType::Menu)
            {
                EndMode2D(); // Just in case

                const Button* buttonsToHighlight[] = {
                    static_cast<const Button*>(window.ButtonFromMode(window.GetMode())),
                    static_cast<const Button*>(window.ButtonFromGate(window.gatePick)),
                };

                // Panels
                // Todo: Make these collapsable
                DrawRectangleIRect(IRect(Button::g_width, window.windowHeight), UIColor(UIColorID::UI_COLOR_BACKGROUND1));

                // Initialize properties panel
                window.CleanPropertiesPane();

                // Cursor stats
                window.PushPropertySubtitle("Cursor");
                window.PushProperty_int("X", window.cursorPos.x / g_gridSize);
                window.PushProperty_int("Y", window.cursorPos.y / g_gridSize);
                window.PushPropertySubtitle("");

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
                window.PushProperty_longStr("Description", buttonsToHighlight[0]->description);
                window.PushPropertySubtitle("");

                // Show element properties in pen/edit mode
                if (window.GetBaseMode() == Mode::PEN || window.GetBaseMode() == Mode::EDIT)
                {
                    window.PushPropertySubtitle("Element");
                    window.PushProperty("Name", GateName(window.gatePick));
                    window.PushProperty_longStr("Description", buttonsToHighlight[1]->description);
                    window.PushPropertySubtitle("");
                }

                window.DrawToolProperties();

                // Background
                for (const Button* const b : allButtons)
                {
                    if (window.CursorInUIBounds(b->Bounds())) [[unlikely]]
                        DrawRectangleIRect(b->Bounds(), UIColor(UIColorID::UI_COLOR_AVAILABLE));
                }

                for (const Button* const b : allButtons)
                {
                    Color color;
                    bool shouldHighlight = false;
                    for (const Button* hb : buttonsToHighlight)
                    {
                        if (b == hb)
                        {
                            shouldHighlight = true;
                            break;
                        }
                    }
                    if (shouldHighlight || window.CursorInUIBounds(b->Bounds())) [[unlikely]]
                        color = UIColor(UIColorID::UI_COLOR_FOREGROUND);
                    else [[likely]]
                        color = UIColor(UIColorID::UI_COLOR_FOREGROUND2);

                    // Icon buttons
                    if (const IconButton* ib = dynamic_cast<const IconButton*>(b))
                        window.DrawUIIcon(*ib->textureSheet, ib->textureSheetPos, ib->Bounds().xy, color);
                    // Text buttons
                    else if (const TextButton* tb = dynamic_cast<const TextButton*>(b))
                        DrawTextIV(tb->buttonText, tb->Bounds().xy, window.FontSize(), color);
                }

                // Tooltips
                for (const Button* const b : allButtons)
                {
                    if (window.CursorInUIBounds(b->Bounds())) [[unlikely]]
                    {
                        DrawTextIV(b->tooltip, b->Bounds().BR() + IVec2(window.FontSize() / 2), window.FontSize(), UIColor(UIColorID::UI_COLOR_FOREGROUND));

                        // Clipboard preview
                        if (b == &window.clipboardButton && window.IsClipboardValid()) [[unlikely]]
                        {
                            window.clipboard->DrawSelectionPreview(
                                b->Bounds().BR() + (IVec2(window.FontSize() / 2) * Height(5)),
                                UIColor(UIColorID::UI_COLOR_BACKGROUND1),
                                UIColor(UIColorID::UI_COLOR_FOREGROUND3),
                                UIColor(UIColorID::UI_COLOR_BACKGROUND2),
                                ColorAlpha(UIColor(UIColorID::UI_COLOR_FOREGROUND3), 0.25f),
                                window.clipboardPreviewLOD);
                        }
                        break;
                    }
                }
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
