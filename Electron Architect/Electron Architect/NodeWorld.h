#pragma once
#include "HUtility.h"
#include "Node.h"
#include "Wire.h"
#include "Group.h"
#include "Blueprint.h"

class NodeWorld
{
private:
    bool orderDirty = false;

    std::vector<Node*> nodes;
    std::vector<Node*> startNodes;
    std::vector<Wire*> wires; // Inputs/outputs don't exist here
    std::vector<Blueprint*> blueprints;
    std::vector<Group*> groups;

    NodeWorld();
    ~NodeWorld();

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
    static NodeWorld& Get();

    const decltype(startNodes)& GetStartNodes() const;

    // Node functions

    /// <summary>CreateNode does not insert at the end of the <see cref="nodes"/>.</summary>
    Node* CreateNode(IVec2 position, Gate gate, uint8_t extendedParam = 0);
    void DestroyNode(Node* node);
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

    Group* CreateGroup(IRect rec);
    void DestroyGroup(Group* group);
    void FindNodesInGroup(std::vector<Node*>& result, Group* group) const;

    // Blueprint functions

    void StoreBlueprint(Blueprint* bp);
    void SpawnBlueprint(Blueprint* bp, IVec2 topLeft);
    const std::vector<Blueprint*>& GetBlueprints() const;

    // Evaluation functions

    // Uses BFS
    void Sort();
    void EvaluateNode(Node* node);
    void Evaluate();

    // Draw functions
    
    void DrawWires() const;
    void DrawNodes() const;
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
