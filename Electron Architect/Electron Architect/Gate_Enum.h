#pragma once

enum class Gate : char
{
    OR,
    AND,
    NOR,
    XOR,

    RESISTOR,
    CAPACITOR,
    LED,
    DELAY,
};

constexpr Gate g_GateOrder[]{
    Gate::OR,
    Gate::AND,
    Gate::NOR,
    Gate::XOR,

    Gate::RESISTOR,
    Gate::CAPACITOR,
    Gate::LED,
    Gate::DELAY,
};

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
