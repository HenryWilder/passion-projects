#pragma once
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include "IVec_Test.h"

enum class Gate : char
{
    OR = '|',
    AND = '&',
    NOR = '!',
    XOR = '^',

    RESISTOR = '~',
    CAPACITOR = '=',
    LED = '@',
    DELAY = ';',

    BATTERY = '#',
};

enum class ElbowConfig : uint8_t
{
    horizontal,
    diagonalA,
    vertical,
    diagonalB,
};

struct NodeBP
{
    bool b_io; // Whether the node is an input/output to the entire system (Should it be shown on the paste preview?)
    Gate gate;
    uint8_t extraParam;
    IVec2 relativePosition;
    std::string name;
};

struct WireBP
{
    size_t startNodeIndex, endNodeIndex;
    ElbowConfig elbowConfig;
};

using Token_t = std::string;

struct Expression
{
    static constexpr unsigned error_value = UINT_MAX;
    static constexpr unsigned skip_value = UINT_MAX - 1;
    struct Element
    {
        inline Element(unsigned value) : isParam(false), value(value) {}
        inline Element(const Token_t& param) : isParam(true), param(param) {}
        inline Element(const Element& base) : isParam(base.isParam)
        {
            if (isParam)
                param = base.param;
            else
                value = base.value;
        }
        ~Element() {}

        bool isParam;
        union
        {
            unsigned value;
            Token_t param;
        };

        unsigned Value(const std::unordered_map<Token_t, unsigned>& src) const;
    };
    enum class Operation : char
    {
        ADD = '+',
        SUB = '-',
        MUL = '*',
        DIV = '/',
        MOD = '%',
    };
    unsigned Operate(Operation op, unsigned lval, unsigned rval);
    std::vector<Element> elements;
    std::vector<Operation> operations; // Should be the size of elements - 1

    // Must be unpackaged from parentheses
    // Returns 0 on no errors
    int ParseStrToExpression(Expression& expr, const std::string& str);

    unsigned Solve(const std::unordered_map<Token_t, unsigned>& values);
};

using AnchorTag_t = std::string;

struct NodeBP_Dynamic
{
    AnchorTag_t anchor;
    std::string name;
    Expression x;
    Expression y;
    bool b_io;
};

struct WireBP_Dynamic
{
    ElbowConfig elbow;
    AnchorTag_t tags[2];
    Expression offsets[2];
};

struct BPLocal
{
    Token_t param;
};

struct BlueprintScope
{
    std::unordered_map<Token_t, BPLocal> locals;
    NodeBP_Dynamic nodes;
    WireBP_Dynamic wires;
    std::vector<BlueprintScope*> nests;
};

struct Blueprint;
struct BlueprintInstance
{
    Blueprint* base;
    IVec2 extents;
    std::vector<NodeBP> nodes;
    std::vector<WireBP> wires;

    bool ShouldDelete() const;
};

struct Blueprint
{
    std::string name;
    std::string desc;
    std::unordered_map<Token_t, unsigned> parameters;
    BlueprintScope top;
    BlueprintInstance* cached; // "Default" instance
    BlueprintInstance* Instantiate(const std::unordered_map<Token_t, unsigned>& values) const;
};

// A static blueprint is just a blueprint with no parameters
Blueprint* LoadStaticBlueprint(std::ifstream& filename, const std::string& name);
Blueprint* LoadDynamicBlueprint(std::ifstream& filename, const std::string& name);
Blueprint* LoadBlueprint(const std::string& filename);
