#include <vector>
#include <unordered_set>
#include <queue>
#include <algorithm>
#include <raylib.h>
//#include <extras\raygui.h>

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

    Int_t x;
    Int_t y;

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
IVec2 Normal(IVec2 vec)
{
    return {
        (Int_t)(vec.x > 0) - (Int_t)(vec.x < 0),
        (Int_t)(vec.x > 0) - (Int_t)(vec.x < 0)
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


class NodeWorld;
class Node;
struct Wire
{
    Wire() = default;
    Wire(Node* start, Node* end) : elbow(), start(start), end(end) {}

    static constexpr Color g_wireColorActive = BROWN;
    static constexpr Color g_wireColorInactive = GRAY;
    static constexpr float g_elbowRadius = 2.0f;
    IVec2 elbow;
    Node* start;
    Node* end;

    void Draw(Color color) const;
    void DrawElbow(Color color) const
    {
        DrawCircle(elbow.x, elbow.y, g_elbowRadius, color);
    }

    Int_t GetStartX() const;
    Int_t GetStartY() const;
    Int_t GetElbowX() const;
    Int_t GetElbowY() const;
    Int_t GetEndX() const;
    Int_t GetEndY() const;
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

    const std::vector<Wire*>& GetWires() const
    {
        return m_wires;
    }

    bool IsInputOnly() const
    {
        if (m_wires.empty())
            return true;

        auto isInput = [this](Wire* wire) { return wire->start != this; };
        auto input = std::find_if(m_wires.begin(), m_wires.end(), isInput);
        if (input == m_wires.end()) // No input
            return true;

        return false;
    }

    // TODO: Improve
    void Draw(Color color) const
    {
        DrawCircle(m_position.x, m_position.y, g_nodeRadius, color);
    }

    friend class NodeWorld;

private: // Accessible by NodeWorld
    Node() = default;
    Node(IVec2 position, Gate gate) : m_position(position), m_gate(gate), m_state(false) {}

    static constexpr Color g_nodeColorActive = RED;
    static constexpr Color g_nodeColorInactive = LIGHTGRAY;
    static constexpr float g_nodeRadius = 3.0f;
    IVec2 m_position;
    Gate m_gate;
    bool m_state;
    std::vector<Wire*> m_wires;
};

void Wire::Draw(Color color) const
{
    DrawLine(start->GetX(), start->GetY(), elbow.x, elbow.y, color);
    DrawLine(elbow.x, elbow.y, end->GetX(), end->GetY(), color);
}

Int_t Wire::GetStartX() const
{
    return start->GetX();
}
Int_t Wire::GetStartY() const
{
    return start->GetY();
}
Int_t Wire::GetElbowX() const
{
    return elbow.x;
}
Int_t Wire::GetElbowY() const
{
    return elbow.y;
}
Int_t Wire::GetEndX() const
{
    return end->GetX();
}
Int_t Wire::GetEndY() const
{
    return end->GetY();
}


class NodeWorld
{
private:
    std::vector<Node*> nodes;
    std::vector<Node*> startNodes;
    std::vector<Wire*> wires;
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

    Node* CreateNode(IVec2 position, Gate gate)
    {
        Node* node = new Node(position, gate);
        nodes.push_back(node);
        startNodes.push_back(node);
        orderDirty = true;
        return node;
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
        orderDirty = true;
    }

    Wire* CreateWire(Node* start, Node* end)
    {
        Wire* wire = new Wire(start, end);
        wire->elbow.x = start->GetX();
        wire->elbow.y = end->GetY();
        wires.push_back(wire);
        start->m_wires.push_back(wire);
        end->m_wires.push_back(wire);

        // Remove end from start nodes if it is no longer an inputless node with this change
        auto it = std::find(startNodes.begin(), startNodes.end(), end);
        if (it != startNodes.end())
            startNodes.erase(it);

        orderDirty = true;
        return wire;
    }
    void DestroyWire(Wire* wire)
    {
        auto it_a = std::find(wire->start->m_wires.begin(), wire->start->m_wires.end(), wire);
        auto it_b = std::find(wire->end->m_wires.begin(), wire->end->m_wires.end(), wire);
        _ASSERT_EXPR(it_a != wire->start->m_wires.end(), "Trying to destroy a wire that is not inside the searched vector");
        _ASSERT_EXPR(it_b != wire->end->m_wires.end(), "Trying to destroy a wire that is not inside the searched vector");
        wire->start->m_wires.erase(it_a);
        wire->end->m_wires.erase(it_b);
        Node* end = wire->end;
        auto it = std::find(wires.begin(), wires.end(), wire);
        _ASSERT_EXPR(it != wires.end(), "Trying to destroy a wire that does not exist");
        wires.erase(it);
        // Push end to start nodes if this has destroyed its last remaining input
        if (end->IsInputOnly())
            startNodes.push_back(end);

        orderDirty = true;
    }

    // Uses BFS
    void Sort()
    {
        nodes.clear();
        nodes.reserve(nodes.size());

        std::queue<Node*> list;
        std::unordered_set<Node*> visited;
        for (Node* node : startNodes)
        {
            list.push(node);
            visited.insert(node);
        }

        while (!list.empty())
        {
            Node* current = list.front();
            for (Wire* wire : current->m_wires)
            {
                Node* next = wire->end;
                if (next == current || visited.find(next) != visited.end())
                    continue;

                visited.insert(next);
                list.push(next);
            }
            nodes.push_back(current);
            list.pop();
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
            wire->Draw(wire->start->m_state ? wire->g_wireColorActive : wire->g_wireColorInactive);
        }
    }
    void DrawNodes() const
    {
        for (Node* node : nodes)
        {
            node->Draw(node->m_state ? node->g_nodeColorActive : node->g_nodeColorInactive);
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

    enum class Mode
    {
        PEN,
        EDIT,
    } mode;

    struct {
        Node* hoveredNode;
        Wire* hoveredWire;
        union
        {
            struct {
                Node* currentWireStart;
            } pen;

            struct {
                Node* nodeBeingDragged;
                Wire* wireBeingDragged;
            } edit;
        };
    } data;

    data.hoveredNode = nullptr;
    data.hoveredWire = nullptr;

    auto SetMode = [&mode, &data](Mode newMode)
    {
        mode = newMode;
        switch (newMode)
        {
        case Mode::PEN:
            data.pen.currentWireStart = nullptr;
            break;

        case Mode::EDIT:
            data.edit.nodeBeingDragged = nullptr;
            data.edit.wireBeingDragged = nullptr;
            break;

        default:
            break;
        }
    };

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

        data.hoveredWire = nullptr;
        data.hoveredNode = NodeWorld::Get().FindNodeAtPos(cursorPos);
        if (!data.hoveredNode)
            data.hoveredWire = NodeWorld::Get().FindWireAtPos(cursorPos);

        switch (mode)
        {
        case Mode::PEN:
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                Node* newNode = data.hoveredNode;
                if (!newNode)
                    newNode = NodeWorld::Get().CreateNode(cursorPos, Gate::OR);

                if (!!data.pen.currentWireStart)
                    NodeWorld::Get().CreateWire(data.pen.currentWireStart, newNode);

                data.pen.currentWireStart = newNode;
            }
            else if (!!GetKeyPressed() || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
            {
                data.pen.currentWireStart = nullptr;
            }
            break;

        case Mode::EDIT:
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                data.edit.nodeBeingDragged = data.hoveredNode;
                data.edit.wireBeingDragged = data.hoveredWire;
            }
            else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
            {
                data.edit.nodeBeingDragged = nullptr;
                data.edit.wireBeingDragged = nullptr;
            }

            // Node
            if (!!data.edit.nodeBeingDragged)
            {
                data.edit.nodeBeingDragged->SetPosition(cursorPos);
            }

            // Wire
            else if (!!data.edit.wireBeingDragged)
            {
                // Todo
            }

            break;
        }

        NodeWorld::Get().Evaluate();

        /******************************************
        *   Draw the frame
        ******************************************/

        BeginDrawing(); {

            ClearBackground(BLACK);

            NodeWorld::Get().DrawWires();

            if (!!data.pen.currentWireStart)
            {
                DrawLine(data.pen.currentWireStart->GetX(), data.pen.currentWireStart->GetY(), data.pen.currentWireStart->GetX(), cursorPos.y, DARKBLUE);
                DrawLine(data.pen.currentWireStart->GetX(), cursorPos.y, cursorPos.x, cursorPos.y, DARKBLUE);
            }

            if (!!data.hoveredWire)
            {
                data.hoveredWire->Draw(GOLD);
                data.hoveredWire->DrawElbow(LIME);
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

        } EndDrawing();
    }

    /******************************************
    *   Unload and free memory
    ******************************************/

    // TODO: Unload variables

    CloseWindow();

	return 0;
}

