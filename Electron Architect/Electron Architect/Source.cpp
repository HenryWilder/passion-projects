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

    void Draw(Color color) const;
    void DrawElbow(Color color) const
    {
        DrawCircle(elbow.x, elbow.y, g_elbowRadius, color);
    }

    IVec2 GetStartPos() const;
    Int_t GetStartX() const;
    Int_t GetStartY() const;

    IVec2 GetElbowPos() const;
    Int_t GetElbowX() const;
    Int_t GetElbowY() const;

    IVec2 GetEndPos() const;
    Int_t GetEndX() const;
    Int_t GetEndY() const;
	
    void GetLegalElbowPositions(IVec2(&legal)[4]) const;
    void UpdateElbowToLegal();
    void SnapElbowToLegal(IVec2 pos);
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

    auto FindConnection(Node* other) const
    {
        return std::find_if(m_wires.begin(), m_wires.end(), [&other](Wire* wire) { return wire->start == other || wire->end == other; });
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
    DrawWireGeneric(start->GetPosition(), elbow, end->GetPosition(), color);
}

IVec2 Wire::GetStartPos() const
{
    return start->GetPosition();
}
Int_t Wire::GetStartX() const
{
    return start->GetX();
}
Int_t Wire::GetStartY() const
{
    return start->GetY();
}

IVec2 Wire::GetElbowPos() const
{
    return elbow;
}
Int_t Wire::GetElbowX() const
{
    return elbow.x;
}
Int_t Wire::GetElbowY() const
{
    return elbow.y;
}

IVec2 Wire::GetEndPos() const
{
    return end->GetPosition();
}
Int_t Wire::GetEndX() const
{
    return end->GetX();
}
Int_t Wire::GetEndY() const
{
    return end->GetY();
}

void Wire::GetLegalElbowPositions(IVec2 (&legal)[4]) const
{
    Int_t shortLength = std::min(
        abs(GetEndX() - GetStartX()),
        abs(GetEndY() - GetStartY())
    );

    legal[0] = IVec2(GetStartX(), GetEndY());
    legal[1] = IVec2(GetEndX(), GetStartY());
    legal[2] = start->GetPosition() + IVec2(GetEndX() < GetStartX() ? -shortLength : shortLength,
                                            GetEndY() < GetStartY() ? -shortLength : shortLength);
    legal[3] = end->GetPosition()   + IVec2(GetStartX() < GetEndX() ? -shortLength : shortLength,
                                            GetStartY() < GetEndY() ? -shortLength : shortLength);
}
void Wire::UpdateElbowToLegal()
{
    IVec2 legal[4];
    GetLegalElbowPositions(legal);
    elbow = legal[elbowConfig];
}
void Wire::SnapElbowToLegal(IVec2 pos)
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

class NodeWorld
{
private:
    class BFS_Tree
    {
    public:
        using LayerIterator_t = std::vector<std::vector<Node*>*>::iterator;
        using NodeIterator_t = std::vector<Node*>::iterator;
        using Iterator_t = std::pair<LayerIterator_t, NodeIterator_t>;

        void Reserve_Layers(size_t count)
        {

        }
        void Push_Layer()
        {
            data.push_back(new std::vector<Node*>);
        }
        // Returns layer pointer to be deleted or otherwise handled
        std::vector<Node*>* Pop_Layer()
        {
            std::vector<Node*>* layerVec;
            data.pop_back();
            return layerVec;
        }

        void Push_Node(LayerIterator_t layer, Node* element)
        {
            (*layer)->push_back(element);
        }
        void Insert_Node(Iterator_t position, Node* element)
        {
            (*position.first)->insert(position.second, element);
        }
        Node* Erase_Node(Iterator_t position)
        {
            Node* node = *position.second;
            (*position.first)->erase(position.second);
            return node;
        }

        template<class _Pred> void For_Each(_Pred predicate)
        {
            for (std::vector<Node*>* layer : data)
            {
                for (Node* node : *layer)
                {
                    predicate(node);
                }
            }
        }
        // Breaks on predicate return true
        template<class _Pred> void For_Each_With_Break(_Pred predicate)
        {
            for (std::vector<Node*>* layer : data)
            {
                for (Node* node : *layer)
                {
                    if (predicate(node))
                        return;
                }
            }
        }
        // Returns iterator on predicate return true
        template<class _Pred> Iterator_t For_Each_With_IterReturn(_Pred predicate)
        {
            LayerIterator_t it = { data.begin(), data[0]->begin() };
            while (it.first != data.end())
            {
                std::vector<Node*>& layer = *(*it.first);
                while (it.second != layer.end())
                {
                    if (predicate(node))
                        return it.second;

                    ++it.second;
                }
                ++it.first;
            }
        }

        Iterator_t Find(Node* element)
        {
            return For_Each_With_IterReturn([&element](Node* node) { return node == element; });
        }

    private:
        std::vector<std::vector<Node*>*> data;
    };
    BFS_Tree nodes;
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
                auto it = wire->end->FindConnection(wire->start);
                _ASSERT_EXPR(it != wire->end->m_wires.end(), "Node connection could not be verified");
                wire->end->m_wires.erase(it);
            }
            else // wire->end == node
            {
                auto it = wire->start->FindConnection(wire->end);
                _ASSERT_EXPR(it != wire->start->m_wires.end(), "Node connection could not be verified");
                wire->start->m_wires.erase(it);
            }
        }
        nodes.erase(node_iter);
        delete node;
        orderDirty = true;
    }
    // Uses BFS insertion
    Node* MergeNodes(Node* a, Node* b)
    {
        _ASSERT_EXPR(!!a && !!b, "Tried to merge a node with nullptr");
        _ASSERT_EXPR(a != b, "Tried to merge a node with itself");

        for (Wire* wire : b->m_wires)
        {
            if (wire->start != a && wire->end != a)
                a->m_wires.push_back(wire);

            if (wire->start == b) // Updating the layer of these might require recursion
                wire->start = a;
            else // wire->end == b
                wire->end = a;
        }

        if (auto it = a->FindConnection(b); it != a->m_wires.end())
        {
            Wire* wire = *it;
            FindAndErase(wires, wire);
            a->m_wires.erase(it);
        }

        FindAndErase_ExpectExisting(nodes, b);
        delete b;

        orderDirty = true;
        return a;
    }

    // Uses BFS insertion
    Wire* CreateWire(Node* start, Node* end)
    {
        _ASSERT_EXPR(start != nullptr && end != nullptr, "Tried to create a wire to nullptr");
        _ASSERT_EXPR(start != end, "Cannot create self-reference wire");

        // Duplicate guard
        {
            auto it = std::find_if(start->m_wires.begin(), start->m_wires.end(), [&end](Wire* wire) { return wire->end == end; });
            if (it != start->m_wires.end())
                return *it;
        }

        Wire* wire = new Wire(start, end);
        wire->elbowConfig = 0;
        wire->UpdateElbowToLegal();
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
    // Uses BFS insertion
    Wire* ReverseWire(Wire* wire)
    {
        std::swap(wire->start, wire->end);
        orderDirty = true;
        return wire;
    }
    // Uses BFS insertion
    void DestroyWire(Wire* wire)
    {
        FindAndErase_ExpectExisting(wires, wire);
        FindAndErase_ExpectExisting(wire->start->m_wires, wire);
        FindAndErase_ExpectExisting(wire->end->m_wires, wire);
        // Push end to start nodes if this has destroyed its last remaining input
        if (wire->end->IsInputOnly())
            startNodes.push_back(wire->end);
        delete wire;
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
                bool selectionWIP;
                Int_t selectionRectangleX;
                Int_t selectionRectangleY;
                Int_t selectionRectangleWidth;
                Int_t selectionRectangleHeight;
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
            data.edit.selectionWIP = false;
            data.edit.selectionRectangleX = 0;
            data.edit.selectionRectangleY = 0;
            data.edit.selectionRectangleWidth = 0;
            data.edit.selectionRectangleHeight = 0;
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

        if (IsKeyPressed(KEY_B))
            SetMode(Mode::PEN);
        else if (IsKeyPressed(KEY_V))
            SetMode(Mode::EDIT);

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
                do {
                    if (!newNode)
                        newNode = NodeWorld::Get().CreateNode(cursorPos, Gate::OR);
                    else if (newNode == data.pen.currentWireStart)
                        break;

                    if (!!data.pen.currentWireStart)
                        NodeWorld::Get().CreateWire(data.pen.currentWireStart, newNode);
                } while (false);
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
                        DrawWireGeneric(data.hoveredWire->GetStartPos(), p, data.hoveredWire->GetEndPos(), uicol::wire::ghost);
                        DrawCircle(p.x, p.y, Wire::g_elbowRadius, uicol::wireElbow::ghost);
                    }

                    Color elbowColor;
                    if (!!data.edit.wireBeingDragged)
                        elbowColor = uicol::wireElbow::drag;
                    else
                        elbowColor = uicol::wireElbow::hover;

                    data.hoveredWire->DrawElbow(elbowColor);
                }

                NodeWorld::Get().DrawNodes();

                if (!!data.hoveredNode)
                {
                    data.hoveredNode->Draw(YELLOW);
                }
            }
                break;
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

// Todo:
// Add delete options 
// Add gate customization