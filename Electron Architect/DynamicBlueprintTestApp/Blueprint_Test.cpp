#include <iostream>
#include <thread>
#include <fstream>
#include <stack>
#include <queue>
#include "Blueprint_Test.h"

Blueprint* LoadStaticBlueprint(std::ifstream& filename, const std::string& name)
{
    return nullptr;
}

struct Token
{
    enum class Type
    {
        keyword = 'a',
        punctuation = ',',
        identifier = '@',
        string = '\"',
        number = '0',
    };

    enum class Keyword : char
    {
        description = 'd',
        parameter = 'p',
        node = 'n',
        wire = 'w',
    };

    enum class Punctuation : char
    {
        open_scope = '{',
        close_scope = '}',

        open_expression = '(',
        close_expression = ')',

        operator_range = ':',
        operator_assign = '=',
        operator_add = '+',
        operator_sub = '-',
        operator_mul = '*',
        operator_div = '/',
        operator_mod = '%',
    };

    Type type;
    Keyword kw;
    Punctuation punc;
    unsigned num;
    std::string id_str;

    Token(const std::string& token)
    {
        _ASSERT_EXPR(!token.empty() && token[0] != '\0', L"Token cannot be null");

        kw = Keyword(0);
        punc = Punctuation(0);
        id_str = "";
        num = 0;

        if (token.find(' ') != token.npos)
        {
            type = Type::string;
            id_str = token;
            return;
        }

        char t = token.front();
        if (token.size() == 1)
        {
            if (t == 'd' || t == 'p' || t == 'n' || t == 'w')
            {
                type = Type::keyword;
                kw = (Keyword)t;
                return;
            }
            else if (t == '{' || t == '}' || t == '(' || t == ')' || t == ':' || t == '=' || t == '+' || t == '-' || t == '*' || t == '/' || t == '%')
            {
                type = Type::punctuation;
                punc = (Punctuation)t;
                return;
            }
        }

        if (t >= '0' && t <= '9')
        {
            type = Type::number;
            for (char c : token)
            {
                if (!(c >= '0' && c <= '9'))
                {
                    type = Type::string;
                    id_str = token;
                    return;
                }
            }
            num = (unsigned)std::stoul(token);
            return;
        }
        else if ((t >= 'a' && t <= 'z') || (t >= 'A' && t <= 'Z') || t == '_')
        {
            type = Type::identifier;
            id_str = token;
            for (char c : token)
            {
                if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_'))
                {
                    type = Type::string;
                    return;
                }
            }
        }
        else
        {
            type = Type::string;
            id_str = token;
        }
    }
    Token(const Token& base) : type(base.type), kw(base.kw), punc(base.punc), num(base.num), id_str(base.id_str) {}
    ~Token() = default;
};

std::queue<std::vector<Token>*> Tokenize(std::ifstream& file)
{
    std::queue<std::vector<Token>*> tokens;

    auto TokenTypeName = [](const Token::Type& what)
    {
        switch (what)
        {
        case Token::Type::keyword: return "Keyword";
        case Token::Type::punctuation: return "Punctuation";
        case Token::Type::identifier: return "Identifier";
        case Token::Type::string: return "String";
        case Token::Type::number: return "Number";
        default: return "ERROR";
        }
    };

    unsigned lineNumber = 1;
    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty())
            continue;

        if (size_t trimStart = line.find_first_not_of("\t "); trimStart != line.npos)
            line = line.substr(trimStart); // Ignore whitespace

        tokens.push(new std::vector<Token>);
        switch (line.front()) // Reserve fewer for tokens with many spaced strings
        {
        case 'd': tokens.back()->reserve(2); break;
        case 'n': tokens.back()->reserve(7); break;
        default: tokens.back()->reserve(std::count(line.begin(), line.end(), ' ')); break;
        }

        std::cout << "Line " << lineNumber << ": \'" << line << "\'\n";

        std::string tokenStr;
        size_t pos = 0;
        while ((pos = line.find(' ')) != std::string::npos)
        {
            tokenStr = line.substr(0, pos);
            line = line.substr(pos + 1);
            tokens.back()->push_back(tokenStr);
            std::cout << "  " << TokenTypeName(tokens.back()->back().type) << ": " << tokenStr << '\n';
            if (tokens.back()->back().type == Token::Type::keyword && tokens.back()->back().kw == Token::Keyword::description)
            {
                tokens.back()->push_back(line);
                std::cout << "  " << TokenTypeName(tokens.back()->back().type) << ": " << line << '\n';
                line.clear();
            }
        }
        if (!line.empty())
        {
            tokens.back()->push_back(line);
            std::cout << "  " << TokenTypeName(tokens.back()->back().type) << ": " << line << '\n';
        }
        ++lineNumber;
    }
    return tokens;
}

struct Error
{
    Error(unsigned lineNumber = 0) : lineNumber(lineNumber) {}
    unsigned lineNumber;
    std::string what() const
    {
        return msg() + "\tLine " + std::to_string(lineNumber);
    }
    virtual std::string msg() const
    {
        return "Unknown error.";
    }
};
struct ParseError : public Error
{
    ParseError(unsigned lineNumber = 0) : Error(lineNumber) {}
    std::string msg() const override
    {
        return "Parse error.";
    }
};
struct MissingExpected : public ParseError
{
    MissingExpected(const std::string& expected, unsigned lineNumber = 0) : ParseError(lineNumber), expected(expected) {}
    std::string expected;
    std::string msg() const override
    {
        return ParseError::msg() + " Expected " + expected;
    }
};
struct Unexpected : public MissingExpected
{
    Unexpected(const std::string& expected, const std::string& found, unsigned lineNumber = 0) : MissingExpected(expected, lineNumber), found(found) {}
    std::string found;
    std::string msg() const override
    {
        return ParseError::msg() + " Expected " + expected + " found " + found;
    }
};

std::vector<Error*> Parse(std::queue<std::vector<Token>*>& tokens)
{
    std::vector<Error*> errors;
    unsigned lineNumber = 1;
    while (!tokens.empty())
    {
        {
            _ASSERT_EXPR(!tokens.front()->empty(), L"Line cannot be empty");
            const std::vector<Token>& line = *tokens.front();
            if (line[0].type == Token::Type::keyword)
            {
                // d <string>
                if (line[0].kw == Token::Keyword::description)
                {
                    if (line.size() == 2)
                    {
                        // Todo
                    }
                    // d
                    else if (line.size() < 2)
                    {
                        errors.push_back(new MissingExpected("a string", lineNumber));
                    }
                    // d <string> ...
                    else if (line.size() > 2)
                    {
                        errors.push_back(new MissingExpected("only one argument", lineNumber));
                    }
                }
                // p <identifier>
                // p <identifier> = <number>
                else if (line[0].kw == Token::Keyword::parameter)
                {
                    if (line[1].type == Token::Type::identifier)
                    {
                        if (line.size() == 2)
                        {
                            // Todo
                        }
                        else if (line.size() == 4)
                        {
                            // Todo
                        }
                        else
                        {
                            errors.push_back(new Unexpected(
                                "either 2 (<indentifier> <identifier>) or 4 (<indentifier> <identifier> = <number>) arguments",
                                std::to_string(line.size()) + " arguments", lineNumber));
                        }
                    }
                    else if (line[1].type != Token::Type::identifier)
                        errors.push_back(new MissingExpected("idententifier", lineNumber));
                }
                // n <identifier> <string> <<number>|<string>> <<number>|<string>>
                // n <identifier> <string> <<number>|<string>> <<number>|<string>> <number>
                // n <identifier> <string> <<number>|<string>> <<number>|<string>> <string>
                // n <identifier> <string> <<number>|<string>> <<number>|<string>> <number> <string>
                else if (line[0].kw == Token::Keyword::node)
                {
                    // Todo
                }
                // w <string> <identifier> <identifier>
                // w <string> <identifier>[<string>] <identifier>
                // w <string> <identifier> <identifier>[<string>]
                // w <string> <identifier>[<string>] <identifier>[<string>]
                else if (line[0].kw == Token::Keyword::node)
                {
                    // Todo
                }
            }
            // <identifier> : <identifier> {
            else if (line[0].type == Token::Type::identifier)
            {
                // Todo
            }
            // error
            else
            {
                errors.push_back(new MissingExpected("a keyword (d, n, p, w)", lineNumber));
            }
        }
        delete tokens.front();
        tokens.pop();
        ++lineNumber;
    }
    return errors;
}

Blueprint* LoadDynamicBlueprint(std::ifstream& file, const std::string& name)
{
    std::cout << "Loading dynamic blueprint\n";
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
    auto tokens = Tokenize(file);
    auto errors = Parse(tokens);
    if (errors.empty())
        std::cout << "\nSuccess!!";
    else
    {
        std::cout << "\nEncountered " + std::to_string(errors.size()) + " errors:\n";
        for (Error* err : errors)
        {
            std::cout << err->what();
        }
    }
    std::cout << '\n';
    return nullptr;

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


unsigned Expression::Element::Value(const std::unordered_map<std::string, unsigned>& src) const
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

unsigned Expression::Solve(const std::unordered_map<std::string, unsigned>& values)
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

BlueprintInstance* Blueprint::Instantiate(const std::unordered_map<std::string, unsigned>& values) const
{
    // Same as cached?
    if (values == parameters)
    {
        return cached;
    }
    std::unordered_map<std::string, std::vector<NodeBP>*> anchors;
    return nullptr;
}
