#pragma once
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <queue>
#include <algorithm>    // Standard functions
#include <limits.h>     // Special values
#include <type_traits>  // Template metaprogramming

// Safe assertions for cases where you need to know, but exiting the program without some cleaning can be dangerous
#if _DEBUG
// Assert that the expr must be true. If it is not, the scoped code will not be executed (in debug).
#define ASSERT_CONDITION(expr, msg) _ASSERT_EXPR(expr, msg L"\nWARNING: Before working on the bug, please click ignore and close the program OR press retry, continue, and then close the program safely! Exiting the program at this moment without some cleanup has been deemed potentially dangerous!!"); if (expr)
#define ASSERTION_FAILSAFE else
#else
#define ASSERT_CONDITION(expr, msg)
#define ASSERTION_FAILSAFE if (false)
#endif
#define ASSERT_SPECIALIZATION default: _ASSERT_EXPR(false, L"Missing specialization for encountered case"); break
#define ASSERT_SPECIALIZATION_NAMED(being_specialized) default: _ASSERT_EXPR(false, L"Missing " being_specialized L" specialization for encountered case"); break

#pragma region Constants

// Custom colors
#define SPACEGRAY CLITERAL(Color){ 28, 26, 41, 255 }
#define LIFELESSNEBULA CLITERAL(Color){ 75, 78, 94, 255 }
#define HAUNTINGWHITE CLITERAL(Color){ 148, 150, 166, 255 }
#define GLEEFULDUST CLITERAL(Color){ 116, 125, 237, 255 }
#define INTERFERENCEGRAY CLITERAL(Color){ 232, 234, 255, 255 }
#define REDSTONE CLITERAL(Color){ 212, 25, 25, 255 }
#define DESTRUCTIVERED CLITERAL(Color){ 219, 18, 18, 255 }
#define DEADCABLE CLITERAL(Color){ 122, 118, 118, 255 }
#define INPUTLAVENDER CLITERAL(Color){ 128, 106, 217, 255 }
#define OUTPUTAPRICOT CLITERAL(Color){ 207, 107, 35, 255 }
#define WIPBLUE CLITERAL(Color){ 26, 68, 161, 255 }
#define CAUTIONYELLOW CLITERAL(Color){ 250, 222, 37, 255 }

constexpr int g_gridSize = 8;

#pragma endregion

// Converts the argument to a const wchar_t* string
#define WSTRINGIFY(arg) L ## #arg

template<typename C, typename T>
concept Container = requires(C x, T e)
{
    { x[0] } -> std::convertible_to<T>;
    x.begin();
    x.end();
    std::find(x.begin(), x.end(), e);
};

template<typename T, Container<T> C>
C::iterator Find(C& c, const T& element)
{
    if (auto it = std::find(c.begin(), c.end(), element); it != c.end())
        return it;
    return c.end();
}
template<typename T, Container<T> C>
C::iterator Find_ExpectExisting(C& c, const T& element)
{
    auto it = std::find(c.begin(), c.end(), element);
    _ASSERT_EXPR(it != c.end(), L"Expected element to be present");
    return it;
}
// Returns true on success
template<typename T, Container<T> C>
bool FindAndErase(C& c, const T& element)
{
    auto it = Find(c, element);
    bool found = it != c.end();
    if (found)
        c.erase(it);
    return found;
}
template<typename T, Container<T> C>
void FindAndErase_ExpectExisting(C& c, const T& element)
{
    c.erase(Find_ExpectExisting(c, element));
}

bool Between_Inclusive(int x, int a, int b);
bool Between_Exclusive(int x, int a, int b);

bool IsKeyDown_Shift();
bool IsKeyDown_Ctrl();
bool IsKeyDown_Alt();
