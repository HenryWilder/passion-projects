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
#include "Action.h"

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
    _ASSERT_EXPR(!bulkNodesBeingMade, L"Tried to double-dip node bulk");
    dragStart = data::cursorPos;
    for (size_t i = 0; i < bulkNodeCount; ++i)
    {
        nodesMadeByDragging[i] = NodeWorld::Get().CreateNode(data::cursorPos, data::gatePick, data::storedExtraParam);
    }
    bulkNodesBeingMade = true;
}
void Tool_Pen::UpdateBulkNodes()
{
    _ASSERT_EXPR(bulkNodesBeingMade, L"Cannot update nodes that don't exist");

    int dist = IntGridDistance(dragStart, data::cursorPos);

    if (dist == 0)
        return;

    IVec2 normal = Snap((data::cursorPos - dragStart) / (dist - 1), g_gridSize);
    for (int i = 0; i < bulkNodeCount; ++i)
    {
        nodesMadeByDragging[i]->SetPosition_Silent(dragStart + normal * i);
    }
}
void Tool_Pen::CancelBulkNodes()
{
    _ASSERT_EXPR(bulkNodesBeingMade, L"Cannot cancel nodes not yet made");
    for (Node* bulkNodeElement : nodesMadeByDragging)
    {
        NodeWorld::Get().DestroyNode(bulkNodeElement);
    }
    bulkNodesBeingMade = false;
}

Node* Tool_Pen::CreateNode()
{
    return NodeWorld::Get().CreateNode(data::cursorPos, data::gatePick, data::storedExtraParam);
}
void Tool_Pen::BisectWireWithNode(Node* node)
{
    NodeWorld::Get().BisectWire(data::hoveredWire, node);
    data::hoveredWire = nullptr;
}
void Tool_Pen::FinishWire(Node* wireEnd)
{
    Node* oldNode = currentWireStart;

    if (!!previousWireStart && IsKeyDown_Shift())
        oldNode = previousWireStart;

    if (oldNode == wireEnd)
        return;
    
    Wire* wire = NodeWorld::Get().CreateWire(oldNode, wireEnd);
    wire->elbowConfig = currentWireElbowConfig;
    wire->UpdateElbowToLegal();
    previousWireStart = oldNode;
}

void Tool_Pen::UpdateHover()
{
    data::hoveredNode = NodeWorld::Get().FindNodeAtPos(data::cursorPos);
    data::hoveredWire = !data::hoveredNode ? NodeWorld::Get().FindWireAtPos(data::cursorPos) : nullptr;
}
void Tool_Pen::CycleElbow()
{
    if (IsKeyDown_Shift())
        --currentWireElbowConfig;
    else
        ++currentWireElbowConfig;
}
void Tool_Pen::CancelWire()
{
    previousWireStart = nullptr;
    currentWireStart = nullptr;
}

class Act_BisectWire : public Action
{
    RedoReservation<Wire*> m_prev;
    RedoReservation<Node*> m_bisector;
    std::pair<RedoReservation<Wire*>, RedoReservation<Wire*>> m_post;

    void Undo() override
    {
        _ASSERT_EXPR(false, L"Incomplete feature");
    }
    void Redo() override
    {
        _ASSERT_EXPR(m_prev.IsValid(), L"Wire being bisected is invalid");
        _ASSERT_EXPR(m_bisector.IsValid(), L"Node bisecting wire is invalid");
        m_post = NodeWorld::Get().BisectWire(*m_prev, *m_bisector);
    }
public:
    Act_BisectWire(Wire* wire, Node* bisector) : m_prev(wire), m_bisector(bisector), m_post({ nullptr, nullptr }) {}
};

class Act_DrawNode : public Action
{
    RedoReservation<Node*> m_target;

    void Undo() override
    {
        _ASSERT_EXPR(false, L"Incomplete feature");
    }
    void Redo() override
    {
        _ASSERT_EXPR(false, L"Incomplete feature");
    }
public:
    Act_DrawNode() : m_target(nullptr) {}
};

void Tool_Pen::Update()
{
    // Mouse move
    if (data::b_cursorMoved)
    {
        UpdateHover();

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && bulkNodesBeingMade)
            UpdateBulkNodes();
    }

    // Press m1
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !data::hoveredWire && !data::hoveredNode)
        CreateBulkNodes();

    // Todo: Make the actions work
    if      (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !!data::hoveredNode) // Click an existing node (wire can't be hovered in this case)
        PushAction(Act_DrawNode());

    else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !data::hoveredNode && !data::hoveredWire) // Click nothing (create a new node)
        PushAction(Act_DrawNode());

    if      (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !data::hoveredNode && !!data::hoveredWire) // Click a wire (bisect it)
        PushAction(Act_DrawNode());

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if (!data::hoveredWire && !data::hoveredNode)
            CreateBulkNodes();

        Node* newNode;

        if (!data::hoveredNode)
            newNode = CreateNode();
        else
            newNode = data::hoveredNode;

        if (!!data::hoveredWire) // Hovered nodes take precedence over hovered wires
            BisectWireWithNode(newNode);

        // Do not create a new node/wire if already hovering the start node
        if (!!currentWireStart && newNode != currentWireStart)
            FinishWire(newNode);

        currentWireStart = newNode;
    }

    // Release m1
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && IntGridDistance(dragStart, data::cursorPos) < bulkNodeCount && bulkNodesBeingMade) // Not enough space
        CancelBulkNodes();

    // Press r
    if (IsKeyPressed(KEY_R))
        CycleElbow();

    // Press m2
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        CancelWire();
}


void Tool_Pen::DrawCurrentWire()
{
    IVec2 start;
    if (!!previousWireStart && IsKeyDown_Shift())
        start = previousWireStart->GetPosition();
    else
        start = currentWireStart->GetPosition();

    IVec2 elbow;
    IVec2 end = data::cursorPos;
    elbow = Wire::GetLegalElbowPosition(start, end, currentWireElbowConfig);
    Wire::Draw(start, elbow, end, WIPBLUE);
    Node::Draw(end, data::gatePick, WIPBLUE);
}
void Tool_Pen::DrawHoveredWire()
{
    data::hoveredWire->Draw(GOLD);
}
void Tool_Pen::DrawHoveredNodeWires()
{
    for (const Wire* wire : data::hoveredNode->GetInputs())
    {
        Color color;
        if (wire->start == data::hoveredNode)
            color = OUTPUTAPRICOT; // Output
        else
            color = INPUTLAVENDER; // Input

        wire->Draw(color);
    }
    for (const Wire* wire : data::hoveredNode->GetOutputs())
    {
        Color color;
        if (wire->start == data::hoveredNode)
            color = OUTPUTAPRICOT; // Output
        else
            color = INPUTLAVENDER; // Input

        wire->Draw(color);
    }
}
void Tool_Pen::DrawHoveredNode()
{
    data::hoveredNode->Draw(WIPBLUE);
}

void Tool_Pen::Draw()
{
    NodeWorld::Get().DrawWires();

    if (!!currentWireStart)
        DrawCurrentWire();

    if (!!data::hoveredWire)
        DrawHoveredWire();

    if (!!data::hoveredNode)
        DrawHoveredNodeWires();

    NodeWorld::Get().DrawNodes();

    if (!!data::hoveredNode)
        DrawHoveredNode();
}
