#include <thread>
#include <unordered_set>
#include <fstream>
#include <queue>
#include <stack>
#include "HUtility.h"
#include "Blueprint.h"
#include "Graph.h"
#include "Tab.h"
#include "Mode.h"
#include "Window.h"

extern Blueprint nativeBlueprints[10];

Graph::Graph(Tab* owner, const std::string& name) : owningTab(owner), name(name)
{
    blueprints.reserve(_countof(nativeBlueprints));
    for (const Blueprint& bp : nativeBlueprints)
    {
        blueprints.push_back(new Blueprint(bp));
    }
    Log(LogType::info, "Constructed graph " + name);
}
Graph::~Graph()
{
    _Free();
}

void Graph::Log(LogType type, const std::string& what) const
{
    owningTab->owningWindow->Log(type, "[Graph] " + what);
}

void Graph::_Free()
{
    Log(LogType::info, "Freed graph " + name);
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
    Log(LogType::info, "Created new node");
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
    Log(LogType::info, "Cleared references to node");
    orderDirty = true;
}
void Graph::_DestroyNode(Node* node)
{
    FindAndErase_ExpectExisting(nodes, node);
    FindAndErase(startNodes, node);
    delete node;
    Log(LogType::info, "Destroyed node");
    orderDirty = true;
}

Wire* Graph::_CreateWire(Wire&& base)
{
    Wire* wire = new Wire(base);
    wires.push_back(wire);
    Log(LogType::info, "Created wire");
    return wire;
}
void Graph::_ClearWireReferences(Wire* wire)
{
    wire->start->RemoveWire_Expected(wire);
    wire->end->RemoveWire_Expected(wire);
    // Push end to start nodes if this has destroyed its last remaining input
    if (wire->end->IsOutputOnly())
        startNodes.push_back(wire->end);
    Log(LogType::info, "Cleared references to wire");
    orderDirty = true;
}
void Graph::_DestroyWire(Wire* wire)
{
    FindAndErase_ExpectExisting(wires, wire);
    delete wire;
    Log(LogType::info, "Destroyed wire");
    orderDirty = true;
}


bool Graph::IsOrderDirty() const
{
    return orderDirty;
}

const std::string& Graph::GetName() const
{
    return name;
}

const decltype(Graph::startNodes)& Graph::GetStartNodes() const
{
    return startNodes;
}

// Node functions

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
    Log(LogType::attempt, "Simple bypass");
    if (!node->IsSpecialErasable())
    {
        Log(LogType::warning, "Node not bypassable");
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
    Log(LogType::success, "Simple bypass complete");
}

void Graph::BypassNode_Complex(Node* node)
{
    Log(LogType::attempt, "Complex bypass");
    if (!node->IsComplexBipassable())
    {
        Log(LogType::error, "Node is not complex bypassable");
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
    Log(LogType::success, "Complex bypass complete");
}

Node* Graph::MergeNodes(Node* depricating, Node* overriding)
{
    Log(LogType::attempt, "Node merge");
    if (!depricating || !overriding)
    {
        Log(LogType::error, "Cannot merge with null");
        exit(1);
    }
    if (depricating == overriding)
    {
        Log(LogType::error, "Cannot merge with self");
        exit(1);
    }

    // Hack: resistance being used as generic ntd data
    Node* c = CreateNode(depricating->GetPosition(), overriding->GetGate(), overriding->m_ntd.r.resistance);

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

    Log(LogType::success, "Node merge complete");
    return c;
}

// Wire functions

// CreateWire can affect the positions of parameter `end` in `nodes`
Wire* Graph::CreateWire(Node* start, Node* end, ElbowConfig elbowConfig)
{
    Log(LogType::attempt, "Wiring nodes");
    if (!start || !end)
    {
        Log(LogType::error, "Cannot wire to null");
        exit(1);
    }
    if (start == end)
    {
        Log(LogType::error, "Cannot wire to self");
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
    Log(LogType::success, "Wire complete");
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
    Log(LogType::info, "Swapped nodes");
    std::swap(a->m_gate, b->m_gate);
}
// Invalidates input wire!
Wire* Graph::ReverseWire(Wire* wire)
{
    Log(LogType::attempt, "Reversing wire");
    if (!wire)
    {
        Log(LogType::error, "Cannot reverse null");
        exit(1);
    }
    if (!wire->start)
    {
        Log(LogType::error, "Wire start was null");
        exit(1);
    }
    if (!wire->end)
    {
        Log(LogType::error, "Wire end was null");
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
    Log(LogType::success, "Wire reversal complete");
    return wire;
}
// Invalidates input wire! (obviously; it's being split in two)
std::pair<Wire*, Wire*> Graph::BisectWire(Wire* wire, Node* bisector)
{
    Log(LogType::attempt, "Wire bisection");
    std::pair<Wire*, Wire*> newWire;

    newWire.first = CreateWire(wire->start, bisector);
    newWire.first->elbowConfig = wire->elbowConfig;
    newWire.first->UpdateElbowToLegal();

    newWire.second = CreateWire(bisector, wire->end);
    newWire.second->elbowConfig = wire->elbowConfig;
    newWire.second->UpdateElbowToLegal();

    DestroyWire(wire);
    Log(LogType::success, "Wire bisection complete");
    return newWire;
}

Group* Graph::CreateGroup(IRect rec, Color color)
{
    Group* group = new Group(rec, color, "Label");
    groups.push_back(group);
    Log(LogType::info, "Created group");
    return group;
}
void Graph::DestroyGroup(Group* group)
{
    FindAndErase_ExpectExisting(groups, group);
    delete group;
    Log(LogType::info, "Destroyed group");
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
void Graph::FindNodesInGroup(_Out_ std::vector<Node*>& result, Group* group) const
{
    FindNodesInRect(result, group->captureBounds);
}


// Uses BFS
void Graph::Sort()
{
    Log(LogType::attempt, "Sorting graph");
    decltype(nodes) sorted;
    sorted.reserve(nodes.size());

    {
        std::stack<size_t> toErase;
        for (size_t i = 0; i < startNodes.size(); ++i)
        {
            if (!startNodes[i]->IsOutputOnly() && startNodes[i]->GetGate() != Gate::BATTERY)
                toErase.push(i);
        }
        while (!toErase.empty())
        {
            startNodes.erase(startNodes.begin() + toErase.top());
            toErase.pop();
        }
    }

    std::queue<Node*> list;
    std::unordered_set<Node*> visited;
    for (Node* node : startNodes)
    {
        list.push(node);
        visited.insert(node);
    }
    // All batteries are start nodes
    for (Node* node : startNodes)
    {
        if (node->GetGate() != Gate::BATTERY || visited.find(node) != visited.end())
            continue;
        list.push(node);
        visited.insert(node);
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
            for (Wire* wire : current->m_wires)
            {
                Node* next = wire->end;
                if (next == current || !nodeIsUnvisited(next))
                    continue;

                visited.insert(next);
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
        visited.insert(firstUnvisitedNode);
    }

    nodes.swap(sorted);

    Log(LogType::success, "Graph sort complete");
    orderDirty = false;
}

void Graph::EvaluateNode(Node* node)
{
    if (node->IsPassthrough())
        return void(node->m_state = (*node->GetInputs().begin())->GetState());

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
        Log(LogType::error,
            "Missing eval specialization for encountered node; node type is " +
            std::to_string((int)node->m_gate) +
            TextFormat(" (\'%c\')", (char)node->m_gate));
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
    constexpr int nodeRadius = (int)Node::g_nodeRadius;
    for (Node* node : nodes)
    {
        if (node->GetGate() == Gate::LED && owningTab->owningWindow->GetBaseMode() == Mode::INTERACT) [[unlikely]]
        {
            if (node->GetState())
            {
                DrawRectangle(
                    node->GetX() - nodeRadius - 1,
                    node->GetY() - nodeRadius - 1,
                    nodeRadius * 2 + 2,
                    nodeRadius * 2 + 2,
                    Node::g_resistanceBands[node->GetColorIndex()]
                );
            }
            else
            {
                DrawRectangle(
                    node->GetX() - nodeRadius - 1,
                    node->GetY() - nodeRadius - 1,
                    nodeRadius * 2 + 2,
                    nodeRadius * 2 + 2,
                    BLACK
                );
            }
        }
        else [[likely]]
            node->Draw(node->GetState() ? colorActive : colorInactive, UIColor(UIColorID::UI_COLOR_BACKGROUND), colorInactive);
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

void Graph::FindNodesInRect(_Out_ std::vector<Node*>& result, IRect rec) const
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
    Log(LogType::success, "Stored blueprint " + copy->name);
}
void Graph::SpawnBlueprint(Blueprint* bp, IVec2 topLeft)
{
    Log(LogType::attempt, "Spawning blueprint " + bp->name);
    std::unordered_map<size_t, Node*> nodeID;
    nodes.reserve(nodes.size() + bp->nodes.size());
    for (size_t i = 0; i < bp->nodes.size(); ++i)
    {
        Node* node = _CreateNode(Node(bp->nodes[i].name.c_str(), bp->nodes[i].relativePosition + topLeft, bp->nodes[i].gate, bp->nodes[i].extraParam));
        nodeID.emplace(i, node);
    }
    wires.reserve(wires.size() + bp->wires.size());
    for (const WireBP& wire_bp : bp->wires)
    {
        Node* start;
        {
            auto it = nodeID.find(wire_bp.startNodeIndex);
            if (it == nodeID.end())
            {
                Log(LogType::error, "Malformed node ID in blueprint");
                exit(1);
            }
            start = it->second;
        }
        Node* end;
        {
            auto it = nodeID.find(wire_bp.endNodeIndex);
            if (it == nodeID.end())
            {
                Log(LogType::error, "Malformed node ID in blueprint");
                exit(1);
            }
            end = it->second;
        }
        Wire* wire = CreateWire(start, end, wire_bp.elbowConfig);
    }
    Log(LogType::success, "Spawned blueprint " + bp->name);
}

const std::vector<Blueprint*>& Graph::GetBlueprints() const
{
    return blueprints;
}

void Graph::Save(const std::string& filename) const
{
    Log(LogType::attempt, "Saving file " + filename);
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
        file << "1.3\n";

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
            if (node->HasName())
            {
                file << ' ' << node->GetName();
                Log(LogType::info, "Stored named node " + node->GetName());
            }
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
    Log(LogType::success, "Save complete");
}

void Graph::Load(const std::string& filename)
{
    Log(LogType::success, "Loading file " + filename);

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

    struct FNodeData { char gate{}; IVec2 pos{}; int extra{}; std::string name{""}; };
    auto initNodes = [](std::vector<Node*>& nodes, const FNodeData* nodeData)
    {
        for (size_t i = 0; i < nodes.size(); ++i)
        {
            *nodes[i] = Node(nodeData[i].name.c_str(), nodeData[i].pos, (Gate)nodeData[i].gate, (uint8_t)nodeData[i].extra);
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
        if (version > 1.3 || version < 1.1)
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
            if (file.peek() != '\n')
            {
                file.ignore(1); // Ignore delimiting space
                std::getline(file, nodeData[i].name);
                Log(LogType::info, "Loaded named node " + nodeData[i].name);
            }
        }

        file.ignore(64, 'w');
        size_t wireCount;
        file >> wireCount;

        FWireData* wireData = new FWireData[wireCount];

        for (size_t i = 0; i < wireCount; ++i)
        {
            file >> wireData[i].config >> wireData[i].startID >> wireData[i].endID;
        }

        if (version >= 1.2) // Version 1.2 feature
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
        if (nodes.size() != nodeCount)
        {
            Log(LogType::error, "Node array size mismatch");
            exit(1);
        }
        std::thread initNodesThread(initNodes, std::ref(nodes), nodeData);

        allocWiresThread.join(); // Wires rely on node pointers to start initialization
        if (wires.size() != wireCount)
        {
            Log(LogType::error, "Wire array size mismatch");
            exit(1);
        }
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

    Log(LogType::success, "Load complete");
}

void Graph::Export(const std::string& filename) const
{
    Log(LogType::attempt, "Exporting SVG " + filename);

    if (nodes.empty())
    {
        Log(LogType::warning, "Nothing to export");
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
            file << "  <use href=\"" << id << "\" x=\"" << x << "\" y=\"" << y << '\"';
            if (node->HasName())
                file << "><title>" << node->GetName() << "</title></use>\n";
            else
                file << "/>\n";
        }
        file << "</svg>";
    }
    file.close();

    Log(LogType::success, "Export complete");
}
