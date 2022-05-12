#include "Graph.h"
#include "Blueprint.h"

// Built-in blueprints
Blueprint nativeBlueprints[] =
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

            NodeBP("D_1", true, Gate::OR, IVec2(2,004) * g_gridSize),
            NodeBP("D_2", true, Gate::OR, IVec2(2,005) * g_gridSize),
            NodeBP("D_3", true, Gate::OR, IVec2(2,006) * g_gridSize),
            NodeBP("D_4", true, Gate::OR, IVec2(2,007) * g_gridSize),
            NodeBP("D_5", true, Gate::OR, IVec2(2,010) * g_gridSize),
            NodeBP("D_6", true, Gate::OR, IVec2(2,011) * g_gridSize),
            NodeBP("D_7", true, Gate::OR, IVec2(2,012) * g_gridSize),
            NodeBP("D_8", true, Gate::OR, IVec2(2,013) * g_gridSize),

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

            WireBP(051, 030, ElbowConfig::vertical),
            WireBP(051, 031, ElbowConfig::vertical),
            WireBP(051, 032, ElbowConfig::vertical),
            WireBP(051, 033, ElbowConfig::vertical),
            WireBP(051, 034, ElbowConfig::vertical),
            WireBP(051, 035, ElbowConfig::vertical),
            WireBP(051, 036, ElbowConfig::vertical),
            WireBP(051, 037, ElbowConfig::vertical),
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
    Blueprint(
        "Byte-Adder",
        // Nodes
        {
            // Bit 1
            NodeBP("A1", true,  Gate::OR,  IVec2(0,0)* g_gridSize),
            NodeBP("B1", true,  Gate::OR,  IVec2(0,1)* g_gridSize),
            NodeBP("Out 1", true,  Gate::XOR, IVec2(3,0)* g_gridSize),
            NodeBP(false, Gate::AND, IVec2(3,1) * g_gridSize),
            // Bit 2
            NodeBP("A2", true,  Gate::OR,  IVec2(0,2)* g_gridSize),
            NodeBP("B2", true,  Gate::OR,  IVec2(0,3)* g_gridSize),
            NodeBP(false,  Gate::OR,  IVec2(2,2) * g_gridSize),
            NodeBP(false, Gate::XOR, IVec2(1,2) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,3) * g_gridSize),
            NodeBP("Out 2", true,  Gate::XOR, IVec2(3,2) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(2,3) * g_gridSize),
            NodeBP(false, Gate::OR,  IVec2(3,3) * g_gridSize),
            // Bit 3
            NodeBP("A3", true,  Gate::OR,  IVec2(0,4) * g_gridSize),
            NodeBP("B3", true,  Gate::OR,  IVec2(0,5) * g_gridSize),
            NodeBP(false,  Gate::OR,  IVec2(2,4) * g_gridSize),
            NodeBP(false, Gate::XOR, IVec2(1,4) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,5) * g_gridSize),
            NodeBP("Out 3", true,  Gate::XOR, IVec2(3,4) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(2,5) * g_gridSize),
            NodeBP(false, Gate::OR,  IVec2(3,5) * g_gridSize),
            // Bit 4
            NodeBP("A4", true,  Gate::OR,  IVec2(0,6) * g_gridSize),
            NodeBP("B4", true,  Gate::OR,  IVec2(0,7) * g_gridSize),
            NodeBP(false,  Gate::OR,  IVec2(2,6) * g_gridSize),
            NodeBP(false, Gate::XOR, IVec2(1,6) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,7) * g_gridSize),
            NodeBP("Out 4", true,  Gate::XOR, IVec2(3,6) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(2,7) * g_gridSize),
            NodeBP(false, Gate::OR,  IVec2(3,7) * g_gridSize),
            // Bit 5
            NodeBP("A5", true,  Gate::OR,  IVec2(0,8) * g_gridSize),
            NodeBP("B5", true,  Gate::OR,  IVec2(0,9) * g_gridSize),
            NodeBP(false,  Gate::OR,  IVec2(2,8) * g_gridSize),
            NodeBP(false, Gate::XOR, IVec2(1,8) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,9) * g_gridSize),
            NodeBP("Out 5", true,  Gate::XOR, IVec2(3,8) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(2,9) * g_gridSize),
            NodeBP(false, Gate::OR,  IVec2(3,9) * g_gridSize),
            // Bit 6
            NodeBP("A6", true,  Gate::OR,  IVec2(0,10) * g_gridSize),
            NodeBP("B6", true,  Gate::OR,  IVec2(0,11) * g_gridSize),
            NodeBP(false,  Gate::OR,  IVec2(2,10) * g_gridSize),
            NodeBP(false, Gate::XOR, IVec2(1,10) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,11) * g_gridSize),
            NodeBP("Out 6", true,  Gate::XOR, IVec2(3,10) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(2,11) * g_gridSize),
            NodeBP(false, Gate::OR,  IVec2(3,11) * g_gridSize),
            // Bit 7
            NodeBP("A7", true,  Gate::OR,  IVec2(0,12) * g_gridSize),
            NodeBP("B7", true,  Gate::OR,  IVec2(0,13) * g_gridSize),
            NodeBP(false,  Gate::OR,  IVec2(2,12) * g_gridSize),
            NodeBP(false, Gate::XOR, IVec2(1,12) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,13) * g_gridSize),
            NodeBP("Out 7", true,  Gate::XOR, IVec2(3,12) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(2,13) * g_gridSize),
            NodeBP(false, Gate::OR,  IVec2(3,13) * g_gridSize),
            // Bit 8
            NodeBP("A8", true,  Gate::OR,  IVec2(0,14) * g_gridSize),
            NodeBP("B8", true,  Gate::OR,  IVec2(0,15) * g_gridSize),
            NodeBP(false,  Gate::OR,  IVec2(2,14) * g_gridSize),
            NodeBP(false, Gate::XOR, IVec2(1,14) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1,15) * g_gridSize),
            NodeBP("Out 8", true,  Gate::XOR, IVec2(3,14)* g_gridSize),
            NodeBP(false, Gate::AND, IVec2(2,15) * g_gridSize),
            NodeBP("Overflow", true,  Gate::OR,  IVec2(3,15)* g_gridSize),
        },
        // Wires
        {
            // Bit 1
            WireBP(0, 2, ElbowConfig::horizontal),
            WireBP(1, 2, ElbowConfig::horizontal),
            WireBP(0, 3, ElbowConfig::vertical),
            WireBP(1, 3, ElbowConfig::vertical),

            WireBP(3, 4 + 2, ElbowConfig::horizontal),
            // Bit 2
            WireBP(4 + 0, 4 + 3, ElbowConfig::horizontal),
            WireBP(4 + 1, 4 + 3, ElbowConfig::horizontal),
            WireBP(4 + 0, 4 + 4, ElbowConfig::vertical),
            WireBP(4 + 1, 4 + 4, ElbowConfig::vertical),
            WireBP(4 + 2, 4 + 5, ElbowConfig::horizontal),
            WireBP(4 + 3, 4 + 5, ElbowConfig::horizontal),
            WireBP(4 + 2, 4 + 6, ElbowConfig::vertical),
            WireBP(4 + 3, 4 + 6, ElbowConfig::vertical),
            WireBP(4 + 6, 4 + 7, ElbowConfig::vertical),
            WireBP(4 + 4, 4 + 7, ElbowConfig::horizontal),

            WireBP(4 + 7, 12 + 2, ElbowConfig::horizontal),
            // Bit 3
            WireBP(12 + 0, 12 + 3, ElbowConfig::horizontal),
            WireBP(12 + 1, 12 + 3, ElbowConfig::horizontal),
            WireBP(12 + 0, 12 + 4, ElbowConfig::vertical),
            WireBP(12 + 1, 12 + 4, ElbowConfig::vertical),
            WireBP(12 + 2, 12 + 5, ElbowConfig::horizontal),
            WireBP(12 + 3, 12 + 5, ElbowConfig::horizontal),
            WireBP(12 + 2, 12 + 6, ElbowConfig::vertical),
            WireBP(12 + 3, 12 + 6, ElbowConfig::vertical),
            WireBP(12 + 6, 12 + 7, ElbowConfig::vertical),
            WireBP(12 + 4, 12 + 7, ElbowConfig::horizontal),

            WireBP(12 + 7, 20 + 2, ElbowConfig::horizontal),
            // Bit 4
            WireBP(20 + 0, 20 + 3, ElbowConfig::horizontal),
            WireBP(20 + 1, 20 + 3, ElbowConfig::horizontal),
            WireBP(20 + 0, 20 + 4, ElbowConfig::vertical),
            WireBP(20 + 1, 20 + 4, ElbowConfig::vertical),
            WireBP(20 + 2, 20 + 5, ElbowConfig::horizontal),
            WireBP(20 + 3, 20 + 5, ElbowConfig::horizontal),
            WireBP(20 + 2, 20 + 6, ElbowConfig::vertical),
            WireBP(20 + 3, 20 + 6, ElbowConfig::vertical),
            WireBP(20 + 6, 20 + 7, ElbowConfig::vertical),
            WireBP(20 + 4, 20 + 7, ElbowConfig::horizontal),

            WireBP(20 + 7, 28 + 2, ElbowConfig::horizontal),
            // Bit 5
            WireBP(28 + 0, 28 + 3, ElbowConfig::horizontal),
            WireBP(28 + 1, 28 + 3, ElbowConfig::horizontal),
            WireBP(28 + 0, 28 + 4, ElbowConfig::vertical),
            WireBP(28 + 1, 28 + 4, ElbowConfig::vertical),
            WireBP(28 + 2, 28 + 5, ElbowConfig::horizontal),
            WireBP(28 + 3, 28 + 5, ElbowConfig::horizontal),
            WireBP(28 + 2, 28 + 6, ElbowConfig::vertical),
            WireBP(28 + 3, 28 + 6, ElbowConfig::vertical),
            WireBP(28 + 6, 28 + 7, ElbowConfig::vertical),
            WireBP(28 + 4, 28 + 7, ElbowConfig::horizontal),

            WireBP(28 + 7, 36 + 2, ElbowConfig::horizontal),
            // Bit 6
            WireBP(36 + 0, 36 + 3, ElbowConfig::horizontal),
            WireBP(36 + 1, 36 + 3, ElbowConfig::horizontal),
            WireBP(36 + 0, 36 + 4, ElbowConfig::vertical),
            WireBP(36 + 1, 36 + 4, ElbowConfig::vertical),
            WireBP(36 + 2, 36 + 5, ElbowConfig::horizontal),
            WireBP(36 + 3, 36 + 5, ElbowConfig::horizontal),
            WireBP(36 + 2, 36 + 6, ElbowConfig::vertical),
            WireBP(36 + 3, 36 + 6, ElbowConfig::vertical),
            WireBP(36 + 6, 36 + 7, ElbowConfig::vertical),
            WireBP(36 + 4, 36 + 7, ElbowConfig::horizontal),

            WireBP(36 + 7, 44 + 2, ElbowConfig::horizontal),
            // Bit 7
            WireBP(44 + 0, 44 + 3, ElbowConfig::horizontal),
            WireBP(44 + 1, 44 + 3, ElbowConfig::horizontal),
            WireBP(44 + 0, 44 + 4, ElbowConfig::vertical),
            WireBP(44 + 1, 44 + 4, ElbowConfig::vertical),
            WireBP(44 + 2, 44 + 5, ElbowConfig::horizontal),
            WireBP(44 + 3, 44 + 5, ElbowConfig::horizontal),
            WireBP(44 + 2, 44 + 6, ElbowConfig::vertical),
            WireBP(44 + 3, 44 + 6, ElbowConfig::vertical),
            WireBP(44 + 6, 44 + 7, ElbowConfig::vertical),
            WireBP(44 + 4, 44 + 7, ElbowConfig::horizontal),

            WireBP(44 + 7, 52 + 2, ElbowConfig::horizontal),
            // Bit 8
            WireBP(52 + 0, 52 + 3, ElbowConfig::horizontal),
            WireBP(52 + 1, 52 + 3, ElbowConfig::horizontal),
            WireBP(52 + 0, 52 + 4, ElbowConfig::vertical),
            WireBP(52 + 1, 52 + 4, ElbowConfig::vertical),
            WireBP(52 + 2, 52 + 5, ElbowConfig::horizontal),
            WireBP(52 + 3, 52 + 5, ElbowConfig::horizontal),
            WireBP(52 + 2, 52 + 6, ElbowConfig::vertical),
            WireBP(52 + 3, 52 + 6, ElbowConfig::vertical),
            WireBP(52 + 6, 52 + 7, ElbowConfig::vertical),
            WireBP(52 + 4, 52 + 7, ElbowConfig::horizontal),
        }),
    Blueprint(
        "Byte-Adder (Separated)",
        // Nodes
        {
            // Bit 1
            NodeBP(false, Gate::OR,  IVec2(1 + 0,0) * g_gridSize),
            NodeBP(false, Gate::OR,  IVec2(1 + 0,1) * g_gridSize),
            NodeBP(true,  Gate::XOR, IVec2(2 + 3,0) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1 + 3,1) * g_gridSize),
            // Bit 2
            NodeBP(false, Gate::OR,  IVec2(1 + 0,2) * g_gridSize),
            NodeBP(false, Gate::OR,  IVec2(1 + 0,3) * g_gridSize),
            NodeBP(false, Gate::OR,  IVec2(1 + 2,2) * g_gridSize),
            NodeBP(false, Gate::XOR, IVec2(1 + 1,2) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1 + 1,3) * g_gridSize),
            NodeBP(true,  Gate::XOR, IVec2(2 + 3,1) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1 + 2,3) * g_gridSize),
            NodeBP(false, Gate::OR,  IVec2(1 + 3,3) * g_gridSize),
            // Bit 3
            NodeBP(false, Gate::OR,  IVec2(1 + 0,4) * g_gridSize),
            NodeBP(false, Gate::OR,  IVec2(1 + 0,5) * g_gridSize),
            NodeBP(false, Gate::OR,  IVec2(1 + 2,4) * g_gridSize),
            NodeBP(false, Gate::XOR, IVec2(1 + 1,4) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1 + 1,5) * g_gridSize),
            NodeBP(true,  Gate::XOR, IVec2(2 + 3,2) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1 + 2,5) * g_gridSize),
            NodeBP(false, Gate::OR,  IVec2(1 + 3,5) * g_gridSize),
            // Bit 4
            NodeBP(false, Gate::OR,  IVec2(1 + 0,6) * g_gridSize),
            NodeBP(false, Gate::OR,  IVec2(1 + 0,7) * g_gridSize),
            NodeBP(false, Gate::OR,  IVec2(1 + 2,6) * g_gridSize),
            NodeBP(false, Gate::XOR, IVec2(1 + 1,6) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1 + 1,7) * g_gridSize),
            NodeBP(true,  Gate::XOR, IVec2(2 + 3,3) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1 + 2,7) * g_gridSize),
            NodeBP(false, Gate::OR,  IVec2(1 + 3,7) * g_gridSize),
            // Bit 5
            NodeBP(false, Gate::OR,  IVec2(1 + 0,8) * g_gridSize),
            NodeBP(false, Gate::OR,  IVec2(1 + 0,9) * g_gridSize),
            NodeBP(false, Gate::OR,  IVec2(1 + 2,8) * g_gridSize),
            NodeBP(false, Gate::XOR, IVec2(1 + 1,8) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1 + 1,9) * g_gridSize),
            NodeBP(true,  Gate::XOR, IVec2(2 + 3,4) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1 + 2,9) * g_gridSize),
            NodeBP(false, Gate::OR,  IVec2(1 + 3,9) * g_gridSize),
            // Bit 6
            NodeBP(false, Gate::OR,  IVec2(1 + 0,10) * g_gridSize),
            NodeBP(false, Gate::OR,  IVec2(1 + 0,11) * g_gridSize),
            NodeBP(false, Gate::OR,  IVec2(1 + 2,10) * g_gridSize),
            NodeBP(false, Gate::XOR, IVec2(1 + 1,10) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1 + 1,11) * g_gridSize),
            NodeBP(true,  Gate::XOR, IVec2(2 + 3,5) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1 + 2,11) * g_gridSize),
            NodeBP(false, Gate::OR,  IVec2(1 + 3,11) * g_gridSize),
            // Bit 7
            NodeBP(false, Gate::OR,  IVec2(1 + 0,12) * g_gridSize),
            NodeBP(false, Gate::OR,  IVec2(1 + 0,13) * g_gridSize),
            NodeBP(false, Gate::OR,  IVec2(1 + 2,12) * g_gridSize),
            NodeBP(false, Gate::XOR, IVec2(1 + 1,12) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1 + 1,13) * g_gridSize),
            NodeBP(true,  Gate::XOR, IVec2(2 + 3,6) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1 + 2,13) * g_gridSize),
            NodeBP(false, Gate::OR,  IVec2(1 + 3,13) * g_gridSize),
            // Bit 8
            NodeBP(false, Gate::OR,  IVec2(1 + 0,14) * g_gridSize),
            NodeBP(false, Gate::OR,  IVec2(1 + 0,15) * g_gridSize),
            NodeBP(false, Gate::OR,  IVec2(1 + 2,14) * g_gridSize),
            NodeBP(false, Gate::XOR, IVec2(1 + 1,14) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1 + 1,15) * g_gridSize),
            NodeBP(true,  Gate::XOR, IVec2(2 + 3,7) * g_gridSize),
            NodeBP(false, Gate::AND, IVec2(1 + 2,15) * g_gridSize),
            NodeBP(true,  Gate::OR,  IVec2(1 + 3,15) * g_gridSize),
            // Inputs
            // A
            NodeBP(true, Gate::OR, IVec2(0,0) * g_gridSize), // 60
            NodeBP(true, Gate::OR, IVec2(0,1) * g_gridSize),
            NodeBP(true, Gate::OR, IVec2(0,2) * g_gridSize),
            NodeBP(true, Gate::OR, IVec2(0,3) * g_gridSize),
            NodeBP(true, Gate::OR, IVec2(0,4) * g_gridSize),
            NodeBP(true, Gate::OR, IVec2(0,5) * g_gridSize),
            NodeBP(true, Gate::OR, IVec2(0,6) * g_gridSize),
            NodeBP(true, Gate::OR, IVec2(0,7) * g_gridSize),
            // B
            NodeBP(true, Gate::OR, IVec2(0,8 + 0) * g_gridSize),
            NodeBP(true, Gate::OR, IVec2(0,8 + 1) * g_gridSize),
            NodeBP(true, Gate::OR, IVec2(0,8 + 2) * g_gridSize),
            NodeBP(true, Gate::OR, IVec2(0,8 + 3) * g_gridSize),
            NodeBP(true, Gate::OR, IVec2(0,8 + 4) * g_gridSize),
            NodeBP(true, Gate::OR, IVec2(0,8 + 5) * g_gridSize),
            NodeBP(true, Gate::OR, IVec2(0,8 + 6) * g_gridSize),
            NodeBP(true, Gate::OR, IVec2(0,8 + 7) * g_gridSize),
        },
        // Wires
        {
            // Bit 1
            WireBP(0, 2, ElbowConfig::horizontal),
            WireBP(1, 2, ElbowConfig::horizontal),
            WireBP(0, 3, ElbowConfig::vertical),
            WireBP(1, 3, ElbowConfig::vertical),

            WireBP(3, 4 + 2, ElbowConfig::horizontal),
            // Bit 2
            WireBP(4 + 0, 4 + 3, ElbowConfig::horizontal),
            WireBP(4 + 1, 4 + 3, ElbowConfig::horizontal),
            WireBP(4 + 0, 4 + 4, ElbowConfig::vertical),
            WireBP(4 + 1, 4 + 4, ElbowConfig::vertical),
            WireBP(4 + 2, 4 + 5, ElbowConfig::horizontal),
            WireBP(4 + 3, 4 + 5, ElbowConfig::horizontal),
            WireBP(4 + 2, 4 + 6, ElbowConfig::vertical),
            WireBP(4 + 3, 4 + 6, ElbowConfig::vertical),
            WireBP(4 + 6, 4 + 7, ElbowConfig::vertical),
            WireBP(4 + 4, 4 + 7, ElbowConfig::horizontal),

            WireBP(4 + 7, 12 + 2, ElbowConfig::horizontal),
            // Bit 3
            WireBP(12 + 0, 12 + 3, ElbowConfig::horizontal),
            WireBP(12 + 1, 12 + 3, ElbowConfig::horizontal),
            WireBP(12 + 0, 12 + 4, ElbowConfig::vertical),
            WireBP(12 + 1, 12 + 4, ElbowConfig::vertical),
            WireBP(12 + 2, 12 + 5, ElbowConfig::horizontal),
            WireBP(12 + 3, 12 + 5, ElbowConfig::horizontal),
            WireBP(12 + 2, 12 + 6, ElbowConfig::vertical),
            WireBP(12 + 3, 12 + 6, ElbowConfig::vertical),
            WireBP(12 + 6, 12 + 7, ElbowConfig::vertical),
            WireBP(12 + 4, 12 + 7, ElbowConfig::horizontal),

            WireBP(12 + 7, 20 + 2, ElbowConfig::horizontal),
            // Bit 4
            WireBP(20 + 0, 20 + 3, ElbowConfig::horizontal),
            WireBP(20 + 1, 20 + 3, ElbowConfig::horizontal),
            WireBP(20 + 0, 20 + 4, ElbowConfig::vertical),
            WireBP(20 + 1, 20 + 4, ElbowConfig::vertical),
            WireBP(20 + 2, 20 + 5, ElbowConfig::horizontal),
            WireBP(20 + 3, 20 + 5, ElbowConfig::horizontal),
            WireBP(20 + 2, 20 + 6, ElbowConfig::vertical),
            WireBP(20 + 3, 20 + 6, ElbowConfig::vertical),
            WireBP(20 + 6, 20 + 7, ElbowConfig::vertical),
            WireBP(20 + 4, 20 + 7, ElbowConfig::horizontal),

            WireBP(20 + 7, 28 + 2, ElbowConfig::horizontal),
            // Bit 5
            WireBP(28 + 0, 28 + 3, ElbowConfig::horizontal),
            WireBP(28 + 1, 28 + 3, ElbowConfig::horizontal),
            WireBP(28 + 0, 28 + 4, ElbowConfig::vertical),
            WireBP(28 + 1, 28 + 4, ElbowConfig::vertical),
            WireBP(28 + 2, 28 + 5, ElbowConfig::horizontal),
            WireBP(28 + 3, 28 + 5, ElbowConfig::horizontal),
            WireBP(28 + 2, 28 + 6, ElbowConfig::vertical),
            WireBP(28 + 3, 28 + 6, ElbowConfig::vertical),
            WireBP(28 + 6, 28 + 7, ElbowConfig::vertical),
            WireBP(28 + 4, 28 + 7, ElbowConfig::horizontal),

            WireBP(28 + 7, 36 + 2, ElbowConfig::horizontal),
            // Bit 6
            WireBP(36 + 0, 36 + 3, ElbowConfig::horizontal),
            WireBP(36 + 1, 36 + 3, ElbowConfig::horizontal),
            WireBP(36 + 0, 36 + 4, ElbowConfig::vertical),
            WireBP(36 + 1, 36 + 4, ElbowConfig::vertical),
            WireBP(36 + 2, 36 + 5, ElbowConfig::horizontal),
            WireBP(36 + 3, 36 + 5, ElbowConfig::horizontal),
            WireBP(36 + 2, 36 + 6, ElbowConfig::vertical),
            WireBP(36 + 3, 36 + 6, ElbowConfig::vertical),
            WireBP(36 + 6, 36 + 7, ElbowConfig::vertical),
            WireBP(36 + 4, 36 + 7, ElbowConfig::horizontal),

            WireBP(36 + 7, 44 + 2, ElbowConfig::horizontal),
            // Bit 7
            WireBP(44 + 0, 44 + 3, ElbowConfig::horizontal),
            WireBP(44 + 1, 44 + 3, ElbowConfig::horizontal),
            WireBP(44 + 0, 44 + 4, ElbowConfig::vertical),
            WireBP(44 + 1, 44 + 4, ElbowConfig::vertical),
            WireBP(44 + 2, 44 + 5, ElbowConfig::horizontal),
            WireBP(44 + 3, 44 + 5, ElbowConfig::horizontal),
            WireBP(44 + 2, 44 + 6, ElbowConfig::vertical),
            WireBP(44 + 3, 44 + 6, ElbowConfig::vertical),
            WireBP(44 + 6, 44 + 7, ElbowConfig::vertical),
            WireBP(44 + 4, 44 + 7, ElbowConfig::horizontal),

            WireBP(44 + 7, 52 + 2, ElbowConfig::horizontal),
            // Bit 8
            WireBP(52 + 0, 52 + 3, ElbowConfig::horizontal),
            WireBP(52 + 1, 52 + 3, ElbowConfig::horizontal),
            WireBP(52 + 0, 52 + 4, ElbowConfig::vertical),
            WireBP(52 + 1, 52 + 4, ElbowConfig::vertical),
            WireBP(52 + 2, 52 + 5, ElbowConfig::horizontal),
            WireBP(52 + 3, 52 + 5, ElbowConfig::horizontal),
            WireBP(52 + 2, 52 + 6, ElbowConfig::vertical),
            WireBP(52 + 3, 52 + 6, ElbowConfig::vertical),
            WireBP(52 + 6, 52 + 7, ElbowConfig::vertical),
            WireBP(52 + 4, 52 + 7, ElbowConfig::horizontal),

            // Inputs
            // A
            WireBP(60,  0, ElbowConfig::vertical),
            WireBP(61,  4, ElbowConfig::vertical),
            WireBP(62, 12, ElbowConfig::vertical),
            WireBP(63, 20, ElbowConfig::vertical),
            WireBP(64, 28, ElbowConfig::vertical),
            WireBP(65, 36, ElbowConfig::vertical),
            WireBP(66, 44, ElbowConfig::vertical),
            WireBP(67, 52, ElbowConfig::vertical),
            // B
            WireBP(68,  1, ElbowConfig::vertical),
            WireBP(69,  5, ElbowConfig::vertical),
            WireBP(70, 13, ElbowConfig::vertical),
            WireBP(71, 21, ElbowConfig::vertical),
            WireBP(72, 29, ElbowConfig::vertical),
            WireBP(73, 37, ElbowConfig::vertical),
            WireBP(74, 45, ElbowConfig::vertical),
            WireBP(75, 53, ElbowConfig::vertical),
        }),
};
