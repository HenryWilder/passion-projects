#include "Action.h"

namespace
{
	std::list<void*> g_RedoableMemory;
	std::stack<Action*> g_backStack; // Backward (undo)
	std::stack<Action*> g_foreStack; // Forward (redo)
}

void Undo()
{
	g_foreStack.push(g_backStack.top());
	g_backStack.top()->Undo();
	g_backStack.pop();
}

void Redo()
{
	g_backStack.push(g_foreStack.top());
	g_foreStack.top()->Redo();
	g_foreStack.pop();
}
