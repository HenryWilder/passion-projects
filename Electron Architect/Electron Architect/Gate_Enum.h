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
