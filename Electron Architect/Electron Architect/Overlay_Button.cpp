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

const Mode Overlay_Button::dropdownModeOrder[] = {
        Mode::PEN,
        Mode::EDIT,
        Mode::ERASE,
        Mode::INTERACT,
};
const Gate Overlay_Button::dropdownGateOrder[] = {
    Gate::OR,
    Gate::AND,
    Gate::NOR,
    Gate::XOR,

    Gate::RESISTOR,
    Gate::CAPACITOR,
    Gate::LED,
    Gate::DELAY,
};
const IRect Overlay_Button::dropdownBounds[] = {
    IRect(0, 16, 16, 16 * (_countof(dropdownModeOrder) - 1)), // Mode
    IRect(16, 16, 16, 16 * (_countof(dropdownGateOrder) - 1)), // Gate
    IRect(32, 16, 16, 16 * (_countof(Node::g_resistanceBands) - 1)), // Parameter
};

Overlay_Button::Overlay_Button()
{
    dropdownActive = data::cursorUIPos.x / 16;
}
Overlay_Button::~Overlay_Button()
{
    // End of mode
}

void Overlay_Button::Update()
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        IRect rec = dropdownBounds[dropdownActive];
        if (data::CursorInUIBounds(rec))
        {
            rec.h = 16;

            switch (dropdownActive)
            {
            case 0: // Mode
            {
                for (Mode m : dropdownModeOrder)
                {
                    if (m == data::GetBaseMode())
                        continue;

                    if (data::CursorInUIBounds(rec))
                    {
                        data::SetMode(m);
                        break;
                    }

                    rec.y += 16;
                }
            }
            break;

            case 1: // Gate
            {
                for (Gate g : dropdownGateOrder)
                {
                    if (g == data::gatePick)
                        continue;

                    if (data::CursorInUIBounds(rec))
                    {
                        data::SetGate(g);
                        break;
                    }

                    rec.y += 16;
                }

                data::ClearOverlayMode();
            }
            break;

            case 2: // Resistance
            {
                for (uint8_t v = 0; v < 10; ++v)
                {
                    if (v == data::storedExtraParam)
                        continue;

                    if (data::CursorInUIBounds(rec))
                    {
                        data::storedExtraParam = v;
                        break;
                    }

                    rec.y += 16;
                }

                data::ClearOverlayMode();
            }
            break;
            }
        }
        else
            data::ClearOverlayMode();
    }
    else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    {
        data::ClearOverlayMode();
    }
}
void Overlay_Button::Draw()
{
    NodeWorld::Get().DrawWires();
    NodeWorld::Get().DrawNodes();

    data::SetMode2D(false);

    IRect rec = dropdownBounds[dropdownActive];
    DrawRectangleIRect(rec, SPACEGRAY);
    rec.h = 16;

    switch (dropdownActive)
    {
    case 0: // Mode
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

    case 1: // Gate
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

    case 2: // Resistance
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