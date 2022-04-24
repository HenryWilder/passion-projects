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

Menu_Select::Menu_Select()
{
    hovering = -1;
}
Menu_Select::~Menu_Select()
{
    // End of mode
}

void Menu_Select::Update()
{

}
void Menu_Select::Draw()
{
    DrawText("[ @TODO make blueprint selection screen ]\nPress Esc to return to circuit graph.", 4, 4, 8, WHITE);
}
