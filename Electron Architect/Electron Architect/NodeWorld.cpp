#include "HUtility.h"
#include "NodeWorld.h"

NodeWorld::NodeWorld() = default;
NodeWorld::~NodeWorld()
{
    for (Node* node : nodes)
    {
        delete node;
    }
    for (Wire* wire : wires)
    {
        delete wire;
    }
    for (Blueprint* bp : blueprints)
    {
        delete bp;
    }
    for (Group* group : groups)
    {
        delete group;
    }
}

Node* NodeWorld::_CreateNode(Node&& base)
{
    Node* node = new Node(base);
    nodes.insert(nodes.begin(), node);
    startNodes.push_back(node);
    return node;
}
void NodeWorld::_ClearNodeReferences(Node* node)
{
    for (Wire* input : node->GetInputs())
    {
        input->start->RemoveWire_Expected(input);
        _DestroyWire(input);
    }
    for (Wire* output : node->GetOutputs())
    {
        output->end->RemoveWire_Expected(output);
        _DestroyWire(output);
    }
    orderDirty = true;
}
void NodeWorld::_DestroyNode(Node* node)
{
    FindAndErase_ExpectExisting(nodes, node);
    FindAndErase(startNodes, node);
    delete node;
    orderDirty = true;
}

Wire* NodeWorld::_CreateWire(Wire&& base)
{
    Wire* wire = new Wire(base);
    wires.push_back(wire);
    return wire;
}
void NodeWorld::_ClearWireReferences(Wire* wire)
{
    wire->start->RemoveWire_Expected(wire);
    wire->end->RemoveWire_Expected(wire);
    // Push end to start nodes if this has destroyed its last remaining input
    if (wire->end->IsOutputOnly())
        startNodes.push_back(wire->end);
    orderDirty = true;
}
void NodeWorld::_DestroyWire(Wire* wire)
{
    FindAndErase_ExpectExisting(wires, wire);
    delete wire;
    orderDirty = true;
}


NodeWorld& NodeWorld::Get()
{
    static NodeWorld g_only;
    return g_only;
}

const decltype(NodeWorld::startNodes)& NodeWorld::GetStartNodes() const
{
    return startNodes;
}

// Node functions

/// <summary>CreateNode does not insert at the end of the <see cref="nodes"/>.</summary>
Node* NodeWorld::CreateNode(IVec2 position, Gate gate, uint8_t extendedParam = 0)
{
    // The order is not dirty at this time due to the node having no connections yet
    return _CreateNode(Node(position, gate, extendedParam));
}
void NodeWorld::DestroyNode(Node* node)
{
    _ClearNodeReferences(node);
    _DestroyNode(node);
    orderDirty = true;
}

// Wire functions

// CreateWire can affect the positions of parameter `end` in `nodes`
Wire* NodeWorld::CreateWire(Node* start, Node* end)
{
    _ASSERT_EXPR(start != nullptr && end != nullptr, "Tried to create a wire to nullptr");
    _ASSERT_EXPR(start != end, "Cannot create self-reference wire");

    // Duplicate guard
    {
        auto it = end->FindConnection(start);
        if (end->IsValidConnection(it))
        {
            Wire* wire = *it;
            if (start == wire->start) // Wire already matches
                return wire;
            else if (start == wire->end) // Wire is reverse of existing
                return ReverseWire(wire);
        }
    }

    Wire* wire = _CreateWire(Wire(start, end));

    start->AddWireOutput(wire);
    end->AddWireInput(wire);

    // Remove end from start nodes, as it is no longer an inputless node with this change
    FindAndErase(startNodes, end);

    orderDirty = true;
    return wire;
}
// CreateWire can affect the positions of parameter `end` in `nodes`
Wire* NodeWorld::CreateWire(Node* start, Node* end, ElbowConfig elbowConfig)
{
    _ASSERT_EXPR(start != nullptr && end != nullptr, "Tried to create a wire to nullptr");
    _ASSERT_EXPR(start != end, "Cannot create self-reference wire");

    // Duplicate guard
    {
        auto it = end->FindConnection(start);
        if (end->IsValidConnection(it))
        {
            Wire* wire = *it;
            if (start == wire->start) // Wire already matches
                return wire;
            else if (start == wire->end) // Wire is reverse of existing
                return ReverseWire(wire);
        }
    }

    Wire* wire = _CreateWire(Wire(start, end, elbowConfig));

    start->AddWireOutput(wire);
    end->AddWireInput(wire);

    // Remove end from start nodes, as it is no longer an inputless node with this change
    FindAndErase(startNodes, end);

    orderDirty = true;
    return wire;
}
void NodeWorld::DestroyWire(Wire* wire)
{
    _ClearWireReferences(wire);
    _DestroyWire(wire);

    orderDirty = true;
}
Node* NodeWorld::MergeNodes(Node* composite, Node* tbRemoved)
{
    _ASSERT_EXPR(!!composite && !!tbRemoved, "Tried to merge a node with nullptr");
    _ASSERT_EXPR(composite != tbRemoved, "Tried to merge a node with itself");

    for (Wire* wire : tbRemoved->m_wires)
    {
        if (wire->start == composite || wire->end == composite)
            continue;

        if (wire->start == tbRemoved)
            CreateWire(composite, wire->end);
        else // wire->end == tbRemoved
            CreateWire(wire->start, composite);
    }
    IVec2 newPos = tbRemoved->m_position;
    DestroyNode(tbRemoved);
    orderDirty = true;
    return composite;
}
// Invalidates input wire!
Wire* NodeWorld::ReverseWire(Wire* wire)
{
    _ASSERT_EXPR(wire != nullptr, "Cannot reverse null wire");
    _ASSERT_EXPR(wire->start != nullptr && wire->end != nullptr, "Malformed wire");
    // Swap
    Node* tbStart = wire->end;
    Node* tbEnd = wire->start;
    IVec2 elbow = wire->elbow;
    DestroyWire(wire);
    wire = CreateWire(tbStart, tbEnd);
    wire->SnapElbowToLegal(elbow);
    orderDirty = true;
    return wire;
}
// Invalidates input wire! (obviously; it's being split in two)
std::pair<Wire*, Wire*> NodeWorld::BisectWire(Wire* wire, Node* bisector)
{
    std::pair<Wire*, Wire*> newWire;

    newWire.first = CreateWire(wire->start, bisector);
    newWire.first->elbowConfig = wire->elbowConfig;
    newWire.first->UpdateElbowToLegal();

    newWire.second = CreateWire(bisector, wire->end);
    newWire.second->elbowConfig = wire->elbowConfig;
    newWire.second->UpdateElbowToLegal();

    DestroyWire(wire);
    return newWire;
}

Group* NodeWorld::CreateGroup(IRect rec)
{
    Group* group = new Group(rec, WIPBLUE, "Label");
    groups.push_back(group);
    return group;
}
void NodeWorld::DestroyGroup(Group* group)
{
    FindAndErase_ExpectExisting(groups, group);
    delete group;
}
Group* NodeWorld::FindGroupAtPos(IVec2 pos) const
{
    for (Group* group : groups)
    {
        if (InBoundingBox(group->labelBounds, pos))
            return group;
    }
    return nullptr;
}
void NodeWorld::FindNodesInGroup(std::vector<Node*>& result, Group* group) const
{
    FindNodesInRect(result, group->captureBounds);
}


// Uses BFS
void NodeWorld::Sort()
{
    decltype(nodes) sorted;
    sorted.reserve(nodes.size());

    std::queue<Node*> list;
    std::unordered_map<Node*, size_t> visited; // Pointer, depth
    for (Node* node : startNodes)
    {
        list.push(node);
        visited.insert({ node, 0 });
    }

    auto nodeIsUnvisited = [&visited](Node* node)
    {
        return visited.find(node) == visited.end();
    };

    while (true)
    {
        while (!list.empty())
        {
            Node* current = list.front();
            size_t nextDepth = visited.find(current)->second + 1;
            for (Wire* wire : current->m_wires)
            {
                Node* next = wire->end;
                if (next == current || !nodeIsUnvisited(next))
                    continue;

                visited.insert({ next, nextDepth });
                list.push(next);
            }
            sorted.push_back(current);
            list.pop();
        }


        auto it = std::find_if(nodes.begin(), nodes.end(), nodeIsUnvisited);
        if (it == nodes.end())
            break;

        Node* firstUnvisitedNode = *it;
        startNodes.push_back(firstUnvisitedNode);
        list.push(firstUnvisitedNode);
        visited.insert({ firstUnvisitedNode, 0 });
    }

    nodes.swap(sorted);

    orderDirty = false;
}

void NodeWorld::EvaluateNode(Node* node)
{
    switch (node->m_gate)
    {
    case Gate::OR:
        for (Wire* wire : node->GetInputs())
        {
            if (wire->GetState())
                return void(node->m_state = true);
        }
        node->m_state = false;
        break;

    case Gate::NOR:
        for (Wire* wire : node->GetInputs())
        {
            if (wire->GetState())
                return void(node->m_state = false);
        }
        node->m_state = true;
        break;

    case Gate::AND:
        for (Wire* wire : node->GetInputs())
        {
            if (!wire->GetState())
                return void(node->m_state = false);
        }
        node->m_state = !!node->GetInputCount();
        break;

    case Gate::XOR:
        node->m_state = false;
        for (Wire* wire : node->GetInputs())
        {
            if (wire->GetState() && !(node->m_state = !node->m_state))
                return;
        }
        break;

    case Gate::RESISTOR:
    {
        int activeInputs = 0;
        for (Wire* wire : node->GetInputs())
        {
            if (wire->GetState() && !!(++activeInputs > node->GetResistance()))
                return void(node->m_state = true);
        }
        node->m_state = false;
    }
    break;

    case Gate::CAPACITOR:
        node->m_state = true;
        for (Wire* wire : node->GetInputs())
        {
            if (wire->GetState())
                return (node->GetCharge() < node->GetCapacity()) ? node->IncrementCharge() : void();
        }
        if (node->GetCharge())
            return node->DecrementCharge();
        node->m_state = false;
        break;
    }
}

void NodeWorld::Evaluate()
{
    if (orderDirty)
    {
        Sort();
        orderDirty = false;
    }

    for (Node* node : nodes)
    {
        EvaluateNode(node);
    }
}

void NodeWorld::DrawWires() const
{
    for (Wire* wire : wires)
    {
        wire->Draw(wire->start->GetState() ? Wire::g_wireColorActive : Wire::g_wireColorInactive);
    }
}
void NodeWorld::DrawNodes() const
{
    for (Node* node : nodes)
    {
        node->Draw(node->GetState() ? node->g_nodeColorActive : node->g_nodeColorInactive);
    }
}
void NodeWorld::DrawGroups() const
{
    for (Group* group : groups)
    {
        group->Draw();
    }
}

Node* NodeWorld::FindNodeAtPos(IVec2 pos) const
{
    for (Node* node : nodes)
    {
        if (node->GetPosition() == pos)
            return node;
    }
    return nullptr;
}
Wire* NodeWorld::FindWireAtPos(IVec2 pos) const
{
    for (Wire* wire : wires)
    {
        if (CheckCollisionIVecPointLine(pos, wire->GetStartPos(), wire->GetElbowPos()) ||
            CheckCollisionIVecPointLine(pos, wire->GetElbowPos(), wire->GetEndPos()))
        {
            return wire;
        }
    }
    return nullptr;
}
Wire* NodeWorld::FindWireElbowAtPos(IVec2 pos) const
{
    auto it = std::find_if(wires.begin(), wires.end(), [&pos](Wire* wire) { return wire->elbow == pos; });
    if (it != wires.end())
        return *it;
    return nullptr;
}

void NodeWorld::FindNodesInRect(std::vector<Node*>& result, IRect rec) const
{
    for (Node* node : nodes)
    {
        if (InBoundingBox(rec, node->GetPosition()))
            result.push_back(node);
    }
}


void NodeWorld::SpawnBlueprint(Blueprint* bp, IVec2 topLeft)
{
    std::unordered_map<size_t, Node*> nodeID;
    nodes.reserve(nodes.size() + bp->nodes.size());
    for (size_t i = 0; i < bp->nodes.size(); ++i)
    {
        Node* node = CreateNode(bp->nodes[i].relativePosition + topLeft, bp->nodes[i].gate);
        nodeID.emplace(i, node);
    }
    wires.reserve(wires.size() + bp->wires.size());
    for (const WireBP& wire_bp : bp->wires)
    {
        Node* start;
        Node* end;
        {
            auto it = nodeID.find(wire_bp.startNodeIndex);
            _ASSERT_EXPR(it != nodeID.end(), "Malformed nodeID");
            start = it->second;
        }
        {
            auto it = nodeID.find(wire_bp.endNodeIndex);
            _ASSERT_EXPR(it != nodeID.end(), "Malformed nodeID");
            end = it->second;
        }
        Wire* wire = CreateWire(start, end, wire_bp.elbowConfig);
    }
}
void NodeWorld::StoreBlueprint(Blueprint* bp)
{
    blueprints.push_back(bp);
}

void NodeWorld::Save(const char* filename) const
{
    std::ofstream file(filename, std::fstream::out | std::fstream::trunc);
    {
        file << "0\n" << std::time(nullptr) << '\n';

        {
            std::unordered_map<Node*, size_t> nodeIDs;
            nodeIDs.reserve(nodes.size());
            for (size_t i = 0; i < nodes.size(); ++i)
            {
                nodeIDs.insert({ nodes[i], i });
            }

            std::unordered_map<Wire*, size_t> wireIDs;
            wireIDs.reserve(wires.size());
            for (size_t i = 0; i < wires.size(); ++i)
            {
                wireIDs.insert({ wires[i], i });
            }

            file << "n " << nodes.size() << '\n';
            for (Node* node : nodes)
            {
                file <<
                    (char)node->m_gate << ' ' <<
                    node->m_position.x << ' ' << node->m_position.y;
                for (Wire* wire : node->m_wires)
                {
                    size_t wireID = wireIDs.find(wire)->second;
                    file << ' ' << wireID;
                }
                file << '\n';
            }

            file << "w " << wires.size() << '\n';
            for (Wire* wire : wires)
            {
                size_t startID = nodeIDs.find(wire->start)->second;
                size_t endID = nodeIDs.find(wire->end)->second;
                file << (int)wire->elbowConfig << ' ' << startID << ' ' << endID << '\n';
            }
        }
    }
    file.close();
}

// Reads both large and small
// TODO
void NodeWorld::Load(const char* filename)
{
    std::ifstream file(filename, std::fstream::in);
    {
        int version;
        char relSize;
        time_t time;
        file >> version;
        if (version != 0) return;
        file >> relSize >> time;


    }
    file.close();
}
