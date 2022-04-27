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

    ProgramData data(1280, 720);
    ModeHandler::data = data;
    ModeHandler* tool;

    NodeWorld::Get().Load("session.cg"); // Construct and load last session

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
        if (!data.ModeIsMenu() && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            if (data.CursorInUIBounds(ProgramData::ButtonBound_Mode() + Width(16 * 2)) &&
                (data.GetCurrentMode() == Mode::BUTTON ? data.CurrentModeAs<Overlay_Button>()->dropdownActive != (data.cursorUIPos.x / 16) : true))
            {
                data.SetMode(Mode::BUTTON);
                goto EVAL; // Skip button sim this frame
            }
            else if (data.CursorInUIBounds(ProgramData::ButtonBound_Blueprints()))
            {
                data.SetMode(Mode::BP_SELECT);
            }
            else if (data.CursorInUIBounds(ProgramData::ButtonBound_Clipboard()))
            {
                if (data.IsClipboardValid())
                    data.SetMode(Mode::PASTE);
                goto EVAL; // Skip button sim this frame
            }
        }

        data.currentMode_object->Update();

    EVAL:
        data.cursorPosPrev = data.cursorPos;
        if (data.tickThisFrame)
            NodeWorld::Get().Evaluate();

        /******************************************
        *   Draw the frame
        ******************************************/

        BeginDrawing(); {

            ClearBackground(BLACK);

            if (!data.ModeIsMenu())
            {
                data.SetMode2D(true);

                data.DrawGrid();

                NodeWorld::Get().DrawGroups();
            }
                

            // Draw
            data.currentMode_object->Draw();

            if (!data.ModeIsMenu())
            {
                data.SetMode2D(false);

                consteval

                // UI
                {
                    constexpr IRect WithClipboard = ProgramData::buttonBounds[0] + Width(16) * (_countof(ProgramData::buttonBounds) - 1);
                    constexpr IRect WithoutClipboard = WithClipboard - Width(ProgramData::ButtonBound_Clipboard());
                    DrawRectangleIRect(data.IsClipboardValid() ? WithClipboard : WithoutClipboard, SPACEGRAY);
                    DrawRectangleIRect(ProgramData::ButtonBound_Parameter(), data.ExtraParamColor());
                }

                // Buttons
                constexpr IVec2 tooltipNameOffset(16 + 4, 16 + 1);
                constexpr IVec2 tooltipSeprOffset = tooltipNameOffset + Height(8 + 8 / 2);
                constexpr IVec2 tooltipDescOffset = tooltipNameOffset + Height(8 + 8);
                // Mode
                if (data.CursorInUIBounds(ProgramData::ButtonBound_Mode()))
                {
                    DrawRectangleIRect(ProgramData::ButtonBound_Mode(), WIPBLUE);
                    // Tooltip
                    const char* name = data.GetModeTooltipName(data.GetBaseMode());
                    DrawTextIV(name, ProgramData::ButtonBound_Mode().xy + tooltipNameOffset, 8, WHITE);
                    Width separatorWidth(MeasureText(name, 8));
                    DrawLineIV(ProgramData::ButtonBound_Mode().xy + tooltipSeprOffset, separatorWidth, WHITE); // Separator
                    DrawTextIV(data.GetModeTooltipDescription(data.GetBaseMode()), ProgramData::ButtonBound_Mode().xy + tooltipDescOffset, 8, WHITE);
                }
                // Gate
                else if (data.CursorInUIBounds(ProgramData::ButtonBound_Gate()))
                {
                    DrawRectangleIRect(ProgramData::ButtonBound_Gate(), WIPBLUE);
                    // Tooltip
                    const char* name = data.GetGateTooltipName(data.gatePick);
                    DrawTextIV(name, ProgramData::ButtonBound_Gate().xy + tooltipNameOffset, 8, WHITE);
                    Width separatorWidth(MeasureText(name, 8));
                    DrawLineIV(ProgramData::ButtonBound_Gate().xy + tooltipSeprOffset, separatorWidth, WHITE);
                    DrawTextIV(data.GetGateTooltipDescription(data.gatePick), ProgramData::ButtonBound_Gate().xy + tooltipDescOffset, 8, WHITE);
                }
                // Extra param
                else if (data.CursorInUIBounds(ProgramData::ButtonBound_Parameter()))
                {
                    DrawRectangleIRect(ProgramData::ButtonBound_Parameter(), WIPBLUE);
                    DrawRectangleIRect(ShrinkIRect(ProgramData::ButtonBound_Parameter(), 2), data.ExtraParamColor());
                    // Tooltip
                    const char* text;
                    if (data.gatePick == Gate::LED)
                        text = TextFormat(data.deviceParameterTextFmt, Node::GetColorName(data.storedExtraParam));
                    else
                        text = TextFormat(data.deviceParameterTextFmt, data.storedExtraParam);
                    DrawTextIV(text, ProgramData::ButtonBound_Parameter().xy + tooltipNameOffset, 8, WHITE);
                }
                // Blueprints
                else if (data.CursorInUIBounds(ProgramData::ButtonBound_Blueprints()))
                {
                    DrawRectangleIRect(ProgramData::ButtonBound_Blueprints(), WIPBLUE);
                    // Tooltip
                    DrawTextIV("Blueprints (WIP)", ProgramData::ButtonBound_Blueprints().xy + tooltipNameOffset, 8, WHITE);
                }
                // Clipboard
                else if (data.CursorInUIBounds(ProgramData::ButtonBound_Clipboard()))
                {
                    DrawRectangleIRect(ProgramData::ButtonBound_Clipboard(), WIPBLUE);
                    // Tooltip
                    DrawTextIV("Clipboard (ctrl+c to copy, ctrl+v to paste)", ProgramData::ButtonBound_Clipboard().xy + tooltipNameOffset, 8, WHITE);
                }

                data.DrawModeIcon(data.GetBaseMode(), ProgramData::ButtonBound_Mode().xy, WHITE);
                data.DrawGateIcon16x(data.gatePick, ProgramData::ButtonBound_Gate().xy, WHITE);
                DrawTextureIV(data.GetClipboardIcon(), ProgramData::ButtonBound_Clipboard().xy, data.IsClipboardValid() ? WHITE : ColorAlpha(WHITE, 0.25f));
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
* -Refactor NodeWorld to have a bulk destroy function to make bulk deletion more efficient
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
