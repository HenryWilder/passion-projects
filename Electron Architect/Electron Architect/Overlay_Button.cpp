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

const Mode Overlay_Button::dropdownModeOrder[4] = {
        Mode::PEN,
        Mode::EDIT,
        Mode::ERASE,
        Mode::INTERACT,
};
const Gate Overlay_Button::dropdownGateOrder[8] = {
    Gate::OR,
    Gate::AND,
    Gate::NOR,
    Gate::XOR,

    Gate::RESISTOR,
    Gate::CAPACITOR,
    Gate::LED,
    Gate::DELAY,
};
constexpr int Overlay_Button::buttonsInDropdown[] = {
    _countof(dropdownModeOrder) - 1,
    _countof(dropdownGateOrder) - 1,
    _countof(Node::g_resistanceBands) - 1
};
constexpr IRect Overlay_Button::dropdownBounds[] = {
    IRect( 0, 16, 16) * Height(buttonsInDropdown[0]), // Mode
    IRect(16, 16, 16) * Height(buttonsInDropdown[1]), // Gate
    IRect(32, 16, 16) * Height(buttonsInDropdown[2]), // Parameter
};

Overlay_Button::Overlay_Button()
{
    dropdownActive = (ButtonID)(data::cursorUIPos.x / 16);
}
Overlay_Button::~Overlay_Button()
{
    // End of mode
}

void Overlay_Button::Update()
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) [[unlikely]] // Very few frames will ever have a click
    {
        uint8_t dropdownIndex = (uint8_t)dropdownActive;
        const IRect& activeDropdownBounds = dropdownBounds[dropdownIndex];
        if (data::CursorInUIBounds(activeDropdownBounds)) [[likely]] // It's more likely that a click will be inside the button if we're already at this point
        {
            IRect rec(activeDropdownBounds.xy, 16);
            switch (dropdownActive)
            {
            case ButtonID::Mode:
            {
                for (Mode m : dropdownModeOrder)
                {
                    if (m == data::GetBaseMode())
                        continue;

                    if (InBoundingBox(rec, data::cursorUIPos))
                    {
                        data::SetMode(m);
                        return; // Exit without clearing overlay
                    }
                    rec.y += 16;
                }
            }
            break;

            case ButtonID::Gate:
            {
                for (Gate g : dropdownGateOrder)
                {
                    if (g == data::gatePick)
                        continue;

                    if (InBoundingBox(rec, data::cursorUIPos))
                    {
                        data::SetGate(g);
                        break;
                    }
                    rec.y += 16;
                }
            }
            break;

            case ButtonID::Parameter:
            {
                for (uint8_t v = 0; v < _countof(Node::g_resistanceBands); ++v)
                {
                    if (v == data::storedExtraParam)
                        continue;

                    if (InBoundingBox(rec, data::cursorUIPos))
                    {
                        data::storedExtraParam = v;
                        break;
                    }
                    rec.y += 16;
                }
            }
            break;
            }
        }
        data::ClearOverlayMode();
    }
    else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) [[unlikely]]
    {
        data::ClearOverlayMode();
    }
}

void Overlay_Button::Draw()
{
    NodeWorld::Get().DrawWires();
    NodeWorld::Get().DrawNodes();

    data::SetMode2D(false);

    uint8_t dropdownIndex = (uint8_t)dropdownActive;

    DrawRectangleIRect(dropdownBounds[dropdownIndex], SPACEGRAY);
    IRect rec(dropdownBounds[dropdownIndex].xy, 16);

    switch (dropdownActive)
    {
    case ButtonID::Mode:
    {
        for (Mode m : dropdownModeOrder)
        {
            if (m == data::GetBaseMode())
                continue;
            Color color;
            if (InBoundingBox(rec, data::cursorUIPos))
            {
                color = WHITE;
                DrawText(data::GetModeTooltipName(m), 20, 17 + rec.y, 8, WHITE);
            }
            else
                color = DEADCABLE;
            data::DrawModeIcon(m, rec.xy, color);
            rec.y += 16;
        }
    }
    break;

    case ButtonID::Gate:
    {
        for (Gate g : dropdownGateOrder)
        {
            if (g == data::gatePick)
                continue;
            Color color;
            if (InBoundingBox(rec, data::cursorUIPos))
            {
                color = WHITE;
                DrawText(data::GetGateTooltipName(g), 20 + 16, 17 + rec.y, 8, WHITE);
            }
            else
                color = DEADCABLE;
            data::DrawGateIcon16x(g, rec.xy, color);
            rec.y += 16;
        }
    }
    break;

    case ButtonID::Parameter:
    {
        for (uint8_t v = 0; v < _countof(Node::g_resistanceBands); ++v)
        {
            if (v == data::storedExtraParam)
                continue;
            Color color = Node::g_resistanceBands[v];
            if (InBoundingBox(rec, data::cursorUIPos))
            {
                DrawRectangleIRect(rec, WIPBLUE);
                DrawRectangleIRect(ExpandIRect(rec, -2), color);
                const char* text;
                if (data::gatePick == Gate::LED)
                    text = TextFormat(data::deviceParameterTextFmt, Node::GetColorName(v));
                else
                    text = TextFormat(data::deviceParameterTextFmt, v);
                DrawText(text, 20 + 32, 17 + rec.y, 8, WHITE);
            }
            else
                DrawRectangleIRect(rec, color);
            rec.y += 16;
        }
    }
    break;
    }
}