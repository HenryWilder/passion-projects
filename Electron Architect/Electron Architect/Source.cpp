#include <fstream>
#include <time.h>
#include <thread>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <raylib.h>
#include <limits.h>
//#include <raymath.h>
//#include <extras\raygui.h>

#pragma region Utility

// Custom colors
#define SPACEGRAY CLITERAL(Color){ 28, 26, 41, 255 }
#define LIFELESSNEBULA CLITERAL(Color){ 75, 78, 94, 255 }
#define HAUNTINGWHITE CLITERAL(Color){ 148, 150, 166, 255 }
#define GLEEFULDUST CLITERAL(Color){ 116, 125, 237, 255 }
#define INTERFERENCEGRAY CLITERAL(Color){ 232, 234, 255, 255 }
#define REDSTONE CLITERAL(Color){ 212, 25, 25, 255 }
#define DESTRUCTIVERED CLITERAL(Color){ 219, 18, 18, 255 }
#define DEADCABLE CLITERAL(Color){ 122, 118, 118, 255 }
#define INPUTLAVENDER CLITERAL(Color){ 128, 106, 217, 255 }
#define OUTPUTAPRICOT CLITERAL(Color){ 207, 107, 35, 255 }
#define WIPBLUE CLITERAL(Color){ 26, 68, 161, 255 }
#define CAUTIONYELLOW CLITERAL(Color){ 250, 222, 37, 255 }

// Returns true on success
template<typename T>
bool FindAndErase(std::vector<T>& vec, const T& element)
{
    if (auto it = std::find(vec.begin(), vec.end(), element); it != vec.end())
    {
        vec.erase(it);
        return true;
    }
    return false;
}
template<typename T>
void FindAndErase_ExpectExisting(std::vector<T>& vec, const T& element)
{
    auto it = std::find(vec.begin(), vec.end(), element);
    _ASSERT_EXPR(it != vec.end(), "Expected element to be present");
    vec.erase(it);
}

// Returns true on success
template<typename T>
bool FindAndErase(std::deque<T>& vec, const T& element)
{
    if (auto it = std::find(vec.begin(), vec.end(), element); it != vec.end())
    {
        vec.erase(it);
        return true;
    }
    return false;
}
template<typename T>
void FindAndErase_ExpectExisting(std::deque<T>& vec, const T& element)
{
    auto it = std::find(vec.begin(), vec.end(), element);
    _ASSERT_EXPR(it != vec.end(), "Expected element to be present");
    vec.erase(it);
}

using Int_t = int;
constexpr Int_t g_gridSize = 8;

bool Between_Inclusive(Int_t x, Int_t a, Int_t b)
{
    auto [min, max] = std::minmax(a, b);
    return min <= x && x <= max;
}
bool Between_Exclusive(Int_t x, Int_t a, Int_t b)
{
    auto [min, max] = std::minmax(a, b);
    return min < x && x < max;
}

struct IVec2
{
    IVec2() = default;
    constexpr IVec2(Int_t x, Int_t y) : x(x), y(y) {}

    Int_t x, y;

    bool operator==(IVec2 b) const
    {
        return x == b.x && y == b.y;
    }
    bool operator!=(IVec2 b) const
    {
        return x != b.x || y != b.y;
    }
};

namespace std
{
    template<>
    struct hash<IVec2>
    {
        size_t operator()(const IVec2& k) const
        {
            size_t first  = hash<Int_t>{}(k.x);
            size_t second = hash<Int_t>{}(k.y);
            return first ^ (second << 1);
        }
    };
}

// Distance between points in an integer number of squares
// I.E. a diagonal line is measured as the number of square-grid points it passes through, rather than the length of its hypotenuse
// Assumes that the points could take each other in chess if the two were the positions of queens
Int_t IntGridDistance(IVec2 a, IVec2 b)
{
    if (a.x == b.x)
        return abs(b.y - a.y);
    else
        return abs(b.x - a.x);
}
long DistanceSqr(IVec2 a, IVec2 b)
{
    long x = b.x - a.x;
    long y = b.y - a.y;
    return x * x + y * y;
}
IVec2 Normal(IVec2 vec)
{
    return {
        (vec.x > 0 ? 1 : 0) - (vec.x < 0 ? 1 : 0),
        (vec.x > 0 ? 1 : 0) - (vec.x < 0 ? 1 : 0)
    };
}

bool InBoundingBox(IVec2 p, IVec2 a, IVec2 b)
{
    return Between_Inclusive(p.x, a.x, b.x) &&
           Between_Inclusive(p.y, a.y, b.y);
}

IVec2 IVec2Zero()
{
    constexpr IVec2 null(0,0);
    return null;
}

IVec2 operator+(IVec2 a, IVec2 b)
{
    return IVec2(a.x + b.x, a.y + b.y);
}
IVec2 operator-(IVec2 a, IVec2 b)
{
    return IVec2(a.x - b.x, a.y - b.y);
}
IVec2 operator*(IVec2 a, IVec2 b)
{
    return IVec2(a.x * b.x, a.y * b.y);
}
IVec2 operator/(IVec2 a, IVec2 b)
{
    return IVec2(a.x / b.x, a.y / b.y);
}

IVec2 IVec2Scale_i(IVec2 a, int b)
{
    return IVec2(a.x * b, a.y * b);
}
inline IVec2 IVec2Scale_i(int a, IVec2 b)
{
    return IVec2Scale_i(b, a);
}
IVec2 IVec2Scale_f(IVec2 a, float b)
{
    return IVec2((Int_t)((float)a.x * b), (Int_t)((float)a.y * b));
}
inline IVec2 IVec2Scale_f(float a, IVec2 b)
{
    return IVec2Scale_f(b, a);
}

bool CheckCollisionIVecPointLine(IVec2 pt, IVec2 p1, IVec2 p2)
{
    auto [minX, maxX] = std::minmax(p1.x, p2.x);
    auto [minY, maxY] = std::minmax(p1.y, p2.y);
    if (pt.x < minX || pt.x > maxX || pt.y < minY || pt.y > maxY)
        return false;

    // Cardinal
    if (p1.x == p2.x || p1.y == p2.y)
        return true;

    // Diagonal
    auto [a, b] = (p1.x < p2.x) ?
        std::pair<const IVec2&,const IVec2&>{ p1, p2 } :
        std::pair<const IVec2&,const IVec2&>{ p2, p1 };

    /*****************
    *   It is either
    *
    *   a
    *    \
    *     \
    *      \
    *       b
    *
    *   Or
    *
    *       b
    *      /
    *     /
    *    /
    *   a
    *
    *****************/

    if (b.y > a.y)
    {
        /*************
        *
        *   a
        *    \
        *     \
        *      \
        *       b
        *
        *************/
        return abs((pt.x - a.x) - (pt.y - a.y)) <= 1;
    }
    else
    {
        /*************
        *
        *       b
        *      /
        *     /
        *    /
        *   a
        *
        *************/
        return abs((pt.x - a.x) - -(pt.y - a.y)) <= 1;
    }
}

void DrawLineIV(IVec2 start, IVec2 end, Color color)
{
    DrawLine(start.x, start.y, end.x, end.y, color);
}
void DrawCircleIV(IVec2 origin, float radius, Color color)
{
    DrawCircle(origin.x, origin.y, radius, color);
}

struct IRect
{
    IRect() = default;
    constexpr IRect(Int_t x, Int_t y, Int_t w, Int_t h) : x(x), y(y), w(w), h(h) {}

    union
    {
        struct { Int_t x, y, w, h; };
        struct { IVec2 xy, wh; };
    };
};

bool InBoundingBox(IRect bounds, IVec2 pt)
{
    return
        pt.x >= bounds.x &&
        pt.y >= bounds.y &&
        pt.x <= bounds.x + bounds.w &&
        pt.y <= bounds.y + bounds.h;
}

void DrawRectangleIRect(IRect rec, Color color)
{
    DrawRectangle(rec.x, rec.y, rec.w, rec.h, color);
}

#pragma endregion

class NodeWorld;
class Node;
struct Wire
{
    Wire() = default;
    Wire(Node* start, Node* end) : elbow(), elbowConfig(0), start(start), end(end) {}

    static constexpr Color g_wireColorActive = REDSTONE;
    static constexpr Color g_wireColorInactive = DEADCABLE;
    static constexpr float g_elbowRadius = 2.0f;

    IVec2 elbow;
    uint8_t elbowConfig;
    Node* start;
    Node* end;

    static void Draw(IVec2 start, IVec2 joint, IVec2 end, Color color)
    {
        DrawLineIV(start, joint, color);
        DrawLineIV(joint, end, color);
    }
    void Draw(Color color) const
    {
        Draw(GetStartPos(), elbow, GetEndPos(), color);
    }
    static void DrawElbow(IVec2 pos, Color color)
    {
        DrawCircle(pos.x, pos.y, g_elbowRadius, color);
    }
    void DrawElbow(Color color) const
    {
        DrawElbow(elbow, color);
    }

    IVec2 GetStartPos() const; // Requires Node definition
    Int_t GetStartX() const
    {
        return GetStartPos().x;
    }
    Int_t GetStartY() const
    {
        return GetStartPos().y;
    }

    IVec2 GetElbowPos() const
    {
        return elbow;
    }
    Int_t GetElbowX() const
    {
        return GetElbowPos().x;
    }
    Int_t GetElbowY() const
    {
        return GetElbowPos().y;
    }

    IVec2 GetEndPos() const; // Requires Node definition
    Int_t GetEndX() const
    {
        return GetEndPos().x;
    }
    Int_t GetEndY() const
    {
        return GetEndPos().y;
    }
	
    static IVec2 GetLegalElbowPosition(IVec2 start, IVec2 end, decltype(elbowConfig) index)
    {
        _ASSERT_EXPR(index < 4, "Subscript error");
        if (index == 0)
            return IVec2(start.x, end.y);
        else if (index == 1)
            return IVec2(end.x, start.y);
        else
        {
            Int_t shortLength = std::min(
                abs(end.x - start.x),
                abs(end.y - start.y)
            );
            IVec2 pos;
            if (index == 2)
            {
                pos = start;

                if (end.x < start.x)
                    pos.x -= shortLength;
                else
                    pos.x += shortLength;

                if (end.y < start.y)
                    pos.y -= shortLength;
                else
                    pos.y += shortLength;
            }
            else // index == 3
            {
                pos = end;

                if (start.x < end.x)
                    pos.x -= shortLength;
                else
                    pos.x += shortLength;

                if (start.y < end.y)
                    pos.y -= shortLength;
                else
                    pos.y += shortLength;
            }
            return pos;
        }
    }
    IVec2 GetLegalElbowPosition(decltype(elbowConfig) index) const
    {
        return GetLegalElbowPosition(GetStartPos(), GetEndPos(), index);
    }
    void GetLegalElbowPositions(IVec2(&legal)[4]) const
    {
        Int_t shortLength = std::min(
            abs(GetEndX() - GetStartX()),
            abs(GetEndY() - GetStartY())
        );

        legal[0] = IVec2(GetStartX(), GetEndY());
        legal[1] = IVec2(GetEndX(), GetStartY());
        legal[2] = GetStartPos() + IVec2(GetEndX() < GetStartX() ? -shortLength : shortLength,
            GetEndY() < GetStartY() ? -shortLength : shortLength);
        legal[3] = GetEndPos() + IVec2(GetStartX() < GetEndX() ? -shortLength : shortLength,
            GetStartY() < GetEndY() ? -shortLength : shortLength);
    }
    void UpdateElbowToLegal()
    {
        elbow = GetLegalElbowPosition(elbowConfig);
    }
    void SnapElbowToLegal(IVec2 pos)
    {
        IVec2 legal[4];
        GetLegalElbowPositions(legal);

        decltype(elbowConfig) pick = 0;
        long shortestDist = LONG_MAX;
        for (decltype(pick) i = 0; i < 4; ++i)
        {
            long dist = DistanceSqr(pos, legal[i]);
            if (dist < shortestDist)
            {
                shortestDist = dist;
                pick = i;
            }
        }
        elbowConfig = pick; // Index of pick
        elbow = legal[pick];
    }
};

enum class Gate : char
{
    OR = '|',
    AND = '&',
    NOR = '!',
    XOR = '^',
};

class Node
{
public:
    IVec2 GetPosition() const
    {
        return m_position;
    }
    void SetPosition(IVec2 position);
    // Moves the node without updating its collision
    void SetPosition_Temporary(IVec2 position)
    {
        m_position = position;
        for (Wire* wire : m_wires)
        {
            wire->UpdateElbowToLegal(); // Keep current configuration but move the elbow
        }
    }
    Int_t GetX() const
    {
        return m_position.x;
    }
    void SetX(Int_t x)
    {
        SetPosition(IVec2(x, GetY()));
    }
    Int_t GetY() const
    {
        return m_position.y;
    }
    void SetY(Int_t y)
    {
        SetPosition(IVec2(GetX(), y));
    }

    Gate GetGate() const
    {
        return m_gate;
    }
    void SetGate(Gate gate)
    {
        m_gate = gate;
    }

    bool GetState() const
    {
        return m_state;
    }

    const std::deque<Wire*>& GetWires() const
    {
        return m_wires;
    }
    auto Inputs_Begin() const
    {
        return m_wires.begin();
    }
    auto Inputs_End() const
    {
        return Inputs_Begin() + m_inputs;
    }
    auto Outputs_Begin() const
    {
        return Inputs_End();
    }
    auto Outputs_End() const
    {
        return m_wires.end();
    }

    auto FindConnection(Node* other) const
    {
        return std::find_if(m_wires.begin(), m_wires.end(), [&other](Wire* wire) { return wire->start == other || wire->end == other; });
    }
    
    size_t GetInputCount() const
    {
        return m_inputs;
    }
    size_t GetOutputCount() const
    {
        return m_wires.size() - m_inputs;
    }
    bool IsOutputOnly() const
    {
        return !m_inputs;
    }

    static void Draw(IVec2 position, Gate gate, Color color)
    {
        constexpr Int_t nodeRadius = static_cast<int>(g_nodeRadius);
        switch (gate)
        {
        case Gate::OR:
            DrawCircle(position.x, position.y, nodeRadius, color);
            break;
        case Gate::AND:
            DrawRectangle(position.x - nodeRadius, position.y - nodeRadius, nodeRadius * 2, nodeRadius * 2, color);
            break;
        case Gate::NOR:
            DrawCircle(position.x, position.y, nodeRadius, color);
            DrawCircle(position.x, position.y, nodeRadius - 1.0f, BLACK);
            break;
        case Gate::XOR:
            DrawCircle(position.x, position.y, nodeRadius + 1.0f, color);
            DrawCircle(position.x, position.y, nodeRadius, BLACK);
            DrawCircle(position.x, position.y, nodeRadius - 1.0f, color);
            break;
        }
    }
    void Draw(Color color) const
    {
        Draw(m_position, m_gate, color);
    }

    bool WireIsInput(Wire* wire) const
    {
        return wire->end == this;
    }
    bool WireIsOutput(Wire* wire) const
    {
        return wire->start == this;
    }

    // Only NodeWorld can play with a node's wires/state
    friend class NodeWorld;

private: // Helpers usable only by NodeWorld

    auto FindWireIter_Expected(Wire* wire)
    {
        auto it = std::find(m_wires.begin(), m_wires.end(), wire);
        _ASSERT_EXPR(it != m_wires.end(), "Expected wire");
        return it;
    }
    auto FindWireIter(Wire* wire)
    {
        return std::find(m_wires.begin(), m_wires.end(), wire);
    }

    void SetState(bool state)
    {
        m_state = state;
    }
    
    void AddWireInput(Wire* input)
    {
        m_wires.push_front(input);
        m_inputs++;
    }
    void AddWireOutput(Wire* output)
    {
        m_wires.push_back(output);
    }

    // Expects the wire to exist; throws a debug exception if it is not found.
    void RemoveWire_Expected(Wire* wire)
    {
        m_wires.erase(FindWireIter_Expected(wire));
        if (WireIsInput(wire))
            m_inputs--;
    }
    void RemoveWire(Wire* wire)
    {
        auto it = FindWireIter(wire);
        if (it == m_wires.end())
            return; // Success!

        m_wires.erase(it);
        if (WireIsInput(wire))
            m_inputs--;
    }

    // Expects the wire to exist; throws a debug exception if it is not found.
    void RemoveConnection_Expected(Node* node)
    {
        auto it = FindConnection(node);
        _ASSERT_EXPR(it != m_wires.end(), "Expected connection to be pre-existing");
        m_wires.erase(it);
    }
    void RemoveConnection(Node* node)
    {
        auto it = FindConnection(node);
        if (it != m_wires.end())
            m_wires.erase(it);
    }

    // Converts an existing wire
    void MakeWireInput(Wire* wire)
    {
        if (WireIsInput(wire))
            return; // Success!

        RemoveWire_Expected(wire);
        AddWireInput(wire);
    }
    // Converts an existing wire
    void MakeWireOutput(Wire* wire)
    {
        if (WireIsOutput(wire))
            return; // Success!

        RemoveWire_Expected(wire);
        AddWireOutput(wire);
    }

    auto Inputs_Begin()
    {
        return m_wires.begin();
    }
    auto Inputs_End()
    {
        return Inputs_Begin() + m_inputs;
    }
    auto Outputs_Begin()
    {
        return Inputs_End();
    }
    auto Outputs_End()
    {
        return m_wires.end();
    }

private: // Accessible by NodeWorld
    Node() = default;
    Node(IVec2 position, Gate gate) : m_position(position), m_gate(gate), m_state(false), m_inputs(0) {}

public:
    static constexpr Color g_nodeColorActive = RED;
    static constexpr Color g_nodeColorInactive = LIGHTGRAY;
    static constexpr float g_nodeRadius = 3.0f;

private:
    IVec2 m_position;
    Gate m_gate;
    bool m_state;
    size_t m_inputs;
    std::deque<Wire*> m_wires; // Please keep this partitioned by inputs vs outputs

public:
    bool IsValidConnection(decltype(m_wires)::const_iterator it) const
    {
        return it != m_wires.end();
    }
};

IVec2 Wire::GetStartPos() const
{
    return start->GetPosition();
}
IVec2 Wire::GetEndPos() const
{
    return end->GetPosition();
}


struct NodeBP
{
    Gate gate;
    IVec2 relativePosition;
};
struct WireBP
{
    size_t startNodeIndex, endNodeIndex;
    decltype(Wire::elbowConfig) elbowConfig;
};
struct Blueprint
{
private: // Multithread functions
    void PopulateNodes(const std::vector<Node*>& src)
    {
        constexpr IRect boundsInit = IRect(
            std::numeric_limits<Int_t>::max(),
            std::numeric_limits<Int_t>::max(),
            std::numeric_limits<Int_t>::min(),
            std::numeric_limits<Int_t>::min());
        bounds = boundsInit;
        for (Node* node : src)
        {
            const IVec2& compare = node->GetPosition();
            if (compare.x < bounds.x)
                bounds.x = compare.x;
            if (compare.y < bounds.y)
                bounds.y = compare.y;

            // Abusing width and height as max x/y
            if (compare.x > bounds.w)
                bounds.w = compare.x;
            if (compare.y > bounds.h)
                bounds.h = compare.y;
        }
        // Disabuse
        bounds.w -= bounds.x;
        bounds.h -= bounds.y;

        IVec2 min = IVec2(bounds.x, bounds.y);
        nodes.reserve(src.size());
        for (Node* node : src)
        {
            nodes.emplace_back(
                node->GetGate(),
                node->GetPosition() - min);
        }
    }
    void PopulateWires(const std::vector<Node*>& src)
    {
        std::unordered_map<Node*, size_t> nodeIndices;
        std::unordered_map<Wire*, bool> visitedWires;

        // Populate nodeIndices
        for (size_t i = 0; i < src.size(); ++i)
        {
            nodeIndices.emplace(src[i], i);
        }

        // Count wires
        {
            {
                size_t totalWires = 0;
                for (Node* node : src)
                {
                    for (Wire* wire : node->GetWires())
                    {
                        ++totalWires;
                    }
                }
                visitedWires.reserve(totalWires);
            }

            {
                size_t uniqueWires = 0;
                for (Node* node : src)
                {
                    for (Wire* wire : node->GetWires())
                    {
                        if (nodeIndices.find(wire->start) == nodeIndices.end() ||
                            nodeIndices.find(wire->end) == nodeIndices.end())
                            continue;
                        if (visitedWires.find(wire) == visitedWires.end())
                        {
                            visitedWires.insert({ wire, false });
                            ++uniqueWires;
                        }
                    }
                }
                visitedWires.clear();
                visitedWires.reserve(uniqueWires);
                wires.reserve(uniqueWires);
            }
        }

        for (Node* node : src)
        {
            for (Wire* wire : node->GetWires())
            {
                if (visitedWires.find(wire) != visitedWires.end() && !visitedWires.find(wire)->second)
                {
                    visitedWires[wire] = true;
                    wires.emplace_back(
                        nodeIndices.find(wire->start)->second,
                        nodeIndices.find(wire->end)->second,
                        wire->elbowConfig);
                }
            }
        }
    }

public:
    Blueprint(const std::vector<Node*>& src)
    {        
        bounds = IRect(0,0,0,0);
        std::thread nodeThread(&Blueprint::PopulateNodes, this, std::ref(src));
        std::thread wireThread(&Blueprint::PopulateWires, this, std::ref(src));
        nodeThread.join();
        wireThread.join();
    }

    IRect bounds;
	std::vector<NodeBP> nodes;
	std::vector<WireBP> wires;
};


class NodeWorld
{
private:
    bool orderDirty = false;

    std::vector<Node*> nodes;
    std::vector<Wire*> wires; // Inputs/outputs don't exist here
    std::vector<Blueprint*> blueprints;

    std::vector<Node*> startNodes;
    std::vector<decltype(nodes)::const_iterator> layers;
    std::unordered_map<IVec2, Node*> nodeGrid;

    NodeWorld() = default;
    ~NodeWorld()
    {
        for (Node* node : nodes)
        {
            delete node;
        }
        for (Wire* wire : wires)
        {
            delete wire;
        }
    }

private: // Internal
    Node* _CreateNode(Node&& base)
    {
        Node* node = new Node(base);
        nodes.insert(nodes.begin(), node);
        startNodes.push_back(node);
        nodeGrid.emplace(base.m_position, node);
        return node;
    }
    void _ClearNodeReferences(Node* node)
    {
        for (auto it = node->Inputs_Begin(); it != node->Inputs_End(); ++it)
        {
            Wire* input = *it;
            input->start->RemoveWire_Expected(input);
            _DestroyWire(input);
        }
        for (auto it = node->Outputs_Begin(); it != node->Outputs_End(); ++it)
        {
            Wire* output = *it;
            output->end->RemoveWire_Expected(output);
            _DestroyWire(output);
        }
        orderDirty = true;
    }
    void _DestroyNode(Node* node)
    {
        FindAndErase_ExpectExisting(nodes, node);
        FindAndErase(startNodes, node);
        auto it = nodeGrid.find(node->m_position);
        if (it != nodeGrid.end() && it->second == node)
            nodeGrid.erase(it);
        delete node;
        orderDirty = true;
    }

    Wire* _CreateWire(Wire&& base)
    {
        Wire* wire = new Wire(base);
        wires.push_back(wire);
        return wire;
    }
    void _ClearWireReferences(Wire* wire)
    {
        wire->start->RemoveWire_Expected(wire);
        wire->end->RemoveWire_Expected(wire);
        // Push end to start nodes if this has destroyed its last remaining input
        if (wire->end->IsOutputOnly())
            startNodes.push_back(wire->end);
        orderDirty = true;
    }
    void _DestroyWire(Wire* wire)
    {
        FindAndErase_ExpectExisting(wires, wire);
        delete wire;
        orderDirty = true;
    }

public:
    static NodeWorld& Get()
    {
        static NodeWorld g_only;
        return g_only;
    }

    const decltype(startNodes)& GetStartNodes() const
    {
        return startNodes;
    }
    size_t LayerCount() const
    {
        return layers.size() - 1;
    }

    // Node functions
    Node* CreateNode(IVec2 position, Gate gate)
    {
        // The order is not dirty at this time due to the node having no connections yet
        return _CreateNode(Node(position, gate));
    }
    void DestroyNode(Node* node)
    {
        _ClearNodeReferences(node);
        _DestroyNode(node);
        orderDirty = true;
    }
    // Only affects node collision
    void MoveNode(Node* node, IVec2 newPosition)
    {
        nodeGrid.erase(node->GetPosition());
        nodeGrid.emplace(newPosition, node);
    }

    // Wire functions
    Wire* CreateWire(Node* start, Node* end)
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

        wire->elbowConfig = 0;
        wire->UpdateElbowToLegal();

        start->AddWireOutput(wire);
        end->AddWireInput(wire);

        // Remove end from start nodes, as it is no longer an inputless node with this change
        FindAndErase(startNodes, end);

        orderDirty = true;
        return wire;
    }
    void DestroyWire(Wire* wire)
    {
        _ClearWireReferences(wire);
        _DestroyWire(wire);

        orderDirty = true;
    }
    Node* MergeNodes(Node* composite, Node* tbRemoved)
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
    Wire* ReverseWire(Wire* wire)
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
    std::pair<Wire*, Wire*> BisectWire(Wire* wire, Node* bisector)
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

    // Uses BFS
    void Sort()
    {
        decltype(nodes) sorted;
        sorted.reserve(nodes.size());

        std::queue<Node*> list;
        std::unordered_map<Node*,size_t> visited; // Pointer, depth
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

        layers.clear();
        layers.reserve(nodes.size() + 1);
        layers.push_back(nodes.begin());
        size_t depth = 0;
        for (auto it = nodes.begin(); it != nodes.end(); ++it)
        {
            if (visited.find(*it)->second != depth)
            {
                ++depth;
                layers.push_back(it);
            }
        }
        layers.push_back(nodes.end());
        layers.shrink_to_fit();

        nodes.swap(sorted);

        orderDirty = false;
    }

    void EvaluateNode(Node* node)
    {
        switch (node->m_gate)
        {
        case Gate::OR:
            node->m_state = false;
            for (Wire* wire : node->m_wires)
            {
                if (wire->start == node)
                    continue;

                if (wire->start->m_state)
                {
                    node->m_state = true;
                    break;
                }
            }
            break;

        case Gate::NOR:
            node->m_state = true;
            for (Wire* wire : node->m_wires)
            {
                if (wire->start == node)
                    continue;

                if (wire->start->m_state)
                {
                    node->m_state = false;
                    break;
                }
            }
            break;

        case Gate::AND:
            if (node->m_inputs == 0)
            {
                node->m_state = false;
                break;
            }

            node->m_state = true;
            for (Wire* wire : node->m_wires)
            {
                if (wire->start == node)
                    continue;

                if (!wire->start->m_state)
                {
                    node->m_state = false;
                    break;
                }
            }
            break;

        case Gate::XOR:
            node->m_state = false;
            bool x = false;
            for (Wire* wire : node->m_wires)
            {
                if (wire->start == node)
                    continue;

                if (wire->start->m_state)
                {
                    if (x)
                    {
                        node->m_state = false;
                        break;
                    }
                    else
                    {
                        x = true;
                        node->m_state = true;
                    }
                }
            }
            break;
        }
    }

    void EvaluateStep(size_t depth)
    {
        if (orderDirty)
        {
            Sort();
            orderDirty = false;
        }

        if (depth >= LayerCount())
            return;

        for (decltype(nodes)::const_iterator it = layers[depth]; it != layers[depth + 1] && it != nodes.end(); ++it)
        {
            EvaluateNode(*it);
        }
    }

    void Evaluate()
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

    void DrawWires() const
    {
        for (Wire* wire : wires)
        {
            wire->Draw(wire->start->GetState() ? wire->g_wireColorActive : wire->g_wireColorInactive);
        }
    }
    void DrawNodes() const
    {
        for (Node* node : nodes)
        {
            node->Draw(node->GetState() ? node->g_nodeColorActive : node->g_nodeColorInactive);
        }
    }

    Node* FindNodeAtPos(IVec2 pos) const
    {
        auto it = nodeGrid.find(pos);
        if (it != nodeGrid.end())
            return it->second;
        return nullptr;
    }
    Wire* FindWireAtPos(IVec2 pos) const
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
    Wire* FindWireElbowAtPos(IVec2 pos) const
    {
        auto it = std::find_if(wires.begin(), wires.end(), [&pos](Wire* wire) { return wire->elbow == pos; });
        if (it != wires.end())
            return *it;
        return nullptr;
    }

    void FindNodesInRect(std::vector<Node*>& result, IRect rec) const
    {
        // Find by node
        size_t area = (size_t)rec.w * (size_t)rec.h;
        if (area < nodes.size())
        {
            IVec2 pt;
            for (pt.x = rec.x; pt.x < (rec.x + rec.w); ++pt.x)
            {
                for (pt.y = rec.y; pt.y < (rec.y + rec.h); ++pt.y)
                {
                    auto it = nodeGrid.find(pt);
                    if (it != nodeGrid.end())
                        result.push_back(it->second);
                }
            }
        }
        // Find by grid
        else
        {
            for (Node* node : nodes)
            {
                if (InBoundingBox(rec, node->GetPosition()))
                    result.push_back(node);
            }
        }
    }

public: // Serialization

    void SpawnBlueprint(Blueprint* bp, IVec2 topLeft)
    {
	    std::unordered_set<NodeBP*, Node*> nodeID;
        nodes.reserve(nodes.size() + bp->nodes.size());
        for (const NodeBP& node_bp : bp->nodes)
        {
            Node* node = CreateNode(node_bp.relativePosition + topLeft, node_bp.gate);
		nodeID.insert({ &node_bp, node });
        }
        wires.reserve(wires.size() + bp->wires.size());
        for (const WireBP& wire_bp : bp->wires)
        {
			Node* start = nodeID.find(&(bp->nodes[wire_bp.startNodeIndex]))->second;
			Node* end = nodeID.find(&(bp->nodes[wire_bp.endNodeIndex]))->second;
            Wire* wire = CreateWire(start, end);
            wire->elbowConfig = wire_bp.elbowConfig;
            wire->UpdateElbowToLegal();
        }
    }
    void StoreBlueprint(Blueprint* bp)
    {
        blueprints.push_back(bp);
    }

    // Larger file, faster startup/save (less analysis)
    void Save_LargeFile(const char* filename) const
    {
        std::ofstream file(filename, std::fstream::out | std::fstream::trunc);
        {
            file << "0 l\n" << std::time(nullptr) << '\n';

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

                file << "\nn " << nodes.size() << '\n';
                for (Node* node : nodes)
                {
                    file <<
                        (char)node->m_gate << ' ' << node->m_state << ' ' <<
                        node->m_position.x << ' ' << node->m_position.y << ' ' <<
                        node->m_wires.size() << ' ' << node->m_inputs;
                    for (Wire* wire : node->m_wires)
                    {
                        size_t wireID = wireIDs.find(wire)->second;
                        file << ' ' << wireID;
                    }
                    file << '\n';
                }

                file << "\ns " << startNodes.size();
                for (Node* node : startNodes)
                {
                    size_t nodeID = nodeIDs.find(node)->second;
                    file << ' ' << nodeID;
                }
                file << '\n';

                file << "\nw " << wires.size() << '\n';
                for (Wire* wire : wires)
                {
                    size_t startID = nodeIDs.find(wire->start)->second;
                    size_t endID = nodeIDs.find(wire->end)->second;
                    file <<
                        (int)wire->elbowConfig << ' ' <<
                        wire->elbow.x << ' ' << wire->elbow.y << ' ' <<
                        startID << ' ' << endID <<
                        '\n';
                }
            }
        }
        file.close();
    }

    // Smaller file, slower startup/save (more analysis)
    void Save_SmallFile(const char* filename) const
    {
        std::ofstream file(filename, std::fstream::out | std::fstream::trunc);
        {
            file << "0 s\n" << std::time(nullptr) << '\n';

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
    void Load(const char* filename)
    {
        std::ifstream file(filename, std::fstream::in);
        {
            int version;
            char relSize;
            time_t time;
            file >> version;
            if (version != 0) return;
            file >> relSize >> time;

            if (relSize == 'l') // Large
            {

            }
            else if (relSize == 's') // Small
            {
                
            }
        }
        file.close();
    }
};

void Node::SetPosition(IVec2 position)
{
    NodeWorld::Get().MoveNode(this, position);
    SetPosition_Temporary(position);
}

int main()
{
    int windowWidth = 1280;
    int windowHeight = 720;
    InitWindow(windowWidth, windowHeight, "Electron Architect");
    SetExitKey(0);
    SetTargetFPS(60);

    /******************************************
    *   Load textures, shaders, and meshes
    ******************************************/

    enum class Mode {
        PEN,
        EDIT,
        ERASE,
        INTERACT,

        GATE,
        BUTTON,
        PASTE,
    } mode, baseMode;

    Texture2D clipboardIcon = LoadTexture("icon_clipboard.png");

    Texture2D modeIcons = LoadTexture("icons_mode.png");
    auto DrawModeIcon = [&modeIcons](Mode mode, IRect dest, Color tint)
    {
        constexpr Int_t width = 16;
        IVec2 offset;
        switch (mode)
        {
        case Mode::PEN:      offset = IVec2(    0,     0); break;
        case Mode::EDIT:     offset = IVec2(width,     0); break;
        case Mode::ERASE:    offset = IVec2(    0, width); break;
        case Mode::INTERACT: offset = IVec2(width, width); break;
        default: return;
        }
        BeginScissorMode(dest.x, dest.y, dest.w, dest.h); {
            DrawTexture(modeIcons, dest.x - offset.x, dest.y - offset.y, tint);
        } EndScissorMode();
    };

    Texture2D gateIcons16x = LoadTexture("icons_gate16x.png");
    auto DrawGateIcon16x = [&gateIcons16x](Gate gate, IRect dest, Color tint)
    {
        constexpr Int_t width = 16;
        IVec2 offset;
        switch (gate)
        {
        case Gate::OR:  offset = IVec2(    0,     0); break;
        case Gate::AND: offset = IVec2(width,     0); break;
        case Gate::NOR: offset = IVec2(    0, width); break;
        case Gate::XOR: offset = IVec2(width, width); break;
        default: return;
        }
        BeginScissorMode(dest.x, dest.y, dest.w, dest.h); {
            DrawTexture(gateIcons16x, dest.x - offset.x, dest.y - offset.y, tint);
        } EndScissorMode();
    };
    
    Texture2D gateIcons32x = LoadTexture("icons_gate32x.png");
    auto DrawGateIcon32x = [&gateIcons32x](Gate gate, IRect dest, Color tint)
    {
        constexpr Int_t width = 32;
        IVec2 offset;
        switch (gate)
        {
        case Gate::OR:  offset = IVec2(    0,     0); break;
        case Gate::AND: offset = IVec2(width,     0); break;
        case Gate::NOR: offset = IVec2(    0, width); break;
        case Gate::XOR: offset = IVec2(width, width); break;
        default: return;
        }
        BeginScissorMode(dest.x, dest.y, dest.w, dest.h); {
            DrawTexture(gateIcons32x, dest.x - offset.x, dest.y - offset.y, tint);
        } EndScissorMode();
    };

    struct {
        Gate gatePick = Gate::OR;
        Gate lastGate = Gate::OR;
        Node* hoveredNode = nullptr;
        Wire* hoveredWire = nullptr;
        Blueprint* clipboard = nullptr;
        std::vector<Node*> selection;

        union // Base mode
        {
            struct {
                Node* currentWireStart;
                decltype(Wire::elbowConfig) currentWireElbowConfig;
            } pen;

            struct {
                IVec2 fallbackPos;
                bool selectionWIP;
                Node* nodeBeingDragged;
                Wire* wireBeingDragged;
                IVec2 selectionStart;
                IRect selectionRec;
            } edit;

            struct {
            } erase;

            struct {
            } interact = {};
        };
        union // Overlay mode - doesn't affect other modes
        {
            struct {
                IVec2 radialMenuCenter;
                uint8_t overlappedSection;
            } gate;

            struct {
                int dropdownActive;
            } button;

            struct {
            } paste = {};
        };
    } data;

    constexpr Mode dropdownModeOrder[] = {
        Mode::PEN,
        Mode::EDIT,
        Mode::ERASE,
        Mode::INTERACT,
    };
    constexpr Gate dropdownGateOrder[] = {
        Gate::OR,
        Gate::AND,
        Gate::NOR,
        Gate::XOR,
    };
    constexpr IRect dropdownBounds[] = {
        IRect( 0, 16, 16, 16 * (_countof(dropdownModeOrder) - 1)), // Mode
        IRect(16, 16, 16, 16 * (_countof(dropdownGateOrder) - 1)), // Gate
    };

    constexpr Gate radialGateOrder[] = {
        Gate::XOR,
        Gate::AND,
        Gate::OR,
        Gate::NOR,
    };

    IVec2 cursorPosPrev = IVec2Zero(); // For checking if there was movement

    auto SetMode = [&baseMode, &mode, &data, &cursorPosPrev](Mode newMode)
    {
        cursorPosPrev = IVec2(-1,-1);
        mode = newMode;
        switch (newMode)
        {
        case Mode::PEN:
            baseMode = Mode::PEN;
            data.pen.currentWireStart = nullptr;
            data.pen.currentWireElbowConfig = 0;
            break;

        case Mode::EDIT:
            baseMode = Mode::EDIT;
            data.edit.fallbackPos = IVec2Zero();
            data.edit.selectionWIP = false;
            data.edit.nodeBeingDragged = nullptr;
            data.edit.wireBeingDragged = nullptr;
            data.edit.selectionStart = IVec2Zero();
            data.edit.selectionRec = IRect(0,0,0,0);
            break;

        case Mode::ERASE:
            baseMode = Mode::ERASE;
            break;

        case Mode::INTERACT:
            baseMode = Mode::INTERACT;
            break;

        case Mode::GATE:
            data.gate.radialMenuCenter = IVec2Zero();
            data.gate.overlappedSection = 0;
            break;

        case Mode::BUTTON:
            data.button.dropdownActive = 0;
            break;

        case Mode::PASTE:
            break;
        }
    };

    mode = Mode::PEN;
    SetMode(Mode::PEN);

    NodeWorld::Get(); // Construct
        
    while (!WindowShouldClose())
    {
        /******************************************
        *   Simulate frame and update variables
        ******************************************/

        IVec2 cursorPos{
            GetMouseX() / g_gridSize,
            GetMouseY() / g_gridSize
        };
        cursorPos = IVec2Scale_i(cursorPos, g_gridSize) + IVec2(g_gridSize / 2, g_gridSize / 2);

        bool b_cursorMoved = cursorPosPrev != cursorPos;
        cursorPosPrev = cursorPos;

        // Hotkeys
        {
            if (IsKeyPressed(KEY_ONE))
                data.gatePick = Gate::OR;
            else if (IsKeyPressed(KEY_TWO))
                data.gatePick = Gate::AND;
            else if (IsKeyPressed(KEY_THREE))
                data.gatePick = Gate::NOR;
            else if (IsKeyPressed(KEY_FOUR))
                data.gatePick = Gate::XOR;

            // KEY COMBOS BEFORE INDIVIDUAL KEYS!

            // Copy
            if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_C))
            {
                if (data.clipboard != nullptr)
                    delete data.clipboard;

                if (data.selection.empty())
                    data.clipboard = nullptr;
                else
                    data.clipboard = new Blueprint(data.selection);
            }
            // Paste
            else if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_V) && !!data.clipboard)
            {
                SetMode(Mode::PASTE);
            }

            else if (IsKeyPressed(KEY_B))
            {
                SetMode(Mode::PEN);
            }
            else if (IsKeyPressed(KEY_V))
            {
                SetMode(Mode::EDIT);
            }
            else if (IsKeyPressed(KEY_G))
            {
                SetMode(Mode::GATE);
                data.gate.radialMenuCenter = cursorPos;
            }
            else if (IsKeyPressed(KEY_X))
            {
                SetMode(Mode::ERASE);
            }
            else if (IsKeyPressed(KEY_F))
            {
                SetMode(Mode::INTERACT);
            }
            else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && (cursorPos.y <= 16 && cursorPos.x <= 32) &&
                (mode == Mode::BUTTON ? data.button.dropdownActive != (cursorPos.x / 16) : true))
            {
                SetMode(Mode::BUTTON);
                data.button.dropdownActive = cursorPos.x / 16;
                goto EVAL;
            }
        }

        // Input
        switch (mode)
        {
        case Mode::PEN:
        {
            if (b_cursorMoved)
            {
                data.hoveredWire = nullptr;
                data.hoveredNode = NodeWorld::Get().FindNodeAtPos(cursorPos);
                if (!data.hoveredNode)
                    data.hoveredWire = NodeWorld::Get().FindWireAtPos(cursorPos);
            }

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                Node* newNode = data.hoveredNode;
                if (!newNode)
                {
                    newNode = NodeWorld::Get().CreateNode(cursorPos, data.gatePick);
                    if (!!data.hoveredWire)
                    {
                        NodeWorld::Get().BisectWire(data.hoveredWire, newNode);
                        data.hoveredWire = nullptr;
                    }
                }
                // Do not create a new node/wire if already hovering the start node
                if (!!data.pen.currentWireStart && newNode != data.pen.currentWireStart)
                {
                    Wire* wire = NodeWorld::Get().CreateWire(data.pen.currentWireStart, newNode);
                    wire->elbowConfig = data.pen.currentWireElbowConfig;
                    wire->UpdateElbowToLegal();
                }
                data.pen.currentWireStart = newNode;
            }
            else if (IsKeyPressed(KEY_R))
            {
                if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
                    data.pen.currentWireElbowConfig--;
                else
                    data.pen.currentWireElbowConfig++;

                data.pen.currentWireElbowConfig %= 4;
            }
            else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
            {
                data.pen.currentWireStart = nullptr;
            }
        }
        break;

        case Mode::EDIT:
        {
            if (b_cursorMoved)
            {
                if (!data.edit.nodeBeingDragged &&
                    !data.edit.wireBeingDragged)
                {
                    data.hoveredWire = nullptr;
                    data.hoveredNode = NodeWorld::Get().FindNodeAtPos(cursorPos);
                    if (!data.hoveredNode)
                        data.hoveredWire = NodeWorld::Get().FindWireElbowAtPos(cursorPos);
                }
            }

            // Press
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                data.selection.clear();
                data.edit.nodeBeingDragged = data.hoveredNode;
                data.edit.wireBeingDragged = data.hoveredWire;
                data.edit.selectionStart = data.edit.fallbackPos = cursorPos;
                data.edit.selectionWIP = !(data.edit.nodeBeingDragged || data.edit.wireBeingDragged);
            }

            // Selection
            if (data.edit.selectionWIP)
            {
                auto [minx, maxx] = std::minmax(cursorPos.x, data.edit.selectionStart.x);
                auto [miny, maxy] = std::minmax(cursorPos.y, data.edit.selectionStart.y);
                data.edit.selectionRec.w = maxx - (data.edit.selectionRec.x = minx);
                data.edit.selectionRec.h = maxy - (data.edit.selectionRec.y = miny);
            }

            // Release
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) || (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)))
            {
                // Cancel
                if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
                {
                    if (!!data.edit.nodeBeingDragged)
                    {
                        data.edit.nodeBeingDragged->SetPosition(data.edit.fallbackPos);
                        data.edit.nodeBeingDragged = nullptr;
                    }
                    else if (data.edit.selectionWIP)
                    {
                        data.edit.selectionRec = IRect(0,0,0,0);
                        data.edit.selectionWIP = false;
                    }
                }
                // Finalize
                else
                {
                    if (!!data.edit.nodeBeingDragged)
                    {
                        data.hoveredNode = NodeWorld::Get().FindNodeAtPos(cursorPos);
                        if (!!data.hoveredNode && data.edit.nodeBeingDragged != data.hoveredNode)
                            data.hoveredNode = data.edit.nodeBeingDragged = NodeWorld::Get().MergeNodes(data.edit.nodeBeingDragged, data.hoveredNode);

                        data.edit.nodeBeingDragged->SetPosition(cursorPos);
                        data.edit.nodeBeingDragged = nullptr;
                    }
                    else if (data.edit.selectionWIP)
                    {
                        NodeWorld::Get().FindNodesInRect(data.selection, data.edit.selectionRec);
                        data.edit.selectionWIP = false;
                    }
                }
                data.edit.wireBeingDragged = nullptr;
            }
            else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && !!data.hoveredNode)
            {
                data.hoveredNode->SetGate(data.gatePick);
            }
            // Node
            else if (!!data.edit.nodeBeingDragged)
            {
                data.edit.nodeBeingDragged->SetPosition_Temporary(cursorPos);
            }

            // Wire
            else if (!!data.edit.wireBeingDragged)
            {
                data.edit.wireBeingDragged->SnapElbowToLegal(cursorPos);
            }
        }
        break;

        case Mode::GATE:
        {
            if (b_cursorMoved)
            {
                if (cursorPos.x < data.gate.radialMenuCenter.x)
                {
                    if (cursorPos.y < data.gate.radialMenuCenter.y)
                        data.gate.overlappedSection = 2;
                    else // cursorPos.y > data.gate.radialMenuCenter.y
                        data.gate.overlappedSection = 3;
                }
                else // cursorPos.x > data.gate.radialMenuCenter.x
                {
                    if (cursorPos.y < data.gate.radialMenuCenter.y)
                        data.gate.overlappedSection = 1;
                    else // cursorPos.y > data.gate.radialMenuCenter.y
                        data.gate.overlappedSection = 0;
                }
            }

            bool leftMouse = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
            if (leftMouse || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
            {
                if (leftMouse)
                    data.gatePick = radialGateOrder[data.gate.overlappedSection];

                mode = baseMode;
                SetMousePosition(data.gate.radialMenuCenter.x, data.gate.radialMenuCenter.y);
                cursorPos = data.gate.radialMenuCenter;
            }
        }
        break;

        case Mode::ERASE:
        {
            if (b_cursorMoved)
            {
                data.hoveredWire = nullptr;
                data.hoveredNode = NodeWorld::Get().FindNodeAtPos(cursorPos);
                if (!data.hoveredNode)
                    data.hoveredWire = NodeWorld::Get().FindWireAtPos(cursorPos);
            }

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                if (!!data.hoveredNode)
                    NodeWorld::Get().DestroyNode(data.hoveredNode);
                else if (!!data.hoveredWire)
                    NodeWorld::Get().DestroyWire(data.hoveredWire);

                data.hoveredNode = nullptr;
                data.hoveredWire = nullptr;
            }
        }
        break;

        case Mode::INTERACT:
        {
            data.hoveredNode = NodeWorld::Get().FindNodeAtPos(cursorPos);
            if (!!data.hoveredNode && !data.hoveredNode->IsOutputOnly())
                data.hoveredNode = nullptr;

            if (!!data.hoveredNode && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                data.hoveredNode->SetGate(data.hoveredNode->GetGate() == Gate::NOR ? Gate::OR : Gate::NOR);
        }
        break;

        case Mode::BUTTON:
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                IRect rec = dropdownBounds[data.button.dropdownActive];
                if (InBoundingBox(rec, cursorPos))
                {
                    rec.h = 16;

                    switch (data.button.dropdownActive)
                    {
                    case 0: // Mode
                    {
                        for (Mode m : dropdownModeOrder)
                        {
                            if (m == baseMode)
                                continue;

                            if (InBoundingBox(rec, cursorPos))
                            {
                                SetMode(m);
                                break;
                            }

                            rec.y += 16;
                        }
                    }
                    break;

                    case 1: // Gate
                    {
                        for (Gate g : dropdownGateOrder)
                        {
                            if (g == data.gatePick)
                                continue;

                            if (InBoundingBox(rec, cursorPos))
                            {
                                data.gatePick = g;
                                break;
                            }

                            rec.y += 16;
                        }

                        SetMode(baseMode);
                    }
                    break;
                    }
                }
                else
                    SetMode(baseMode);
            }
            else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
            {
                SetMode(baseMode);
            }
        }
        break;

        case Mode::PASTE:
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                NodeWorld::Get().SpawnBlueprint(data.clipboard, cursorPos);
                data.selection.clear();
                SetMode(baseMode);
            }
        }
        break;
        }

    EVAL:
        NodeWorld::Get().Evaluate();

        /******************************************
        *   Draw the frame
        ******************************************/

        BeginDrawing(); {

            ClearBackground(BLACK);

            // Grid
            for (Int_t y = 0; y < windowHeight; y += g_gridSize)
            {
                DrawLine(0, y, windowWidth, y, SPACEGRAY);
            }
            for (Int_t x = 0; x < windowWidth; x += g_gridSize)
            {
                DrawLine(x, 0, x, windowHeight, SPACEGRAY);
            }

            // Draw
            switch (mode)
            {
            case Mode::PEN:
            {
                NodeWorld::Get().DrawWires();

                if (!!data.pen.currentWireStart)
                {
                    IVec2 start = data.pen.currentWireStart->GetPosition();
                    IVec2 elbow;
                    IVec2 end = cursorPos;
                    elbow = Wire::GetLegalElbowPosition(start, end, data.pen.currentWireElbowConfig);
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
            break;

            case Mode::EDIT:
            {
                DrawRectangleIRect(data.edit.selectionRec, ColorAlpha(SPACEGRAY, 0.5));
                DrawRectangleLines(data.edit.selectionRec.x, data.edit.selectionRec.y, data.edit.selectionRec.w, data.edit.selectionRec.h, LIFELESSNEBULA);

                NodeWorld::Get().DrawWires();

                for (Node* node : data.selection)
                {
                    DrawCircleIV(node->GetPosition(), node->g_nodeRadius + 3, WIPBLUE);
                }

                if (!!data.hoveredWire)
                {
                    IVec2 pts[4];
                    data.hoveredWire->GetLegalElbowPositions(pts);
                    for (const IVec2& p : pts)
                    {
                        Wire::Draw(data.hoveredWire->GetStartPos(), p, data.hoveredWire->GetEndPos(), ColorAlpha(WIPBLUE, 0.25f));
                        DrawCircle(p.x, p.y, Wire::g_elbowRadius, ColorAlpha(WIPBLUE, 0.5f));
                    }

                    data.hoveredWire->Draw(WIPBLUE);
                    Color elbowColor;
                    if (!!data.edit.wireBeingDragged)
                        elbowColor = WIPBLUE;
                    else
                        elbowColor = CAUTIONYELLOW;
                    data.hoveredWire->DrawElbow(elbowColor);
                }

                NodeWorld::Get().DrawNodes();

                if (!!data.hoveredNode)
                {
                    data.hoveredNode->Draw(CAUTIONYELLOW);
                }
                if (!!data.hoveredWire)
                {
                    data.hoveredWire->start->Draw(INPUTLAVENDER);
                    data.hoveredWire->end->Draw(OUTPUTAPRICOT);
                }
            }
            break;

            case Mode::ERASE:
            {
                auto DrawCross = [](IVec2 center, Color color)
                {
                    int radius = 3;
                    int expandedRad = radius + 1;
                    DrawRectangle(center.x - expandedRad, center.y - expandedRad, expandedRad * 2, expandedRad * 2, BLACK);
                    DrawLine(center.x - radius, center.y - radius,
                        center.x + radius, center.y + radius,
                        color);
                    DrawLine(center.x - radius, center.y + radius,
                        center.x + radius, center.y - radius,
                        color);
                };

                NodeWorld::Get().DrawWires();

                if (!!data.hoveredWire)
                {
                    data.hoveredWire->Draw(MAGENTA);
                    DrawCross(cursorPos, DESTRUCTIVERED);
                }
                else if (!!data.hoveredNode)
                {
                    for (Wire* wire : data.hoveredNode->GetWires())
                    {
                        wire->Draw(MAGENTA);
                    }
                }

                NodeWorld::Get().DrawNodes();

                if (!!data.hoveredNode)
                {
                    data.hoveredNode->Draw(BLACK);
                    DrawCross(data.hoveredNode->GetPosition(), DESTRUCTIVERED);
                }
            }
            break;

            case Mode::INTERACT:
            {
                NodeWorld::Get().DrawWires();
                NodeWorld::Get().DrawNodes();

                for (const Node* node : NodeWorld::Get().GetStartNodes())
                {
                    node->Draw(WIPBLUE);
                }

                if (!!data.hoveredNode)
                {
                    data.hoveredNode->Draw(CAUTIONYELLOW);
                }
            }
            break;

            case Mode::GATE:
            {
                if (baseMode == Mode::PEN)
                {
                    NodeWorld::Get().DrawWires();

                    if (!!data.pen.currentWireStart)
                    {
                        IVec2 start = data.pen.currentWireStart->GetPosition();
                        IVec2 elbow;
                        IVec2 end = data.gate.radialMenuCenter;
                        elbow = Wire::GetLegalElbowPosition(start, end, data.pen.currentWireElbowConfig);
                        Wire::Draw(start, elbow, end, WIPBLUE);
                        Node::Draw(end, data.gatePick, WIPBLUE);
                    }

                    NodeWorld::Get().DrawNodes();
                }
                else
                {
                    NodeWorld::Get().DrawWires();
                    NodeWorld::Get().DrawNodes();
                }

                constexpr int menuRadius = 64;
                constexpr int menuIconOffset = 12;
                constexpr IVec2 menuOff[4] = {
                    IVec2(0, 0),
                    IVec2(0,-1),
                    IVec2(-1,-1),
                    IVec2(-1, 0),
                };
                constexpr IRect iconDest[4] = {
                    IRect(menuIconOffset,       menuIconOffset,      32, 32),
                    IRect(menuIconOffset,      -menuIconOffset - 32, 32, 32),
                    IRect(-menuIconOffset - 32, -menuIconOffset - 32, 32, 32),
                    IRect(-menuIconOffset - 32,  menuIconOffset,      32, 32),
                };
                int x = data.gate.radialMenuCenter.x;
                int y = data.gate.radialMenuCenter.y;
                constexpr Color menuBackground = SPACEGRAY;
                DrawCircleIV(data.gate.radialMenuCenter, static_cast<float>(menuRadius + 4), menuBackground);

                for (int i = 0; i < 4; ++i)
                {
                    Color colorA;
                    Color colorB;
                    int radius;

                    if (i == data.gate.overlappedSection) // Active
                    {
                        colorA = GLEEFULDUST;
                        colorB = INTERFERENCEGRAY;
                        radius = menuRadius + 8;
                    }
                    else // Inactive
                    {
                        colorA = LIFELESSNEBULA;
                        colorB = HAUNTINGWHITE;
                        radius = menuRadius;
                    }
                    BeginScissorMode(x + (menuOff[i].x * 8 + 4) + menuOff[i].x * radius, y + (menuOff[i].y * 8 + 4) + menuOff[i].y * radius, radius, radius);

                    float startAngle = static_cast<float>(i * 90);
                    DrawCircleSector({ static_cast<float>(x), static_cast<float>(y) }, static_cast<float>(radius), startAngle, startAngle + 90.0f, 8, colorA);
                    IRect rec = iconDest[i];
                    rec.x += x;
                    rec.y += y;
                    DrawGateIcon32x(radialGateOrder[i], rec, colorB);

                    EndScissorMode();
                }
            }
            break;

            case Mode::BUTTON:
            {
                NodeWorld::Get().DrawWires();
                NodeWorld::Get().DrawNodes();

                IRect rec = dropdownBounds[data.button.dropdownActive];
                DrawRectangleIRect(dropdownBounds[data.button.dropdownActive], SPACEGRAY);
                rec.h = 16;

                switch (data.button.dropdownActive)
                {
                case 0: // Mode
                {
                    for (Mode m : dropdownModeOrder)
                    {
                        if (m == baseMode)
                            continue;
                        Color color = InBoundingBox(rec, cursorPos) ? WHITE : DEADCABLE;
                        DrawModeIcon(m, rec, color);
                        rec.y += 16;
                    }
                }
                break;

                case 1: // Gate
                {
                    for (Gate g : dropdownGateOrder)
                    {
                        if (g == data.gatePick)
                            continue;
                        Color color = InBoundingBox(rec, cursorPos) ? WHITE : GRAY;
                        DrawGateIcon16x(g, rec, color);
                        rec.y += 16;
                    }
                }
                break;
                }
            }
            break;

            case Mode::PASTE:
            {
                NodeWorld::Get().DrawWires();
                NodeWorld::Get().DrawNodes();

                IRect rec = data.clipboard->bounds;
                rec.xy = cursorPos;
                DrawRectangleIRect(rec, ColorAlpha(LIFELESSNEBULA, 0.5f));
            }
            break;
            }

            // Global UI

            // Buttons
            if (cursorPos.y <= 16)
            {
                if (cursorPos.x <= 16)
                {
                    const char* text;
                    switch (baseMode)
                    {
                    case Mode::PEN:   text = "Mode: Draw";        break;
                    case Mode::EDIT:  text = "Mode: Edit";        break;
                    case Mode::GATE:  text = "Mode: Gate select"; break;
                    case Mode::ERASE: text = "Mode: Erase";       break;
                    default:          text = "";                  break;
                    }
                    DrawText(text, 20, 17, 8, WHITE);
                    DrawRectangle(0, 0, 16, 16, SPACEGRAY);
                }
                else if (cursorPos.x <= 32)
                {
                    const char* text;
                    switch (data.gatePick)
                    {
                    case Gate::OR:  text = "Gate: | (or)";  break;
                    case Gate::AND: text = "Gate: & (and)"; break;
                    case Gate::NOR: text = "Gate: ! (nor)"; break;
                    case Gate::XOR: text = "Gate: ^ (xor)"; break;
                    default:        text = "";              break;
                    }
                    DrawText(text, 36, 17, 8, WHITE);
                    DrawRectangle(16, 0, 16, 16, SPACEGRAY);
                }
            }

            IRect rec = { 0, 0, 16, 16 };
            DrawRectangleIRect(rec, SPACEGRAY);
            DrawModeIcon(baseMode, rec, WHITE);
            rec.x += 16;
            DrawRectangleIRect(rec, SPACEGRAY);
            DrawGateIcon16x(data.gatePick, rec, WHITE);
            if (!!data.clipboard)
            {
                rec.x += 16;
                DrawRectangleIRect(rec, SPACEGRAY);
                DrawTexture(clipboardIcon, rec.x, rec.y, WHITE);
            }

        } EndDrawing();
    }

    /******************************************
    *   Unload and free memory
    ******************************************/

    if (data.clipboard != nullptr)
        delete data.clipboard;

    NodeWorld::Get().Save_LargeFile("dataL.cg");
    NodeWorld::Get().Save_SmallFile("dataS.cg");

    UnloadTexture(gateIcons32x);
    UnloadTexture(gateIcons16x);
    UnloadTexture(modeIcons);
    UnloadTexture(clipboardIcon);

    CloseWindow();

	return 0;
}

/* Todo
* Major
* -Selection move-together and delete-together
* -Blueprint pallet
* -Blueprint pallet icons (User-made combination of 4 premade icons. See Factorio for inspiration)
* -Save/load
* -Save file menu
* -Save file thumbnails (based on cropped screenshot)
* -Menu screen (Open to file menu with "new" at the top)
*
* Quality of Life
* -Special erase (keep wires, erase node)
* -Groups (labelled persistent selections)
* -Hotkeys for output-only gate state toggles (Like the Reason on-screen piano)
*
* Stretch goals
* -Multiple color pallets
* -Step-by-step evaluation option
* -Log files for debug/crashes
*/
