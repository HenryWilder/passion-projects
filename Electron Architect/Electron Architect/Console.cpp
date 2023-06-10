#include "Console.h"

const Style WARNING_STYLE = { .backgroundColor = "", .color = "" };
const Style ERROR_STYLE = { .backgroundColor = "", .color = "" };

void Console::IndentText(std::string& text)
{
	std::string indentation(indent * indentSize, ' ');

	size_t newlinesCount = 1; // Start is also counted as a newline because it gets indentation.
	for (char ch : text)
	{
		newlinesCount += (size_t)(ch == '\n');
	}

	text.reserve(text.size() + (newlinesCount * indentSize));

	text.insert(0, indentation);
	size_t current = 0;
	while ((current = text.find('\n', current)) != std::string::npos)
	{
		text.insert(++current, indentation);
	}
}

void Console::Log(std::string str)
{
	IndentText(str);
	consoleLink->Push(str);
}

void Console::Warn(std::string str)
{
	IndentText(str);
	consoleLink->Push(WARNING_STYLE, str);
}

void Console::Error(std::string str)
{
	IndentText(str);
	consoleLink->Push(ERROR_STYLE, str);
}

void Console::Group(std::string str)
{
	IndentText(str);
	consoleLink->Push(str);
	++indent;
}

void Console::GroupEnd()
{
	--indent;
}

void Console::Clear()
{
	consoleLink->Clear();
}

Console console;

void ConsoleController::Push(const std::string& text)
{
	this->lines.push_back(new ConsoleLine(text));
}

void ConsoleController::Push(const Style style, const std::string& text)
{
	this->lines.push_back(new ConsoleLine(style, text));
}

void ConsoleController::Clear()
{
	lines.clear();
}
