#include <vector>
#include <queue>
#include <deque>
#include <algorithm>
#include <raylib.h>
#include <extras\raygui.h>
#include <extras\easings.h>

using IVecInt_t = int;

struct IVec2
{
    IVec2() = default;
    constexpr IVec2(IVecInt_t x, IVecInt_t y) : x(x), y(y) {}

    IVecInt_t x;
    IVecInt_t y;

    bool operator==(IVec2 b) const
    {
        return x == b.x && y == b.y;
    }
    bool operator!=(IVec2 b) const
    {
        return x != b.x || y != b.y;
    }
};

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
    return IVec2((IVecInt_t)((float)a.x * b), (IVecInt_t)((float)a.y * b));
}
inline IVec2 IVec2Scale_f(float a, IVec2 b)
{
    return IVec2Scale_f(b, a);
}


class NodeWorld;
class Node;
struct Wire
{
    Wire() = default;
    Wire(Node* start, Node* end) : elbow(), start(start), end(end) {}

    static constexpr float g_elbowRadius = 1.0f;
    IVec2 elbow;
    Node* start;
    Node* end;
};

enum class Gate
{
    OR = '|',
    AND = '&',
    NOR = '!',
    XOR = '^',
};

class Node
{
public:
    Node() = default;
    Node(IVec2 position, Gate gate) : m_position(position), m_gate(gate) {}

    IVec2 GetPosition() const
    {
        return m_position;
    }
    void SetPosition(IVec2 position)
    {
        m_position = position;
    }

    Gate GetGate() const
    {
        return m_gate;
    }
    void SetGate(Gate gate)
    {
        m_gate = gate;
    }

    const std::vector<Wire*>& GetWires() const
    {
        return m_wires;
    }

    bool IsInputOnly() const
    {
        if (m_wires.empty())
            return true;

        auto isInput = [this](Wire* wire) { wire->start != this; };
        auto input = std::find_if(m_wires.begin(), m_wires.end(), isInput);
        if (input == m_wires.end()) // No input
            return true;

        return false;
    }

    friend class NodeWorld;

private:
    static constexpr float g_nodeRadius = 1.0f;
    IVec2 m_position;
    Gate m_gate;
    std::vector<Wire*> m_wires;
};

class NodeWorld
{
private:
    std::vector<Node*> nodes; // For deleting
    std::vector<Node*> startNodes;
    std::vector<Wire*> wires;

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

    void CreateNode(IVec2 position, Gate gate)
    {
        Node* node = new Node(position, gate);
        nodes.push_back(node);
        startNodes.push_back(node);
    }
    void DestroyNode(Node* node)
    {
        auto node_iter = std::find(nodes.begin(), nodes.end(), node);
        _ASSERT_EXPR(node_iter != nodes.end(), "Cannot remove a node that does not exist");
        for (Wire* wire : node->m_wires)
        {
            if (wire->start == node)
            {
                Node* end = wire->end;
                auto wire_iter = std::find_if(end->m_wires.begin(), end->m_wires.end(), [&node](Wire* wire) { return wire->start == node; });
                _ASSERT_EXPR(wire_iter != end->m_wires.end(), "Node connection could not be verified");
                end->m_wires.erase(wire_iter);
            }
            else // wire->end == node
            {
                Node* start = wire->start;
                auto wire_iter = std::find_if(start->m_wires.begin(), start->m_wires.end(), [&node](Wire* wire) { return wire->end == node; });
                _ASSERT_EXPR(wire_iter != start->m_wires.end(), "Node connection could not be verified");
                start->m_wires.erase(wire_iter);
            }
        }
        nodes.erase(node_iter);
        delete node;
    }

    void CreateWire(Node* start, Node* end)
    {
        Wire* wire = new Wire(start, end);
        wires.push_back(wire);
        start->m_wires.push_back(wire);
        end->m_wires.push_back(wire);
    }
    void DestroyWire(Wire* wire)
    {
        auto it_a = std::find(wire->start->m_wires.begin(), wire->start->m_wires.end(), wire);
        auto it_b = std::find(wire->end->m_wires.begin(), wire->end->m_wires.end(), wire);
        _ASSERT_EXPR(it_a != wire->start->m_wires.end(), "Trying to destroy a wire that is not inside the searched vector");
        _ASSERT_EXPR(it_b != wire->end->m_wires.end(), "Trying to destroy a wire that is not inside the searched vector");
        wire->start->m_wires.erase(it_a);
        wire->end->m_wires.erase(it_b);
        auto it = std::find(wires.begin(), wires.end(), wire);
        _ASSERT_EXPR(it != wires.end(), "Trying to destroy a wire that does not exist");
        wires.erase(it);
    }

    void Sort()
    {
        std::queue<int> list;
        bool* visited = new bool[nodes.size()](false);
        for (Node* node : nodes)
        {

        }
    }

    void Evaluate()
    {
        std::queue<Node*> todo;
        todo.insert();
    }
};


int main()
{
    int windowWidth = 1280;
    int windowHeight = 720;
    InitWindow(windowWidth, windowHeight, "Electron Architect");
    SetTargetFPS(60);

    /******************************************
    *   Load textures, shaders, and meshes
    ******************************************/

    // TODO: Load persistent assets & variables

    while (!WindowShouldClose())
    {
        /******************************************
        *   Simulate frame and update variables
        ******************************************/

        // TODO: simulate frame

        /******************************************
        *   Draw the frame
        ******************************************/

        BeginDrawing(); {

            ClearBackground(BLACK);

            // TODO: Draw frame

        } EndDrawing();
    }

    /******************************************
    *   Unload and free memory
    ******************************************/

    // TODO: Unload variables

    CloseWindow();

	return 0;
}

