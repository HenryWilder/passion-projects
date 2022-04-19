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

Tool_Pen::Tool_Pen()
{
    // Start of mode
}
Tool_Pen::~Tool_Pen()
{
    // End of mode
}

void Tool_Pen::Update()
{
    if (data.b_cursorMoved) // On move
    {
        data.hoveredWire = nullptr;
        data.hoveredNode = NodeWorld::Get().FindNodeAtPos(data.cursorPos);
        if (!data.hoveredNode)
            data.hoveredWire = NodeWorld::Get().FindWireAtPos(data.cursorPos);

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && data.Pen_NodesMadeByDragging().size() != 0)
        {
            uint8_t dist = IntGridDistance(data.Pen_DragStart(), data.cursorPos);
            if (dist != 0)
            {
                IVec2 normal = (data.cursorPos - data.Pen_DragStart()) / (std::min(data.Pen_NodesMadeByDragging().size(), dist) - 1);
                normal /= g_gridSize;
                normal *= g_gridSize;
                for (uint8_t i = 0; i < std::min(data.Pen_NodesMadeByDragging().size(), dist); ++i)
                {
                    data.Pen_NodesMadeByDragging()[i]->SetPosition_Temporary(data.Pen_DragStart() + normal * i);
                }
            }
        }
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if (!data.hoveredWire && !data.hoveredNode)
        {
            data.Pen_DragStart() = data.cursorPos;
            for (size_t i = 0; i < data.Pen_NodesMadeByDragging().capacity(); ++i)
            {
                data.Pen_NodesMadeByDragging().push(NodeWorld::Get().CreateNode(data.cursorPos, data.gatePick, data.storedExtraParam));
            }
        }

        Node* newNode = data.hoveredNode;
        if (!newNode)
        {
            newNode = NodeWorld::Get().CreateNode(data.cursorPos, data.gatePick, data.storedExtraParam);
            if (!!data.hoveredWire)
            {
                NodeWorld::Get().BisectWire(data.hoveredWire, newNode);
                data.hoveredWire = nullptr;
            }
        }
        // Do not create a new node/wire if already hovering the start node
        if (!!data.Pen_CurrentWireStart() && newNode != data.Pen_CurrentWireStart())
        {
            Node* oldNode;
            if (!!data.Pen_PreviousWireStart() && (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)))
                oldNode = data.Pen_PreviousWireStart();
            else
                oldNode = data.Pen_CurrentWireStart();

            if (oldNode != newNode)
            {
                Wire* wire = NodeWorld::Get().CreateWire(oldNode, newNode);
                wire->elbowConfig = data.Pen_CurrentWireElbowConfig();
                wire->UpdateElbowToLegal();
                data.Pen_PreviousWireStart() = oldNode;
            }
        }
        data.Pen_CurrentWireStart() = newNode;
    }
    else if (IsKeyPressed(KEY_R))
    {
        if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
            --data.Pen_CurrentWireElbowConfig();
        else
            ++data.Pen_CurrentWireElbowConfig();
    }
    else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    {
        data.Pen_PreviousWireStart() = data.Pen_CurrentWireStart() = nullptr;
    }
    else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
    {
        int start = std::min(IntGridDistance(data.Pen_DragStart(), data.cursorPos), 8);
        for (int i = start; i < data.Pen_NodesMadeByDragging().size(); ++i)
        {
            NodeWorld::Get().DestroyNode(data.Pen_NodesMadeByDragging()[i]);
        }
        data.Pen_NodesMadeByDragging().clear();
    }
}
void Tool_Pen::Draw()
{
    NodeWorld::Get().DrawWires();

    if (!!data.Pen_CurrentWireStart())
    {
        IVec2 start;
        if (!!data.Pen_PreviousWireStart() && (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)))
            start = data.Pen_PreviousWireStart()->GetPosition();
        else
            start = data.Pen_CurrentWireStart()->GetPosition();
        IVec2 elbow;
        IVec2 end = data.cursorPos;
        elbow = Wire::GetLegalElbowPosition(start, end, data.Pen_CurrentWireElbowConfig());
        Wire::Draw(start, elbow, end, WIPBLUE);
        Node::Draw(end, data.gatePick, WIPBLUE);
    }

    if (!!data.hoveredWire)
    {
        data.hoveredWire->Draw(GOLD);
    }

    if (!!data.hoveredNode)
    {
        for (const Wire* wire : data.hoveredNode->GetWires())
        {
            Color color;
            if (wire->start == data.hoveredNode)
                color = OUTPUTAPRICOT; // Output
            else
                color = INPUTLAVENDER; // Input

            wire->Draw(color);
        }
    }

    NodeWorld::Get().DrawNodes();

    if (!!data.hoveredNode)
    {
        data.hoveredNode->Draw(WIPBLUE);
    }
}
