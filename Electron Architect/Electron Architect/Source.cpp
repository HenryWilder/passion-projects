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

        data::currentMode_object->Update();

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
            }
                

            // Draw
            data::currentMode_object->Draw();

            if (!data::ModeIsMenu()) [[likely]]
            {
                data::SetMode2D(false);

                // UI
                {
                    constexpr IRect WithClipboard = data::ButtonBounds(0) + Width(16) * ((int)ButtonID::count - 1);
                    constexpr IRect WithoutClipboard = WithClipboard - Width(data::ButtonBounds(ButtonID::Clipboard));
                    DrawRectangleIRect(data::IsClipboardValid() ? WithClipboard : WithoutClipboard, SPACEGRAY);
                    DrawRectangleIRect(data::ButtonBounds(ButtonID::Parameter), data::ExtraParamColor());
                }

                // Buttons
                constexpr IVec2 tooltipNameOffset(16 + 4, 16 + 1);
                constexpr IVec2 tooltipSeprOffset = tooltipNameOffset + Height(8 + 8 / 2);
                constexpr IVec2 tooltipDescOffset = tooltipNameOffset + Height(8 + 8);
                // Mode
                if (data::CursorInUIBounds(data::ButtonBounds(ButtonID::Mode)))
                {
                    DrawRectangleIRect(data::ButtonBounds(ButtonID::Mode), WIPBLUE);
                    // Tooltip
                    const char* name = data::GetModeTooltipName(data::GetBaseMode());
                    DrawTextIV(name, data::ButtonBounds(ButtonID::Mode).xy + tooltipNameOffset, 8, WHITE);
                    Width separatorWidth(MeasureText(name, 8));
                    DrawLineIV(data::ButtonBounds(ButtonID::Mode).xy + tooltipSeprOffset, separatorWidth, WHITE); // Separator
                    DrawTextIV(data::GetModeTooltipDescription(data::GetBaseMode()), data::ButtonBounds(ButtonID::Mode).xy + tooltipDescOffset, 8, WHITE);
                }
                // Gate
                else if (data::CursorInUIBounds(data::ButtonBounds(ButtonID::Gate)))
                {
                    DrawRectangleIRect(data::ButtonBounds(ButtonID::Gate), WIPBLUE);
                    // Tooltip
                    const char* name = data::GetGateTooltipName(data::gatePick);
                    DrawTextIV(name, data::ButtonBounds(ButtonID::Gate).xy + tooltipNameOffset, 8, WHITE);
                    Width separatorWidth(MeasureText(name, 8));
                    DrawLineIV(data::ButtonBounds(ButtonID::Gate).xy + tooltipSeprOffset, separatorWidth, WHITE);
                    DrawTextIV(data::GetGateTooltipDescription(data::gatePick), data::ButtonBounds(ButtonID::Gate).xy + tooltipDescOffset, 8, WHITE);
                }
                // Extra param
                else if (data::CursorInUIBounds(data::ButtonBounds(ButtonID::Parameter)))
                {
                    DrawRectangleIRect(data::ButtonBounds(ButtonID::Parameter), WIPBLUE);
                    DrawRectangleIRect(ShrinkIRect(data::ButtonBounds(ButtonID::Parameter), 2), data::ExtraParamColor());
                    // Tooltip
                    const char* text;
                    if (data::gatePick == Gate::LED)
                        text = TextFormat(data::deviceParameterTextFmt, Node::GetColorName(data::storedExtraParam));
                    else
                        text = TextFormat(data::deviceParameterTextFmt, data::storedExtraParam);
                    DrawTextIV(text, data::ButtonBounds(ButtonID::Parameter).xy + tooltipNameOffset, 8, WHITE);
                }
                // Blueprints
                else if (data::CursorInUIBounds(data::ButtonBounds(ButtonID::Blueprints))) [[unlikely]] // Not likely while this feature is still in development
                {
                    DrawRectangleIRect(data::ButtonBounds(ButtonID::Blueprints), WIPBLUE);
                    // Tooltip
                    DrawTextIV("Blueprints (WIP)", data::ButtonBounds(ButtonID::Blueprints).xy + tooltipNameOffset, 8, WHITE);
                }
                // Clipboard
                else if (data::CursorInUIBounds(data::ButtonBounds(ButtonID::Clipboard)))
                {
                    DrawRectangleIRect(data::ButtonBounds(ButtonID::Clipboard), WIPBLUE);
                    // Tooltip
                    DrawTextIV("Clipboard (ctrl+c to copy, ctrl+v to paste)", data::ButtonBounds(ButtonID::Clipboard).xy + tooltipNameOffset, 8, WHITE);
                }

                data::DrawModeIcon(data::GetBaseMode(), data::ButtonBounds(ButtonID::Mode).xy, WHITE);
                data::DrawGateIcon16x(data::gatePick, data::ButtonBounds(ButtonID::Gate).xy, WHITE);
                DrawTextureIV(data::GetClipboardIcon(), data::ButtonBounds(ButtonID::Clipboard).xy, data::IsClipboardValid() ? WHITE : ColorAlpha(WHITE, 0.25f));
            }

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
