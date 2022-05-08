#include <thread>
#include <unordered_set>
#include <fstream>
#include "HUtility.h"
#include "Blueprint.h"
#include "Graph.h"
#include "Tab.h"
#include "Window.h"

extern Blueprint nativeBlueprints[10];

Graph::Graph(Tab* owner, const char* name) : owningTab(owner), name(name)
{
    blueprints.reserve(_countof(nativeBlueprints));
    for (const Blueprint& bp : nativeBlueprints)
    {
        blueprints.push_back(new Blueprint(bp));
    }
}
Graph::~Graph()
{
    _Free();
}

void Graph::Log(const char* what) const
{
    owningTab->owningWindow->Log(what);
}
void Graph::LogMessage(const char* what) const
{
    owningTab->owningWindow->LogMessage(what);
}
void Graph::LogAttempt(const char* what) const
{
    owningTab->owningWindow->LogAttempt(what);
}
void Graph::LogError(const char* what) const
{
    owningTab->owningWindow->LogError(what);
}
void Graph::LogSuccess(const char* what) const
{
    owningTab->owningWindow->LogSuccess(what);
}

void Graph::_Free()
{
    LogMessage(TextFormat("Freed graph 0x%p \"%s\"", this, name));
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

void Graph::_Clear()
{
    _Free();
}

Node* Graph::_CreateNode(Node&& base)
{
    Node* node = new Node(base);
    nodes.insert(nodes.begin(), node);
    startNodes.push_back(node);
    LogMessage(TextFormat("Created new node 0x%p", node));
    return node;
}
void Graph::_ClearNodeReferences(Node* node)
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
    LogMessage(TextFormat("Cleared references to node 0x%p", node));
    orderDirty = true;
}
void Graph::_DestroyNode(Node* node)
{
    FindAndErase_ExpectExisting(nodes, node);
    FindAndErase(startNodes, node);
    delete node;
    LogMessage(TextFormat("Deleted node 0x%p", node));
    orderDirty = true;
}

Wire* Graph::_CreateWire(Wire&& base)
{
    Wire* wire = new Wire(base);
    wires.push_back(wire);
    LogMessage(TextFormat("Created wire 0x%p", wire));
    return wire;
}
void Graph::_ClearWireReferences(Wire* wire)
{
    wire->start->RemoveWire_Expected(wire);
    wire->end->RemoveWire_Expected(wire);
    // Push end to start nodes if this has destroyed its last remaining input
    if (wire->end->IsOutputOnly())
        startNodes.push_back(wire->end);
    LogMessage(TextFormat("Cleared references to wire 0x%p", wire));
    orderDirty = true;
}
void Graph::_DestroyWire(Wire* wire)
{
    FindAndErase_ExpectExisting(wires, wire);
    delete wire;
    LogMessage(TextFormat("Deleted wire 0x%p", wire));
    orderDirty = true;
}


bool Graph::IsOrderDirty() const
{
    return orderDirty;
}

const char* Graph::GetName() const
{
    return name;
}

const decltype(Graph::startNodes)& Graph::GetStartNodes() const
{
    return startNodes;
}

// Node functions

/// <summary>CreateNode does not insert at the end of the <see cref="nodes"/>.</summary>
Node* Graph::CreateNode(IVec2 position, Gate gate, uint8_t extendedParam)
{
    // The order is not dirty at this time due to the node having no connections yet
    return _CreateNode(Node(position, gate, extendedParam));
}
void Graph::DestroyNode(Node* node)
{
    _ClearNodeReferences(node);
    _DestroyNode(node);
    orderDirty = true;
}

size_t Graph::NodeID(Node* node)
{
    return std::find(nodes.begin(), nodes.end(), node) - nodes.begin();
}

size_t Graph::StartNodeID(Node* node)
{
    return std::find(startNodes.begin(), startNodes.end(), node) - startNodes.begin();
}

void Graph::DestroyNodes(std::vector<Node*>& removeList)
{
    // Todo...
#if 0
    std::unordered_set<Node*> internals(removeList.begin(), removeList.end());
    std::unordered_set<Wire*> uniqueWires;

    auto aLambda = [&]()
    {
        for (Node* node : removeList)
        {
            for (Wire* wire : node->GetWires())
            {
                uniqueWires.insert(wire);
            }
        }
    };
    auto bLambda = [&]()
    {
        for (Node* node : removeList)
        {
            for (Wire* wire : node->GetWires())
            {
                uniqueWires.insert(wire);
            }
            for (Wire* wire : node->GetInputs())
            {
                if (internals.find(wire->start) == internals.end())
                    wire->start->RemoveWire_Expected(wire);
            }
            for (Wire* wire : node->GetOutputs())
            {
                if (internals.find(wire->end) == internals.end())
                    wire->end->RemoveWire_Expected(wire);
            }
        }
    };
    auto thread1Lambda = [&]()
    {
        std::stable_partition(nodes.begin(), nodes.end(), [&internals](Node* node) { return internals.find(node) != internals.end(); });
        while (internals.find(nodes.back()) != internals.end())
        {
            delete nodes.back();
            nodes.pop_back();
        }
    };
    auto thread2Lambda = [&]()
    {
        std::stable_partition(startNodes.begin(), startNodes.end(), [&internals](Node* node) { return internals.find(node) != internals.end(); });
        while (internals.find(startNodes.back()) != internals.end())
        {
            delete startNodes.back();
            startNodes.pop_back();
        }
    };
    auto thread3Lambda = [&]()
    {
        std::stable_partition(wires.begin(), wires.end(), [&uniqueWires](Wire* wire) { return uniqueWires.find(wire) != uniqueWires.end(); });
        while (uniqueWires.find(wires.back()) != uniqueWires.end())
        {
            delete wires.back();
            wires.pop_back();
        }
    };

    std::thread a(aLambda);
    std::thread b(bLambda);
    a.join();
    b.join();
    std::thread thread1(thread1Lambda, std::cref(internals), std::ref(nodes));
    std::thread thread2(thread2Lambda, std::cref(internals), std::ref(startNodes));
    std::thread thread3(thread3Lambda, std::cref(uniqueWires), std::ref(wires));
    thread1.join();
    thread2.join();
    thread3.join();
#else
    for (Node* node : removeList)
    {
        DestroyNode(node);
    }
#endif
    orderDirty = true;
}

void Graph::BypassNode(Node* node)
{
    LogAttempt(TextFormat("Simple bypass on node 0x%p", node));
    if (!node->IsSpecialErasable())
    {
        LogError(TextFormat("Node 0x%p is not bypassable.", node));
        return;
    }
    // Multiple outputs, one input
    if (node->GetInputCount() == 1)
    {
        Wire* input = *(node->GetInputs().begin()); // Get the first (only) input
        for (Wire* wire : node->GetOutputs())
        {
            CreateWire(input->start, wire->end, input->elbowConfig);
        }
    }
    // Multiple inputs, one output
    else // OutputCount() == 1
    {
        Wire* output = *(node->GetOutputs().begin()); // Get the first (only) input
        for (Wire* wire : node->GetInputs())
        {
            CreateWire(wire->start, output->end, wire->elbowConfig);
        }
    }
    DestroyNode(node);
    LogSuccess(TextFormat("Simple bypassed node 0x%p", node));
}

void Graph::BypassNode_Complex(Node* node)
{
    LogAttempt(TextFormat("Complex bypass on node 0x%p", node));
    if (!node->IsComplexBipassable())
    {
        LogError(TextFormat("Node 0x%p is not bypassable.", node));
        return;
    }

    for (Wire* input : node->GetInputs())
    {
        for (Wire* output : node->GetOutputs())
        {
            CreateWire(input->start, output->end, input->elbowConfig);
        }
    }
    DestroyNode(node);
    LogSuccess(TextFormat("Complex bypassed node 0x%p", node));
}

Node* Graph::MergeNodes(Node* depricating, Node* overriding)
{
    LogAttempt(TextFormat("Merge node 0x%p with 0x%p", depricating, overriding));
    if (!depricating)
    {
        LogError(TextFormat("Cannot merge 0x%p with nullptr.", overriding));
        exit(1);
    }
    if (!overriding)
    {
        LogError(TextFormat("Cannot merge 0x%p with nullptr.", depricating));
        exit(1);
    }
    if (depricating == overriding)
    {
        LogError(TextFormat("Cannot merge 0x%p with itself.", depricating));
        exit(1);
    }

    Node* c = CreateNode(depricating->GetPosition(), overriding->GetGate(), overriding->m_ntd.r.resistance); // Generic ntd data

    for (Wire* wire : depricating->GetInputs())
    {
        if (wire->start == overriding) continue;
        CreateWire(wire->start, c, wire->elbowConfig);
    }
    for (Wire* wire : overriding->GetInputs())
    {
        if (wire->start == depricating) continue;
        CreateWire(wire->start, c, wire->elbowConfig);
    }
    for (Wire* wire : depricating->GetOutputs())
    {
        if (wire->end == overriding) continue;
        CreateWire(c, wire->end, wire->elbowConfig);
    }
    for (Wire* wire : overriding->GetOutputs())
    {
        if (wire->end == depricating) continue;
        CreateWire(c, wire->end, wire->elbowConfig);
    }

    DestroyNode(depricating);
    DestroyNode(overriding);

    LogSuccess(TextFormat("Merged nodes 0x%p and 0x%p into node 0x%p", depricating, overriding, c));
    return c;
}

// Wire functions

// CreateWire can affect the positions of parameter `end` in `nodes`
Wire* Graph::CreateWire(Node* start, Node* end)
{
    LogAttempt(TextFormat("Wire node 0x%p to node 0x%p...", start, end));
    if (!start)
    {
        LogError(TextFormat("Wire failed. Cannot wire node 0x%p to nullptr.", end));
        exit(1);
    }
    if (!end)
    {
        LogError(TextFormat("Cannot wire node 0x%p to nullptr.", start));
        exit(1);
    }
    if (start == end)
    {
        LogError(TextFormat("Cannot wire node 0x%p to itself.", start));
        exit(1);
    }

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
    LogSuccess(TextFormat("Wired node 0x%p to node 0x%p with wire 0x%p", start, end, wire));
    return wire;
}
// CreateWire can affect the positions of parameter `end` in `nodes`
Wire* Graph::CreateWire(Node* start, Node* end, ElbowConfig elbowConfig)
{
    LogAttempt(TextFormat("Wire node 0x%p to node 0x%p", start, end));
    if (!start)
    {
        LogError(TextFormat("Cannot wire node 0x%p to nullptr.", end));
        exit(1);
    }
    if (!end)
    {
        LogError(TextFormat("Cannot wire node 0x%p to nullptr.", start));
        exit(1);
    }
    if (start == end)
    {
        LogError(TextFormat("Cannot wire node 0x%p to itself.", start));
        exit(1);
    }

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
    LogSuccess(TextFormat("Wired node 0x%p to node 0x%p with wire 0x%p", start, end, wire));
    return wire;
}
void Graph::DestroyWire(Wire* wire)
{
    _ClearWireReferences(wire);
    _DestroyWire(wire);

    orderDirty = true;
}
void Graph::SwapNodes(Node* a, Node* b)
{
    LogMessage(TextFormat("Swapped type of node 0x%p with node 0x%p", a, b));
    std::swap(a->m_gate, b->m_gate);
}
// Invalidates input wire!
Wire* Graph::ReverseWire(Wire* wire)
{
    LogAttempt(TextFormat("Reverse wire 0x%p", wire));
    if (!wire)
    {
        LogError("Cannot reverse nullptr.");
        exit(1);
    }
    if (!wire->start)
    {
        LogError("Wire start was nullptr.");
        exit(1);
    }
    if (!wire->end)
    {
        LogError("Wire end was nullptr.");
        exit(1);
    }

    // Swap
    Node* tbStart = wire->end;
    Node* tbEnd = wire->start;
    IVec2 elbow = wire->elbow;
    DestroyWire(wire);
    wire = CreateWire(tbStart, tbEnd);
    wire->SnapElbowToLegal(elbow);
    orderDirty = true;
    LogSuccess(TextFormat("Reversed wire 0x%p", wire));
    return wire;
}
// Invalidates input wire! (obviously; it's being split in two)
std::pair<Wire*, Wire*> Graph::BisectWire(Wire* wire, Node* bisector)
{
    LogAttempt(TextFormat("Bisect wire 0x%p with node 0x%p", wire, bisector));
    std::pair<Wire*, Wire*> newWire;

    newWire.first = CreateWire(wire->start, bisector);
    newWire.first->elbowConfig = wire->elbowConfig;
    newWire.first->UpdateElbowToLegal();

    newWire.second = CreateWire(bisector, wire->end);
    newWire.second->elbowConfig = wire->elbowConfig;
    newWire.second->UpdateElbowToLegal();

    DestroyWire(wire);
    LogSuccess(TextFormat("Bisected wire 0x%p into the wires 0x%p and 0x%p", wire, newWire.first, newWire.second));
    return newWire;
}

Group* Graph::CreateGroup(IRect rec, Color color)
{
    Group* group = new Group(rec, color, "Label");
    groups.push_back(group);
    LogMessage(TextFormat("Created new group 0x%p", group));
    return group;
}
void Graph::DestroyGroup(Group* group)
{
    FindAndErase_ExpectExisting(groups, group);
    delete group;
    LogMessage(TextFormat("Deleted group 0x%p", group));
}
Group* Graph::FindGroupAtPos(IVec2 pos) const
{
    for (Group* group : groups)
    {
        if (InBoundingBox(group->labelBounds, pos))
            return group;
    }
    return nullptr;
}
GroupCorner Graph::FindGroupCornerAtPos(IVec2 pos) const
{
    for (Group* g : groups)
    {
        if (!InBoundingBox(g->GetCaptureBounds(), pos))
            continue;
        IRect corners[4];
        g->GetResizeCollisions(corners);
        for (uint8_t i = 0; i < 4; ++i)
        {
            if (InBoundingBox(corners[i], pos))
                return { g, i };
        }
    }
    return { nullptr, 0 };
}
void Graph::FindNodesInGroup(std::vector<Node*>& result, Group* group) const
{
    FindNodesInRect(result, group->captureBounds);
}


// Uses BFS
void Graph::Sort()
{
    LogMessage(TextFormat("Sorting node graph 0x%p", this));
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

    LogSuccess("Sort finished");
    orderDirty = false;
}

void Graph::EvaluateNode(Node* node)
{
    switch (node->m_gate)
    {
    case Gate::LED:
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

    case Gate::DELAY:
        node->m_state = node->m_ntd.d.lastState;
        for (Wire* wire : node->GetInputs())
        {
            if (wire->GetState())
                return void(node->m_ntd.d.lastState = true);
        }
        node->m_ntd.d.lastState = false;
        break;

    case Gate::BATTERY:
        node->m_state = true;
        break;

    default:
        LogError("Missing specialization for selected gate evaluation");
        exit(1);
        break;
    }
}

void Graph::Evaluate()
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

void Graph::DrawWires(Color colorActive, Color colorInactive) const
{
    for (Wire* wire : wires)
    {
        wire->Draw(wire->start->GetState() ? colorActive : colorInactive);
    }
}
void Graph::DrawNodes(Color colorActive, Color colorInactive) const
{
    for (Node* node : nodes)
    {
        node->Draw(node->GetState() ? colorActive : colorInactive);
    }
}
void Graph::DrawGroups() const
{
    for (Group* group : groups)
    {
        group->Draw();
    }
}

Node* Graph::FindNodeAtPos(IVec2 pos) const
{
    for (Node* node : nodes)
    {
        if (node->GetPosition() == pos)
            return node;
    }
    return nullptr;
}
Wire* Graph::FindWireAtPos(IVec2 pos) const
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
Wire* Graph::FindWireElbowAtPos(IVec2 pos) const
{
    auto it = std::find_if(wires.begin(), wires.end(), [&pos](Wire* wire) { return wire->elbow == pos; });
    if (it != wires.end())
        return *it;
    return nullptr;
}

void Graph::FindNodesInRect(std::vector<Node*>& result, IRect rec) const
{
    // Exclusive bounds
    IRect bounds = ShrinkIRect(rec);
    for (Node* node : nodes)
    {
        if (InBoundingBox(bounds, node->GetPosition()))
            result.push_back(node);
    }
}


void Graph::StoreBlueprint(Blueprint* bp)
{
    Blueprint* copy = new Blueprint(*bp);
    for (Blueprint* existing : blueprints)
    {
        if (existing->name == copy->name)
        {
            copy->name.append(" (1)");
        }
    }
    blueprints.push_back(copy);
    LogMessage(TextFormat("Stored blueprint 0x%p \"%s\" to position 0x%p [index %u] (now \"%s\")", bp, bp->name, copy, blueprints.size() - 1, copy->name));
}
void Graph::SpawnBlueprint(Blueprint* bp, IVec2 topLeft)
{
    LogAttempt(TextFormat("Spawning blueprint 0x%p", bp));
    std::unordered_map<size_t, Node*> nodeID;
    nodes.reserve(nodes.size() + bp->nodes.size());
    for (size_t i = 0; i < bp->nodes.size(); ++i)
    {
        Node* node = CreateNode(bp->nodes[i].relativePosition + topLeft, bp->nodes[i].gate, bp->nodes[i].extraParam);
        nodeID.emplace(i, node);
    }
    wires.reserve(wires.size() + bp->wires.size());
    for (const WireBP& wire_bp : bp->wires)
    {
        Node* start;
        Node* end;
        {
            auto it = nodeID.find(wire_bp.startNodeIndex);
            _ASSERT_EXPR(it != nodeID.end(), L"Malformed nodeID");
            start = it->second;
        }
        {
            auto it = nodeID.find(wire_bp.endNodeIndex);
            _ASSERT_EXPR(it != nodeID.end(), L"Malformed nodeID");
            end = it->second;
        }
        Wire* wire = CreateWire(start, end, wire_bp.elbowConfig);
    }
    LogSuccess(TextFormat("Spawned blueprint 0x%p", bp));
}

const std::vector<Blueprint*>& Graph::GetBlueprints() const
{
    return blueprints;
}

void Graph::Save(const char* filename) const
{
    LogAttempt(TextFormat("Saving file \"%s\"", filename));
    auto prepNodeIDs = [](std::unordered_map<Node*, size_t>& nodeIDs, const std::vector<Node*>& nodes)
    {
        nodeIDs.reserve(nodes.size());
        for (size_t i = 0; i < nodes.size(); ++i)
        {
            nodeIDs.insert({ nodes[i], i });
        }
    };

    auto prepWireIDs = [](std::unordered_map<Wire*, size_t>& wireIDs, const std::vector<Wire*>& wires)
    {
        wireIDs.reserve(wires.size());
        for (size_t i = 0; i < wires.size(); ++i)
        {
            wireIDs.insert({ wires[i], i });
        }
    };

    std::ofstream file(filename, std::fstream::out | std::fstream::trunc);
    {
        file << "1.2\n";

        std::unordered_map<Node*, size_t> nodeIDs;
        std::unordered_map<Wire*, size_t> wireIDs;
        std::thread a(prepNodeIDs, std::ref(nodeIDs), std::cref(nodes));
        std::thread b(prepWireIDs, std::ref(wireIDs), std::cref(wires));
        a.join();
        b.join();

        // Nodes
        file << TextFormat("n %i\n", nodes.size());
        for (Node* node : nodes)
        {
            file << TextFormat("%c %i %i", (char)node->m_gate, node->GetX(), node->GetY());
            if (node->GetGate() == Gate::RESISTOR)
                file << TextFormat(" %i", node->GetResistance());
            else if (node->GetGate() == Gate::LED)
                file << TextFormat(" %i", node->GetColorIndex());
            else if (node->GetGate() == Gate::CAPACITOR)
                file << TextFormat(" %i", node->GetCapacity());
            file << '\n';
        }

        // Wires
        file << TextFormat("w %i\n", wires.size());
        for (Wire* wire : wires)
        {
            file << TextFormat("%i %i %i\n", wire->elbowConfig, nodeIDs.find(wire->start)->second, nodeIDs.find(wire->end)->second);
        }

        // Groups
        file << TextFormat("g %i\n", groups.size());
        for (Group* group : groups)
        {
            file << TextFormat("%s %i %i %i %i %i %i %i %i\n",
                group->label.c_str(),
                group->captureBounds.x, group->captureBounds.y, group->captureBounds.w, group->captureBounds.h,
                (int)group->color.r, (int)group->color.g, (int)group->color.b, (int)group->color.a);
        }
    }
    file.close();
    LogSuccess(TextFormat("Saved file \"%s\"", filename));
}

void Graph::Load(const char* filename)
{
    LogAttempt(TextFormat("Loading file \"%s\"", filename));

    // HACK: Does not conform to standard _CreateNode/Wire functions!!
    auto allocNodes = [](std::vector<Node*>& nodes, size_t nodeCount)
    {
        nodes.reserve(nodeCount);
        for (size_t i = 0; i < nodeCount; ++i)
        {
            nodes.push_back(new Node);
        }
    };

    auto allocWires = [](std::vector<Wire*>& wires, size_t wireCount)
    {
        wires.reserve(wireCount);
        for (size_t i = 0; i < wireCount; ++i)
        {
            wires.push_back(new Wire);
        }
    };

    struct FNodeData { char gate{}; IVec2 pos{}; int extra{}; };
    auto initNodes = [](std::vector<Node*>& nodes, const FNodeData* nodeData)
    {
        for (size_t i = 0; i < nodes.size(); ++i)
        {
            *nodes[i] = Node(nodeData[i].pos, (Gate)nodeData[i].gate, (uint8_t)nodeData[i].extra);
        }
    };

    struct FWireData { int config; size_t startID, endID; };
    auto initWires = [](const std::vector<Node*>& nodes, std::vector<Wire*>& wires, const FWireData* wireData)
    {
        for (size_t i = 0; i < wires.size(); ++i)
        {
            *wires[i] = Wire(nodes[wireData[i].startID], nodes[wireData[i].endID], (ElbowConfig)(uint8_t)wireData[i].config);
        }
    };

    std::ifstream file(filename, std::fstream::in);
    {
        double version;
        file >> version;
        if (version != 1.2 && version != 1.1)
            return;

        // Unload existing
        for (Node* node : nodes)
        {
            DestroyNode(node);
        }
        for (Wire* wire : wires)
        {
            DestroyWire(wire);
        }
        nodes.clear();
        wires.clear();
        // Todo: groups?

        file.ignore(64, 'n');
        size_t nodeCount;
        file >> nodeCount;

        FNodeData* nodeData = new FNodeData[nodeCount]; // Living a little dangerous here...

        for (size_t i = 0; i < nodeCount; ++i)
        {
            file >> nodeData[i].gate >> nodeData[i].pos.x >> nodeData[i].pos.y;
            if ((Gate)nodeData[i].gate == Gate::RESISTOR || (Gate)nodeData[i].gate == Gate::LED || (Gate)nodeData[i].gate == Gate::CAPACITOR)
                file >> nodeData[i].extra;
        }

        file.ignore(64, 'w');
        size_t wireCount;
        file >> wireCount;

        FWireData* wireData = new FWireData[wireCount];

        for (size_t i = 0; i < wireCount; ++i)
        {
            file >> wireData[i].config >> wireData[i].startID >> wireData[i].endID;
        }

        if (version == 1.2) // Version 1.2 feature
        {
            file.ignore(64, 'g');
            size_t groupCount;
            file >> groupCount;
            groups.reserve(groupCount);
            for (size_t i = 0; i < groupCount; ++i)
            {
                int x, y, w, h;
                int r, g, b, a;
                std::string label;
                file
                    >> label
                    >> x >> y >> w >> h
                    >> r >> g >> b >> a;
                groups.push_back(new Group(IRect(x, y, w, h), Color((uint8_t)r, (uint8_t)g, (uint8_t)b, (uint8_t)a), label));
            }
        }

        std::thread allocNodesThread(allocNodes, std::ref(nodes), nodeCount);
        std::thread allocWiresThread(allocWires, std::ref(wires), wireCount);

        allocNodesThread.join(); // Nodes do not rely on wire pointers to start initialization
        _ASSERT_EXPR(nodes.size() == nodeCount, L"Node array size mismatch!");
        std::thread initNodesThread(initNodes, std::ref(nodes), nodeData);

        allocWiresThread.join(); // Wires rely on node pointers to start initialization
        _ASSERT_EXPR(wires.size() == wireCount, L"Wire array size mismatch!");
        std::thread initWiresThread(initWires, std::cref(nodes), std::ref(wires), wireData);

        initNodesThread.join();
        delete[] nodeData;
        initWiresThread.join();
        delete[] wireData;
        
        for (Wire* wire : wires)
        {
            wire->start->AddWireOutput(wire);
            wire->end->AddWireInput(wire);
            wire->UpdateElbowToLegal();
        }

        orderDirty = true;
    }
    file.close();

    LogSuccess(TextFormat("Loaded file \"%s\"", filename));
}

void Graph::Export(const char* filename) const
{
    LogAttempt(TextFormat("Exporting SVG \"%s\"", filename));

    if (nodes.empty())
    {
        LogMessage("Nothing to export.");
        return;
    }

    std::ofstream file(filename, std::fstream::out | std::fstream::trunc);
    {
        constexpr int r = (int)Node::g_nodeRadius;
        constexpr int w = r * 2;

        int minx = INT_MAX;
        int miny = INT_MAX;
        int maxx = INT_MIN;
        int maxy = INT_MIN;

        // Get extents
        for (Node* node : nodes)
        {
            if      (node->GetX() < minx) minx = node->GetX();
            else if (node->GetX() > maxx) maxx = node->GetX();
            if      (node->GetY() < miny) miny = node->GetY();
            else if (node->GetY() > maxy) maxy = node->GetY();
        }

        file << "<svg viewbox=\"" << TextFormat("%i %i %i %i", minx, miny, maxx, maxy) << "\" xmlns=\"http://www.w3.org/2000/svg\">\n";

        bool ORs  = false;
        bool ANDs = false;
        bool NORs = false;
        bool XORs = false;
        bool RESs = false;
        bool CAPs = false;
        bool LEDs = false;
        bool DELs = false;
        bool BATs = false;

        for (Node* node : nodes)
        {
            switch (node->GetGate())
            {
            case Gate::OR:        ORs  = true; break;
            case Gate::AND:       ANDs = true; break;
            case Gate::NOR:       NORs = true; break;
            case Gate::XOR:       XORs = true; break;
            case Gate::RESISTOR:  RESs = true; break;
            case Gate::CAPACITOR: CAPs = true; break;
            case Gate::LED:       LEDs = true; break;
            case Gate::DELAY:     DELs = true; break;
            case Gate::BATTERY:   BATs = true; break;
            }
            if (ORs && ANDs && NORs && XORs && RESs && CAPs && LEDs && DELs && BATs)
                break;
        }

        file << "  <defs>\n"; // As long as there's at least one node on the graph we're sure to have a def.
        if (ORs)
        {
            file <<
                "    <!-- reusable 'OR' gate shape -->\n"
                "    <g id=\"gate_or\">\n"
                "      <circle cx=\"0\" cy=\"0\" r=\"" << r << "\" stroke=\"none\" stroke-width=\"0\" fill=\"black\" />\n"
                "    </g>\n";
        }
        if (ANDs)
        {
            file <<
                "    <!-- reusable 'AND' gate shape -->\n"
                "    <g id=\"gate_and\">\n"
                "      <rect x=\"" << -r << "\" y=\"" << -r << "\" width=\"" << w << "\" height=\"" << w << "\" stroke=\"none\" stroke-width=\"0\" fill=\"black\" />\n"
                "    </g>\n";
        }
        if (NORs)
        {
            file <<
                "    <!-- reusable 'NOR' gate shape -->\n"
                "    <g id=\"gate_nor\">\n"
                "      <circle cx=\"0\" cy=\"0\" r=\"" << r << "\" stroke=\"black\" stroke-width=\"1\" fill=\"white\" />\n"
                "    </g>\n";
        }
        if (XORs)
        {
            file <<
                "    <!-- reusable 'XOR' gate shape -->\n"
                "    <g id=\"gate_xor\">\n"
                "      <circle cx=\"0\" cy=\"0\" r=\"" << r << "\" stroke=\"black\" stroke-width=\"1\" fill=\"white\" />\n"
                "      <circle cx=\"0\" cy=\"0\" r=\"" << r << "\" stroke=\"none\" stroke-width=\"0\" fill=\"black\" />\n"
                "    </g>\n";
        }
        if (RESs)
        {
            file <<
                "    <!-- reusable resistor shape -->\n"
                "    <g id=\"gate_res\">\n"
                "      <rect x=\"" << -r << "\" y=\"" << -r << "\" width=\"" << w << "\" height=\"" << w << "\" stroke=\"black\" stroke-width=\"1\" fill=\"white\" />\n"
                "    </g>\n";
        }
        if (CAPs)
        {
            file <<
                "    <!-- reusable capacitor shape -->\n"
                "    <g id=\"gate_cap\">\n"
                "      <rect x=\"" << -r << "\" y=\"" << -r << "\" width=\"" << w << "\" height=\"" << w << "\" stroke=\"black\" stroke-width=\"1\" fill=\"white\" />\n"
                "      <rect x=\"" << -r + 1 << "\" y=\"" << -r + 1 << "\" width=\"" << w - 2 << "\" height=\"" << w - 2 << "\" stroke=\"none\" stroke-width=\"0\" fill=\"black\" />\n"
                "    </g>\n";
        }
        if (LEDs)
        {
            static const float v[] =
            {
                0,
                r * 1.5f,
                r * sinf(2 * PI / 3) * 1.25f,
                r * cosf(2 * PI / 3) * 1.25f,
                r * sinf(4 * PI / 3) * 1.25f,
                r * cosf(4 * PI / 3) * 1.25f
            };
            file <<
                "    <!-- reusable LED shape -->\n"
                "    <g id=\"gate_led\">\n"
                "      <polygon points=\"" << TextFormat("%f,%f %f,%f %f,%f", v[0],v[1], v[2],v[3], v[4],v[5]) << "\" stroke=\"black\" stroke-width=\"1\" fill=\"black\" />\n"
                "    </g>\n";
        }
        if (DELs)
        {
            file <<
                "    <!-- reusable delay shape -->\n"
                "    <g id=\"gate_del\">\n"
                "      <rect x=\"" << -r << "\" y=\"" << -r << "\" width=\"" << w << "\" height=\"" << w << "\" stroke=\"black\" stroke-width=\"1\" fill=\"white\" />\n"
                "      <line x1=\"" << -r << "\" y1=\"" << 0 << "\" x2=\"" << r << "\" y2=\"" << 0 << "\" stroke=\"black\" stroke-width=\"1\" fill=\"none\" />\n"
                "    </g>\n";
        }
        if (BATs)
        {
            file <<
                "    <!-- reusable battery shape -->\n"
                "    <g id=\"gate_bat\">\n"
                "      <rect x=\"" << -r << "\" y=\"" << -r << "\" width=\"" << w << "\" height=\"" << w << "\" stroke=\"black\" stroke-width=\"1\" fill=\"white\" />\n"
                "      <rect x=\"" << -r << "\" y=\"" << -r << "\" width=\"" << w << "\" height=\"" << w / 2 << "\" stroke=\"none\" stroke-width=\"0\" fill=\"black\" />\n"
                "    </g>\n";
        }
        file << "  </defs>\n";

        if (!wires.empty())
        {
            file <<
                "  <!-- Wires (drawn first so they don't obscure nodes) -->\n"
                "  <g style=\"stroke:black; stroke-width:1; fill:none;\">\n"; // Style applied to all wires
            for (Wire* wire : wires)
            {
                int x1 = wire->GetStartX();
                int x2 = wire->GetElbowX();
                int x3 = wire->GetEndX();
                int y1 = wire->GetStartY();
                int y2 = wire->GetElbowY();
                int y3 = wire->GetEndY();
                file << "    <path d=\"M" << x1 << ',' << y1 << " L" << x2 << ',' << y2 << " L" << x3 << ',' << y3 << "\" />\n";
            }
            file << "  </g>\n";
        }
        file << "  <!-- Nodes (reuse shapes defined in header <def> section) -->\n";
        for (Node* node : nodes)
        {
            const char* id;
            int x = node->GetX();
            int y = node->GetY();
            switch (node->GetGate())
            {
            case Gate::OR:        id = "#gate_or";  break;
            case Gate::AND:       id = "#gate_and"; break;
            case Gate::NOR:       id = "#gate_nor"; break;
            case Gate::XOR:       id = "#gate_xor"; break;
            case Gate::RESISTOR:  id = "#gate_res"; break;
            case Gate::CAPACITOR: id = "#gate_cap"; break;
            case Gate::LED:       id = "#gate_led"; break;
            case Gate::DELAY:     id = "#gate_del"; break;
            case Gate::BATTERY:   id = "#gate_bat"; break;
            default: _ASSERT_EXPR(false, L"No SVG export specialization for selected gate"); id = ""; break;
            }
            file << "  <use href=\"" << id << "\" x=\"" << x <<"\" y=\"" << y << "\" />\n";
        }
        file << "</svg>";
    }
    file.close();

    LogSuccess(TextFormat("Exported SVG \"%s\"", filename));
}
