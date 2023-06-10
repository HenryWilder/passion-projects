#pragma once
#include <string>
#include <vector>

struct Style
{
	std::string backgroundColor = "transparent";
	std::string color = "white";
};

struct ConsoleLine
{
	ConsoleLine(const std::string& text) : style{}, text(text) {}
	ConsoleLine(const Style style, const std::string& text) : style(style), text(text) {}

	Style style;
	std::string text;
};

// Storage for console stream
class ConsoleController
{
private:
	std::vector<ConsoleLine*> lines;

public:
	void Push(const std::string& text);
	void Push(const Style style, const std::string& text);
	void Clear();
};

// Interaction with the console
class Console
{
private:
	static constexpr size_t indentSize = 2;
	size_t indent = 0;
	ConsoleController* consoleLink = nullptr;

	// Indents the provided string
	void IndentText(std::string& text);

public:
	void Log(std::string str);
	void Warn(std::string str);
	void Error(std::string str);
	void Group(std::string str);
	void GroupEnd();
	void Clear();
};

extern Console console;
