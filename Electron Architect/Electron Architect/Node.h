#pragma once
#include "IVec.h"

struct Wire;

#include "Gate_Enum.h"
constexpr char GateToChar(Gate gate)
{
    switch (gate)
    {
        ASSERT_SPECIALIZATION;

    case Gate::OR: return '|';
    case Gate::AND: return '&';
    case Gate::NOR: return '!';
    case Gate::XOR: return '^';

    case Gate::RESISTOR: return '~';
    case Gate::CAPACITOR: return '=';
    case Gate::LED: return '@';
    case Gate::DELAY: return ';';
    }
}
constexpr Gate CharToGate(char symbol)
{
    switch (symbol)
    {
        ASSERT_SPECIALIZATION;

    case '|': return Gate::OR;
    case '&': return Gate::AND;
    case '!': return Gate::NOR;
    case '^': return Gate::XOR;

    case '~': return Gate::RESISTOR;
    case '=': return Gate::CAPACITOR;
    case '@': return Gate::LED;
    case ';': return Gate::DELAY;
    }
}

class Node
{
public:
    IVec2 GetPosition() const;
    // Moves the node without updating its collision
    void SetPosition_Temporary(IVec2 position);
    // Sets the position of the node and updates its collision in NodeWorld
    // NOTE: NodeWorld collision is currently a saved-for-later feature
    void SetPosition(IVec2 position);
    int GetX() const;
    void SetX(int x);
    int GetY() const;
    void SetY(int y);

    Gate GetGate() const;
    void SetGate(Gate gate);

    uint8_t GetResistance() const;
    uint8_t GetColorIndex() const;
    static const char* GetColorName(uint8_t index);
    uint8_t GetCapacity() const;
    uint8_t GetCharge() const;
    float GetChargePercent() const;
    void SetResistance(uint8_t resistance);
    void SetColorIndex(uint8_t colorIndex);
    void SetCapacity(uint8_t capacity);
    void SetCharge(uint8_t charge);
    void IncrementCharge();
    void DecrementCharge();

    bool GetState() const;

    std::deque<Wire*>::const_iterator FindConnection(Node* other) const;

    size_t GetInputCount() const;
    size_t GetOutputCount() const;
    bool IsInputOnly() const;
    bool IsOutputOnly() const;
    bool IsSpecialErasable() const;
    bool IsComplexBipassable() const;

    static void Draw(IVec2 position, Gate gate, Color color);
    void Draw(Color color) const;

    bool WireIsInput(Wire* wire) const;
    bool WireIsOutput(Wire* wire) const;

    // Only NodeWorld can play with a node's wires/state
    friend class NodeWorld;
    friend class Component;

private: // Helpers usable only by NodeWorld

    std::deque<Wire*>::iterator FindWireIter_Expected(Wire* wire);
    std::deque<Wire*>::iterator FindWireIter(Wire* wire);

    void SetState(bool state);

    void AddWireInput(Wire* input);
    void AddWireOutput(Wire* output);

    // Expects the wire to exist; throws a debug exception if it is not found.
    void RemoveWire_Expected(Wire* wire);
    void RemoveWire(Wire* wire);

    // Expects the wire to exist; throws a debug exception if it is not found.
    void RemoveConnection_Expected(Node* node);
    void RemoveConnection(Node* node);

    // Converts an existing wire
    void MakeWireInput(Wire* wire);
    // Converts an existing wire
    void MakeWireOutput(Wire* wire);

private: // Accessible by NodeWorld
    Node() = default;
    Node(IVec2 position, Gate gate);
    // It is entirely safe to pass in an extra param even if the node cannot use it!
    Node(IVec2 position, Gate gate, uint8_t extraParam);

public:
    static constexpr Color g_nodeColorActive = RED;
    static constexpr Color g_nodeColorInactive = LIGHTGRAY;
    static constexpr Color g_resistanceBands[] = {
        { 0,0,0, 255 },
        { 126,63,0, 255 },
        { 255,0,0, 255 },
        { 255,127,0, 255 },
        { 255,255,0, 255 },
        { 0,255,0, 255 },
        { 0,0,255, 255 },
        { 127,0,255, 255 },
        { 127,127,127, 255 },
        { 255,255,255, 255 }
    };
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

        struct LEDData
        {
            uint8_t colorIndex; // Number of nodes needed to evaluate true
        } l;

        struct CapacitorData
        {
            uint8_t capacity; // Max value of charge
            uint8_t charge; // Stored ticks of evaluating true
        } c;

        struct DelayData
        {
            bool lastState = false; // State previous tick
        } d;

        NonTransistorData() { memset(this, 0, sizeof(NonTransistorData)); }

    } m_ntd;
    // Keep this partitioned by inputs vs outputs
    std::deque<Wire*> m_wires;

private:
    Range<std::deque<Wire*>::iterator> GetInputs();
    Range<std::deque<Wire*>::iterator> GetOutputs();

public:
    const std::deque<Wire*>& GetWires() const;
    Range<std::deque<Wire*>::const_iterator> GetInputs() const;
    Range<std::deque<Wire*>::const_iterator> GetOutputs() const;

    bool IsValidConnection(std::deque<Wire*>::const_iterator it) const;
};
