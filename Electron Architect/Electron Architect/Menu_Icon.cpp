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

Menu_Icon::Menu_Icon()
{
    object = new BlueprintIcon;
    draggingIcon = -1;
}
Menu_Icon::~Menu_Icon()
{
    delete object;
}

void Menu_Icon::Update()
{
    if (data.b_cursorMoved && draggingIcon == -1)
    {
        iconID = BlueprintIcon::GetIconAtColRow(BlueprintIcon::PixelToColRow(sheetRec.xy, data.cursorUIPos));
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        if (data.CursorInUIBounds(sheetRec) && iconCount < 4 && !!iconID)
        {
            data.cursorUIPos = pos;
            SetMousePosition(data.cursorUIPos.x + BlueprintIcon::g_size / 2, data.cursorUIPos.y + BlueprintIcon::g_size / 2);
            object->combo[iconCount] = { iconID, 0,0 };
            draggingIcon = iconCount;
            iconCount++;
        }
        else if (data.CursorInUIBounds(IRect(pos, BlueprintIcon::g_size * 2)))
        {
            draggingIcon = -1;
            for (int i = 0; i < iconCount; ++i)
            {
                if (object->combo[i].id == NULL)
                    continue;

                IRect bounds(
                    pos.x,
                    pos.y,
                    BlueprintIcon::g_size,
                    BlueprintIcon::g_size
                );
                bounds.xy = bounds.xy + object->combo[i].Pos();
                if (data.CursorInUIBounds(bounds))
                {
                    draggingIcon = i;
                    iconID = object->combo[i].id;
                    break;
                }
            }
        }
    }
    if ((IsMouseButtonReleased(MOUSE_LEFT_BUTTON) || IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) && draggingIcon != -1)
    {
        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
        {
            if (draggingIcon < 3)
            {
                memcpy(
                    object->combo + draggingIcon,
                    object->combo + draggingIcon + 1,
                    sizeof(IconPos) * (4ull - (size_t)draggingIcon));
            }
            object->combo[3] = { NULL, 0,0 };
            iconCount--;
        }
        draggingIcon = -1;
    }
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && !!iconID)
    {
        constexpr IVec2 centerOffset = IVec2(BlueprintIcon::g_size / 2);
        IVec2 colRow = (data.cursorUIPos - pos - centerOffset) / centerOffset;
        colRow.x = std::min(std::max(colRow.x, 0), 2);
        colRow.y = std::min(std::max(colRow.y, 0), 2);
        object->combo[draggingIcon].x = colRow.x;
        object->combo[draggingIcon].y = colRow.y;
    }
}
void Menu_Icon::Draw()
{
    _ASSERT_EXPR(!!object, L"Blueprint icon object not initialized");
    _ASSERT_EXPR(data.IsClipboardValid(), L"Bad entry into Mode::BP_ICON");

    object->DrawBackground(pos, SPACEGRAY);
    object->Draw(pos, WHITE);

    for (size_t i = 0; i < 4; ++i)
    {
        for (int i = 0; i < 4; ++i)
        {
            if (object->combo[i].id == NULL)
                continue;

            IRect bounds(pos, BlueprintIcon::g_size);
            bounds.xy = bounds.xy + object->combo[i].Pos();
            if (data.CursorInUIBounds(bounds))
                DrawRectangleIRect(bounds, ColorAlpha(WIPBLUE, 0.25f));
        }
    }
    if (draggingIcon != -1)
        BlueprintIcon::DrawBPIcon(iconID, pos + object->combo[draggingIcon].Pos(), WIPBLUE);

    BlueprintIcon::DrawSheet(sheetRec.xy, SPACEGRAY, WHITE);
}

void Menu_Icon::SaveBlueprint()
{
    SetMode(Mode::BP_ICON); // todo: What is this here for??
    object = new BlueprintIcon;
    pos = data.cursorPos - IVec2(BlueprintIcon::g_size / 2, BlueprintIcon::g_size / 2);
    sheetRec.xy = pos + IVec2(BlueprintIcon::g_size * 2, BlueprintIcon::g_size * 2);
    sheetRec.wh = BlueprintIcon::GetSheetSize_Px();
}
