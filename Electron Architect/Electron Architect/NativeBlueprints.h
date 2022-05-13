#pragma once
#include "Blueprint.h"

// Built-in blueprints
inline Blueprint nativeBlueprints[] =
{
    Blueprint(
        "NAND Gate",
        // Nodes
        {
            NodeBP(true, Gate::AND,  IVec2(0,0) * g_gridSize),
            NodeBP(true, Gate::NOR,  IVec2(1,0) * g_gridSize),
        },
        // Wires
        {
            WireBP(0, 1, ElbowConfig::horizontal),
        }),
    Blueprint(
        "XNOR Gate",
        // Nodes
        {
            NodeBP(true, Gate::XOR,  IVec2(0,0) * g_gridSize),
            NodeBP(true, Gate::NOR,  IVec2(1,0) * g_gridSize),
        },
        // Wires
        {
            WireBP(0, 1, ElbowConfig::horizontal),
        }),
    Blueprint(
        "Switch",
        // Nodes
        {
            NodeBP("In",        true, Gate::OR,  IVec2(0,0)* g_gridSize),
            NodeBP("Interface", true, Gate::OR,  IVec2(1,0)* g_gridSize),
            NodeBP("Out",       true, Gate::AND, IVec2(2,0)* g_gridSize),
        },
        // Wires
        {
            WireBP(0, 2, ElbowConfig::horizontal),
            WireBP(1, 2, ElbowConfig::horizontal),
        }),
    Blueprint(
        "Gated SR Latch",
        // Nodes
        {
            NodeBP("Enable", true, Gate::NOR, IVec2(0,1) * g_gridSize), // NOR instead of OR to prevent uninitialized-flicker

            NodeBP("Set", true, Gate::OR,  IVec2(0,0)* g_gridSize),

            NodeBP(false, Gate::OR,  IVec2(1,0) * g_gridSize),
            NodeBP("Reset (inverse of Set)", false, Gate::NOR, IVec2(1,1) * g_gridSize),

            NodeBP(false, Gate::AND, IVec2(2,0) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(2,1) * g_gridSize),

            NodeBP(false, Gate::OR,  IVec2(3,0) * g_gridSize),
            NodeBP(false, Gate::OR,  IVec2(3,1) * g_gridSize),

            NodeBP("Q", false, Gate::NOR, IVec2(4,0)* g_gridSize),
            NodeBP("Q (Out)", true, Gate::NOR, IVec2(4,1)* g_gridSize),
        },
        // Wires
        {
            WireBP(0, 2, ElbowConfig::diagonalA),
            WireBP(1, 3, ElbowConfig::diagonalA),
            WireBP(1, 4, ElbowConfig::diagonalA),
            WireBP(2, 4, ElbowConfig::diagonalA),
            WireBP(2, 5, ElbowConfig::diagonalA),
            WireBP(3, 5, ElbowConfig::diagonalA),
            WireBP(4, 6, ElbowConfig::diagonalA),
            WireBP(5, 7, ElbowConfig::diagonalA),
            WireBP(6, 8, ElbowConfig::diagonalA),
            WireBP(7, 9, ElbowConfig::diagonalA),
            WireBP(8, 7, ElbowConfig::diagonalA),
            WireBP(9, 6, ElbowConfig::diagonalA),
        }),
    Blueprint(
        "1-to-2 Byte Multiplexer",
        // Nodes
        {
            NodeBP("I0_1", true, Gate::OR, IVec2(0,000) * g_gridSize),
            NodeBP("I0_2", true, Gate::OR, IVec2(0,001) * g_gridSize),
            NodeBP("I0_3", true, Gate::OR, IVec2(0,002) * g_gridSize),
            NodeBP("I0_4", true, Gate::OR, IVec2(0,003) * g_gridSize),
            NodeBP("I0_5", true, Gate::OR, IVec2(0,004) * g_gridSize),
            NodeBP("I0_6", true, Gate::OR, IVec2(0,005) * g_gridSize),
            NodeBP("I0_7", true, Gate::OR, IVec2(0,006) * g_gridSize),
            NodeBP("I0_8", true, Gate::OR, IVec2(0,007) * g_gridSize),

            NodeBP(false, Gate::AND, IVec2(1,000) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,001) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,002) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,003) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,004) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,005) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,006) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,007) * g_gridSize),

            NodeBP("I1_1", true, Gate::OR, IVec2(0,010) * g_gridSize),
            NodeBP("I1_2", true, Gate::OR, IVec2(0,011) * g_gridSize),
            NodeBP("I1_3", true, Gate::OR, IVec2(0,012) * g_gridSize),
            NodeBP("I1_4", true, Gate::OR, IVec2(0,013) * g_gridSize),
            NodeBP("I1_5", true, Gate::OR, IVec2(0,014) * g_gridSize),
            NodeBP("I1_6", true, Gate::OR, IVec2(0,015) * g_gridSize),
            NodeBP("I1_7", true, Gate::OR, IVec2(0,016) * g_gridSize),
            NodeBP("I1_8", true, Gate::OR, IVec2(0,017) * g_gridSize),

            NodeBP(false, Gate::AND, IVec2(1,010) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,011) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,012) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,013) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,014) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,015) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,016) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,017) * g_gridSize),

            NodeBP("O_1", true, Gate::OR, IVec2(2,004) * g_gridSize),
            NodeBP("O_2", true, Gate::OR, IVec2(2,005) * g_gridSize),
            NodeBP("O_3", true, Gate::OR, IVec2(2,006) * g_gridSize),
            NodeBP("O_4", true, Gate::OR, IVec2(2,007) * g_gridSize),
            NodeBP("O_5", true, Gate::OR, IVec2(2,010) * g_gridSize),
            NodeBP("O_6", true, Gate::OR, IVec2(2,011) * g_gridSize),
            NodeBP("O_7", true, Gate::OR, IVec2(2,012) * g_gridSize),
            NodeBP("O_8", true, Gate::OR, IVec2(2,013) * g_gridSize),

            NodeBP("A", true, Gate::OR, IVec2(1,020)* g_gridSize),
            NodeBP("A'", false, Gate::NOR, IVec2(0,020)* g_gridSize),
        },
        {
            WireBP(000, 010, ElbowConfig::horizontal),
            WireBP(001, 011, ElbowConfig::horizontal),
            WireBP(002, 012, ElbowConfig::horizontal),
            WireBP(003, 013, ElbowConfig::horizontal),
            WireBP(004, 014, ElbowConfig::horizontal),
            WireBP(005, 015, ElbowConfig::horizontal),
            WireBP(006, 016, ElbowConfig::horizontal),
            WireBP(007, 017, ElbowConfig::horizontal),

            WireBP(020, 030, ElbowConfig::horizontal),
            WireBP(021, 031, ElbowConfig::horizontal),
            WireBP(022, 032, ElbowConfig::horizontal),
            WireBP(023, 033, ElbowConfig::horizontal),
            WireBP(024, 034, ElbowConfig::horizontal),
            WireBP(025, 035, ElbowConfig::horizontal),
            WireBP(026, 036, ElbowConfig::horizontal),
            WireBP(027, 037, ElbowConfig::horizontal),

            WireBP(010, 040, ElbowConfig::diagonalA),
            WireBP(011, 041, ElbowConfig::diagonalA),
            WireBP(012, 042, ElbowConfig::diagonalA),
            WireBP(013, 043, ElbowConfig::diagonalA),
            WireBP(014, 044, ElbowConfig::diagonalA),
            WireBP(015, 045, ElbowConfig::diagonalA),
            WireBP(016, 046, ElbowConfig::diagonalA),
            WireBP(017, 047, ElbowConfig::diagonalA),

            WireBP(030, 040, ElbowConfig::diagonalA),
            WireBP(031, 041, ElbowConfig::diagonalA),
            WireBP(032, 042, ElbowConfig::diagonalA),
            WireBP(033, 043, ElbowConfig::diagonalA),
            WireBP(034, 044, ElbowConfig::diagonalA),
            WireBP(035, 045, ElbowConfig::diagonalA),
            WireBP(036, 046, ElbowConfig::diagonalA),
            WireBP(037, 047, ElbowConfig::diagonalA),

            WireBP(050, 051, ElbowConfig::horizontal),

            WireBP(050, 010, ElbowConfig::vertical),
            WireBP(050, 011, ElbowConfig::vertical),
            WireBP(050, 012, ElbowConfig::vertical),
            WireBP(050, 013, ElbowConfig::vertical),
            WireBP(050, 014, ElbowConfig::vertical),
            WireBP(050, 015, ElbowConfig::vertical),
            WireBP(050, 016, ElbowConfig::vertical),
            WireBP(050, 017, ElbowConfig::vertical),

            WireBP(051, 030, ElbowConfig::diagonalB),
            WireBP(051, 031, ElbowConfig::diagonalB),
            WireBP(051, 032, ElbowConfig::diagonalB),
            WireBP(051, 033, ElbowConfig::diagonalB),
            WireBP(051, 034, ElbowConfig::diagonalB),
            WireBP(051, 035, ElbowConfig::diagonalB),
            WireBP(051, 036, ElbowConfig::diagonalB),
            WireBP(051, 037, ElbowConfig::diagonalB),
        }),
    Blueprint(
        "2-to-1 Byte Multiplexer",
        // Nodes
        {
            NodeBP("O0_1", true, Gate::OR, IVec2(2,000) * g_gridSize),
            NodeBP("O0_2", true, Gate::OR, IVec2(2,001) * g_gridSize),
            NodeBP("O0_3", true, Gate::OR, IVec2(2,002) * g_gridSize),
            NodeBP("O0_4", true, Gate::OR, IVec2(2,003) * g_gridSize),
            NodeBP("O0_5", true, Gate::OR, IVec2(2,004) * g_gridSize),
            NodeBP("O0_6", true, Gate::OR, IVec2(2,005) * g_gridSize),
            NodeBP("O0_7", true, Gate::OR, IVec2(2,006) * g_gridSize),
            NodeBP("O0_8", true, Gate::OR, IVec2(2,007) * g_gridSize),

            NodeBP(false, Gate::AND, IVec2(1,000) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,001) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,002) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,003) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,004) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,005) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,006) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,007) * g_gridSize),

            NodeBP("O1_1", true, Gate::OR, IVec2(2,010) * g_gridSize),
            NodeBP("O1_2", true, Gate::OR, IVec2(2,011) * g_gridSize),
            NodeBP("O1_3", true, Gate::OR, IVec2(2,012) * g_gridSize),
            NodeBP("O1_4", true, Gate::OR, IVec2(2,013) * g_gridSize),
            NodeBP("O1_5", true, Gate::OR, IVec2(2,014) * g_gridSize),
            NodeBP("O1_6", true, Gate::OR, IVec2(2,015) * g_gridSize),
            NodeBP("O1_7", true, Gate::OR, IVec2(2,016) * g_gridSize),
            NodeBP("O1_8", true, Gate::OR, IVec2(2,017) * g_gridSize),

            NodeBP(false, Gate::AND, IVec2(1,010) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,011) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,012) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,013) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,014) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,015) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,016) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,017) * g_gridSize),

            NodeBP("I_1", true, Gate::OR, IVec2(0,004) * g_gridSize),
            NodeBP("I_2", true, Gate::OR, IVec2(0,005) * g_gridSize),
            NodeBP("I_3", true, Gate::OR, IVec2(0,006) * g_gridSize),
            NodeBP("I_4", true, Gate::OR, IVec2(0,007) * g_gridSize),
            NodeBP("I_5", true, Gate::OR, IVec2(0,010) * g_gridSize),
            NodeBP("I_6", true, Gate::OR, IVec2(0,011) * g_gridSize),
            NodeBP("I_7", true, Gate::OR, IVec2(0,012) * g_gridSize),
            NodeBP("I_8", true, Gate::OR, IVec2(0,013) * g_gridSize),

            NodeBP("A", true, Gate::OR, IVec2(1,020)* g_gridSize),
            NodeBP("A'", false, Gate::NOR, IVec2(2,020)* g_gridSize),
        },
        {
            WireBP(010, 000, ElbowConfig::horizontal),
            WireBP(011, 001, ElbowConfig::horizontal),
            WireBP(012, 002, ElbowConfig::horizontal),
            WireBP(013, 003, ElbowConfig::horizontal),
            WireBP(014, 004, ElbowConfig::horizontal),
            WireBP(015, 005, ElbowConfig::horizontal),
            WireBP(016, 006, ElbowConfig::horizontal),
            WireBP(017, 007, ElbowConfig::horizontal),

            WireBP(030, 020, ElbowConfig::horizontal),
            WireBP(031, 021, ElbowConfig::horizontal),
            WireBP(032, 022, ElbowConfig::horizontal),
            WireBP(033, 023, ElbowConfig::horizontal),
            WireBP(034, 024, ElbowConfig::horizontal),
            WireBP(035, 025, ElbowConfig::horizontal),
            WireBP(036, 026, ElbowConfig::horizontal),
            WireBP(037, 027, ElbowConfig::horizontal),

            WireBP(040, 010, ElbowConfig::diagonalB),
            WireBP(041, 011, ElbowConfig::diagonalB),
            WireBP(042, 012, ElbowConfig::diagonalB),
            WireBP(043, 013, ElbowConfig::diagonalB),
            WireBP(044, 014, ElbowConfig::diagonalB),
            WireBP(045, 015, ElbowConfig::diagonalB),
            WireBP(046, 016, ElbowConfig::diagonalB),
            WireBP(047, 017, ElbowConfig::diagonalB),

            WireBP(040, 030, ElbowConfig::diagonalB),
            WireBP(041, 031, ElbowConfig::diagonalB),
            WireBP(042, 032, ElbowConfig::diagonalB),
            WireBP(043, 033, ElbowConfig::diagonalB),
            WireBP(044, 034, ElbowConfig::diagonalB),
            WireBP(045, 035, ElbowConfig::diagonalB),
            WireBP(046, 036, ElbowConfig::diagonalB),
            WireBP(047, 037, ElbowConfig::diagonalB),

            WireBP(050, 051, ElbowConfig::horizontal),

            WireBP(050, 010, ElbowConfig::vertical),
            WireBP(050, 011, ElbowConfig::vertical),
            WireBP(050, 012, ElbowConfig::vertical),
            WireBP(050, 013, ElbowConfig::vertical),
            WireBP(050, 014, ElbowConfig::vertical),
            WireBP(050, 015, ElbowConfig::vertical),
            WireBP(050, 016, ElbowConfig::vertical),
            WireBP(050, 017, ElbowConfig::vertical),

            WireBP(051, 030, ElbowConfig::diagonalB),
            WireBP(051, 031, ElbowConfig::diagonalB),
            WireBP(051, 032, ElbowConfig::diagonalB),
            WireBP(051, 033, ElbowConfig::diagonalB),
            WireBP(051, 034, ElbowConfig::diagonalB),
            WireBP(051, 035, ElbowConfig::diagonalB),
            WireBP(051, 036, ElbowConfig::diagonalB),
            WireBP(051, 037, ElbowConfig::diagonalB),
        }),
    Blueprint(
        "Half-Adder",
        // Nodes
        {
            NodeBP("B", true, Gate::OR,  IVec2(0,1)* g_gridSize),
            NodeBP("A", true, Gate::OR,  IVec2(0,0)* g_gridSize),
            NodeBP("Sum", true, Gate::XOR, IVec2(1,0)* g_gridSize),
            NodeBP("Carry", true, Gate::AND, IVec2(1,1)* g_gridSize),
        },
        // Wires
        {
            WireBP(0, 2, ElbowConfig::horizontal),
            WireBP(1, 2, ElbowConfig::horizontal),
            WireBP(0, 3, ElbowConfig::vertical),
            WireBP(1, 3, ElbowConfig::vertical),
        }),
    Blueprint(
        "Half-Subtractor",
        // Nodes
        {
            NodeBP("A", true,  Gate::OR,  IVec2(0,0)* g_gridSize), // A
            NodeBP("B", true,  Gate::OR,  IVec2(0,1)* g_gridSize), // B
            NodeBP(false, Gate::NOR, IVec2(1,1) * g_gridSize),
            NodeBP("Difference", true,  Gate::XOR, IVec2(2,0) * g_gridSize), // Difference
            NodeBP("Borrow", true,  Gate::AND, IVec2(2,1) * g_gridSize), // Borrow
        },
        // Wires
        {
            WireBP(0, 2, ElbowConfig::vertical),
            WireBP(0, 3, ElbowConfig::horizontal),
            WireBP(1, 3, ElbowConfig::horizontal),
            WireBP(1, 4, ElbowConfig::vertical),
            WireBP(2, 4, ElbowConfig::vertical),
        }),
    Blueprint(
        "Full-Adder",
        // Nodes
        {
            NodeBP("A", true,  Gate::OR,  IVec2(0,0)* g_gridSize),
            NodeBP("B", true,  Gate::OR,  IVec2(0,1)* g_gridSize),
            NodeBP("Carry In", true,  Gate::OR,  IVec2(2,0)* g_gridSize),
            NodeBP(false, Gate::XOR, IVec2(1,0) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,1) * g_gridSize),
            NodeBP("Sum", true,  Gate::XOR, IVec2(3,0)* g_gridSize),
            NodeBP(false, Gate::AND, IVec2(2,1) * g_gridSize),
            NodeBP("Carry Out", true,  Gate::OR,  IVec2(3,1)* g_gridSize),
        },
        // Wires
        {
            WireBP(0, 3, ElbowConfig::horizontal),
            WireBP(1, 3, ElbowConfig::horizontal),
            WireBP(0, 4, ElbowConfig::vertical),
            WireBP(1, 4, ElbowConfig::vertical),
            WireBP(2, 5, ElbowConfig::horizontal),
            WireBP(3, 5, ElbowConfig::horizontal),
            WireBP(2, 6, ElbowConfig::vertical),
            WireBP(3, 6, ElbowConfig::vertical),
            WireBP(6, 7, ElbowConfig::vertical),
            WireBP(4, 7, ElbowConfig::horizontal),
        }),
    Blueprint(
        "Full-Subtractor",
        // Nodes
        {
            NodeBP("A", true,  Gate::OR,  IVec2(0,0)* g_gridSize),
            NodeBP("B", true,  Gate::OR,  IVec2(0,1)* g_gridSize),
            NodeBP("Borrow In", true,  Gate::OR,  IVec2(1,0)* g_gridSize),
            NodeBP(false, Gate::NOR, IVec2(1,1) * g_gridSize),
            NodeBP(false, Gate::XOR, IVec2(2,0) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(2,1) * g_gridSize),
            NodeBP("Difference", true,  Gate::XOR, IVec2(4,0)* g_gridSize),
            NodeBP(false, Gate::AND, IVec2(3,1) * g_gridSize),
            NodeBP(false, Gate::NOR, IVec2(3,0) * g_gridSize),
            NodeBP("Borrow Out", true,  Gate::OR,  IVec2(4,1)* g_gridSize),
        },
        // Wires
        {
            WireBP(0, 3, ElbowConfig::vertical),
            WireBP(1, 5, ElbowConfig::vertical),
            WireBP(3, 5, ElbowConfig::vertical),
            WireBP(2, 6, ElbowConfig::vertical),
            WireBP(2, 7, ElbowConfig::vertical),
            WireBP(8, 7, ElbowConfig::vertical),
            WireBP(1, 4, ElbowConfig::vertical),
            WireBP(0, 4, ElbowConfig::vertical),
            WireBP(4, 8, ElbowConfig::vertical),
            WireBP(4, 6, ElbowConfig::vertical),
            WireBP(5, 9, ElbowConfig::vertical),
            WireBP(7, 9, ElbowConfig::vertical),
        }),
};
