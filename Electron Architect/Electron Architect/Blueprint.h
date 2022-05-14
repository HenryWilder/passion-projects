#pragma once
#ifndef RAYLIB_H
#include <raylib.h>
#endif
#include "HUtility.h"
#include "IVec.h"
#include "Node.h"
#include "Wire.h"

struct NodeBP
{
    NodeBP() : b_io(), gate(), extraParam(), relativePosition(), name("") {}
    constexpr NodeBP(bool b_io, Gate gate, IVec2 relativePosition) :
        b_io(b_io), gate(gate), extraParam(0), relativePosition(relativePosition), name() {}
    constexpr NodeBP(bool b_io, Gate gate, uint8_t extraParam, IVec2 relativePosition) :
        b_io(b_io), gate(gate), extraParam(extraParam), relativePosition(relativePosition), name() {}
    constexpr NodeBP(const std::string& name, bool b_io, Gate gate, IVec2 relativePosition) :
        b_io(b_io), gate(gate), extraParam(0), relativePosition(relativePosition), name(name) {}
    constexpr NodeBP(const std::string& name, bool b_io, Gate gate, uint8_t extraParam, IVec2 relativePosition) :
        b_io(b_io), gate(gate), extraParam(extraParam), relativePosition(relativePosition), name(name) {}

    bool b_io; // Whether the node is an input/output to the entire system (Should it be shown on the paste preview?)
    Gate gate;
    uint8_t extraParam;
    IVec2 relativePosition;
    std::string name;
};

struct WireBP
{
    WireBP() = default;
    constexpr WireBP(size_t startNodeIndex, size_t endNodeIndex, ElbowConfig elbowConfig) :
        startNodeIndex(startNodeIndex), endNodeIndex(endNodeIndex), elbowConfig(elbowConfig) {}

    size_t startNodeIndex, endNodeIndex;
    ElbowConfig elbowConfig;
};

using Token_t = std::string;
using TokenNameSet_t = std::unordered_set<Token_t>;
using TokenValueMap_t = std::unordered_map<Token_t, unsigned>;

struct Expression
{
    static constexpr unsigned error_value = UINT_MAX;
    static constexpr unsigned skip_value = UINT_MAX - 1;
    struct Element
    {
        inline Element(unsigned value) : isParam(false), value(value) {}
        inline Element(const Token_t& param) : isParam(true), param(param) {}

        bool isParam;
        union
        {
            unsigned value;
            Token_t param;
        };

        unsigned Value(const TokenValueMap_t& src) const;
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

    inline unsigned Solve(const TokenValueMap_t& values);
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

struct Blueprint
{
    std::string name;
    std::string desc;
    TokenNameSet_t parameters;
    BlueprintScope top;
    void Instantiate(_Out_ BlueprintInstance& dest, const TokenValueMap_t& values);
}; 

struct BlueprintInstance
{
private: // Multithread functions
    void PopulateNodes(const std::vector<Node*>& src);
    void PopulateWires(const std::vector<Node*>& src);

public:
    IVec2 extents;
    std::vector<NodeBP> nodes;
    std::vector<WireBP> wires;

    void DrawSelectionPreview(IVec2 pos, Color backgroundColor, Color nodeColor, Color ioNodeColor, Color wireColor, uint8_t lod) const;
    IRect GetSelectionPreviewRect(IVec2 pos) const;

    void Save() const;
};

void CreateBlueprint(const std::vector<Node*>& src);

Blueprint* LoadStaticBlueprint(std::ifstream& filename, const std::string& name);
Blueprint* LoadDynamicBlueprint(std::ifstream& filename, const std::string& name);
Blueprint* LoadBlueprint(const std::string& filename);
