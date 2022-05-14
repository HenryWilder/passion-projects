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
        Element(unsigned value) : isParam(false), value(value) {}
        Element(const Token_t& param) : isParam(true), param(param) {}

        bool isParam;
        union
        {
            unsigned value;
            Token_t param;
        };

        unsigned Value(const TokenValueMap_t& src) const
        {
            if (!isParam)
                return value;

            if (auto it = src.find(param); it != src.end())
                return it->second;

            return error_value;
        }
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
        case Operation::SUB: return ((lval > rval) ? (lval - rval) : skip_value);
        case Operation::MUL: return lval * rval;
        case Operation::DIV: return ((rval != 0) ? (lval / rval) : error_value);
        case Operation::MOD: return ((rval != 0) ? (lval % rval) : error_value);
        default: return 0;
        }
    }
    std::vector<Element> elements;
    std::vector<Operation> operations; // Should be the size of elements - 1

    // Must be unpackaged from parentheses
    // Returns 0 on no errors
    unsigned ParseStrToExpression(Expression& expr, const std::string& str)
    {
        size_t pos = 0;
        bool op = false;
        std::string::const_iterator last = str.begin();
        while ((pos = str.find(' ')) != str.npos)
        {
            std::string_view token(last, str.begin() + pos);
            if (op)
            {
                if (token.size() > 1)
                    return error_value;

                expr.operations.push_back((Operation)token.back());
            }
            else
            {
                if (token.front() >= '0' && token.front() <= '9') // Expect a number
                {
                    for (const char c : token)
                    {
                        if (c < '0' || c > '9')
                            return error_value;
                    }
                    expr.elements.emplace_back((unsigned)std::stoul(token.data()));
                }
                expr.elements.emplace_back(token.data());
            }
        }

        return 0;
    }

    inline unsigned Solve(const TokenValueMap_t& values)
    {
        if (operations.size() >= elements.size())
            return error_value;

        unsigned current = elements[0].Value(values);

        for (size_t i = 0; i < operations.size(); ++i)
        {
            unsigned next = elements[i + 1].Value(values);
            current = Operate(operations[i], current, next);
        }
    }
};

struct Blueprint
{
    std::string name;
    std::string desc;
    TokenNameSet_t parameters;
    void Instantiate(_Out_ BlueprintInstance& dest, const TokenValueMap_t& values)
    {

    }
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
