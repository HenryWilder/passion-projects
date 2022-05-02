#include <thread>
#include <unordered_set>
#include <fstream>
#include "HUtility.h"
#include "NodeWorld.h"
#include "Blueprint.h"

// Built-in blueprints
Blueprint nativeBlueprints[] =
{
    Blueprint(
        "NAND Gate",
        IVec2(1,0) * g_gridSize,
        // Nodes
        {
            NodeBP(true, Gate::AND,  IVec2(0,0) * g_gridSize),
            NodeBP(true, Gate::NOR,  IVec2(1,0) * g_gridSize),
        },
        // Wires
        {
            WireBP(0, 1, ElbowConfig::horizontal),
        }),
    Blueprint(
        "XNOR Gate",
        IVec2(1,0) * g_gridSize,
        // Nodes
        {
            NodeBP(true, Gate::XOR,  IVec2(0,0) * g_gridSize),
            NodeBP(true, Gate::NOR,  IVec2(1,0) * g_gridSize),
        },
        // Wires
        {
            WireBP(0, 1, ElbowConfig::horizontal),
        }),
    Blueprint(
        "Switch",
        IVec2(2,0) * g_gridSize,
        // Nodes
        {
            NodeBP(true, Gate::OR,  IVec2(0,0) * g_gridSize),
            NodeBP(true, Gate::OR,  IVec2(1,0) * g_gridSize),
            NodeBP(true, Gate::AND, IVec2(2,0) * g_gridSize),
        },
        // Wires
        {
            WireBP(0, 2, ElbowConfig::horizontal),
            WireBP(1, 2, ElbowConfig::horizontal),
        }),
    Blueprint(
        "Gated SR Latch",
        IVec2(4,1) * g_gridSize,
        // Nodes
        {
            NodeBP(true,  Gate::NOR, IVec2(0,1) * g_gridSize), // NOR instead of OR to prevent uninitialized-flicker

            NodeBP(true,  Gate::OR,  IVec2(0,0) * g_gridSize),

            NodeBP(false, Gate::OR,  IVec2(1,0) * g_gridSize),
            NodeBP(false, Gate::NOR, IVec2(1,1) * g_gridSize),

            NodeBP(false, Gate::AND, IVec2(2,0) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(2,1) * g_gridSize),

            NodeBP(false, Gate::OR,  IVec2(3,0) * g_gridSize),
            NodeBP(false, Gate::OR,  IVec2(3,1) * g_gridSize),

            NodeBP(false, Gate::NOR, IVec2(4,0) * g_gridSize),
            NodeBP(false, Gate::NOR, IVec2(4,1) * g_gridSize),
        },
        // Wires
        {
            WireBP(0, 2, ElbowConfig::diagonalA),
            WireBP(1, 3, ElbowConfig::diagonalA),
            WireBP(1, 4, ElbowConfig::diagonalA),
            WireBP(2, 4, ElbowConfig::diagonalA),
            WireBP(2, 5, ElbowConfig::diagonalA),
            WireBP(3, 5, ElbowConfig::diagonalA),
            WireBP(4, 6, ElbowConfig::diagonalA),
            WireBP(5, 7, ElbowConfig::diagonalA),
            WireBP(6, 8, ElbowConfig::diagonalA),
            WireBP(7, 9, ElbowConfig::diagonalA),
            WireBP(8, 7, ElbowConfig::diagonalA),
            WireBP(9, 6, ElbowConfig::diagonalA),
        }),
    Blueprint(
        "Half-Adder",
        IVec2(1,1) * g_gridSize,
        // Nodes
        {
            NodeBP(true, Gate::OR,  IVec2(0,1) * g_gridSize),
            NodeBP(true, Gate::OR,  IVec2(0,0) * g_gridSize),
            NodeBP(true, Gate::XOR, IVec2(1,0) * g_gridSize),
            NodeBP(true, Gate::AND, IVec2(1,1) * g_gridSize),
        },
        // Wires
        {
            WireBP(0, 2, ElbowConfig::horizontal),
            WireBP(1, 2, ElbowConfig::horizontal),
            WireBP(0, 3, ElbowConfig::vertical),
            WireBP(1, 3, ElbowConfig::vertical),
        }),
    Blueprint(
        "Full-Adder",
        IVec2(3,1) * g_gridSize,
        // Nodes
        {
            NodeBP(true,  Gate::OR,  IVec2(0,0) * g_gridSize),
            NodeBP(true,  Gate::OR,  IVec2(0,1) * g_gridSize),
            NodeBP(true,  Gate::OR,  IVec2(2,0) * g_gridSize),
            NodeBP(false, Gate::XOR, IVec2(1,0) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,1) * g_gridSize),
            NodeBP(true,  Gate::XOR, IVec2(3,0) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(2,1) * g_gridSize),
            NodeBP(true,  Gate::OR,  IVec2(3,1) * g_gridSize),
        },
        // Wires
        {
            WireBP(0, 3, ElbowConfig::horizontal),
            WireBP(1, 3, ElbowConfig::horizontal),
            WireBP(0, 4, ElbowConfig::vertical),
            WireBP(1, 4, ElbowConfig::vertical),
            WireBP(2, 5, ElbowConfig::horizontal),
            WireBP(3, 5, ElbowConfig::horizontal),
            WireBP(2, 6, ElbowConfig::vertical),
            WireBP(3, 6, ElbowConfig::vertical),
            WireBP(6, 7, ElbowConfig::vertical),
            WireBP(4, 7, ElbowConfig::horizontal),
        }),
    Blueprint(
        "Byte-Adder",
        IVec2(3,15) * g_gridSize,
        // Nodes
        {
            // Bit 1
            NodeBP(true,  Gate::OR,  IVec2(0,1) * g_gridSize),
            NodeBP(true,  Gate::OR,  IVec2(0,0) * g_gridSize),
            NodeBP(true,  Gate::XOR, IVec2(1,0) * g_gridSize),
            NodeBP(true,  Gate::AND, IVec2(1,1) * g_gridSize),
            // Bit 2
            NodeBP(true,  Gate::OR,  IVec2(0,2) * g_gridSize),
            NodeBP(true,  Gate::OR,  IVec2(0,3) * g_gridSize),
            NodeBP(true,  Gate::OR,  IVec2(2,2) * g_gridSize),
            NodeBP(false, Gate::XOR, IVec2(1,2) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,3) * g_gridSize),
            NodeBP(true,  Gate::XOR, IVec2(3,2) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(2,3) * g_gridSize),
            NodeBP(true,  Gate::OR,  IVec2(3,3) * g_gridSize),
            // Bit 3
            NodeBP(true,  Gate::OR,  IVec2(0,4) * g_gridSize),
            NodeBP(true,  Gate::OR,  IVec2(0,5) * g_gridSize),
            NodeBP(true,  Gate::OR,  IVec2(2,4) * g_gridSize),
            NodeBP(false, Gate::XOR, IVec2(1,4) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,5) * g_gridSize),
            NodeBP(true,  Gate::XOR, IVec2(3,4) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(2,5) * g_gridSize),
            NodeBP(true,  Gate::OR,  IVec2(3,5) * g_gridSize),
            // Bit 4
            NodeBP(true,  Gate::OR,  IVec2(0,6) * g_gridSize),
            NodeBP(true,  Gate::OR,  IVec2(0,7) * g_gridSize),
            NodeBP(true,  Gate::OR,  IVec2(2,6) * g_gridSize),
            NodeBP(false, Gate::XOR, IVec2(1,6) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,7) * g_gridSize),
            NodeBP(true,  Gate::XOR, IVec2(3,6) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(2,7) * g_gridSize),
            NodeBP(true,  Gate::OR,  IVec2(3,7) * g_gridSize),
            // Bit 5
            NodeBP(true,  Gate::OR,  IVec2(0,8) * g_gridSize),
            NodeBP(true,  Gate::OR,  IVec2(0,9) * g_gridSize),
            NodeBP(true,  Gate::OR,  IVec2(2,8) * g_gridSize),
            NodeBP(false, Gate::XOR, IVec2(1,8) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,9) * g_gridSize),
            NodeBP(true,  Gate::XOR, IVec2(3,8) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(2,9) * g_gridSize),
            NodeBP(true,  Gate::OR,  IVec2(3,9) * g_gridSize),
            // Bit 6
            NodeBP(true,  Gate::OR,  IVec2(0,10) * g_gridSize),
            NodeBP(true,  Gate::OR,  IVec2(0,11) * g_gridSize),
            NodeBP(true,  Gate::OR,  IVec2(2,10) * g_gridSize),
            NodeBP(false, Gate::XOR, IVec2(1,10) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,11) * g_gridSize),
            NodeBP(true,  Gate::XOR, IVec2(3,10) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(2,11) * g_gridSize),
            NodeBP(true,  Gate::OR,  IVec2(3,11) * g_gridSize),
            // Bit 7
            NodeBP(true,  Gate::OR,  IVec2(0,12) * g_gridSize),
            NodeBP(true,  Gate::OR,  IVec2(0,13) * g_gridSize),
            NodeBP(true,  Gate::OR,  IVec2(2,12) * g_gridSize),
            NodeBP(false, Gate::XOR, IVec2(1,12) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,13) * g_gridSize),
            NodeBP(true,  Gate::XOR, IVec2(3,12) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(2,13) * g_gridSize),
            NodeBP(true,  Gate::OR,  IVec2(3,13) * g_gridSize),
            // Bit 8
            NodeBP(true,  Gate::OR,  IVec2(0,14) * g_gridSize),
            NodeBP(true,  Gate::OR,  IVec2(0,15) * g_gridSize),
            NodeBP(true,  Gate::OR,  IVec2(2,14) * g_gridSize),
            NodeBP(false, Gate::XOR, IVec2(1,14) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,15) * g_gridSize),
            NodeBP(true,  Gate::XOR, IVec2(3,14) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(2,15) * g_gridSize),
            NodeBP(true,  Gate::OR,  IVec2(3,15) * g_gridSize),
        },
        // Wires
        {
            // Bit 1
            WireBP(0, 2, ElbowConfig::horizontal),
            WireBP(1, 2, ElbowConfig::horizontal),
            WireBP(0, 3, ElbowConfig::vertical),
            WireBP(1, 3, ElbowConfig::vertical),

            WireBP(3, 4+2, ElbowConfig::horizontal),
            // Bit 2
            WireBP(4+0, 4+3, ElbowConfig::horizontal),
            WireBP(4+1, 4+3, ElbowConfig::horizontal),
            WireBP(4+0, 4+4, ElbowConfig::vertical),
            WireBP(4+1, 4+4, ElbowConfig::vertical),
            WireBP(4+2, 4+5, ElbowConfig::horizontal),
            WireBP(4+3, 4+5, ElbowConfig::horizontal),
            WireBP(4+2, 4+6, ElbowConfig::vertical),
            WireBP(4+3, 4+6, ElbowConfig::vertical),
            WireBP(4+6, 4+7, ElbowConfig::vertical),
            WireBP(4+4, 4+7, ElbowConfig::horizontal),

            WireBP(4+7, 12+2, ElbowConfig::horizontal),
            // Bit 3
            WireBP(12+0, 12+3, ElbowConfig::horizontal),
            WireBP(12+1, 12+3, ElbowConfig::horizontal),
            WireBP(12+0, 12+4, ElbowConfig::vertical),
            WireBP(12+1, 12+4, ElbowConfig::vertical),
            WireBP(12+2, 12+5, ElbowConfig::horizontal),
            WireBP(12+3, 12+5, ElbowConfig::horizontal),
            WireBP(12+2, 12+6, ElbowConfig::vertical),
            WireBP(12+3, 12+6, ElbowConfig::vertical),
            WireBP(12+6, 12+7, ElbowConfig::vertical),
            WireBP(12+4, 12+7, ElbowConfig::horizontal),

            WireBP(12+7, 20+2, ElbowConfig::horizontal),
            // Bit 4
            WireBP(20+0, 20+3, ElbowConfig::horizontal),
            WireBP(20+1, 20+3, ElbowConfig::horizontal),
            WireBP(20+0, 20+4, ElbowConfig::vertical),
            WireBP(20+1, 20+4, ElbowConfig::vertical),
            WireBP(20+2, 20+5, ElbowConfig::horizontal),
            WireBP(20+3, 20+5, ElbowConfig::horizontal),
            WireBP(20+2, 20+6, ElbowConfig::vertical),
            WireBP(20+3, 20+6, ElbowConfig::vertical),
            WireBP(20+6, 20+7, ElbowConfig::vertical),
            WireBP(20+4, 20+7, ElbowConfig::horizontal),

            WireBP(20+7, 28+2, ElbowConfig::horizontal),
            // Bit 5
            WireBP(28+0, 28+3, ElbowConfig::horizontal),
            WireBP(28+1, 28+3, ElbowConfig::horizontal),
            WireBP(28+0, 28+4, ElbowConfig::vertical),
            WireBP(28+1, 28+4, ElbowConfig::vertical),
            WireBP(28+2, 28+5, ElbowConfig::horizontal),
            WireBP(28+3, 28+5, ElbowConfig::horizontal),
            WireBP(28+2, 28+6, ElbowConfig::vertical),
            WireBP(28+3, 28+6, ElbowConfig::vertical),
            WireBP(28+6, 28+7, ElbowConfig::vertical),
            WireBP(28+4, 28+7, ElbowConfig::horizontal),

            WireBP(28+7, 36+2, ElbowConfig::horizontal),
            // Bit 6
            WireBP(36+0, 36+3, ElbowConfig::horizontal),
            WireBP(36+1, 36+3, ElbowConfig::horizontal),
            WireBP(36+0, 36+4, ElbowConfig::vertical),
            WireBP(36+1, 36+4, ElbowConfig::vertical),
            WireBP(36+2, 36+5, ElbowConfig::horizontal),
            WireBP(36+3, 36+5, ElbowConfig::horizontal),
            WireBP(36+2, 36+6, ElbowConfig::vertical),
            WireBP(36+3, 36+6, ElbowConfig::vertical),
            WireBP(36+6, 36+7, ElbowConfig::vertical),
            WireBP(36+4, 36+7, ElbowConfig::horizontal),

            WireBP(36+7, 44+2, ElbowConfig::horizontal),
            // Bit 7
            WireBP(44+0, 44+3, ElbowConfig::horizontal),
            WireBP(44+1, 44+3, ElbowConfig::horizontal),
            WireBP(44+0, 44+4, ElbowConfig::vertical),
            WireBP(44+1, 44+4, ElbowConfig::vertical),
            WireBP(44+2, 44+5, ElbowConfig::horizontal),
            WireBP(44+3, 44+5, ElbowConfig::horizontal),
            WireBP(44+2, 44+6, ElbowConfig::vertical),
            WireBP(44+3, 44+6, ElbowConfig::vertical),
            WireBP(44+6, 44+7, ElbowConfig::vertical),
            WireBP(44+4, 44+7, ElbowConfig::horizontal),

            WireBP(44+7, 52+2, ElbowConfig::horizontal),
            // Bit 8
            WireBP(52+0, 52+3, ElbowConfig::horizontal),
            WireBP(52+1, 52+3, ElbowConfig::horizontal),
            WireBP(52+0, 52+4, ElbowConfig::vertical),
            WireBP(52+1, 52+4, ElbowConfig::vertical),
            WireBP(52+2, 52+5, ElbowConfig::horizontal),
            WireBP(52+3, 52+5, ElbowConfig::horizontal),
            WireBP(52+2, 52+6, ElbowConfig::vertical),
            WireBP(52+3, 52+6, ElbowConfig::vertical),
            WireBP(52+6, 52+7, ElbowConfig::vertical),
            WireBP(52+4, 52+7, ElbowConfig::horizontal),
        }),
};

NodeWorld::NodeWorld()
{
    for (Blueprint& bp : nativeBlueprints)
    {
        blueprints.push_back(new Blueprint(bp));
    }
}
NodeWorld::~NodeWorld()
{
    _Free();
}

void NodeWorld::_Free()
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

void NodeWorld::_Clear()
{
    _Free();

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
Node* NodeWorld::CreateNode(IVec2 position, Gate gate, uint8_t extendedParam)
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

void NodeWorld::DestroyNodes(std::vector<Node*>& removeList)
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

void NodeWorld::BypassNode(Node* node)
{
    _ASSERT_EXPR(node->IsSpecialErasable(), L"Must be bypassable");
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
}

void NodeWorld::BypassNode_Complex(Node* node)
{
    _ASSERT_EXPR(node->IsComplexBipassable(), L"Must be complex bypassable");
    for (Wire* input : node->GetInputs())
    {
        for (Wire* output : node->GetOutputs())
        {
            CreateWire(input->start, output->end, input->elbowConfig);
        }
    }
    DestroyNode(node);
}

Node* NodeWorld::MergeNodes(Node* depricating, Node* overriding)
{
    _ASSERT_EXPR(!!depricating && !!overriding, L"Tried to merge a nullptr");
    _ASSERT_EXPR(depricating != overriding, L"Tried to merge a node with itself");
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

    return c;
}

// Wire functions

// CreateWire can affect the positions of parameter `end` in `nodes`
Wire* NodeWorld::CreateWire(Node* start, Node* end)
{
    _ASSERT_EXPR(start != nullptr && end != nullptr, L"Tried to create a wire to nullptr");
    _ASSERT_EXPR(start != end, L"Cannot create self-reference wire");

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
    _ASSERT_EXPR(start != nullptr && end != nullptr, L"Tried to create a wire to nullptr");
    _ASSERT_EXPR(start != end, L"Cannot create self-reference wire");

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
void NodeWorld::SwapNodes(Node* a, Node* b)
{
    std::swap(a->m_gate, b->m_gate);
}
// Invalidates input wire!
Wire* NodeWorld::ReverseWire(Wire* wire)
{
    _ASSERT_EXPR(wire != nullptr, L"Cannot reverse null wire");
    _ASSERT_EXPR(wire->start != nullptr && wire->end != nullptr, L"Malformed wire");
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

    default:
        _ASSERT_EXPR(false, L"No specialization for selected gate evaluation");
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


void NodeWorld::StoreBlueprint(Blueprint* bp)
{
    blueprints.push_back(bp);
}
void NodeWorld::SpawnBlueprint(Blueprint* bp, IVec2 topLeft)
{
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
}

const std::vector<Blueprint*>& NodeWorld::GetBlueprints() const
{
    return blueprints;
}

void NodeWorld::Save(const char* filename) const
{
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
        file << "1.1\n";

        std::unordered_map<Node*, size_t> nodeIDs;
        std::unordered_map<Wire*, size_t> wireIDs;
        std::thread a(prepNodeIDs, std::ref(nodeIDs), std::cref(nodes));
        std::thread b(prepWireIDs, std::ref(wireIDs), std::cref(wires));
        a.join();
        b.join();

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

        file << TextFormat("w %i\n", wires.size());

        for (Wire* wire : wires)
        {
            file << TextFormat("%i %i %i\n", wire->elbowConfig, nodeIDs.find(wire->start)->second, nodeIDs.find(wire->end)->second);
        }
    }
    file.close();
}

void NodeWorld::Load(const char* filename)
{
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


    struct FNodeData { char gate; IVec2 pos; int extra; };
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
        if (version != 1.1)
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
}

void NodeWorld::Export(const char* filename) const
{
    if (nodes.empty())
        return;

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
            }
            if (ORs && ANDs && NORs && XORs && RESs && CAPs && LEDs && DELs)
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
            default: _ASSERT_EXPR(false, L"No SVG export specialization for selected gate"); id = ""; break;
            }
            file << "  <use href=\"" << id << "\" x=\"" << x <<"\" y=\"" << y << "\" />\n";
        }
        file << "</svg>";
    }
    file.close();
}
