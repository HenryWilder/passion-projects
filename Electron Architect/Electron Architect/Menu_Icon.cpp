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
    // Start of mode
}
Menu_Icon::~Menu_Icon()
{
    // End of mode
}

void Menu_Icon::Update()
{
    if (data.b_cursorMoved && data.BPIcon_DraggingIcon() == -1)
    {
        data.BPIcon_IconID() = BlueprintIcon::GetIconAtColRow(BlueprintIcon::PixelToColRow(data.BPIcon_SheetRec().xy, data.cursorUIPos));
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        if (data.CursorInUIBounds(data.BPIcon_SheetRec()) && data.BPIcon_IconCount() < 4 && !!data.BPIcon_IconID())
        {
            data.cursorUIPos = data.BPIcon_Pos();
            SetMousePosition(data.cursorUIPos.x + BlueprintIcon::g_size / 2, data.cursorUIPos.y + BlueprintIcon::g_size / 2);
            data.BPIcon_Object()->combo[data.BPIcon_IconCount()] = { data.BPIcon_IconID(), 0,0 };
            data.BPIcon_DraggingIcon() = data.BPIcon_IconCount();
            data.BPIcon_IconCount()++;
        }
        else if (data.CursorInUIBounds(IRect(data.BPIcon_Pos(), BlueprintIcon::g_size * 2)))
        {
            data.BPIcon_DraggingIcon() = -1;
            for (int i = 0; i < data.BPIcon_IconCount(); ++i)
            {
                if (data.BPIcon_Object()->combo[i].id == NULL)
                    continue;

                IRect bounds(
                    data.BPIcon_Pos().x,
                    data.BPIcon_Pos().y,
                    BlueprintIcon::g_size,
                    BlueprintIcon::g_size
                );
                bounds.xy = bounds.xy + data.BPIcon_Object()->combo[i].Pos();
                if (data.CursorInUIBounds(bounds))
                {
                    data.BPIcon_DraggingIcon() = i;
                    data.BPIcon_IconID() = data.BPIcon_Object()->combo[i].id;
                    break;
                }
            }
        }
    }
    if ((IsMouseButtonReleased(MOUSE_LEFT_BUTTON) || IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) && data.BPIcon_DraggingIcon() != -1)
    {
        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
        {
            if (data.BPIcon_DraggingIcon() < 3)
            {
                memcpy(
                    data.BPIcon_Object()->combo + data.BPIcon_DraggingIcon(),
                    data.BPIcon_Object()->combo + data.BPIcon_DraggingIcon() + 1,
                    sizeof(IconPos) * (4ull - (size_t)data.BPIcon_DraggingIcon()));
            }
            data.BPIcon_Object()->combo[3] = { NULL, 0,0 };
            data.BPIcon_IconCount()--;
        }
        data.BPIcon_DraggingIcon() = -1;
    }
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && !!data.BPIcon_IconID())
    {
        constexpr IVec2 centerOffset = IVec2(BlueprintIcon::g_size / 2);
        IVec2 colRow = (data.cursorUIPos - data.BPIcon_Pos() - centerOffset) / centerOffset;
        colRow.x = std::min(std::max(colRow.x, 0), 2);
        colRow.y = std::min(std::max(colRow.y, 0), 2);
        data.BPIcon_Object()->combo[data.BPIcon_DraggingIcon()].x = colRow.x;
        data.BPIcon_Object()->combo[data.BPIcon_DraggingIcon()].y = colRow.y;
    }
}
void Menu_Icon::Draw()
{
    _ASSERT_EXPR(!!data.BPIcon_Object(), L"Blueprint icon object not initialized");
    _ASSERT_EXPR(data.IsClipboardValid(), L"Bad entry into Mode::BP_ICON");

    data.BPIcon_Object()->DrawBackground(data.BPIcon_Pos(), SPACEGRAY);
    data.BPIcon_Object()->Draw(data.BPIcon_Pos(), WHITE);

    for (size_t i = 0; i < 4; ++i)
    {
        for (int i = 0; i < 4; ++i)
        {
            if (data.BPIcon_Object()->combo[i].id == NULL)
                continue;

            IRect bounds(data.BPIcon_Pos(), BlueprintIcon::g_size);
            bounds.xy = bounds.xy + data.BPIcon_Object()->combo[i].Pos();
            if (data.CursorInUIBounds(bounds))
                DrawRectangleIRect(bounds, ColorAlpha(WIPBLUE, 0.25f));
        }
    }
    if (data.BPIcon_DraggingIcon() != -1)
        BlueprintIcon::DrawBPIcon(data.BPIcon_IconID(), data.BPIcon_Pos() + data.BPIcon_Object()->combo[data.BPIcon_DraggingIcon()].Pos(), WIPBLUE);

    BlueprintIcon::DrawSheet(data.BPIcon_SheetRec().xy, SPACEGRAY, WHITE);
}
