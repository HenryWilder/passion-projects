#pragma once
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <queue>
#include <algorithm>
#include <limits.h>

// Safe assertions for cases where you need to know, but exiting the program without some cleaning can be dangerous
#if _DEBUG
// Assert that the expr must be true. If it is not, the scoped code will not be executed (in debug).
#define ASSERT_CONDITION(expr, msg) _ASSERT_EXPR(expr, msg L"\nWARNING: Before working on the bug, please click ignore and close the program OR press retry, continue, and then close the program safely! Exiting the program at this moment without some cleanup has been deemed potentially dangerous!!"); if (expr)
#define ASSERTION_FAILSAFE else
#else
#define ASSERT_CONDITION(expr, msg)
#define ASSERTION_FAILSAFE if (false)
#endif
#define ASSERT_SPECIALIZATION(being_specialized) default: _ASSERT_EXPR(false, L"Missing " being_specialized L" specialization for encountered case"); break

constexpr int g_gridSize = 8;

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
    _ASSERT_EXPR(it != container.end(), L"Expected element to be present");
    container.erase(it);
}

bool Between_Inclusive(int x, int a, int b);
bool Between_Exclusive(int x, int a, int b);
