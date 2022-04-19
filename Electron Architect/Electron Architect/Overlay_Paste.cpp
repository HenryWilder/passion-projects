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

Overlay_Paste::Overlay_Paste()
{
    // Start of mode
}
Overlay_Paste::~Overlay_Paste()
{
    // End of mode
}

void Overlay_Paste::Update()
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            NodeWorld::Get().SpawnBlueprint(data.clipboard, data.cursorPos);
        data.ClearSelection();
        data.ClearOverlayMode();
    }
}
void Overlay_Paste::Draw()
{
    NodeWorld::Get().DrawWires();
    NodeWorld::Get().DrawNodes();

    data.clipboard->DrawPreview(data.cursorPos, ColorAlpha(LIFELESSNEBULA, 0.5f), HAUNTINGWHITE);
}
