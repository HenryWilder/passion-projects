#include <iostream>
#include <thread>
#include <fstream>
#include <stack>
#include "Blueprint_Test.h"

Blueprint* LoadStaticBlueprint(std::ifstream& filename, const std::string& name)
{
    return nullptr;
}

struct TokenType
{
    enum class Tag : char
    {
        reserved,
        string,
        number,
    } tag;
    std::string expect; // Only if reserved
};

void Split(std::vector<std::string_view>& tokens, const std::string& line, const std::string& delimiter = " ")
{
    tokens.clear();
    if (line.empty())
        return;
    size_t tokenCount = std::count(line.begin(), line.end(), delimiter);
    std::cout << tokenCount;
    tokens.reserve(tokens.size() + tokenCount);

    size_t pos = 0;
    if (line.find(delimiter) == line.npos)
    {
        tokens.push_back({ line.begin(), line.end() });
    }
    else
    {
        while ((pos = line.find(delimiter, pos + 1)) != line.npos)
        {

        }
    }
}

// Return 0 on success
int Parse(const std::vector<std::string_view>& tokens, std::vector<TokenType> syntax)
{
    if (tokens.size() != syntax.size())
        return 1;


    return 0;
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
        if (size_t trimStart = line.find_first_not_of("\t "); trimStart != line.npos)
            line = line.substr(trimStart); // Ignore whitespace

        if (line == "desc {")
        {
            // Too many descriptions
            if (!output.desc.empty())
            {
                std::cout << "Blueprint cannot contain multiple descriptions.";
                return nullptr;
            }

            while (std::getline(file, line, '\n'))
            {
                if (!line.empty() && line.front() == '}')
                {
                    output.desc.pop_back(); // Remove excess newline
                    break;
                }
                size_t start = line.find_first_not_of("\t ");
                size_t end = line.find_last_not_of("\t ") + 1;
                output.desc += line.substr(start, end - start) + '\n';
            }
            std::cout << "DESCRIPTION:\n\"" << output.desc << "\"\n";
        }
        else if (!line.empty())
        {
            size_t firstSpace = line.find(' ');
            if (firstSpace != line.npos && (firstSpace + 1) < line.size() && line[firstSpace + 1] == ':')
            {
                size_t startOfParam = line.find(' ', firstSpace + 1) + 1;
                size_t endOfParam = line.find(' ', startOfParam);
                std::cout <<
                    "Local: \"" << line.substr(0, firstSpace) <<
                    "\" in range of \"" << line.substr(startOfParam, endOfParam - startOfParam) << "\"\n";

                if (line.size() <= (endOfParam + 1) || line[endOfParam + 1] != '{')
                {
                    std::cout << "Expected \'{\'";
                    if (line.size() > (endOfParam + 1))
                        std::cout << "Read: \"" << line.substr(endOfParam + 1) << "\"";
                    return nullptr;
                }
            }
            else if (firstSpace == 1)
            {
                std::cout << "TOKEN: " << line.front() << "; ";
                switch (line.front())
                {
                case 'p': std::cout << "Parameter\n"; break;
                case 'n': std::cout << "Node\n"; break;
                case 'w': std::cout << "Wire\n"; break;
                }
            }
            else
            {
                std::cout << "Error";
                return nullptr;
            }
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
