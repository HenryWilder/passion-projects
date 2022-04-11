#include "HUtility.h"
#include "Wire.h"
#include "Node.h"

IVec2 Node::GetPosition() const
{
    return m_position;
}
// Moves the node without updating its collision
void Node::SetPosition_Temporary(IVec2 position)
{
    m_position = position;
    for (Wire* wire : m_wires)
    {
        wire->UpdateElbowToLegal(); // Keep current configuration but move the elbow
    }
}
// Sets the position of the node and updates its collision in NodeWorld
// NOTE: NodeWorld collision is currently a saved-for-later feature
void Node::SetPosition(IVec2 position)
{
    SetPosition_Temporary(position);
}
int Node::GetX() const
{
    return m_position.x;
}
void Node::SetX(int x)
{
    SetPosition(IVec2(x, GetY()));
}
int Node::GetY() const
{
    return m_position.y;
}
void Node::SetY(int y)
{
    SetPosition(IVec2(GetX(), y));
}

Gate Node::GetGate() const
{
    return m_gate;
}
void Node::SetGate(Gate gate)
{
    m_gate = gate;
}

// Only use if this is a resistor
uint8_t Node::GetResistance() const
{
    _ASSERT_EXPR(m_gate == Gate::RESISTOR, "Cannot access the resistance of a non-resistor.");
    return m_ntd.r.resistance;
}
// Only use if this is a capacitor
uint8_t Node::GetCapacity() const
{
    _ASSERT_EXPR(m_gate == Gate::CAPACITOR, "Cannot access the capacity of a non-capacitor.");
    return m_ntd.c.capacity;
}
// Only use if this is a capacitor
uint8_t Node::GetCharge() const
{
    _ASSERT_EXPR(m_gate == Gate::CAPACITOR, "Cannot access the charge of a non-capacitor.");
    return m_ntd.c.charge;
}
// Only use if this is a capacitor
float Node::GetChargePercent() const
{
    _ASSERT_EXPR(m_gate == Gate::CAPACITOR, "Cannot access capacitor members of a non-capacitor.");
    return (float)m_ntd.c.charge / (float)m_ntd.c.capacity;
}
// Only use if this is a resistor
void Node::SetResistance(uint8_t resistance)
{
    _ASSERT_EXPR(m_gate == Gate::RESISTOR, "Cannot access the resistance of a non-resistor.");
    _ASSERT_EXPR(resistance <= 9, "Resistance must be <= 9");
    m_ntd.r.resistance = resistance;
}
// Only use if this is a capacitor
void Node::SetCapacity(uint8_t capacity)
{
    _ASSERT_EXPR(m_gate == Gate::CAPACITOR, "Cannot access the capacity of a non-capacitor.");
    m_ntd.c.capacity = capacity;
}
// Only use if this is a capacitor
void Node::SetCharge(uint8_t charge)
{
    _ASSERT_EXPR(m_gate == Gate::CAPACITOR, "Cannot access the charge of a non-capacitor.");
    m_ntd.c.charge = charge;
}
// Only use if this is a capacitor
void Node::IncrementCharge()
{
    _ASSERT_EXPR(m_gate == Gate::CAPACITOR, "Cannot access the charge of a non-capacitor.");
    m_ntd.c.charge++;
}
// Only use if this is a capacitor
void Node::DecrementCharge()
{
    _ASSERT_EXPR(m_gate == Gate::CAPACITOR, "Cannot access the charge of a non-capacitor.");
    m_ntd.c.charge--;
}

bool Node::GetState() const
{
    return m_state;
}

std::deque<Wire*>::const_iterator Node::FindConnection(Node* other) const
{
    return std::find_if(m_wires.begin(), m_wires.end(), [&other](Wire* wire) { return wire->start == other || wire->end == other; });
}

size_t Node::GetInputCount() const
{
    return m_inputs;
}
size_t Node::GetOutputCount() const
{
    return m_wires.size() - m_inputs;
}
bool Node::IsInputOnly() const
{
    _ASSERT_EXPR(m_wires.size() >= m_inputs, "Malformed Node inputs");
    return m_wires.size() == m_inputs;
}
bool Node::IsOutputOnly() const
{
    return !m_inputs;
}

void Node::Draw(IVec2 position, Gate gate, Color color)
{
    constexpr int nodeRadius = static_cast<int>(g_nodeRadius);
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
void Node::Draw(Color color) const
{
    constexpr int nodeRadius = static_cast<int>(g_nodeRadius);

    Draw(m_position, m_gate, color);

    if (m_gate == Gate::RESISTOR)
    {
        DrawRectangle(GetX() - nodeRadius + 1, GetY() - nodeRadius + 1, nodeRadius * 2 - 2, nodeRadius * 2 - 2, g_resistanceBands[GetResistance()]);
        DrawRectangle(GetX() - nodeRadius + 2, GetY() - nodeRadius + 2, nodeRadius * 2 - 4, nodeRadius * 2 - 4, BLACK);
    }
    else if (m_gate == Gate::CAPACITOR)
    {
        DrawRectangle(GetX() - nodeRadius + 1, GetY() - nodeRadius + 1, nodeRadius * 2 - 2, nodeRadius * 2 - 2, ColorAlpha(g_nodeColorInactive, 1.0f - GetChargePercent()));
    }
}

bool Node::WireIsInput(Wire* wire) const
{
    return wire->end == this;
}
bool Node::WireIsOutput(Wire* wire) const
{
    return wire->start == this;
}


std::deque<Wire*>::iterator Node::FindWireIter_Expected(Wire* wire)
{
    auto it = std::find(m_wires.begin(), m_wires.end(), wire);
    _ASSERT_EXPR(it != m_wires.end(), "Expected wire");
    return it;
}
std::deque<Wire*>::iterator Node::FindWireIter(Wire* wire)
{
    return std::find(m_wires.begin(), m_wires.end(), wire);
}

void Node::SetState(bool state)
{
    m_state = state;
}

void Node::AddWireInput(Wire* input)
{
    m_wires.push_front(input);
    m_inputs++;
}
void Node::AddWireOutput(Wire* output)
{
    m_wires.push_back(output);
}

// Expects the wire to exist; throws a debug exception if it is not found.
void Node::RemoveWire_Expected(Wire* wire)
{
    m_wires.erase(FindWireIter_Expected(wire));
    if (WireIsInput(wire))
        m_inputs--;
}
void Node::RemoveWire(Wire* wire)
{
    auto it = FindWireIter(wire);
    if (it == m_wires.end())
        return; // Success!

    m_wires.erase(it);
    if (WireIsInput(wire))
        m_inputs--;
}

// Expects the wire to exist; throws a debug exception if it is not found.
void Node::RemoveConnection_Expected(Node* node)
{
    auto it = FindConnection(node);
    _ASSERT_EXPR(it != m_wires.end(), "Expected connection to be pre-existing");
    m_wires.erase(it);
}
void Node::RemoveConnection(Node* node)
{
    auto it = FindConnection(node);
    if (it != m_wires.end())
        m_wires.erase(it);
}

// Converts an existing wire
void Node::MakeWireInput(Wire* wire)
{
    if (WireIsInput(wire))
        return; // Success!

    RemoveWire_Expected(wire);
    AddWireInput(wire);
}
// Converts an existing wire
void Node::MakeWireOutput(Wire* wire)
{
    if (WireIsOutput(wire))
        return; // Success!

    RemoveWire_Expected(wire);
    AddWireOutput(wire);
}


Node::Node(IVec2 position, Gate gate) : m_position(position), m_gate(gate), m_state(false), m_inputs(0), m_ntd() {}
Node::Node(IVec2 position, Gate gate, uint8_t extraParam) : m_position(position), m_gate(gate), m_state(false), m_inputs(0)
{
    if (gate == Gate::RESISTOR)
        m_ntd.r.resistance = extraParam;
    else if (gate == Gate::CAPACITOR)
    {
        m_ntd.c.capacity = extraParam;
        m_ntd.c.charge = 0;
    }
}


Range<std::deque<Wire*>::iterator> Node::GetInputs()
{
    return MakeRange<std::deque<Wire*>>(m_wires, 0, m_inputs);
}
Range<std::deque<Wire*>::iterator> Node::GetOutputs()
{
    return MakeRange<std::deque<Wire*>>(m_wires, m_inputs, m_wires.size());
}


const std::deque<Wire*>& Node::GetWires() const
{
    return m_wires;
}
Range<std::deque<Wire*>::const_iterator> Node::GetInputs() const
{
    return MakeRange<std::deque<Wire*>>(m_wires, 0, m_inputs);
}
Range<std::deque<Wire*>::const_iterator> Node::GetOutputs() const
{
    return MakeRange<std::deque<Wire*>>(m_wires, m_inputs, m_wires.size());
}

bool Node::IsValidConnection(std::deque<Wire*>::const_iterator it) const
{
    return it != m_wires.end();
}