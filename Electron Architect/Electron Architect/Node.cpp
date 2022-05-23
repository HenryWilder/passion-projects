#include "HUtility.h"
#include "Wire.h"
#include "Node.h"

#include "nodeicons/code/nodeIconsBasic8x.h"
#include "nodeicons/code/nodeIconsNTD8x.h"
#include "nodeicons/code/nodeIconsBackground8x.h"
#include "nodeicons/code/nodeIconsHighlight8x.h"
#include "nodeicons/code/nodeIconsBasic16x.h"
#include "nodeicons/code/nodeIconsNTD16x.h"
#include "nodeicons/code/nodeIconsBackground16x.h"
#include "nodeicons/code/nodeIconsHighlight16x.h"
#include "nodeicons/code/nodeIconsBasic32x.h"
#include "nodeicons/code/nodeIconsNTD32x.h"
#include "nodeicons/code/nodeIconsBackground32x.h"
#include "nodeicons/code/nodeIconsHighlight32x.h"

enum class NodeIconType
{
    Basic,
    NTD,
    Background,
    Highlight,
};
enum class NodeIconScale
{
    x8,
    x16,
    x32,
};
Texture2D g_nodeIcons[4][3];
Texture2D& NodeIcon(NodeIconType type, NodeIconScale scale)
{
    return g_nodeIcons[(int)type][(int)scale];
}

void DrawNodeIcon(IVec2 pos, NodeIconType type, float zoom, Gate gate, Color tint)
{
    NodeIconScale scale;
    int width = 8 * zoom;

    Rectangle dest;
    dest.x = (float)pos.x;
    dest.y = (float)pos.y;
    dest.width = dest.height = (float)width / zoom;

    if (zoom < 1.0f)
    {
        DrawRectangleRec(dest, tint);
        return;
    }

    switch ((int)(zoom))
    {
    default:
    case 1: scale = NodeIconScale::x8;  break;
    case 2: scale = NodeIconScale::x16; break;
    case 4: scale = NodeIconScale::x32; break;
    }

    const Texture2D& tex = NodeIcon(type, scale);
    IVec2 textureSheetPos;
    switch (gate)
    {
    default: _ASSERT_EXPR(false, L"Gate type not given specialize draw method");
    case Gate::OR:          textureSheetPos = IVec2(0, 0); break;
    case Gate::NOR:         textureSheetPos = IVec2(1, 0); break;
    case Gate::AND:         textureSheetPos = IVec2(2, 0); break;
    case Gate::XOR:         textureSheetPos = IVec2(3, 0); break;
    case Gate::RESISTOR:    textureSheetPos = IVec2(0, 1); break;
    case Gate::CAPACITOR:   textureSheetPos = IVec2(1, 1); break;
    case Gate::LED:         textureSheetPos = IVec2(2, 1); break;
    case Gate::DELAY:       textureSheetPos = IVec2(3, 1); break;
    case Gate::BATTERY:     textureSheetPos = IVec2(0, 2); break;
    }

    Rectangle src;
    src.x = (float)width * (float)textureSheetPos.x;
    src.y = (float)width * (float)textureSheetPos.y;
    src.width = src.height  = (float)width;

    DrawTexturePro(tex, src, dest, { 0 }, 0.0f, tint);
}

void InitNodeIcons()
{
    NodeIcon(NodeIconType::Basic,       NodeIconScale::x8 ) = LoadTextureFromImage(MEMORY_IMAGE(NODEICONSBASIC8X ));
    NodeIcon(NodeIconType::Basic,       NodeIconScale::x16) = LoadTextureFromImage(MEMORY_IMAGE(NODEICONSBASIC16X));
    NodeIcon(NodeIconType::Basic,       NodeIconScale::x32) = LoadTextureFromImage(MEMORY_IMAGE(NODEICONSBASIC32X));
    NodeIcon(NodeIconType::NTD,         NodeIconScale::x8 ) = LoadTextureFromImage(MEMORY_IMAGE(NODEICONSNTD8X ));
    NodeIcon(NodeIconType::NTD,         NodeIconScale::x16) = LoadTextureFromImage(MEMORY_IMAGE(NODEICONSNTD16X));
    NodeIcon(NodeIconType::NTD,         NodeIconScale::x32) = LoadTextureFromImage(MEMORY_IMAGE(NODEICONSNTD32X));
    NodeIcon(NodeIconType::Background,  NodeIconScale::x8 ) = LoadTextureFromImage(MEMORY_IMAGE(NODEICONSBACKGROUND8X ));
    NodeIcon(NodeIconType::Background,  NodeIconScale::x16) = LoadTextureFromImage(MEMORY_IMAGE(NODEICONSBACKGROUND16X));
    NodeIcon(NodeIconType::Background,  NodeIconScale::x32) = LoadTextureFromImage(MEMORY_IMAGE(NODEICONSBACKGROUND32X));
    NodeIcon(NodeIconType::Highlight,   NodeIconScale::x8 ) = LoadTextureFromImage(MEMORY_IMAGE(NODEICONSHIGHLIGHT8X ));
    NodeIcon(NodeIconType::Highlight,   NodeIconScale::x16) = LoadTextureFromImage(MEMORY_IMAGE(NODEICONSHIGHLIGHT16X));
    NodeIcon(NodeIconType::Highlight,   NodeIconScale::x32) = LoadTextureFromImage(MEMORY_IMAGE(NODEICONSHIGHLIGHT32X));
}

void FreeNodeIcons()
{
    for (auto& row : g_nodeIcons)
    {
        for (const Texture2D& tex : row)
        {
            UnloadTexture(tex);
        }
    }
}

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
// Sets the position of the node and updates its collision in Graph
// NOTE: Graph collision is currently a saved-for-later feature
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

bool Node::HasName() const
{
    return !m_name.empty() && m_name[0] != '\0';
}

const std::string& Node::GetName() const
{
    return m_name;
}

void Node::SetName(const std::string& name)
{
    m_name = name;
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
    _ASSERT_EXPR(m_gate == Gate::RESISTOR, L"Cannot access the resistance of a non-resistor.");
    return m_ntd.r.resistance;
}
uint8_t Node::GetColorIndex() const
{
    _ASSERT_EXPR(m_gate == Gate::LED, L"Cannot access the color of a non-LED.");
    return m_ntd.l.colorIndex;
}
const char* Node::GetColorName(uint8_t index)
{
    constexpr const char* colorName[]
    {
        "black",
        "brown",
        "red",
        "orange",
        "yellow",
        "green",
        "blue",
        "violet",
        "gray",
        "white",
    };
    return colorName[index];
}
// Only use if this is a capacitor
uint8_t Node::GetCapacity() const
{
    _ASSERT_EXPR(m_gate == Gate::CAPACITOR, L"Cannot access the capacity of a non-capacitor.");
    return m_ntd.c.capacity;
}
// Only use if this is a capacitor
uint8_t Node::GetCharge() const
{
    _ASSERT_EXPR(m_gate == Gate::CAPACITOR, L"Cannot access the charge of a non-capacitor.");
    return m_ntd.c.charge;
}
// Only use if this is a capacitor
float Node::GetChargePercent() const
{
    _ASSERT_EXPR(m_gate == Gate::CAPACITOR, L"Cannot access capacitor members of a non-capacitor.");
    return (float)m_ntd.c.charge / (float)m_ntd.c.capacity;
}
// Only use if this is a resistor
void Node::SetResistance(uint8_t resistance)
{
    _ASSERT_EXPR(m_gate == Gate::RESISTOR, L"Cannot access the resistance of a non-resistor.");
    _ASSERT_EXPR(resistance <= 9, L"Selected resistance out of bounds");
    m_ntd.r.resistance = resistance;
}
void Node::SetColorIndex(uint8_t colorIndex)
{
    _ASSERT_EXPR(m_gate == Gate::LED, L"Cannot access the color of a non-LED.");
    _ASSERT_EXPR(colorIndex <= 9, L"Selected color out of bounds");
    m_ntd.l.colorIndex = colorIndex;
}
// Only use if this is a capacitor
void Node::SetCapacity(uint8_t capacity)
{
    _ASSERT_EXPR(m_gate == Gate::CAPACITOR, L"Cannot access the capacity of a non-capacitor.");
    m_ntd.c.capacity = capacity;
}
// Only use if this is a capacitor
void Node::SetCharge(uint8_t charge)
{
    _ASSERT_EXPR(m_gate == Gate::CAPACITOR, L"Cannot access the charge of a non-capacitor.");
    m_ntd.c.charge = charge;
}
// Only use if this is a capacitor
void Node::IncrementCharge()
{
    _ASSERT_EXPR(m_gate == Gate::CAPACITOR, L"Cannot access the charge of a non-capacitor.");
    m_ntd.c.charge++;
}
// Only use if this is a capacitor
void Node::DecrementCharge()
{
    _ASSERT_EXPR(m_gate == Gate::CAPACITOR, L"Cannot access the charge of a non-capacitor.");
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
    _ASSERT_EXPR(m_wires.size() >= m_inputs, L"Malformed Node inputs");
    return m_wires.size() == m_inputs;
}
bool Node::IsOutputOnly() const
{
    return !m_inputs;
}

bool Node::IsSpecialErasable() const
{
    return
        (GetInputCount()  == 1 && !IsInputOnly()) ||
        (GetOutputCount() == 1 && !IsOutputOnly());
}

bool Node::IsComplexBipassable() const
{
    return (GetInputCount() > 1 && GetOutputCount() > 1);
}

bool Node::IsPassthrough() const
{
    return m_gate == Gate::OR && GetInputCount() == 1 && GetOutputCount() > 0;
}

bool Node::IsInteractive() const
{
    return IsOutputOnly() && (m_gate == Gate::OR || m_gate == Gate::NOR);
}

void Node::Draw(float zoom, IVec2 position, Gate gate, Color foreground, Color background)
{
    IVec2 topleft = position - IVec2(g_gridSize / 2);
    DrawNodeIcon(topleft, NodeIconType::Background, zoom, gate, background);
    DrawNodeIcon(topleft, NodeIconType::Basic, zoom, gate, foreground);
}
void Node::DrawHighlight(float zoom, IVec2 position, Gate gate, Color highlight)
{
    DrawNodeIcon(position - IVec2(g_gridSize / 2), NodeIconType::Highlight, zoom, gate, highlight);
}
void Node::Draw(float zoom, Color foreground, Color background, Color CapacitorInactive) const
{
    if (IsPassthrough())
        return;

    Draw(zoom, m_position, m_gate, foreground, background);

    if (m_gate == Gate::RESISTOR || m_gate == Gate::LED || m_gate == Gate::CAPACITOR)
    {
        Color color;
        switch (m_gate)
        {
        default: _ASSERT_EXPR(false, L"NDT type in IF condition but not switch statement");
        case Gate::RESISTOR:    color = g_resistanceBands[GetResistance()]; break;
        case Gate::CAPACITOR:   color = ColorAlpha(CapacitorInactive, 1.0f - GetChargePercent()); break;
        case Gate::LED:         color = g_resistanceBands[GetColorIndex()]; break;
        }
        DrawNodeIcon(m_position - IVec2(g_gridSize / 2), NodeIconType::NTD, zoom, m_gate, color);
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
    _ASSERT_EXPR(it != m_wires.end(), L"Wire expected to be connected to node was not found in node's data");
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
    _ASSERT_EXPR(it != m_wires.end(), L"Expected connection to be pre-existing");
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


Node::Node(IVec2 position, Gate gate) :
    m_name(""), m_position(position), m_gate(gate), m_state(false), m_inputs(0), m_ntd() {}
Node::Node(IVec2 position, Gate gate, uint8_t extraParam) :
    m_name(""), m_position(position), m_gate(gate), m_state(false), m_inputs(0)
{
    if (gate == Gate::RESISTOR)
        m_ntd.r.resistance = extraParam;
    else if (gate == Gate::LED)
        m_ntd.l.colorIndex = extraParam;
    else if (gate == Gate::CAPACITOR)
    {
        m_ntd.c.capacity = extraParam;
        m_ntd.c.charge = 0;
    }
}
Node::Node(const char* name, IVec2 position, Gate gate, uint8_t extraParam) :
    m_name(name), m_position(position), m_gate(gate), m_state(false), m_inputs(0)
{
    if (gate == Gate::RESISTOR)
        m_ntd.r.resistance = extraParam;
    else if (gate == Gate::LED)
        m_ntd.l.colorIndex = extraParam;
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
Range<std::deque<Wire*>::const_iterator> Node::GetInputsConst() const
{
    return MakeRange<std::deque<Wire*>>(m_wires, 0, m_inputs);
}
Range<std::deque<Wire*>::const_iterator> Node::GetOutputsConst() const
{
    return MakeRange<std::deque<Wire*>>(m_wires, m_inputs, m_wires.size());
}

bool Node::IsValidConnection(std::deque<Wire*>::const_iterator it) const
{
    return it != m_wires.end();
}
