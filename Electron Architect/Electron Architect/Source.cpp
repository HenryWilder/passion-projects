#include <vector>
#include <unordered_set>
#include <queue>
#include <algorithm>
#include <raylib.h>
#include <limits.h>
//#include <raymath.h>
//#include <extras\raygui.h>

// UI Colors
namespace uicol
{
    namespace wire
    {
        constexpr Color hidden = { 255, 255, 255,  10 }; // Gray (12.5%)
        constexpr Color ghost  = {  };
        constexpr Color normal = {  };
        constexpr Color hover  = {  };
        constexpr Color drag   = {  };
    }
    namespace wireElbow
    {
        constexpr Color hidden = {  }; // Dark green (25%)
        constexpr Color ghost  = {  }; // Dark green (50%)
        constexpr Color normal = {  }; // Dark green
        constexpr Color hover  = {  }; // Lime green
        constexpr Color drag   = {  }; // Green
    }
    namespace node
    {
        constexpr Color hidden = {  }; // Dark gray
        constexpr Color ghost  = {  }; // Gray
        constexpr Color normal = {  }; // Light gray
        constexpr Color hover  = {  }; // Yellow
        constexpr Color drag   = {  }; // White
    }
}

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

struct IRect
{
    IRect() = default;
    constexpr IRect(Int_t x, Int_t y, Int_t w, Int_t h) : x(x), y(y), w(w), h(h) {}

    Int_t x, y, w, h;
};

void DrawLineIV(IVec2 start, IVec2 end, Color color)
{
    DrawLine(start.x, start.y, end.x, end.y, color);
}
void DrawWireGeneric(IVec2 start, IVec2 joint, IVec2 end, Color color)
{
    DrawLineIV(start, joint, color);
    DrawLineIV(joint, end, color);
}

class NodeWorld;
class Node;
struct Wire
{
    Wire() = default;
    Wire(Node* start, Node* end) : elbow(), elbowConfig(0), start(start), end(end) {}

    static constexpr Color g_wireColorActive = BROWN;
    static constexpr Color g_wireColorInactive = GRAY;
    static constexpr float g_elbowRadius = 2.0f;

    IVec2 elbow;
    size_t elbowConfig;
    Node* start;
    Node* end;

    void Draw(Color color) const
    {
        DrawWireGeneric(GetStartPos(), elbow, GetEndPos(), color);
    }
    void DrawElbow(Color color) const
    {
        DrawCircle(elbow.x, elbow.y, g_elbowRadius, color);
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
	
    IVec2 GetLegalElbowPosition(decltype(elbowConfig) index) const
    {
        _ASSERT_EXPR(index < 4, "Subscript error");
        if (index == 0)
            return IVec2(GetStartX(), GetEndY());
        else if (index == 1)
            return IVec2(GetEndX(), GetStartY());
        else
        {
            Int_t shortLength = std::min(
                abs(GetEndX() - GetStartX()),
                abs(GetEndY() - GetStartY())
            );
            IVec2 pos;
            if (index == 2)
            {
                pos = GetStartPos();

                if (GetEndX() < GetStartX())
                    pos.x -= shortLength;
                else
                    pos.x += shortLength;

                if (GetEndY() < GetStartY())
                    pos.y -= shortLength;
                else
                    pos.y += shortLength;
            }
            else // index == 3
            {
                pos = GetEndPos();

                if (GetStartX() < GetEndX())
                    pos.x -= shortLength;
                else
                    pos.x += shortLength;

                if (GetStartY() < GetEndY())
                    pos.y -= shortLength;
                else
                    pos.y += shortLength;
            }
            return pos;
        }
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

        size_t pick = 0;
        long shortestDist = LONG_MAX;
        for (size_t i = 0; i < 4; ++i)
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
    void SetPosition(IVec2 position)
    {
        m_position = position;
    }
    Int_t GetX() const
    {
        return m_position.x;
    }
    void SetX(Int_t x)
    {
        m_position.x = x;
    }
    Int_t GetY() const
    {
        return m_position.y;
    }
    void SetY(Int_t y)
    {
        m_position.y = y;
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
    bool IsInputOnly() const
    {
        return !m_inputs;
    }

    void Draw(Color color) const
    {
        switch (m_gate)
        {
        case Gate::OR:
            DrawCircle(m_position.x, m_position.y, g_nodeRadius, color);
            break;
        case Gate::AND:
            DrawRectangle(m_position.x - g_nodeRadius, m_position.y - g_nodeRadius, g_nodeRadius * 2, g_nodeRadius * 2, color);
            break;
        case Gate::NOR:
            DrawCircle(m_position.x, m_position.y, g_nodeRadius, color);
            DrawCircle(m_position.x, m_position.y, g_nodeRadius - 1.0f, BLACK);
            break;
        case Gate::XOR:
            DrawCircle(m_position.x, m_position.y, g_nodeRadius + 1.0f, color);
            DrawCircle(m_position.x, m_position.y, g_nodeRadius, BLACK);
            DrawCircle(m_position.x, m_position.y, g_nodeRadius - 1.0f, color);
            break;
        }
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

    static constexpr Color g_nodeColorActive = RED;
    static constexpr Color g_nodeColorInactive = LIGHTGRAY;
    static constexpr float g_nodeRadius = 3.0f;

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

class NodeWorld
{
private:
    std::vector<Node*> nodes;
    std::vector<Node*> startNodes;
    std::vector<Wire*> wires; // Inputs/outputs don't exist here
    bool orderDirty = false;

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

public:
    static NodeWorld& Get()
    {
        static NodeWorld g_only;
        return g_only;
    }

    // Node functions
    Node* CreateNode(IVec2 position, Gate gate)
    {
        Node* node = new Node(position, gate);
        nodes.insert(nodes.begin(), node);
        startNodes.push_back(node);
        // The order is not dirty at this time due to the node having no connections yet
        return node;
    }
    void DestroyNode(Node* node)
    {
        for (Wire* wire : node->m_wires)
        {
            Node* start = wire->start;
            Node* end = wire->end;

            if (start == node)
            {
                if (end->IsInputOnly())
                    startNodes.push_back(end);

                end->RemoveConnection_Expected(start);
            }
            else // wire->end == node
                start->RemoveConnection_Expected(end);
        }
        FindAndErase_ExpectExisting(nodes, node);
        delete node;
        orderDirty = true;
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

        Wire* wire = new Wire(start, end);

        wire->elbowConfig = 0;
        wire->UpdateElbowToLegal();

        wires.push_back(wire);

        start->AddWireOutput(wire);
        end->AddWireInput(wire);

        // Remove end from start nodes, as it is no longer an inputless node with this change
        FindAndErase(startNodes, end);

        orderDirty = true;
        return wire;
    }
    void DestroyWire(Wire* wire)
    {
        Node* start = wire->start;
        Node* end = wire->end;

        FindAndErase_ExpectExisting(wires, wire);
        start->RemoveWire_Expected(wire);
        end->RemoveWire_Expected(wire);
        delete wire;

        // Push end to start nodes if this has destroyed its last remaining input
        if (end->IsInputOnly())
            startNodes.push_back(end);

        orderDirty = true;
    }
    Node* MergeNodes(Node* composite, Node* tbRemoved)
    {
        _ASSERT_EXPR(!!composite && !!tbRemoved, "Tried to merge a node with nullptr");
        _ASSERT_EXPR(composite != tbRemoved, "Tried to merge a node with itself");

        for (Wire* wire : tbRemoved->m_wires)
        {
            if (wire->start == composite || wire->end == composite)
            {
                composite->RemoveWire_Expected(wire);
                delete wire;
            }
            else
            {
                if (wire->start == tbRemoved)
                {
                    wire->start = composite;
                    composite->AddWireOutput(wire);
                }
                else // wire->end == b
                {
                    wire->end = composite;
                    composite->AddWireInput(wire);
                }
            }
        }
        FindAndErase_ExpectExisting(nodes, tbRemoved);
        delete tbRemoved;
        orderDirty = true;
        return composite;
    }
    Wire* ReverseWire(Wire* wire)
    {
        // Swap
        Node* temp = wire->start;
        wire->start = wire->end;
        wire->end = temp;

        wire->start->MakeWireInput(wire);
        wire->end->MakeWireOutput(wire);

        orderDirty = true;
        return wire;
    }

    // Uses BFS
    void Sort()
    {
        decltype(nodes) sorted;
        sorted.reserve(nodes.size());

        std::queue<Node*> list;
        std::unordered_set<Node*> visited;
        for (Node* node : startNodes)
        {
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

        orderDirty = false;
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
        auto it = std::find_if(nodes.begin(), nodes.end(), [&pos](Node* node) { return node->GetPosition() == pos; });
        if (it != nodes.end())
            return *it;
        return nullptr;
    }
    Wire* FindWireAtPos(IVec2 pos) const
    {
        for (Wire* wire : wires)
        {
            if ((InBoundingBox(pos, wire->start->GetPosition(), wire->elbow) &&
                Normal(wire->elbow - wire->start->GetPosition()) == Normal(pos - wire->start->GetPosition())) ||
                (InBoundingBox(pos, wire->elbow, wire->end->GetPosition()) &&
                Normal(wire->end->GetPosition() - wire->elbow) == Normal(pos - wire->elbow)))
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
};

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
        GATE,
        ERASE,
    } mode, lastMode;

    Texture2D modeIcons = LoadTexture("icons_mode.png");
    auto DrawModeIcon = [&modeIcons](Mode mode, Rectangle dest, Color tint)
    {
        constexpr float width = 16.0f;
        Rectangle src{ 0,0,width,width };
        switch (mode)
        {
        case Mode::PEN:   src = { 0*width,0*width, width,width }; break;
        case Mode::EDIT:  src = { 1*width,0*width, width,width }; break;
        case Mode::GATE:  return;
        case Mode::ERASE: src = { 0*width,1*width, width,width }; break;
        }
        DrawTexturePro(modeIcons, src, dest, { 0,0 }, 0.0f, tint);
    };

    Texture2D gateIcons16x = LoadTexture("icons_gate16x.png");
    auto DrawGateIcon16x = [&gateIcons16x](Gate gate, Rectangle dest, Color tint)
    {
        constexpr float width = 16.0f;
        Rectangle src{ 0,0,width,width };
        switch (gate)
        {
        case Gate::OR:  src = { 0*width,0*width, width,width }; break;
        case Gate::AND: src = { 1*width,0*width, width,width }; break;
        case Gate::NOR: src = { 0*width,1*width, width,width }; break;
        case Gate::XOR: src = { 1*width,1*width, width,width }; break;
        }
        DrawTexturePro(gateIcons16x, src, dest, { 0,0 }, 0.0f, tint);
    };
    
    Texture2D gateIcons32x = LoadTexture("icons_gate32x.png");
    auto DrawGateIcon32x = [&gateIcons32x](Gate gate, Rectangle dest, Color tint)
    {
        constexpr float width = 32.0f;
        Rectangle src{ 0,0,width,width };
        switch (gate)
        {
        case Gate::OR:  src = { 0*width,0*width, width,width }; break;
        case Gate::AND: src = { 1*width,0*width, width,width }; break;
        case Gate::NOR: src = { 0*width,1*width, width,width }; break;
        case Gate::XOR: src = { 1*width,1*width, width,width }; break;
        }
        DrawTexturePro(gateIcons32x, src, dest, { 0,0 }, 0.0f, tint);
    };

    struct {
        Node* hoveredNode;
        Wire* hoveredWire;
        Gate gatePick;
        union
        {
            struct {
                Node* currentWireStart;
            } pen;

            struct {
                Node* nodeBeingDragged;
                Wire* wireBeingDragged;
                bool selectionWIP;
                Int_t selectionRectangleX;
                Int_t selectionRectangleY;
                Int_t selectionRectangleWidth;
                Int_t selectionRectangleHeight;
            } edit;

            struct {
                IVec2 radialMenuCenter;
                uint8_t overlappedSection;
            } gate;

            struct {
            } erase;
        };
    } data;

    data.hoveredNode = nullptr;
    data.hoveredWire = nullptr;
    data.gatePick = Gate::OR;

    static constexpr Gate radialGateOrder[4] = {
        Gate::XOR,
        Gate::AND,
        Gate::OR,
        Gate::NOR,
    };

    auto SetMode = [&lastMode, &mode, &data](Mode newMode)
    {
        lastMode = mode;
        mode = newMode;
        switch (newMode)
        {
        case Mode::PEN:
            data.pen.currentWireStart = nullptr;
            break;

        case Mode::EDIT:
            data.edit.nodeBeingDragged = nullptr;
            data.edit.wireBeingDragged = nullptr;
            data.edit.selectionWIP = false;
            data.edit.selectionRectangleX = 0;
            data.edit.selectionRectangleY = 0;
            data.edit.selectionRectangleWidth = 0;
            data.edit.selectionRectangleHeight = 0;
            break;

        case Mode::GATE:
            data.gate.radialMenuCenter = IVec2Zero();
            data.gate.overlappedSection = 0;
            break;
        
        case Mode::ERASE:
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
        cursorPos = IVec2Scale_i(cursorPos, g_gridSize);
        cursorPos = cursorPos + IVec2(g_gridSize / 2, g_gridSize / 2);

        if (IsKeyPressed(KEY_B))
            SetMode(Mode::PEN);
        else if (IsKeyPressed(KEY_V))
            SetMode(Mode::EDIT);
        else if (IsKeyPressed(KEY_G))
        {
            SetMode(Mode::GATE);
            data.gate.radialMenuCenter = cursorPos;
        }
        else if (IsKeyPressed(KEY_X))
            SetMode(Mode::ERASE);
        
        if (IsKeyPressed(KEY_BACKSLASH)) // |
            data.gatePick = Gate::OR;
        else if (IsKeyPressed(KEY_SEVEN)) // &
            data.gatePick = Gate::AND;
        else if (IsKeyPressed(KEY_ONE)) // !
            data.gatePick = Gate::NOR;
        else if (IsKeyPressed(KEY_SIX)) // ^
            data.gatePick = Gate::XOR;

        switch (mode)
        {
        case Mode::PEN:
        {
            data.hoveredWire = nullptr;
            data.hoveredNode = NodeWorld::Get().FindNodeAtPos(cursorPos);
            if (!data.hoveredNode)
                data.hoveredWire = NodeWorld::Get().FindWireAtPos(cursorPos);

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                Node* newNode = data.hoveredNode;
                if (!newNode)
                    newNode = NodeWorld::Get().CreateNode(cursorPos, data.gatePick);
		        // Do not create a new node/wire if already hovering the start node
                if (!!data.pen.currentWireStart && newNode != data.pen.currentWireStart)
                {
                    NodeWorld::Get().CreateWire(data.pen.currentWireStart, newNode);
                }
                data.pen.currentWireStart = newNode;
            }
            else if (!!GetKeyPressed() || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
            {
                data.pen.currentWireStart = nullptr;
            }
        }
            break;

        case Mode::EDIT:
        {
            bool lastFrameUpdate = false;

            if (!data.edit.nodeBeingDragged &&
                !data.edit.wireBeingDragged)
            {
                data.hoveredWire = nullptr;
                data.hoveredNode = NodeWorld::Get().FindNodeAtPos(cursorPos);
                if (!data.hoveredNode)
                    data.hoveredWire = NodeWorld::Get().FindWireElbowAtPos(cursorPos);
            }

            // Press
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                data.edit.nodeBeingDragged = data.hoveredNode;
                data.edit.wireBeingDragged = data.hoveredWire;
                if (!data.edit.nodeBeingDragged && !data.edit.wireBeingDragged)
                {
                    data.edit.selectionRectangleX = cursorPos.x;
                    data.edit.selectionRectangleY = cursorPos.y;
                    data.edit.selectionWIP = true;
                }
            }
            // Release
            else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
            {
                if (!!data.edit.nodeBeingDragged)
                {
                    data.hoveredNode = NodeWorld::Get().FindNodeAtPos(cursorPos);
                    if (data.hoveredNode && data.edit.nodeBeingDragged != data.hoveredNode)
                        data.hoveredNode = NodeWorld::Get().MergeNodes(data.edit.nodeBeingDragged, data.hoveredNode);
                }

                data.edit.nodeBeingDragged = nullptr;
                data.edit.wireBeingDragged = nullptr;
                lastFrameUpdate = data.edit.selectionWIP;
                data.edit.selectionWIP = false;
            }

            // Selection
            if (data.edit.selectionWIP || lastFrameUpdate)
            {
                data.edit.selectionRectangleWidth = cursorPos.x - data.edit.selectionRectangleX;
                data.edit.selectionRectangleHeight = cursorPos.y - data.edit.selectionRectangleY;
                lastFrameUpdate = false;
            }

            // Node
            else if (!!data.edit.nodeBeingDragged)
            {
                data.edit.nodeBeingDragged->SetPosition(cursorPos);
                for (Wire* wire : data.edit.nodeBeingDragged->GetWires())
                {
                    wire->UpdateElbowToLegal(); // Keep current configuration but move the elbow
                }
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

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                data.gatePick = radialGateOrder[data.gate.overlappedSection];
                SetMode(lastMode);
            }
            else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
            {
                SetMode(lastMode);
            }
        }
            break;

        case Mode::ERASE:
        {
            data.hoveredWire = nullptr;
            data.hoveredNode = NodeWorld::Get().FindNodeAtPos(cursorPos);
            if (!data.hoveredNode)
                data.hoveredWire = NodeWorld::Get().FindWireAtPos(cursorPos);
        }
            break;
        }

        NodeWorld::Get().Evaluate();

        /******************************************
        *   Draw the frame
        ******************************************/

        BeginDrawing(); {

            ClearBackground(BLACK);

            switch (mode)
            {
            case Mode::PEN:
            {
                NodeWorld::Get().DrawWires();

                if (!!data.pen.currentWireStart)
                {
                    DrawWireGeneric(data.pen.currentWireStart->GetPosition(), IVec2(data.pen.currentWireStart->GetX(), cursorPos.y), cursorPos, DARKBLUE);
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
                            color = DARKBLUE; // Output
                        else
                            color = DARKPURPLE; // Input

                        wire->Draw(color);
                    }
                }

                NodeWorld::Get().DrawNodes();

                if (!!data.hoveredNode)
                {
                    data.hoveredNode->Draw(YELLOW);
                }
            }
                break;

            case Mode::EDIT:
            {
                DrawRectangle(
                    data.edit.selectionRectangleX,
                    data.edit.selectionRectangleY,
                    data.edit.selectionRectangleWidth,
                    data.edit.selectionRectangleHeight,
                    ColorAlpha(YELLOW, 0.125));

                DrawRectangleLines(
                    data.edit.selectionRectangleX,
                    data.edit.selectionRectangleY,
                    data.edit.selectionRectangleWidth,
                    data.edit.selectionRectangleHeight,
                    ColorAlpha(YELLOW, 0.25));

                NodeWorld::Get().DrawWires();

                if (!!data.hoveredWire)
                {
                    data.hoveredWire->Draw(LIGHTGRAY);
                    IVec2 pts[4];
                    data.hoveredWire->GetLegalElbowPositions(pts);
                    for (const IVec2& p : pts)
                    {
                        DrawWireGeneric(data.hoveredWire->GetStartPos(), p, data.hoveredWire->GetEndPos(), ColorAlpha(LIGHTGRAY, 0.125));
                        DrawCircle(p.x, p.y, Wire::g_elbowRadius, ColorAlpha(DARKGREEN, 0.5));
                    }

                    Color elbowColor;
                    if (!!data.edit.wireBeingDragged)
                        elbowColor = LIME;
                    else
                        elbowColor = GREEN;

                    data.hoveredWire->DrawElbow(elbowColor);
                }

                NodeWorld::Get().DrawNodes();

                if (!!data.hoveredNode)
                {
                    data.hoveredNode->Draw(YELLOW);
                }
            }
                break;

            case Mode::GATE:
            {
                NodeWorld::Get().DrawWires();
                NodeWorld::Get().DrawNodes();


                constexpr float menuRadius = 64.0f;
                constexpr float menuHalfRadius = menuRadius / 2;
                constexpr float menuHoleRadius = 16.0f;
                constexpr IVec2 menuOff[4] = {
                    IVec2( 4,               4             ),
                    IVec2( 4,              -4 - menuRadius),
                    IVec2(-4 - menuRadius, -4 - menuRadius),
                    IVec2(-4 - menuRadius,  4             ),
                };
                constexpr IVec2 menuOff_hover[4] = {
                    IVec2(0,                    0                   ),
                    IVec2(0,                    0 - (menuRadius + 4)),
                    IVec2(0 - (menuRadius + 4), 0 - (menuRadius + 4)),
                    IVec2(0 - (menuRadius + 4), 0                   ),
                };
                constexpr Rectangle iconDest[4] = {
                    Rectangle{  menuHoleRadius * 3 / 4,       menuHoleRadius * 3 / 4,      32, 32 },
                    Rectangle{  menuHoleRadius * 3 / 4,      -menuHoleRadius * 3 / 4 - 32, 32, 32 },
                    Rectangle{ -menuHoleRadius * 3 / 4 - 32, -menuHoleRadius * 3 / 4 - 32, 32, 32 },
                    Rectangle{ -menuHoleRadius * 3 / 4 - 32,  menuHoleRadius * 3 / 4,      32, 32 },
                };
                int x = data.gate.radialMenuCenter.x;
                int y = data.gate.radialMenuCenter.y;
                Vector2 centerVec{ x, y };
                Color menuBackground = ColorAlpha(DARKGRAY, 0.4f);
                DrawCircleV(centerVec, menuRadius + 4.0f, menuBackground);

                for (int i = 0; i < 4; ++i)
                {
                    Color colorA;
                    Color colorB;
                    float radius;

                    if (i == data.gate.overlappedSection)
                    {
                        colorA = WHITE;
                        colorB = BLUE;
                        radius = menuRadius + 4.0f;
                        BeginScissorMode(x + menuOff_hover[i].x, y + menuOff_hover[i].y, radius, radius);
                    }
                    else
                    {
                        colorA = GRAY;
                        colorB = WHITE;
                        radius = menuRadius;
                        BeginScissorMode(x + menuOff[i].x, y + menuOff[i].y, radius, radius);
                    }

                    float startAngle = static_cast<float>(i * 90);
                    DrawCircleSector(centerVec, radius, startAngle, startAngle + 90.0f, 8, colorA);
                    Rectangle rec = iconDest[i];
                    rec.x += x;
                    rec.y += y;
                    DrawGateIcon32x(radialGateOrder[i], rec, colorB);

                    EndScissorMode();
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
                    DrawCross(cursorPos, RED);
                }

                NodeWorld::Get().DrawNodes();

                if (!!data.hoveredNode)
                {
                    data.hoveredNode->Draw(BLACK);
                    DrawCross(data.hoveredNode->GetPosition(), RED);
                }
            }
                break;
            }

            if (mode == Mode::GATE)
                DrawModeIcon(lastMode, { 0,0,16,16 }, WHITE);
            else
                DrawModeIcon(mode, { 0,0,16,16 }, WHITE);

            DrawGateIcon16x(data.gatePick, { 16,0,16,16 }, WHITE);

        } EndDrawing();
    }

    /******************************************
    *   Unload and free memory
    ******************************************/

    UnloadTexture(modeIcons);
    UnloadTexture(gateIcons16x);
    UnloadTexture(gateIcons32x);

    CloseWindow();

	return 0;
}

// Todo:
// Add delete options 
// Add gate customization
