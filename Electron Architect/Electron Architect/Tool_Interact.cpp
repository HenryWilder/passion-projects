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

Tool_Interact::Tool_Interact()
{
    // Start of mode
}
Tool_Interact::~Tool_Interact()
{
    // End of mode
}

void Tool_Interact::Update()
{
    data.hoveredNode = NodeWorld::Get().FindNodeAtPos(data.cursorPos);
    if (!!data.hoveredNode && !data.hoveredNode->IsOutputOnly())
        data.hoveredNode = nullptr;

    if (!!data.hoveredNode && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        data.hoveredNode->SetGate(data.hoveredNode->GetGate() == Gate::NOR ? Gate::OR : Gate::NOR);
}
void Tool_Interact::Draw()
{
    NodeWorld::Get().DrawWires();
    NodeWorld::Get().DrawNodes();

    for (const Node* node : NodeWorld::Get().GetStartNodes())
    {
        node->Draw(WIPBLUE);
    }

    if (!!data.hoveredNode)
    {
        data.hoveredNode->Draw(CAUTIONYELLOW);
    }
}