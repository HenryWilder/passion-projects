#pragma once
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <queue>
#include <algorithm>
#include <limits.h>

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

template <class Iter>
class Range {
    Iter first;
    Iter last;

public:
    Range(Iter first, Iter last) : first(first), last(last) {}

    Iter begin()
    {
        return first;
    }
    Iter end()
    {
        return last;
    }
};

template <class Container>
Range<typename Container::iterator> MakeRange(Container& container, size_t begin, size_t end)
{
    return {
        container.begin() + begin,
        container.begin() + end
    };
}
template <class Container>
Range<typename Container::const_iterator> MakeRange(const Container& container, size_t begin, size_t end)
{
    return {
        container.begin() + begin,
        container.begin() + end
    };
}

// Returns true on success
template<typename Container>
bool FindAndErase(Container& container, const typename Container::value_type& element)
{
    if (auto it = std::find(container.begin(), container.end(), element); it != container.end())
    {
        container.erase(it);
        return true;
    }
    return false;
}
template<typename Container>
void FindAndErase_ExpectExisting(Container& container, const typename Container::value_type& element)
{
    auto it = std::find(container.begin(), container.end(), element);
    _ASSERT_EXPR(it != container.end(), "Expected element to be present");
    container.erase(it);
}

bool Between_Inclusive(int x, int a, int b);
bool Between_Exclusive(int x, int a, int b);