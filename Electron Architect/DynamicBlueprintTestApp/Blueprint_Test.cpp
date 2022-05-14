#include <iostream>
#include <thread>
#include <fstream>
#include <stack>
#include "Blueprint_Test.h"

Blueprint* LoadStaticBlueprint(std::ifstream& filename, const std::string& name)
{
    return nullptr;
}

Blueprint* LoadDynamicBlueprint(std::ifstream& file, const std::string& name)
{
    Blueprint output;
    output.name = name;
    enum class Context
    {
        global,
        desc,
        scoped,
    } c = Context::global;
    std::stack<BlueprintScope*> scope;
    std::string line;
    while (std::getline(file, line))
    {
        if (size_t trimStart = line.find_first_not_of("\t "); trimStart != std::string::npos)
            line = line.substr(trimStart); // Ignore whitespace

        std::cout << line << '\n';

        if (line == "desc {")
        {
            if (output.desc.empty() && std::getline(file, line, '}'))
            {
                output.desc = line;
                output.desc.replace(outpu, "");
                std::cout << "DESCRIPTION: \"" << output.desc << "\"\n";
            }
            else
                return nullptr;
        }
    }
    return new Blueprint(output);
}

Blueprint* LoadBlueprint(const std::string& name)
{
    std::ifstream file(name + ".bp");
    std::cout << "Opened!";
    file.ignore(1, 'v');
    double version;
    char type;
    file >> version >> type;
    if (version != 1.4)
        return nullptr;
    switch (type)
    {
    case 's': return LoadStaticBlueprint(file, name); // Static
    case 'd': return LoadDynamicBlueprint(file, name); // Dynamic
    default: return nullptr; // Incompatible
    }
}


unsigned Expression::Element::Value(const std::unordered_map<Token_t, unsigned>& src) const
{
    if (!isParam)
        return value;

    if (auto it = src.find(param); it != src.end())
        return it->second;

    return error_value;
}

unsigned Expression::Operate(Operation op, unsigned lval, unsigned rval)
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

int Expression::ParseStrToExpression(Expression& expr, const std::string& str)
{
    // No spaces (just a value)
    if (str.find(' ') != str.npos)
    {
        for (const char c : str)
        {
            if (c < '0' || c > '9')
                return 1;
        }
        expr.elements.emplace_back((unsigned)std::stoul(str));
        return 0;
    }

    size_t pos = 0;
    bool op = false;
    std::string::const_iterator last = str.begin();
    while ((pos = str.find(' ')) != str.npos)
    {
        std::string_view token(last, str.begin() + pos);
        if (op)
        {
            if (token.size() > 1)
                return 1;

            expr.operations.push_back((Operation)token.back());
        }
        else
        {
            if (token.front() >= '0' && token.front() <= '9') // Expect a number
            {
                for (const char c : token)
                {
                    if (c < '0' || c > '9')
                        return 1;
                }
                expr.elements.emplace_back((unsigned)std::stoul(token.data()));
            }
            expr.elements.emplace_back(token.data());
        }
        op = !op;
    }

    return 0;
}

unsigned Expression::Solve(const std::unordered_map<Token_t, unsigned>& values)
{
    if (operations.size() >= elements.size())
        return error_value;

    unsigned current = elements[0].Value(values);

    for (size_t i = 0; i < operations.size(); ++i)
    {
        unsigned next = elements[i + 1].Value(values);
        current = Operate(operations[i], current, next);
    }

    return current;
}

bool BlueprintInstance::ShouldDelete() const
{
    return this != base->cached;
}

BlueprintInstance* Blueprint::Instantiate(const std::unordered_map<Token_t, unsigned>& values) const
{
    // Same as cached?
    if (values == parameters)
    {
        return cached;
    }
    std::unordered_map<AnchorTag_t, std::vector<NodeBP>*> anchors;
    return nullptr;
}
