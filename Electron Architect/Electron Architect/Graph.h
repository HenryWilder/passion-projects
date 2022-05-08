#pragma once
#include "HUtility.h"
#include "Node.h"
#include "Wire.h"
#include "Group.h"
#include "Blueprint.h"

struct Tab;

class Graph
{
private:
    bool orderDirty = false;

    Tab* owningTab;
    const char* name;

    std::vector<Node*> nodes;
    std::vector<Node*> startNodes;
    std::vector<Wire*> wires; // Inputs/outputs don't exist here
    std::vector<Blueprint*> blueprints;
    std::vector<Group*> groups;

private: // Internal
    void _Free();
    // Already calls _Free!
    void _Clear();
    Node* _CreateNode(Node&& base);
    void _ClearNodeReferences(Node* node);
    void _DestroyNode(Node* node);

    Wire* _CreateWire(Wire&& base);
    void _ClearWireReferences(Wire* wire);
    void _DestroyWire(Wire* wire);

public:

    Graph(Tab* owner, const char* name = "Unnamed graph");
    ~Graph();

    const char* GetName() const;

    bool IsOrderDirty() const;

    const decltype(startNodes)& GetStartNodes() const;

    // Node functions

    // CreateNode does not insert at the end of the nodes
    Node* CreateNode(IVec2 position, Gate gate, uint8_t extendedParam = 0);
    void DestroyNode(Node* node);
    // Gets the index of the node in nodes
    size_t NodeID(Node* node);
    // Gets the index of the node in startNodes
    size_t StartNodeID(Node* node);
    // More efficient for bulk operation
    void DestroyNodes(std::vector<Node*>& removeList);
    // Invalidates input node and all its wires!
    void BypassNode(Node* node);
    void BypassNode_Complex(Node* node);
    // Invalidates both nodes and creates a new, composit node!
    Node* MergeNodes(Node* depricating, Node* overriding);

    // Wire functions

    // CreateWire can affect the positions of parameter `end` in `nodes`
    Wire* CreateWire(Node* start, Node* end);
    // CreateWire can affect the positions of parameter `end` in `nodes`
    Wire* CreateWire(Node* start, Node* end, ElbowConfig elbowConfig);
    void DestroyWire(Wire* wire);
    // Looks like it swaps the two nodes, but really only swaps the gate!
    void SwapNodes(Node* a, Node* b);
    // Invalidates input wire!
    Wire* ReverseWire(Wire* wire);
    // Invalidates input wire! (obviously; it's being split in two)
    std::pair<Wire*, Wire*> BisectWire(Wire* wire, Node* bisector);

    // Group functions

    Group* CreateGroup(IRect rec, Color color);
    void DestroyGroup(Group* group);
    void FindNodesInGroup(std::vector<Node*>& result, Group* group) const;

    // Blueprint functions

    void StoreBlueprint(Blueprint* bp); // Todo: make this window-wide instead of tab-specific
    void SpawnBlueprint(Blueprint* bp, IVec2 topLeft);
    const std::vector<Blueprint*>& GetBlueprints() const;

    // Evaluation functions

    // Uses BFS
    void Sort();
    void EvaluateNode(Node* node);
    void Evaluate();

    // Draw functions
    
    void DrawWires(Color colorActive, Color colorInactive) const;
    void DrawNodes(Color colorActive, Color colorInactive) const;
    void DrawGroups() const;

    // Search functions

    Node* FindNodeAtPos(IVec2 pos) const;
    Wire* FindWireAtPos(IVec2 pos) const;
    Wire* FindWireElbowAtPos(IVec2 pos) const;
    Group* FindGroupAtPos(IVec2 pos) const;
    GroupCorner FindGroupCornerAtPos(IVec2 pos) const;

    void FindNodesInRect(std::vector<Node*>& result, IRect rec) const;

    // Serialization functions

    void Save(const char* filename) const;
    void Load(const char* filename);
    // Saves the graph in SVG format
    void Export(const char* filename) const;
};
