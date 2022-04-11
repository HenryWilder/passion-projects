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

template <class Iter>
class Range {
    Iter b;
    Iter e;

public:
    Range(Iter begin, Iter end) : b(begin), e(end) {}

    Iter begin() { return b; }
    Iter end() { return e; }
};

template <class Container>
Range<typename Container::iterator> MakeRange(Container& container, size_t begin, size_t end)
{
    return {
        container.begin() + begin,
        container.begin() + end
    };
}
template <class Container>
Range<typename Container::const_iterator> MakeRange(const Container& container, size_t begin, size_t end)
{
    return {
        container.begin() + begin,
        container.begin() + end
    };
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

constexpr IVec2 IVec2Zero()
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

IVec2 IVec2Scale_i(IVec2 a, Int_t b)
{
    return IVec2(a.x * b, a.y * b);
}
inline IVec2 IVec2Scale_i(Int_t a, IVec2 b)
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
IVec2 IVec2Divide_i(IVec2 a, Int_t b)
{
    return IVec2(a.x / b, a.y / b);
}
IVec2 IVec2Divide_i(Int_t a, IVec2 b)
{
    return IVec2(a / b.x, a / b.y);
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

void DrawTextureIVec2(Texture2D texture, IVec2 pos, Color tint)
{
    DrawTexture(texture, pos.x, pos.y, tint);
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

void BeginScissorMode(IRect area)
{
    BeginScissorMode(area.x, area.y, area.w, area.h);
}

#pragma endregion

class NodeWorld;
class Node;
using ElbowIndex_t = uint8_t;
struct Wire
{
    Wire() = default;
    Wire(Node* start, Node* end) : elbow(), elbowConfig(0), start(start), end(end) {}
    Wire(Node* start, Node* end, ElbowIndex_t elbowConfig) : elbow(), elbowConfig(elbowConfig), start(start), end(end) { UpdateElbowToLegal(); }

    static constexpr Color g_wireColorActive = REDSTONE;
    static constexpr Color g_wireColorInactive = DEADCABLE;
    static constexpr float g_elbowRadius = 2.0f;

    IVec2 elbow;
    ElbowIndex_t elbowConfig;
    Node* start;
    Node* end;

    bool GetState() const;
    static void Draw(IVec2 start, IVec2 joint, IVec2 end, Color color)
    {
        DrawLineIV(start, joint, color);
        DrawLineIV(joint, end, color);
    }
    void Draw(Color color) const
    {
        constexpr IVec2 activeOffset = IVec2(1,-1);
        if (GetState())
            Draw(GetStartPos() + activeOffset, elbow + activeOffset, GetEndPos() + activeOffset, color);
        else 
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
	
    static IVec2 GetLegalElbowPosition(IVec2 start, IVec2 end, ElbowIndex_t index)
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
    IVec2 GetLegalElbowPosition(ElbowIndex_t index) const
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

        ElbowIndex_t pick = 0;
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

    RESISTOR = '~',
    CAPACITOR = '=',
};

class Node
{
public:
    IVec2 GetPosition() const
    {
        return m_position;
    }
    /// Sets the position of the node and updates its collision in NodeWorld
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

    // Only use if this is a resistor
    uint8_t GetResistance() const
    {
        _ASSERT_EXPR(m_gate == Gate::RESISTOR, "Cannot access the resistance of a non-resistor.");
        return m_ntd.r.resistance;
    }
    // Only use if this is a capacitor
    uint8_t GetCapacity() const
    {
        _ASSERT_EXPR(m_gate == Gate::CAPACITOR, "Cannot access the capacity of a non-capacitor.");
        return m_ntd.c.capacity;
    }
    // Only use if this is a capacitor
    uint8_t GetCharge() const
    {
        _ASSERT_EXPR(m_gate == Gate::CAPACITOR, "Cannot access the charge of a non-capacitor.");
        return m_ntd.c.charge;
    }
    // Only use if this is a capacitor
    float GetChargePercent() const
    {
        _ASSERT_EXPR(m_gate == Gate::CAPACITOR, "Cannot access capacitor members of a non-capacitor.");
        return (float)m_ntd.c.charge / (float)m_ntd.c.capacity;
    }
    // Only use if this is a resistor
    void SetResistance(uint8_t resistance)
    {
        _ASSERT_EXPR(m_gate == Gate::RESISTOR, "Cannot access the resistance of a non-resistor.");
        _ASSERT_EXPR(resistance <= 9, "Resistance must be <= 9");
        m_ntd.r.resistance = resistance;
    }
    // Only use if this is a capacitor
    void SetCapacity(uint8_t capacity)
    {
        _ASSERT_EXPR(m_gate == Gate::CAPACITOR, "Cannot access the capacity of a non-capacitor.");
        m_ntd.c.capacity = capacity;
    }
    // Only use if this is a capacitor
    void SetCharge(uint8_t charge)
    {
        _ASSERT_EXPR(m_gate == Gate::CAPACITOR, "Cannot access the charge of a non-capacitor.");
        m_ntd.c.charge = charge;
    }
    // Only use if this is a capacitor
    void IncrementCharge()
    {
        _ASSERT_EXPR(m_gate == Gate::CAPACITOR, "Cannot access the charge of a non-capacitor.");
        m_ntd.c.charge++;
    }
    // Only use if this is a capacitor
    void DecrementCharge()
    {
        _ASSERT_EXPR(m_gate == Gate::CAPACITOR, "Cannot access the charge of a non-capacitor.");
        m_ntd.c.charge--;
    }

    bool GetState() const
    {
        return m_state;
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
        _ASSERT_EXPR(m_wires.size() >= m_inputs, "Malformed Node inputs");
        return m_wires.size() == m_inputs;
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

        case Gate::RESISTOR:
            DrawRectangle(position.x - nodeRadius, position.y - nodeRadius, nodeRadius * 2, nodeRadius * 2, color);
            DrawRectangle(position.x - nodeRadius + 1, position.y - nodeRadius + 1, nodeRadius * 2 - 2, nodeRadius * 2 - 2, BLACK);
            break;
        case Gate::CAPACITOR:
            DrawRectangle(position.x - nodeRadius - 1, position.y - nodeRadius - 1, nodeRadius * 2 + 2, nodeRadius * 2 + 2, color);
            DrawRectangle(position.x - nodeRadius, position.y - nodeRadius, nodeRadius * 2, nodeRadius * 2, BLACK);
            DrawRectangle(position.x - nodeRadius + 1, position.y - nodeRadius + 1, nodeRadius * 2 - 2, nodeRadius * 2 - 2, color);
            break;
        }
    }
    void Draw(Color color) const
    {
        constexpr Int_t nodeRadius = static_cast<int>(g_nodeRadius);

        Draw(m_position, m_gate, color);

        if (m_gate == Gate::RESISTOR)
        {
            DrawRectangle(GetX() - nodeRadius + 1, GetY() - nodeRadius + 1, nodeRadius * 2 - 2, nodeRadius * 2 - 2, g_resistanceBands[GetResistance()]);
            DrawRectangle(GetX() - nodeRadius + 2, GetY() - nodeRadius + 2, nodeRadius * 2 - 4, nodeRadius * 2 - 4, BLACK);
        }
        else if (m_gate == Gate::CAPACITOR)
        {
            DrawRectangle(GetX() - nodeRadius + 1, GetY() - nodeRadius + 1, nodeRadius * 2 - 2, nodeRadius * 2 - 2, ColorAlpha(g_nodeColorActive, GetChargePercent()));
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

private: // Accessible by NodeWorld
    Node() = default;
    Node(IVec2 position, Gate gate) : m_position(position), m_gate(gate), m_state(false), m_inputs(0), m_ntd() {}
    Node(IVec2 position, Gate gate, uint8_t extraParam) : m_position(position), m_gate(gate), m_state(false), m_inputs(0)
    {
        if (gate == Gate::RESISTOR)
            m_ntd.r.resistance = extraParam;
        else if (gate == Gate::CAPACITOR)
        {
            m_ntd.c.capacity = extraParam;
            m_ntd.c.charge = 0;
        }
    }

public:
    static constexpr Color g_nodeColorActive = RED;
    static constexpr Color g_nodeColorInactive = LIGHTGRAY;
    static constexpr Color g_resistanceBands[] = { BLACK, BROWN, RED, ORANGE, YELLOW, GREEN, BLUE, VIOLET, GRAY, INTERFERENCEGRAY };
    static constexpr float g_nodeRadius = 3.0f;

private:
    IVec2 m_position;
    Gate m_gate;
    bool m_state;
    size_t m_inputs;
    union NonTransistorData
    {
        struct ResistorData
        {
            uint8_t resistance; // Number of nodes needed to evaluate true
        } r;

        struct CapacitorData
        {
            uint8_t capacity; // Max value of charge
            uint8_t charge; // Stored ticks of evaluating true
        } c;

        NonTransistorData() { memset(this, 0, sizeof(NonTransistorData)); }

    } m_ntd;
    std::deque<Wire*> m_wires; // Please keep this partitioned by inputs vs outputs

private:
    Range<decltype(m_wires)::iterator> GetInputs()
    {
        return MakeRange<decltype(m_wires)>(m_wires, 0, m_inputs);
    }
    Range<decltype(m_wires)::iterator> GetOutputs()
    {
        return MakeRange<decltype(m_wires)>(m_wires, m_inputs, m_wires.size());
    }

public:
    const decltype(m_wires)& GetWires() const
    {
        return m_wires;
    }
    Range<decltype(m_wires)::const_iterator> GetInputs() const
    {
        return MakeRange<decltype(m_wires)>(m_wires, 0, m_inputs);
    }
    Range<decltype(m_wires)::const_iterator> GetOutputs() const
    {
        return MakeRange<decltype(m_wires)>(m_wires, m_inputs, m_wires.size());
    }

    bool IsValidConnection(decltype(m_wires)::const_iterator it) const
    {
        return it != m_wires.end();
    }
};

bool Wire::GetState() const
{
    return start->GetState();
}
IVec2 Wire::GetStartPos() const
{
    return start->GetPosition();
}
IVec2 Wire::GetEndPos() const
{
    return end->GetPosition();
}


template<Int_t width, Int_t height = width>
void DrawIcon(Texture2D iconSheet, IVec2 iconColRow, IVec2 pos, Color tint)
{
    BeginScissorMode(pos.x, pos.y, width, height);
    DrawTexture(iconSheet,
        pos.x - iconColRow.x * width,
        pos.y - iconColRow.y * height,
        tint);
    EndScissorMode();
}

// Combo of up to four icons for representing a blueprint
struct BlueprintIcon
{
private: // Global
    static constexpr Int_t g_size = 16;
    static Texture2D g_iconSheet;
    static IVec2 g_iconSheetDimensions; // Rows and columns, not pixels

public:
    using IconID_t = uint16_t;

private:
    static IVec2 ColRowFromIcon(IconID_t icon)
    {
        return IVec2((Int_t)icon % g_iconSheetDimensions.x, (Int_t)icon / g_iconSheetDimensions.y);
    }

public:
    static IconID_t GetIconAtColRow(IVec2 colRow)
    {
        if (colRow.x < 0 || colRow.x >= g_iconSheetDimensions.x ||
            colRow.y < 0 || colRow.y >= g_iconSheetDimensions.y)
            return NULL;
        return (IconID_t)((colRow.y * g_iconSheetDimensions.x) + colRow.x);
    }
    static IVec2 PixelToColRow(IVec2 sheetPos, IVec2 selectPos)
    {
        return IVec2Divide_i(selectPos - sheetPos, g_size);
    }
    static IVec2 GetSheetSize_RC() // Rows and columns
    {
        return g_iconSheetDimensions;
    }
    static IVec2 GetSheetSize_Px() // Pixels
    {
        return IVec2Scale_i(g_iconSheetDimensions, g_size);
    }
    static void DrawBPIcon(IconID_t icon, IVec2 pos, Color tint)
    {
        if (icon != NULL)
            DrawIcon<g_size>(g_iconSheet, ColRowFromIcon(icon), pos, tint);
    }
    struct IconPos
    {
        static constexpr Int_t g_unit = BlueprintIcon::g_size / 2;

        BlueprintIcon::IconID_t id;
        uint8_t x; // 0 for left, 1 for center, 2 for right
        uint8_t y; // 0 for top,  1 for center, 2 for bottom

        // Position on screen
        IVec2 Pos() const
        {
            return IVec2(x * g_unit, y * g_unit);
        }

        void Draw(IVec2 start, Color tint) const
        {
            if (id != NULL)
                DrawBPIcon(id, start + Pos(), tint);
        }
    };

    static void DrawSheet(IVec2 pos, Color background, Color tint)
    {
        DrawRectangle(pos.x, pos.y, g_iconSheet.width, g_iconSheet.height, background);
        DrawTexture(g_iconSheet, pos.x, pos.y, tint);
    }

public:
    // Global
    static void Load(const char* filename)
    {
        g_iconSheet = LoadTexture(filename);
        g_iconSheetDimensions.x = g_iconSheet.width / g_size;
        g_iconSheetDimensions.y = g_iconSheet.height / g_size;
    }
    static void Unload()
    {
        UnloadTexture(g_iconSheet);
    }

    // Instance
    BlueprintIcon() = default;
    BlueprintIcon(const std::vector<IconPos>& icons)
    {
        _ASSERT_EXPR(icons.size() <= 4, "Icon vector too large");
        size_t i = 0;
        for (; i < icons.size(); ++i)
        {
            combo[i] = icons[i];
        }
        for (; i < 4; ++i)
        {
            combo[i] = { NULL, 0,0 };
        }

    }
    BlueprintIcon(IconPos(&icons)[4])
    {
        memcpy(combo, icons, sizeof(IconPos) * 4);
    }

    void DrawBackground(IVec2 pos, Color color) const
    {
        constexpr int width = g_size * 2;
        DrawRectangle(pos.x, pos.y, width, width, color);
    }
    void Draw(IVec2 pos, Color tint) const
    {
        // Draw
        for (const IconPos& icon : combo)
        {
            icon.Draw(pos, tint);
        }
    }

public: // Instance
    IconPos combo[4] = { { NULL, 0,0 }, { NULL, 0,0 }, { NULL, 0,0 }, { NULL, 0,0 }, };
};
using BlueprintIconID_t = BlueprintIcon::IconID_t;
Texture2D BlueprintIcon::g_iconSheet;
IVec2 BlueprintIcon::g_iconSheetDimensions = IVec2Zero();


struct NodeBP
{
    bool b_io;
    Gate gate;
    IVec2 relativePosition;
};
struct WireBP
{
    size_t startNodeIndex, endNodeIndex;
    ElbowIndex_t elbowConfig;
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
        IRect bounds = boundsInit;
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
        extents = IVec2(bounds.w, bounds.h);

        IVec2 min = IVec2(bounds.x, bounds.y);
        nodes.reserve(src.size());
        std::unordered_set<Node*> nodeSet(src.begin(), src.end());
        for (Node* node : src)
        {
            bool isIO = node->IsOutputOnly() || node->IsInputOnly();
            if (!isIO)
            {
                for (Wire* wire : node->GetWires())
                {
                    if (nodeSet.find(wire->start == node ? wire->end : wire->start) == nodeSet.end())
                    {
                        isIO = true;
                        break;
                    }
                }
            }

            nodes.emplace_back(
                isIO,
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
                wires.reserve(uniqueWires);
            }
        }

        for (Node* node : src)
        {
            for (Wire* wire : node->GetWires())
            {
                if (visitedWires.find(wire) != visitedWires.end() && // Is an internal wire
                    !visitedWires.find(wire)->second) // Has not been visited in this loop
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
        extents = IVec2Zero();
        std::thread nodeThread(&Blueprint::PopulateNodes, this, std::ref(src));
        std::thread wireThread(&Blueprint::PopulateWires, this, std::ref(src));
        nodeThread.join();
        wireThread.join();
    }

    IVec2 extents;
	std::vector<NodeBP> nodes;
	std::vector<WireBP> wires;

    void DrawPreview(IVec2 pos, Color boxColor, Color nodeColor) const
    {
        constexpr Int_t halfGrid = g_gridSize / 2;
        DrawRectangle(pos.x - halfGrid, pos.y - halfGrid, extents.x + g_gridSize, extents.y + g_gridSize, boxColor);
        for (const NodeBP& node_bp : nodes)
        {
            if (node_bp.b_io)
            {
                DrawCircleIV(node_bp.relativePosition + pos, Node::g_nodeRadius, nodeColor);
                DrawCircleIV(node_bp.relativePosition + pos, Node::g_nodeRadius - 1.0f, BLACK); // In case boxColor is transparent
                DrawCircleIV(node_bp.relativePosition + pos, Node::g_nodeRadius - 1.0f, boxColor);
            }
        }
    }
};


class Group
{
    static constexpr Int_t g_fontSize = g_gridSize;
    static constexpr Int_t g_labelHeight = g_fontSize * 2;

    IRect labelBounds;
    IRect captureBounds;
    Color color;
    std::string label;

    friend class NodeWorld;

public:
    Group() = default;
    // Takes captureBounds as rec
    Group(IRect rec, Color color) : labelBounds(rec.x, rec.y - g_labelHeight, rec.w, g_labelHeight), captureBounds(rec), color(color), label() {}
    Group(IRect rec, Color color, const std::string & label) : labelBounds(rec.x, rec.y - g_labelHeight, rec.w, g_labelHeight), captureBounds(rec), color(color), label(label) {}

    void Draw() const
    {
        DrawRectangleIRect(captureBounds, ColorAlpha(color, 0.25));
        DrawRectangleLines(captureBounds.x, captureBounds.y, captureBounds.w, captureBounds.h, color);
        DrawRectangleIRect(labelBounds, color);
        constexpr Int_t padding = g_labelHeight / 4;
        DrawText(label.c_str(), labelBounds.x + padding, labelBounds.y + padding, g_fontSize, WHITE);
    }
    void Highlight(Color highlight) const
    {
        IRect rec = labelBounds;
        rec.h += captureBounds.h;
        DrawRectangleLines(rec.x - 1, rec.y - 1, rec.w + 2, rec.h + 2, highlight);
    }

    IVec2 GetPosition() const
    {
        return captureBounds.xy;
    }
    void SetPosition(IVec2 pos)
    {
        labelBounds.xy = captureBounds.xy = pos;
        labelBounds.y -= g_labelHeight;
    }
};


class NodeWorld
{
private:
    bool orderDirty = false;

    std::vector<Node*> nodes;
    std::vector<Wire*> wires; // Inputs/outputs don't exist here
    std::vector<Blueprint*> blueprints;
    std::vector<Group*> groups;

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
        for (Blueprint* bp : blueprints)
        {
            delete bp;
        }
        for (Group* group : groups)
        {
            delete group;
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

    /// <summary>CreateNode does not insert at the end of the <see cref="nodes"/>.</summary>
    Node* CreateNode(IVec2 position, Gate gate, uint8_t extendedParam = 0)
    {
        // The order is not dirty at this time due to the node having no connections yet
        return _CreateNode(Node(position, gate, extendedParam));
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

    // CreateWire can affect the positions of parameter `end` in `nodes`
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

        start->AddWireOutput(wire);
        end->AddWireInput(wire);

        // Remove end from start nodes, as it is no longer an inputless node with this change
        FindAndErase(startNodes, end);

        orderDirty = true;
        return wire;
    }
    // CreateWire can affect the positions of parameter `end` in `nodes`
    Wire* CreateWire(Node* start, Node* end, ElbowIndex_t elbowConfig)
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

        Wire* wire = _CreateWire(Wire(start, end, elbowConfig));

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

    Group* CreateGroup(IRect rec)
    {
        Group* group = new Group(rec, WIPBLUE, "Label");
        groups.push_back(group);
        return group;
    }
    void DestroyGroup(Group* group)
    {
        FindAndErase_ExpectExisting(groups, group);
        delete group;
    }
    Group* FindGroupAtPos(IVec2 pos) const
    {
        for (Group* group : groups)
        {
            if (InBoundingBox(group->labelBounds, pos))
                return group;
        }
        return nullptr;
    }
    void FindNodesInGroup(std::vector<Node*>& result, Group* group) const
    {
        FindNodesInRect(result, group->captureBounds);
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
            wire->Draw(wire->start->GetState() ? Wire::g_wireColorActive : Wire::g_wireColorInactive);
        }
    }
    void DrawNodes() const
    {
        for (Node* node : nodes)
        {
            node->Draw(node->GetState() ? node->g_nodeColorActive : node->g_nodeColorInactive);
        }
    }
    void DrawGroups() const
    {
        for (Group* group : groups)
        {
            group->Draw();
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
	    std::unordered_map<size_t, Node*> nodeID;
        nodes.reserve(nodes.size() + bp->nodes.size());
        for (size_t i = 0; i < bp->nodes.size(); ++i)
        {
            Node* node = CreateNode(bp->nodes[i].relativePosition + topLeft, bp->nodes[i].gate);
		    nodeID.emplace(i, node);
        }
        wires.reserve(wires.size() + bp->wires.size());
        for (const WireBP& wire_bp : bp->wires)
        {
            Node* start;
            Node* end;
            {
                auto it = nodeID.find(wire_bp.startNodeIndex);
                _ASSERT_EXPR(it != nodeID.end(), "Malformed nodeID");
                start = it->second;
            }
            {
                auto it = nodeID.find(wire_bp.endNodeIndex);
                _ASSERT_EXPR(it != nodeID.end(), "Malformed nodeID");
                end = it->second;
            }
            Wire* wire = CreateWire(start, end, wire_bp.elbowConfig);
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
        BP_ICON,
    } mode, baseMode;

    Texture2D clipboardIcon = LoadTexture("icon_clipboard.png");

    Texture2D modeIcons = LoadTexture("icons_mode.png");
    auto DrawModeIcon = [&modeIcons](Mode mode, IVec2 pos, Color tint)
    {
        IVec2 offset;
        switch (mode)
        {
        case Mode::PEN:      offset = IVec2(0, 0); break;
        case Mode::EDIT:     offset = IVec2(1, 0); break;
        case Mode::ERASE:    offset = IVec2(0, 1); break;
        case Mode::INTERACT: offset = IVec2(1, 1); break;
        default: return;
        }
        DrawIcon<16>(modeIcons, offset, pos, tint);
    };

    Texture2D gateIcons16x = LoadTexture("icons_gate16x.png");
    auto DrawGateIcon16x = [&gateIcons16x](Gate gate, IVec2 pos, Color tint)
    {
        IVec2 offset;
        switch (gate)
        {
        case Gate::OR:  offset = IVec2(0, 0); break;
        case Gate::AND: offset = IVec2(1, 0); break;
        case Gate::NOR: offset = IVec2(0, 1); break;
        case Gate::XOR: offset = IVec2(1, 1); break;

        case Gate::RESISTOR:  offset = IVec2(0, 2); break;
        case Gate::CAPACITOR: offset = IVec2(1, 2); break;
        default: return;
        }
        DrawIcon<16>(gateIcons16x, offset, pos, tint);
    };

    Texture2D gateIcons32x = LoadTexture("icons_gate32x.png");
    auto DrawGateIcon32x = [&gateIcons32x](Gate gate, IVec2 pos, Color tint)
    {
        IVec2 offset;
        switch (gate)
        {
        case Gate::OR:  offset = IVec2(0, 0); break;
        case Gate::AND: offset = IVec2(1, 0); break;
        case Gate::NOR: offset = IVec2(0, 1); break;
        case Gate::XOR: offset = IVec2(1, 1); break;

        case Gate::RESISTOR:  offset = IVec2(0, 2); break;
        case Gate::CAPACITOR: offset = IVec2(1, 2); break;
        default: return;
        }
        DrawIcon<32>(gateIcons32x, offset, pos, tint);
    };

    BlueprintIcon::Load("icons_blueprint.png");

    struct {
        Gate gatePick = Gate::OR;
        Gate lastGate = Gate::OR;
        union { // Alternate names for the same value
            uint8_t storedResistance;
            uint8_t storedCapacity;
            uint8_t storedExtendedParam = 0;
        };
        Node* hoveredNode = nullptr;
        Wire* hoveredWire = nullptr;
        Blueprint* clipboard = nullptr;
        std::vector<Node*> selection;

        union // Base mode
        {
            struct {
                Node* currentWireStart;
                ElbowIndex_t currentWireElbowConfig;
            } pen;

            struct {
                IVec2 fallbackPos;
                bool selectionWIP;
                IVec2 selectionStart;
                IRect selectionRec;
                bool draggingGroup;
                Group* hoveredGroup;
                Node* nodeBeingDragged;
                Wire* wireBeingDragged;
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

            struct {
                BlueprintIcon* object;
                IVec2 pos; // Width and height are fixed
                IRect sheetRec;
                BlueprintIconID_t iconID;
                uint8_t iconCount;
                int draggingIcon; // -1 for none/not dragging
            } bp_icon;
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

        Gate::RESISTOR,
        Gate::CAPACITOR,
    };
    constexpr IRect dropdownBounds[] = {
        IRect( 0, 16, 16, 16 * (_countof(dropdownModeOrder) - 1)), // Mode
        IRect(16, 16, 16, 16 * (_countof(dropdownGateOrder) - 1)), // Gate
        IRect(32, 16, 16, 16 * (_countof(Node::g_resistanceBands) - 1)), // Parameter
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
        if (mode == Mode::BP_ICON)
        {
            delete data.bp_icon.object;
            data.bp_icon.object = nullptr;
        }

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
            data.edit.selectionStart = IVec2Zero();
            data.edit.selectionRec = IRect(0,0,0,0);
            data.edit.draggingGroup = false;
            data.edit.hoveredGroup = nullptr;
            data.edit.nodeBeingDragged = nullptr;
            data.edit.wireBeingDragged = nullptr;
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

        case Mode::BP_ICON:
            data.bp_icon.object = nullptr;
            data.bp_icon.pos = IVec2Zero();
            data.bp_icon.sheetRec = IRect(0,0,0,0);
            data.bp_icon.iconID = NULL;
            data.bp_icon.iconCount = 0;
            data.bp_icon.draggingIcon = -1;
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

        // Hotkeys
        {
            if (mode == baseMode || mode == Mode::GATE)
            {
                if (IsKeyPressed(KEY_ONE))
                    data.gatePick = Gate::OR;
                else if (IsKeyPressed(KEY_TWO))
                    data.gatePick = Gate::AND;
                else if (IsKeyPressed(KEY_THREE))
                    data.gatePick = Gate::NOR;
                else if (IsKeyPressed(KEY_FOUR))
                    data.gatePick = Gate::XOR;
                else if (IsKeyPressed(KEY_FIVE))
                    data.gatePick = Gate::RESISTOR;
                else if (IsKeyPressed(KEY_SIX))
                    data.gatePick = Gate::CAPACITOR;
            }

            // KEY COMBOS BEFORE INDIVIDUAL KEYS!
            
            // Ctrl
            if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))
            {
                // Copy
                if (IsKeyPressed(KEY_C) && mode == Mode::EDIT)
                {
                    if (data.clipboard != nullptr)
                        delete data.clipboard;

                    if (data.selection.empty())
                        data.clipboard = nullptr;
                    else
                        data.clipboard = new Blueprint(data.selection);
                }
                // Paste
                else if (IsKeyPressed(KEY_V) && !!data.clipboard)
                {
                    SetMode(Mode::PASTE);
                }
                // Group
                else if (IsKeyPressed(KEY_G) && mode == Mode::EDIT && !data.edit.selectionWIP &&
                    !(data.edit.selectionRec.w == 0 || data.edit.selectionRec.h == 0))
                {
                    NodeWorld::Get().CreateGroup(data.edit.selectionRec);
                    data.edit.selectionRec = IRect(0,0,0,0);
                    data.selection.clear();
                }
                // Save
                else if (IsKeyPressed(KEY_S))
                {
                    // Save blueprint
                    if (mode == Mode::PASTE)
                    {
                        SetMode(Mode::BP_ICON);
                        data.bp_icon.object = new BlueprintIcon;
                        data.bp_icon.pos = cursorPos - IVec2(BlueprintIcon::IconPos::g_unit, BlueprintIcon::IconPos::g_unit);
                        data.bp_icon.sheetRec.xy = data.bp_icon.pos + IVec2(BlueprintIcon::IconPos::g_unit * 4, BlueprintIcon::IconPos::g_unit * 4);
                        data.bp_icon.sheetRec.wh = BlueprintIcon::GetSheetSize_Px();
                    }
                    // Save file
                    else
                    {
                        NodeWorld::Get().Save_SmallFile("dataS.cg");
                    }
                }
            }
            else
            {
                if (IsKeyPressed(KEY_ESCAPE))
                {
                    if (mode != baseMode)
                        SetMode(baseMode);
                    else if (!!data.clipboard)
                    {
                        delete data.clipboard;
                        data.clipboard = nullptr;
                    }
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
                else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && mode != Mode::BP_ICON && (cursorPos.y <= 16 && cursorPos.x <= 48) &&
                    (mode == Mode::BUTTON ? data.button.dropdownActive != (cursorPos.x / 16) : true))
                {
                    SetMode(Mode::BUTTON);
                    data.button.dropdownActive = cursorPos.x / 16;
                    goto EVAL;
                }
            }
        }

        // Input
        switch (mode)
        {

        // Basic

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
                    newNode = NodeWorld::Get().CreateNode(cursorPos, data.gatePick, data.storedExtendedParam);
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
                    !data.edit.wireBeingDragged &&
                    !data.edit.draggingGroup)
                {
                    data.edit.hoveredGroup = nullptr;
                    data.hoveredWire = nullptr;
                    data.hoveredNode = NodeWorld::Get().FindNodeAtPos(cursorPos);
                    if (!data.hoveredNode)
                    {
                        data.hoveredWire = NodeWorld::Get().FindWireElbowAtPos(cursorPos);
                        if (!data.hoveredWire)
                        {
                            data.edit.hoveredGroup = NodeWorld::Get().FindGroupAtPos(cursorPos);
                        }
                    }
                }
            }

            // Press
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                data.selection.clear();
                data.edit.nodeBeingDragged = data.hoveredNode;
                data.edit.wireBeingDragged = data.hoveredWire;

                // selectionStart being used as an offset here
                if (data.edit.draggingGroup = !!data.edit.hoveredGroup)
                {
                    NodeWorld::Get().FindNodesInGroup(data.selection, data.edit.hoveredGroup);
                    data.edit.selectionStart = (cursorPos - (data.edit.fallbackPos = data.edit.hoveredGroup->GetPosition()));
                }

                if (data.edit.selectionWIP = !(data.edit.nodeBeingDragged || data.edit.wireBeingDragged || data.edit.draggingGroup))
                    data.edit.selectionStart = data.edit.fallbackPos = cursorPos;
            }

            // Selection
            if (data.edit.selectionWIP)
            {
                auto [minx, maxx] = std::minmax(cursorPos.x, data.edit.selectionStart.x);
                auto [miny, maxy] = std::minmax(cursorPos.y, data.edit.selectionStart.y);
                data.edit.selectionRec.w = maxx - (data.edit.selectionRec.x = minx);
                data.edit.selectionRec.h = maxy - (data.edit.selectionRec.y = miny);
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
            // Group
            else if (data.edit.draggingGroup)
            {
                data.edit.hoveredGroup->SetPosition(cursorPos - data.edit.selectionStart);
                for (Node* node : data.selection)
                {
                    IVec2 offset = cursorPos - cursorPosPrev;
                    node->SetPosition_Temporary(node->GetPosition() + offset);
                }
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
                    }
                    else if (data.edit.draggingGroup)
                    {
                        data.edit.hoveredGroup->SetPosition(data.edit.fallbackPos);
                        for (Node* node : data.selection)
                        {
                            IVec2 offset = (data.edit.fallbackPos + data.edit.selectionStart) - cursorPos;
                            node->SetPosition_Temporary(node->GetPosition() + offset);
                        }
                    }
                    else if (data.edit.selectionWIP)
                    {
                        data.edit.selectionRec = IRect(0,0,0,0);
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
                    }
                    else if (data.edit.draggingGroup)
                    {
                        for (Node* node : data.selection)
                        {
                            node->SetPosition(node->GetPosition());
                        }
                    }
                    else if (data.edit.selectionWIP)
                    {
                        NodeWorld::Get().FindNodesInRect(data.selection, data.edit.selectionRec);
                    }
                }
                if (data.edit.draggingGroup)
                    data.selection.clear();
                data.edit.nodeBeingDragged = nullptr;
                data.edit.selectionWIP = false;
                data.edit.draggingGroup = false;
                data.edit.wireBeingDragged = nullptr;
            }
            // Right click
            else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && !!data.hoveredNode)
            {
                data.hoveredNode->SetGate(data.gatePick);
                if (data.hoveredNode->GetGate() == Gate::RESISTOR)
                    data.hoveredNode->SetResistance(data.storedResistance);
                else if (data.hoveredNode->GetGate() == Gate::CAPACITOR)
                    data.hoveredNode->SetCapacity(data.storedCapacity);
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


        // Overlay

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

                    case 2: // Resistance
                    {
                        for (uint8_t v = 0; v < 10; ++v)
                        {
                            if (v == data.storedExtendedParam)
                                continue;

                            if (InBoundingBox(rec, cursorPos))
                            {
                                data.storedExtendedParam = v;
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
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
            {
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                {
                    IVec2 pos = cursorPos - IVec2Divide_i(data.clipboard->extents, 2);
                    pos = IVec2Scale_i(IVec2Divide_i(pos, g_gridSize), g_gridSize);
                    pos = pos + IVec2(g_gridSize / 2, g_gridSize / 2);
                    NodeWorld::Get().SpawnBlueprint(data.clipboard, pos);
                }
                data.selection.clear();
                SetMode(baseMode);
            }
        }
        break;

        case Mode::BP_ICON:
        {
            if (b_cursorMoved && data.bp_icon.draggingIcon == -1)
            {
                data.bp_icon.iconID = BlueprintIcon::GetIconAtColRow(BlueprintIcon::PixelToColRow(data.bp_icon.sheetRec.xy, cursorPos));
            }

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                if (InBoundingBox(data.bp_icon.sheetRec, cursorPos) && data.bp_icon.iconCount < 4 && !!data.bp_icon.iconID)
                {
                    cursorPos = data.bp_icon.pos;
                    SetMousePosition(cursorPos.x + BlueprintIcon::IconPos::g_unit, cursorPos.y + BlueprintIcon::IconPos::g_unit);
                    data.bp_icon.object->combo[data.bp_icon.iconCount] = { data.bp_icon.iconID, 0,0 };
                    data.bp_icon.draggingIcon = data.bp_icon.iconCount;
                    data.bp_icon.iconCount++;
                }
                else if (InBoundingBox(IRect(data.bp_icon.pos.x, data.bp_icon.pos.y, BlueprintIcon::IconPos::g_unit * 4, BlueprintIcon::IconPos::g_unit * 4), cursorPos))
                {
                    data.bp_icon.draggingIcon = -1;
                    for (decltype(data.bp_icon.draggingIcon) i = 0; i < data.bp_icon.iconCount; ++i)
                    {
                        if (data.bp_icon.object->combo[i].id == NULL)
                            continue;

                        IRect bounds(
                            data.bp_icon.pos.x,
                            data.bp_icon.pos.y,
                            BlueprintIcon::IconPos::g_unit * 2,
                            BlueprintIcon::IconPos::g_unit * 2
                        );
                        bounds.xy = bounds.xy + data.bp_icon.object->combo[i].Pos();
                        if (InBoundingBox(bounds, cursorPos))
                        {
                            data.bp_icon.draggingIcon = i;
				data.bp_icon.iconID = data.bp_icon.object->combo[i].id;
                            break;
                        }
                    }
                }
            }
            if ((IsMouseButtonReleased(MOUSE_LEFT_BUTTON) || IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) && data.bp_icon.draggingIcon != -1)
            {
                if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
                {
                    if (data.bp_icon.draggingIcon < 3)
                    {
                        memcpy(
                            data.bp_icon.object->combo + data.bp_icon.draggingIcon,
                            data.bp_icon.object->combo + data.bp_icon.draggingIcon + 1,
                            sizeof(BlueprintIcon::IconPos) * (4ull - (size_t)data.bp_icon.draggingIcon));
                    }
                    data.bp_icon.object->combo[3] = { NULL, 0,0 };
                    data.bp_icon.iconCount--;
                }
                data.bp_icon.draggingIcon = -1;
            }
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && !!data.bp_icon.iconID)
            {
                constexpr IVec2 centerOffset(BlueprintIcon::IconPos::g_unit, BlueprintIcon::IconPos::g_unit);
                IVec2 colRow = IVec2Divide_i(cursorPos - data.bp_icon.pos - centerOffset, BlueprintIcon::IconPos::g_unit);
                colRow.x = std::min(std::max(colRow.x, 0), 2);
                colRow.y = std::min(std::max(colRow.y, 0), 2);
                data.bp_icon.object->combo[data.bp_icon.draggingIcon].x = colRow.x;
                data.bp_icon.object->combo[data.bp_icon.draggingIcon].y = colRow.y;
            }
        }
        break;
        }

    EVAL:
        cursorPosPrev = cursorPos;
        NodeWorld::Get().Evaluate();

        /******************************************
        *   Draw the frame
        ******************************************/

        BeginDrawing(); {

            ClearBackground(BLACK);

            if (mode == Mode::BP_ICON)
            {
                _ASSERT_EXPR(!!data.bp_icon.object, "Blueprint icon object not initialized");
                _ASSERT_EXPR(!!data.clipboard, "Bad entry into Mode::BP_ICON");

                data.bp_icon.object->DrawBackground(data.bp_icon.pos, SPACEGRAY);
                data.bp_icon.object->Draw(data.bp_icon.pos, WHITE);

                for (size_t i = 0; i < 4; ++i)
                {
                    for (decltype(data.bp_icon.draggingIcon) i = 0; i < 4; ++i)
                    {
                        if (data.bp_icon.object->combo[i].id == NULL)
                            continue;

                        IRect bounds(
                            data.bp_icon.pos.x,
                            data.bp_icon.pos.y,
                            BlueprintIcon::IconPos::g_unit * 2,
                            BlueprintIcon::IconPos::g_unit * 2
                        );
                        bounds.xy = bounds.xy + data.bp_icon.object->combo[i].Pos();
                        if (InBoundingBox(bounds, cursorPos))
                        {
                            DrawRectangleIRect(bounds, ColorAlpha(WIPBLUE, 0.25f));
                        }
                    }
                }
                if (data.bp_icon.draggingIcon != -1)
                    BlueprintIcon::DrawBPIcon(data.bp_icon.iconID, data.bp_icon.pos + data.bp_icon.object->combo[data.bp_icon.draggingIcon].Pos(), WIPBLUE);

                BlueprintIcon::DrawSheet(data.bp_icon.sheetRec.xy, SPACEGRAY, WHITE);
            }
            else
            {
                // Grid
                for (Int_t y = 0; y < windowHeight; y += g_gridSize)
                {
                    DrawLine(0, y, windowWidth, y, SPACEGRAY);
                }
                for (Int_t x = 0; x < windowWidth; x += g_gridSize)
                {
                    DrawLine(x, 0, x, windowHeight, SPACEGRAY);
                }

                NodeWorld::Get().DrawGroups();

                // Draw
                switch (mode)
                {

                // Base mode

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
                    if (!!data.edit.hoveredGroup)
                        data.edit.hoveredGroup->Highlight(INTERFERENCEGRAY);

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


                // Overlay mode

                case Mode::GATE: // Todo: Maybe change rect to vec?
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
                        DrawGateIcon32x(radialGateOrder[i], rec.xy, colorB);

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
                            DrawModeIcon(m, rec.xy, color);
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
                            DrawGateIcon16x(g, rec.xy, color);
                            rec.y += 16;
                        }
                    }
                    break;

                    case 2: // Resistance
                    {
                        for (uint8_t v = 0; v < 10; ++v)
                        {
                            _ASSERT_EXPR(v < _countof(Node::g_resistanceBands), "Resistance out of bounds");
                            if (v == data.storedExtendedParam)
                                continue;
                            Color color = Node::g_resistanceBands[v];
                            if (InBoundingBox(rec, cursorPos))
                            {
                                DrawRectangleIRect(rec, WIPBLUE);
                                IRect smaller = rec;
                                smaller.x += 2;
                                smaller.y += 2;
                                smaller.w -= 4;
                                smaller.h -= 4;
                                DrawRectangleIRect(smaller, color);
                            }
                            else
                                DrawRectangleIRect(rec, color);
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

                    IVec2 pos = cursorPos - IVec2Divide_i(data.clipboard->extents, 2);
                    pos = IVec2Scale_i(IVec2Divide_i(pos, g_gridSize), g_gridSize);
                    pos = pos + IVec2(g_gridSize / 2, g_gridSize / 2);
                    data.clipboard->DrawPreview(pos, ColorAlpha(LIFELESSNEBULA, 0.5f), HAUNTINGWHITE);
                }
                break;

                case Mode::BP_ICON:
                {
                    _ASSERT_EXPR(false, "Henry made a mistake.");
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
                        case Gate::OR:        text = "Gate: | (or)";         break;
                        case Gate::AND:       text = "Gate: & (and)";        break;
                        case Gate::NOR:       text = "Gate: ! (nor)";        break;
                        case Gate::XOR:       text = "Gate: ^ (xor)";        break;
                        case Gate::RESISTOR:  text = "Component: Resistor";  break;
                        case Gate::CAPACITOR: text = "Component: Capacitor"; break;
                        default:              text = "";                     break;
                        }
                        DrawText(text, 36, 17, 8, WHITE);
                        DrawRectangle(16, 0, 16, 16, SPACEGRAY);
                    }
                    else if (cursorPos.x <= 48)
                    {
                        const char* text;
                        if (data.gatePick == Gate::RESISTOR)
                            text = "Resistance: %i inputs";
                        else if (data.gatePick == Gate::RESISTOR)
                            text = "Capacity : %i ticks";
                        else
                            text = "Stored data: %i";
                        DrawText(TextFormat(text, data.storedExtendedParam), 52, 17, 8, WHITE);
                        _ASSERT_EXPR(data.storedExtendedParam < _countof(Node::g_resistanceBands), "Stored resistance out of bounds");
                        Color color = Node::g_resistanceBands[data.storedExtendedParam];
                        DrawRectangle(32, 0, 16, 16, WIPBLUE);
                        DrawRectangle(34, 2, 12, 12, Node::g_resistanceBands[data.storedExtendedParam]);
                    }
                }

                IRect rec(0, 0, 16, 16);
                DrawRectangleIRect(rec, SPACEGRAY);
                DrawModeIcon(baseMode, rec.xy, WHITE);
                rec.x += 16;
                DrawRectangleIRect(rec, SPACEGRAY);
                DrawGateIcon16x(data.gatePick, rec.xy, WHITE);
                rec.x += 16;
                if (!(cursorPos.y <= 16 && cursorPos.x > 32 && cursorPos.x <= 48))
                {
                    _ASSERT_EXPR(data.storedExtendedParam < _countof(Node::g_resistanceBands), "Stored resistance out of bounds");
                    DrawRectangleIRect(rec, Node::g_resistanceBands[data.storedExtendedParam]);
                }
                if (!!data.clipboard)
                {
                    rec.x += 16;
                    DrawRectangleIRect(rec, SPACEGRAY);
                    DrawTextureIVec2(clipboardIcon, rec.xy, WHITE);
                }
            }

        } EndDrawing();
    }

    /******************************************
    *   Unload and free memory
    ******************************************/

    if (data.clipboard != nullptr)
        delete data.clipboard;

    BlueprintIcon::Unload();
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
* -Save/load (https://en.cppreference.com/w/cpp/filesystem/directory_iterator)
* -Save file menu
* -Save file thumbnails (based on cropped screenshot)
* -Menu screen (Open to file menu with "new" at the top)
*
* Quality of Life
* -Special erase (keep wires, erase node)
* -Hotkeys for output-only gate state toggles (Like the Reason on-screen piano)
*
* Stretch goals
* -Multiple color pallets
* -Step-by-step evaluation option
* -Log files for debug/crashes
* -Export as SVG
*/
