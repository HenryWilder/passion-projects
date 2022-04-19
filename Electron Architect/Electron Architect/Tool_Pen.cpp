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

void Tool_Pen::CreateBulkNodes()
{
    dragStart = data.cursorPos;
    for (size_t i = 0; i < bulkNodeCount; ++i)
    {
        nodesMadeByDragging[i] = NodeWorld::Get().CreateNode(data.cursorPos, data.gatePick, data.storedExtraParam);
    }
    bulkNodesBeingMade = true;
}
void Tool_Pen::UpdateBulkNodes()
{
    size_t dist = IntGridDistance(dragStart, data.cursorPos);
    if (dist != 0)
    {
        IVec2 normal = (data.cursorPos - dragStart) / (std::min(bulkNodeCount, dist) - 1);
        normal /= g_gridSize;
        normal *= g_gridSize;
        for (uint8_t i = 0; i < std::min(bulkNodeCount, dist); ++i) // todo: figure out wtf this is doing. I kinda just did it, and it works, but idfk why.
        {
            nodesMadeByDragging[i]->SetPosition_Temporary(dragStart + normal * i);
        }
    }
}
void Tool_Pen::DestroyBulkNodes()
{
    int start = std::min(IntGridDistance(dragStart, data.cursorPos), 8);
    for (int i = start; i < bulkNodeCount; ++i)
    {
        NodeWorld::Get().DestroyNode(nodesMadeByDragging[i]);
    }
    bulkNodesBeingMade = false;
}

void Tool_Pen::OnMouseMove()
{
    data.hoveredNode = NodeWorld::Get().FindNodeAtPos(data.cursorPos);
    data.hoveredWire = !data.hoveredNode ? NodeWorld::Get().FindWireAtPos(data.cursorPos) : nullptr;

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && bulkNodesBeingMade)
        UpdateBulkNodes();
}

Node* Tool_Pen::CreateNode()
{
    Node* node = NodeWorld::Get().CreateNode(data.cursorPos, data.gatePick, data.storedExtraParam);
    if (!!data.hoveredWire)
    {
        NodeWorld::Get().BisectWire(data.hoveredWire, node);
        data.hoveredWire = nullptr;
    }
}

void Tool_Pen::FinishWire(Node* wireEnd)
{
    Node* oldNode = currentWireStart;

    if (!!previousWireStart && (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)))
        oldNode = previousWireStart;

    if (oldNode == wireEnd)
        return;
    
    Wire* wire = NodeWorld::Get().CreateWire(oldNode, wireEnd);
    wire->elbowConfig = currentWireElbowConfig;
    wire->UpdateElbowToLegal();
    previousWireStart = oldNode;
}

void Tool_Pen::OnLeftClick()
{
    if (!data.hoveredWire && !data.hoveredNode)
        CreateBulkNodes();

    Node* newNode = data.hoveredNode;

    if (!newNode)
        CreateNode();

    // Do not create a new node/wire if already hovering the start node
    if (!!currentWireStart && newNode != currentWireStart)
        FinishWire(newNode);

    currentWireStart = newNode;
}

void Tool_Pen::CycleElbow()
{
    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
        --currentWireElbowConfig;
    else
        ++currentWireElbowConfig;
}

void Tool_Pen::CancelWire()
{
    previousWireStart = currentWireStart = nullptr;
}

void Tool_Pen::OnLeftRelease()
{
    DestroyBulkNodes(); // todo: what? why?
}

void Tool_Pen::Update()
{
    if (data.b_cursorMoved)
        OnMouseMove();

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        OnLeftClick();

    else if (IsKeyPressed(KEY_R))
        CycleElbow();

    else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        CancelWire();

    else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        OnLeftRelease();
}


void Tool_Pen::DrawCurrentWire()
{
    IVec2 start;
    if (!!previousWireStart && (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)))
        start = previousWireStart->GetPosition();
    else
        start = currentWireStart->GetPosition();

    IVec2 elbow;
    IVec2 end = data.cursorPos;
    elbow = Wire::GetLegalElbowPosition(start, end, currentWireElbowConfig);
    Wire::Draw(start, elbow, end, WIPBLUE);
    Node::Draw(end, data.gatePick, WIPBLUE);
}

void Tool_Pen::DrawHoveredWire()
{
    data.hoveredWire->Draw(GOLD);
}

void Tool_Pen::DrawHoveredNodeWires()
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

void Tool_Pen::DrawHoveredNode()
{
    data.hoveredNode->Draw(WIPBLUE);
}

void Tool_Pen::Draw()
{
    NodeWorld::Get().DrawWires();

    if (!!currentWireStart)
        DrawCurrentWire();

    if (!!data.hoveredWire)
        DrawHoveredWire();

    if (!!data.hoveredNode)
        DrawHoveredNodeWires();

    NodeWorld::Get().DrawNodes();

    if (!!data.hoveredNode)
        DrawHoveredNode();
}
