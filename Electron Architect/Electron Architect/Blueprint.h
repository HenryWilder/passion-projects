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

struct Expression
{
    struct Element
    {
        bool isParam;
        union
        {
            unsigned value;
            std::string paramName;
        };
    };
    enum class Operation : char
    {
        ADD = '+',
        SUB = '-',
        MUL = '*',
        DIV = '/',
        MOD = '%',
    };
    inline unsigned Operate(Operation op, unsigned lval, unsigned rval)
    {
        switch (op)
        {
        case Operation::ADD: return lval + rval;
        case Operation::SUB: return lval - rval;
        case Operation::MUL: return lval * rval;
        case Operation::DIV: return ((rval != 0) ? (lval / rval) : 0);
        case Operation::MOD: return ((rval != 0) ? (lval % rval) : 0);
        default: return 0;
        }
    }
    std::vector<Element> elements;
    std::vector<Operation> operations; // Should be the size of elements - 1
};

struct Blueprint
{
    std::string name;
    std::unordered_set<std::string> parameters;
    void Instantiate(_Out_ BlueprintInstance& dest, std::unordered_map<std::string, unsigned> values);
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

Blueprint* LoadBlueprint(const char* filename);
