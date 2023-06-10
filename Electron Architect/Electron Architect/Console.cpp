#include "Console.h"

const Style WARNING_STYLE = { .backgroundColor = "", .color = "" };
const Style ERROR_STYLE = { .backgroundColor = "", .color = "" };

/**
 * indent = 1
 * indentSize = 2
 * [Apples\nOranges\nBananas\nMangos]
 * [Apples\nOranges\nBananas\nMangos------->] // Resize
 * [Apples\nOranges\nBananas\n------->Mangos]
 * [Apples\nOranges\n----->Bananas\n  Mangos]
 * [Apples\n--->Oranges\n  Bananas\n  Mangos]
 * [->Apples\n  Oranges\n  Bananas\n  Mangos]
 * [  Apples\n  Oranges\n  Bananas\n  Mangos]
 * 
 * To clarify:
 * 
 * [Apples\nOranges\nBananas\n------->Mangos]:
 * 
 * offset = 8 (number of indents * size of indent)
 *                                 .-------v
 * [Apples\nOranges\nBananas\nMangos        ]
 *                                .-------v
 * [Apples\nOranges\nBananas\nMango        s]
 *                               .-------v
 * [Apples\nOranges\nBananas\nMang        os]
 * [Apples\nOranges\nBananas\nMan        gos]
 * [Apples\nOranges\nBananas\nMa        ngos]
 * [Apples\nOranges\nBananas\nM        angos]
 * [Apples\nOranges\nBananas\n        Mangos]
 * 
 * [Apples\nOranges\n----->Bananas\n  Mangos]:
 * 
 * offset = 6 (number of indents remaining * size of indent)
 *                          .-----v
 * [Apples\nOranges\nBananas\n        Mangos]
 *                         .-----v
 * [Apples\nOranges\nBananas      \n  Mangos]
 *                        .-----v
 * [Apples\nOranges\nBanana      s\n  Mangos]
 * [Apples\nOranges\nBanan      as\n  Mangos]
 * [Apples\nOranges\nBana      nas\n  Mangos]
 * [Apples\nOranges\nBan      anas\n  Mangos]
 * [Apples\nOranges\nBa      nanas\n  Mangos]
 * [Apples\nOranges\nB      ananas\n  Mangos]
 * [Apples\nOranges\n      Bananas\n  Mangos]
 */
void Console::IndentText(std::string& text)
{
    // Number of total additional chars per newline when indenting
    const size_t indentChars = indent * indentSize;

    size_t newlinesCount = 1; // Start is also counted as a newline because it gets indentation.
    for (char ch : text)
    {
        newlinesCount += (size_t)(ch == '\n');
    }

    size_t offset = newlinesCount * indentChars;

    /*
     * Here we are resizing instead of reserving because we want to assign chars at specific indices before visiting the indices between.
     * 
     * i.e.
     *   '.' is `0 <= i < text.size()`, and therefore safely accessible.
     *   '*' is `text.size() <= i < text.capacity()`, and therefore will cause std::string's subscript operator to throw an out of bounds error.
     * 
     *   If we reserved:
     *            ................................********
     *     text = Apples\nOranges\nBananas\nMangos%^$*(#@(
     * 
     *     `text[36] = text[28]` errors because text.size() is only 29, even though we have reserved 37 elements.
     *   
     *   Because we resize:
     *            ........................................
     *     text = Apples\nOranges\nBananas\nMangos&%(#@#@(
     * 
     *     `text[36] = text[28]` does not error because text.size() is 37.
     * 
     * Note:
     *   Depending on "resize" implementation, the garble may be either terminating chars or uninitialized mem.
     *   This function is unaffected by this, as it overwrites the resized mem without reading it either way.
     */

    text.resize(text.size() + offset);

    for (size_t n = text.size(); n > offset; --n)
    {
        size_t i = n - 1;
        size_t o = i - offset;

        // Reduce offset if the char we are about to move is a newline (before moving it)
        // The fact that o is not being modified is intentional.
        // Both i and offset are changing by the same amount, so o would stay the same anyway.
        if (text[o] == '\n')
        {
            i = (n -= indentChars) - 1;
            offset -= indentChars;
        }

        text[i] = text[o];
        text[o] = ' ';
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
