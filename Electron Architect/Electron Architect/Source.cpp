#include <raylib.h>
#include "HUtility.h"
#include "IVec.h"
#include "Node.h"
#include "Wire.h"
#include "Blueprint.h"
#include "Group.h"
#include "NodeWorld.h"
#include "ProgramData.h"
#include "ToolModes.h"

int main()
{
    /******************************************
    *   Load textures, shaders, and meshes
    ******************************************/

    data::Init(1280, 720);
    ModeHandler* tool = new Tool_Pen;

    NodeWorld::Get().Load("session.cg"); // Construct and load last session

    while (!WindowShouldClose())
    {
        /******************************************
        *   Simulate frame and update variables
        ******************************************/

        data::IncrementTick();

        data::UpdateCursorPos();

        data::UpdateCamera();

        data::CheckHotkeys();

        // UI buttons
        if (!data::ModeIsMenu() && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            [[unlikely]] // Very, very few frames will have a click to begin with (Who on earth is clicking at 60hz in this??)
        {
            if (data::CursorInUIBounds(data::ButtonBounds(ButtonID::Mode) + Width(16 * 2)) &&
                (data::GetCurrentMode() == Mode::BUTTON ? data::CurrentModeAs<Overlay_Button>()->dropdownActive != (data::cursorUIPos.x / 16) : true))
            {
                data::SetMode(Mode::BUTTON);
                goto EVAL; // Skip button sim this frame
            }
            else if (data::CursorInUIBounds(data::ButtonBounds(ButtonID::Blueprints)))
                [[unlikely]] // Todo: Change this not to be unlikely once there's an actual reason for people to click it
            {
                data::SetMode(Mode::BP_SELECT);
            }
            else if (data::CursorInUIBounds(data::ButtonBounds(ButtonID::Clipboard)))
            {
                if (data::IsClipboardValid())
                    data::SetMode(Mode::PASTE);
                goto EVAL; // Skip button sim this frame
            }
        }

        data::Update();

    EVAL:
        data::cursorPosPrev = data::cursorPos;
        if (data::tickThisFrame) [[unlikely]] // Only 10 times a second
            NodeWorld::Get().Evaluate();

        /******************************************
        *   Draw the frame
        ******************************************/

        BeginDrawing(); {

            ClearBackground(BLACK);

            if (!data::ModeIsMenu()) [[likely]]
            {
                data::SetMode2D(true);

                data::DrawGrid();

                NodeWorld::Get().DrawGroups();

                data::Draw();

                data::DrawUI();
            }
            else [[unlikely]]
                data::Draw();

        } EndDrawing();
    }

    /******************************************
    *   Unload and free memory
    ******************************************/

	return 0;
}

/* Features
* Requirements for v1.0.0
* Feature: Improve groups
* Feature: Fix wonky zooming
* Feature: More explanation of controls
* Feature: Undo/redo
* Feature: Blueprint pallet
* Feature: Prefab blueprints for things like timers, counters, and latches
* Feature: Blueprint pallet icons (User-made combination of 4 premade icons. See Factorio for inspiration)
* Feature: File menu (https://en.cppreference.com/w/cpp/filesystem/directory_iterator)
* Feature: File thumbnails (based on file contents)
* Feature: Menu screen (Open to file menu with "new" at the top)
*
* Refactors
* Feature: Refactor NodeWorld to have a bulk destroy function to make bulk deletion more efficient
* Feature: Refactor buttons to be classes/structs instead of freeform
* 
* Beyond v1.0.0
* Feature: Parallel node drawing with pen when dragging - up to 8 at a time
* Feature: Parallel node connection of selected nodes being shift-clicked (simmilar to "bridge" in 3D programs)
* Feature: Shift-click pen should create intermediate nodes at wire elbow and connect with those instead of the parent node
* (Maybe? It might be more computationally expensive. Perhaps there could be an "intermediate/repeater node" which takes only one input and gives one output and has no eval?)
* Feature: Multiple wire stacking on single node
* Feature: Hotkeys for output-only gate state toggles (Like the Reason on-screen piano (Yes, this is different from interact mode))
*
* Optional
* Feature: Multiple color pallets
* Feature: Log files for debug/crashes
*/
