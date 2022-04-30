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

namespace data
{
    void Init(int windowWidth, int windowHeight)
    {
        windowWidth = windowWidth;
        windowHeight = windowHeight;
        InitWindow(windowWidth, windowHeight, "Electron Architect");
        SetExitKey(0);
        SetTargetFPS(60);

        BlueprintIcon::Load("icons_blueprint.png");

        iconTextures::clipboardIcon = LoadTexture("icon_clipboard.png");
        iconTextures::modeIcons = LoadTexture("icons_mode.png");
        iconTextures::gateIcons16x = LoadTexture("icons_gate16x.png");
        iconTextures::gateIcons32x = LoadTexture("icons_gate32x.png");
        basicMode_object = new Tool_Edit;
        SetMode(Mode::PEN);
        SetGate(Gate::OR);
    }

    void DeInit()
    {
        if (clipboard != nullptr)
            delete clipboard;

        NodeWorld::Get().Save("session.cg");
        NodeWorld::Get().Export("render.svg");
        BlueprintIcon::Unload();
        UnloadTexture(iconTextures::gateIcons32x);
        UnloadTexture(iconTextures::gateIcons16x);
        UnloadTexture(iconTextures::modeIcons);
        UnloadTexture(iconTextures::clipboardIcon);

        CloseWindow();
    }

#pragma region members

    namespace iconTextures
    {
        Texture2D clipboardIcon;
        Texture2D modeIcons;
        Texture2D gateIcons16x;
        Texture2D gateIcons32x;
    }

    int windowWidth;
    int windowHeight;

    Tool* basicMode_object DEBUG_MEMBER_INIT(nullptr);
    Overlay* overlayMode_object = nullptr;

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

#pragma endregion

    Texture2D GetClipboardIcon()
    {
        return iconTextures::clipboardIcon;
    }

    void DrawModeIcon(Mode mode, IVec2 pos, Color tint)
    {
        IVec2 offset;
        switch (mode)
        {
            ASSERT_SPECIALIZATION;

        case Mode::PEN:      offset = IVec2(0, 0); break;
        case Mode::EDIT:     offset = IVec2(1, 0); break;
        case Mode::ERASE:    offset = IVec2(0, 1); break;
        case Mode::INTERACT: offset = IVec2(1, 1); break;
        }
        DrawIcon<16>(iconTextures::modeIcons, offset, pos, tint);
    }

    void DrawGateIcon16x(Gate gate, IVec2 pos, Color tint)
    {
        IVec2 offset;
        switch (gate)
        {
            ASSERT_SPECIALIZATION;

        case Gate::OR:  offset = IVec2(0, 0); break;
        case Gate::AND: offset = IVec2(1, 0); break;
        case Gate::NOR: offset = IVec2(0, 1); break;
        case Gate::XOR: offset = IVec2(1, 1); break;

        case Gate::RESISTOR:  offset = IVec2(0, 2); break;
        case Gate::CAPACITOR: offset = IVec2(1, 2); break;
        case Gate::LED:       offset = IVec2(0, 3); break;
        case Gate::DELAY:     offset = IVec2(1, 3); break;
        }
        DrawIcon<16>(iconTextures::gateIcons16x, offset, pos, tint);
    }

    void DrawGateIcon32x(Gate gate, IVec2 pos, Color tint)
    {
        IVec2 offset;
        switch (gate)
        {
            ASSERT_SPECIALIZATION;

        case Gate::OR:  offset = IVec2(0, 0); break;
        case Gate::AND: offset = IVec2(1, 0); break;
        case Gate::NOR: offset = IVec2(0, 1); break;
        case Gate::XOR: offset = IVec2(1, 1); break;

        case Gate::RESISTOR:  offset = IVec2(0, 2); break;
        case Gate::CAPACITOR: offset = IVec2(1, 2); break;
        case Gate::LED:       offset = IVec2(0, 3); break;
        }
        DrawIcon<32>(iconTextures::gateIcons32x, offset, pos, tint);
    }

    bool ModeIsMenu(Mode mode)
    {
        // Modes which disable use of basic UI and drawing of certain UI elements
        return mode == Mode::BP_ICON || mode == Mode::BP_SELECT;
    }
    bool ModeIsMenu()
    {
        return ModeIsMenu(GetCurrentMode());
    }

    bool ModeIsOverlay(Mode mode)
    {
        // Modes which can be active simultaneously with a non-overlay mode
        return mode == Mode::BUTTON || mode == Mode::PASTE || ModeIsMenu(mode);
    }
    bool ModeIsOverlay()
    {
        return ModeIsOverlay(GetCurrentMode());
    }

    bool ModeIsBasic(Mode mode)
    {
        return !ModeIsOverlay(mode);
    }

    bool ModeIsBasic()
    {
        return ModeIsBasic(GetCurrentMode());
    }

    Mode GetOverlayMode()
    {
        if (!overlayMode_object)
            return Mode::null;
        return overlayMode_object->GetMode();
    }
    Mode GetBaseMode()
    {
        _ASSERTE(!!basicMode_object);
        return basicMode_object->GetMode();
    }
    Mode GetCurrentMode()
    {
        Mode ret = GetOverlayMode();
        if (ret == Mode::null)
            ret = GetBaseMode();
        return ret;
    }

    ModeHandler* CurrentTopMode()
    {
        if (!!overlayMode_object)
            return overlayMode_object;
        else
            return basicMode_object;
    }

    void SetMode(Mode newMode)
    {
        b_cursorMoved = true; // Make sure the first frame of the new mode performs all its initial checks

        // Storing these for when their objects get deleted
        Mode overlayMode = GetOverlayMode();
        Mode baseMode = GetBaseMode();
        Mode currentMode = GetCurrentMode();

        // No change
        if (newMode == currentMode)
            return;

        if (!!overlayMode_object)
            delete overlayMode_object;

        if (ModeIsBasic(newMode))
        {
            overlayMode_object = nullptr;
            delete basicMode_object;
            switch (newMode)
            {
                ASSERT_SPECIALIZATION;
            case Mode::PEN:      basicMode_object = new Tool_Pen;      break;
            case Mode::EDIT:     basicMode_object = new Tool_Edit;     break;
            case Mode::ERASE:    basicMode_object = new Tool_Erase;    break;
            case Mode::INTERACT: basicMode_object = new Tool_Interact; break;
            }
            return;
        }
        else
        {
            switch (newMode)
            {
                ASSERT_SPECIALIZATION;
            case Mode::BUTTON:    overlayMode_object = new Overlay_Button; break;
            case Mode::PASTE:     overlayMode_object = new Overlay_Paste;  break;
            case Mode::BP_ICON:   overlayMode_object = new Menu_Icon;      break;
            case Mode::BP_SELECT: overlayMode_object = new Menu_Select;    break;
            }
            return;
        }
    }

    void SetGate(Gate newGate)
    {
        gatePick = newGate;

        switch (newGate)
        {
        default: [[likely]] // More gates are non-special than special, and they all have this singular option.
            deviceParameterTextFmt = "Component parameter: %i";
            break;

        case Gate::RESISTOR:
            deviceParameterTextFmt = "Resistance: %i inputs";
            break;
        case Gate::CAPACITOR:
            deviceParameterTextFmt = "Capacity: %i ticks";
            break;
        case Gate::LED:
            deviceParameterTextFmt = "Color: %s";
            break;
        }
    }

    ModeHandler* BaseModeAsPolymorphic()
    {
        return static_cast<ModeHandler*>(basicMode_object);
    }

    void ClearOverlayMode()
    {
        SetMode(GetBaseMode());
    }

    const char* GetModeTooltipName(Mode mode)
    {
        switch (mode)
        {
            ASSERT_SPECIALIZATION_RET("ERROR");

        case Mode::PEN:
            return "Mode: Draw [b]";
        case Mode::EDIT:
            return "Mode: Edit [v]";
        case Mode::ERASE:
            return "Mode: Erase [x]";
        case Mode::INTERACT:
            return "Mode: Interact [f]";
        }
    }

    const char* GetModeTooltipDescription(Mode mode)
    {
        switch (mode)
        {
            ASSERT_SPECIALIZATION_RET("ERROR");

        case Mode::PEN:
            return
                "Left click to create a new node and start a wire from it, or to start a wire from an existing node.\n"
                "Left click again to connect the wire to a new node or an existing one, and start a new wire from there.\n"
                "Right click while creating a wire to cancel it.";
        case Mode::EDIT:
            return
                "Left click and drag nodes to move them around.\n"
                "Left click and drag wire elbows to snap them to a preferred angle.\n"
                "Right click nodes to apply the currently selected gate/settings to them.";
        case Mode::ERASE:
            return
                "Left click a node to erase it and all wires directly connected to it (collateral will render in MAGENTA).\n"
                "Left click a wire to erase only that wire, disconnecting the nodes without erasing them.";
        case Mode::INTERACT:
            return
                "Left click a node without any inputs (such nodes will render in BLUE) to toggle it between outputting true and false.\n"
                "NOTE: Framerate is intentionally lowered from 120 to 24 while in this mode for ease of inspection.";
        }
    }

    const char* GetGateTooltipName(Gate gate)
    {
        switch (gate)
        {
            ASSERT_SPECIALIZATION_RET("ERROR");

        case Gate::OR:
            return "Gate: Or [1]";
        case Gate::AND:
            return "Gate: And [2]";
        case Gate::NOR:
            return "Gate: Nor [3]";
        case Gate::XOR:
            return "Gate: Xor [4]";
        case Gate::RESISTOR:
            return "Device: Resistor [5]";
        case Gate::CAPACITOR:
            return "Device: Capacitor [6]";
        case Gate::LED:
            return "Device: LED [7]";
        case Gate::DELAY:
            return "Device: Delay [8]";
        }
    }

    const char* GetGateTooltipDescription(Gate gate)
    {
        switch (gate)
        {
            ASSERT_SPECIALIZATION_RET("ERROR");

        case Gate::OR:
            return
                "Outputs true if any input is true,\n"
                "Outputs false otherwise.";
        case Gate::AND:
            return
                "Outputs true if all inputs are true,\n"
                "Outputs false otherwise.";
        case Gate::NOR:
            return
                "Outputs false if any input is true.\n"
                "Outputs true otherwise.";
        case Gate::XOR:
            return
                "Outputs true if exactly 1 input is true,\n"
                "Outputs false otherwise.\n"
                "Order of inputs does not matter.";
        case Gate::RESISTOR:
            return
                "Outputs true if greater than [resistance] inputs are true,\n"
                "Outputs false otherwise.\n"
                "Order of inputs does not matter.";
        case Gate::CAPACITOR:
            return
                "Stores charge while any input is true.\n"
                "Stops charging once charge equals [capacity].\n"
                "Drains charge while no input is true.\n"
                "Outputs true while charge is greater than zero,\n"
                "Outputs true while any input is true,\n"
                "Outputs false otherwise.";
        case Gate::LED:
            return
                "Treats I/O the same as an OR gate.\n"
                "Lights up with the selected color when powered.";
        case Gate::DELAY:
            return
                "Treats I/O the same as an OR gate.\n"
                "Outputs with a 1-tick delay.\n"
                "Sequntial delay devices are recommended for delay greater than 1 tick.";
        }
    }

    void IncrementTick()
    {
        ++tickFrame;
        tickFrame %= framesPerTick;
        tickThisFrame = tickFrame == 0;
    }

    void UpdateCursorPos()
    {
        cursorUIPos = IVec2(GetMouseX(), GetMouseY());

        cursorPos = Snap(IVec2(
            (int)(GetMouseX() / camera.zoom) + (int)camera.target.x,
            (int)(GetMouseY() / camera.zoom) + (int)camera.target.y
        ), g_gridSize);
        {
            constexpr int halfgrid = g_gridSize / 2;
            if (cursorPos.x < 0) cursorPos.x -= halfgrid;
            else                 cursorPos.x += halfgrid;
            if (cursorPos.y < 0) cursorPos.y -= halfgrid;
            else                 cursorPos.y += halfgrid;
        }

        b_cursorMoved = cursorPosPrev != cursorPos;
    }

    // Zoom/pan

    void UpdateCamera()
    {
        if (!ModeIsMenu())
        {
            if (GetMouseWheelMove() > 0 && camera.zoom < 2.0f)
            {
                camera.zoom *= 2;
            }
            else if (GetMouseWheelMove() < 0 && camera.zoom > 0.125f)
            {
                camera.zoom /= 2;
            }

            camera.target.x += (float)(IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT)) * g_gridSize;
            camera.target.y += (float)(IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP)) * g_gridSize;
        }
    }

    void CopySelectionToClipboard()
    {
        if (clipboard != nullptr)
            delete clipboard;

        if (selection.empty())
            clipboard = nullptr;
        else
            clipboard = new Blueprint(selection);
    }

    bool IsClipboardValid()
    {
        return !!clipboard;
    }

    void ClearClipboard()
    {
        delete clipboard;
        clipboard = nullptr;
    }

    bool SelectionExists()
    {
        return !selection.empty();
    }

    void DestroySelection()
    {
        NodeWorld::Get().DestroyNodes(selection);
        CurrentModeAs<Tool_Edit>()->ClearSelection();
    }

    void CheckHotkeys()
    {
        // KEY COMBOS BEFORE INDIVIDUAL KEYS!

        // Ctrl
        if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))
        {

            // Ctrl-Alt
            if (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT))
            {
                // Ctrl-Alt-Shift
                if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
                {
                    return; // Don't miscommunicate to the user!!
                }
                return; // Don't miscommunicate to the user!!
            }

            // Ctrl-Shift
            else if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
            {
                return; // Don't miscommunicate to the user!!
            }


            // Copy
            if (IsKeyPressed(KEY_C) && GetCurrentMode() == Mode::EDIT)
                CopySelectionToClipboard();

            // Paste
            if (IsKeyPressed(KEY_V) && IsClipboardValid())
                SetMode(Mode::PASTE);

            // Group
            if (IsKeyPressed(KEY_G))
                CurrentModeAs<Tool_Edit>()->TryGrouping();

            // Save
            if (IsKeyPressed(KEY_S))
            {
                // Save blueprint
                if (GetCurrentMode() == Mode::PASTE)
                    SaveClipboardBlueprint();

                // Save file
                else NodeWorld::Get().Save("save.cg");
            }

            return; // Don't miscommunicate to the user!!
        }

        // Shift
        else if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
        {
            // Shift-Alt
            if (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT))
            {
                return; // Don't miscommunicate to the user!!
            }
            return; // Don't miscommunicate to the user!!
        }

        // Alt
        else if (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT))
        {
            return; // Don't miscommunicate to the user!!
        }

        // Gate hotkeys
        if (ModeIsBasic())
        {
            int gate = GetCharPressed() - '0';
            if (0 <= gate && gate <= 9)
                SetGate(g_GateOrder[gate]);
        }

        // Mode hotkeys
        if (IsKeyPressed(KEY_B)) SetMode(Mode::PEN);
        else if (IsKeyPressed(KEY_V)) SetMode(Mode::EDIT);
        //else if (IsKeyPressed(KEY_G)) SetMode(Mode::GATE); // TODO: Gate mode being depricated
        else if (IsKeyPressed(KEY_X)) SetMode(Mode::ERASE);
        else if (IsKeyPressed(KEY_F)) SetMode(Mode::INTERACT);

        // Escape in stages
        if (IsKeyPressed(KEY_ESCAPE))
        {
            // In order of priority!

            // Exit overlay mode
            if (!ModeIsBasic())
                ClearOverlayMode();

            // Clear selection
            else if (SelectionExists())
                CurrentModeAs<Tool_Edit>()->ClearSelection();

            // Clear clipboard
            else if (IsClipboardValid())
                ClearClipboard();

            // Reset to default mode
            else SetMode(Mode::PEN);
        }

        // Selection delete
        if ((IsKeyPressed(KEY_DELETE) || IsKeyPressed(KEY_BACKSPACE)) && SelectionExists())
            DestroySelection();
    }

    IVec2 GetCursorDelta()
    {
        return cursorPos - cursorPosPrev;
    }

    // Inclusive

    bool CursorInRangeX(int xMin, int xMax)
    {
        return cursorPos.x >= xMin && cursorPos.x <= xMax;
    }

    // Inclusive

    bool CursorInUIRangeX(int xMin, int xMax)
    {
        return cursorUIPos.x >= xMin && cursorUIPos.x <= xMax;
    }

    // Inclusive

    bool CursorInRangeY(int yMin, int yMax)
    {
        return cursorPos.y >= yMin && cursorPos.y <= yMax;
    }

    // Inclusive

    bool CursorInUIRangeY(int yMin, int yMax)
    {
        return cursorUIPos.y >= yMin && cursorUIPos.y <= yMax;
    }

    bool CursorInBounds(IRect bounds)
    {
        return InBoundingBox(bounds, cursorPos);
    }

    bool CursorInUIBounds(IRect uiBounds)
    {
        return InBoundingBox(uiBounds, cursorUIPos);
    }

    void DrawGrid(int gridSize)
    {
        // Grid
        {
            IVec2 extents((int)((float)windowWidth / camera.zoom), (int)((float)windowHeight / camera.zoom));
            IRect bounds(IVec2(camera.target), extents);

            // "If the number of world pixels compacted into a single screen pixel equal or exceed the pixels between gridlines"
            if ((int)(1.0f / camera.zoom) >= gridSize)
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
            int halfgrid = gridSize / 2;
            DrawLine(bounds.x, -halfgrid, bounds.x + bounds.w, -halfgrid, LIFELESSNEBULA);
            DrawLine(-halfgrid, bounds.y, -halfgrid, bounds.y + bounds.h, LIFELESSNEBULA);
        }
    }

    IRect GetSelectionBounds(const std::vector<Node*>& vec)
    {
        IRect bounds = IRect::Abused();
        for (Node* node : vec)
        {
            if (node->GetX() < bounds.minx) bounds.minx = node->GetX();
            else if (node->GetX() > bounds.maxx) bounds.maxx = node->GetX();
            if (node->GetY() < bounds.miny) bounds.miny = node->GetY();
            else if (node->GetY() > bounds.maxy) bounds.maxy = node->GetY();
        }
        bounds.DeAbuse();
        return bounds;
    }

    IRect GetSelectionBounds()
    {
        return GetSelectionBounds(selection);
    }

    Color ResistanceBandColor(uint8_t index)
    {
        _ASSERT_EXPR(index < _countof(Node::g_resistanceBands), L"Stored parameter out of bounds");
        return Node::g_resistanceBands[index];
    }

    Color ExtraParamColor()
    {
        return ResistanceBandColor(storedExtraParam);
    }

    void SetMode2D(bool value)
    {
        if (b_twoDee == value)
            return;

        if (b_twoDee = value) // 2D mode
            BeginMode2D(camera);
        else // UI mode
            EndMode2D();
    }

    void DrawTooltipAtCursor(const char* text, Color color)
    {
        auto wat_do = [&]() { DrawTextIV(text, cursorUIPos + IVec2(16), 8, color); };
        // Force UI mode
        if (b_twoDee)
        {
            EndMode2D();
            wat_do();
            BeginMode2D(camera);
        }
        else
            wat_do();
    }

    void SaveClipboardBlueprint()
    {
        SetMode(Mode::BP_ICON);
        CurrentModeAs<Menu_Icon>()->SaveBlueprint();
    }

    void Update()
    {
        CurrentTopMode()->Update();
    }
    void Draw()
    {
        CurrentTopMode()->Draw();
    }

    void DrawUI()
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
}