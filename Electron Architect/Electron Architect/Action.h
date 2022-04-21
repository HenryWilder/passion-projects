#pragma once
#include <stack>
#include <type_traits>

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

extern std::stack<Action*> g_backStack; // Backward (undo)
extern std::stack<Action*> g_foreStack; // Forward (redo)

template<class T>
void PushAction(T&& action)
{
	static_assert(std::is_base_of_v<Action, T>, "PushAction argument must derive from class Action.");
	while (!g_foreStack.empty())
	{
		delete g_foreStack.top();
		g_foreStack.pop();
	}
	g_backStack.push(new T(action));
	g_backStack.top()->Redo(); // Do for first time
}

void Undo();
void Redo();
