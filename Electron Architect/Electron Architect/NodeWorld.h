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

    // Wire functions

    // CreateWire can affect the positions of parameter `end` in `nodes`
    Wire* CreateWire(Node* start, Node* end);
    // CreateWire can affect the positions of parameter `end` in `nodes`
    Wire* CreateWire(Node* start, Node* end, ElbowConfig elbowConfig);
    void DestroyWire(Wire* wire);
    Node* MergeNodes(Node* composite, Node* tbRemoved);
    // Invalidates input wire!
    Wire* ReverseWire(Wire* wire);
    // Invalidates input wire! (obviously; it's being split in two)
    std::pair<Wire*, Wire*> BisectWire(Wire* wire, Node* bisector);

    Group* CreateGroup(IRect rec);
    void DestroyGroup(Group* group);
    Group* FindGroupAtPos(IVec2 pos) const;
    void FindNodesInGroup(std::vector<Node*>& result, Group* group) const;


    // Uses BFS
    void Sort();

    void EvaluateNode(Node* node);

    void Evaluate();

    void DrawWires() const;
    void DrawNodes() const;
    void DrawGroups() const;

    Node* FindNodeAtPos(IVec2 pos) const;
    Wire* FindWireAtPos(IVec2 pos) const;
    Wire* FindWireElbowAtPos(IVec2 pos) const;

    void FindNodesInRect(std::vector<Node*>& result, IRect rec) const;

public: // Serialization

    void SpawnBlueprint(Blueprint* bp, IVec2 topLeft);
    void StoreBlueprint(Blueprint* bp);

    void Save(const char* filename) const;

    // TODO
    void Load(const char* filename);
};
