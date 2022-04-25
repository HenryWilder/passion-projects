#pragma once
#include <stack>
#include <list>
#include <any>
#include <type_traits>

namespace ActStack
{
	extern std::list<void*> g_RedoableMemory;
}

template<class T>
class RedoReservation
{
	decltype(ActStack::g_RedoableMemory)::iterator position;
	decltype(ActStack::g_RedoableMemory)::iterator& Iterator()
	{
		_ASSERT_EXPR(position != ActStack::g_RedoableMemory.end(), L"Tried to access invalid reservation");
		return position;
	}
	const decltype(ActStack::g_RedoableMemory)::iterator& Iterator() const
	{
		_ASSERT_EXPR(position != ActStack::g_RedoableMemory.end(), L"Tried to access invalid reservation");
		return position;
	}
	T Pointer() const
	{
		_ASSERT_EXPR(IsValid(), L"Tried to access invalid reservation");
		return (T)*Iterator();
	}
public:
	RedoReservation(T object)
	{
		static_assert(std::is_pointer_v<T>, "RedoReceipt can only accept pointers. Non-referencial data can be stored as POD and doesn't need a receipt.");
		ActStack::g_RedoableMemory.push_back(object);
		position = ActStack::g_RedoableMemory.end()--;
	}
	~RedoReservation()
	{
		ActStack::g_RedoableMemory.erase(Iterator());
	}
	bool IsValid() const
	{
		// Doesn't need the assertion
		return !!*Iterator();
	}
	void Set(T pointer)
	{
		Pointer() = pointer;
	}
	T Get() const
	{
		return (T)Pointer();
	}
	T operator*() const
	{
		return Get();
	}
	T operator->() const
	{
		return Get();
	}
};

class Action
{
	/* Example members:
	T m_target; // Target of the action (should be initialized explicitly in constructor)
	T m_prev;   // Value when "undo" is called (should be initialized from m_target)
	T m_post;   // Value when "redo" is called (should be initialized explicitly in constructor)
	*/
	virtual void Undo() = 0;
	virtual void Redo() = 0;

	template<class T> friend void PushAction(T&& action);
	friend void Undo();
	friend void Redo();

	// Constructor must be public and can have any parameters needed
};

namespace ActStack
{
	extern std::stack<Action*> g_backStack; // Backward (undo)
	extern std::stack<Action*> g_foreStack; // Forward (redo)
}

template<class T>
void PushAction(T&& action)
{
	static_assert(std::is_base_of_v<Action, T>, "PushAction argument must derive from class Action.");
	while (!ActStack::g_foreStack.empty())
	{
		delete ActStack::g_foreStack.top();
		ActStack::g_foreStack.pop();
	}
	ActStack::g_backStack.push(new T(action));
	ActStack::g_backStack.top()->Redo(); // Do for first time
}

void Undo();
void Redo();
