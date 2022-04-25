#include "Action.h"

namespace ActStack
{
	std::list<void*> g_RedoableMemory;
	std::stack<Action*> ActStack::g_backStack; // Backward (undo)
	std::stack<Action*> ActStack::g_foreStack; // Forward (redo)
}

void Undo()
{
	ActStack::g_foreStack.push(ActStack::g_backStack.top());
	ActStack::g_backStack.top()->Undo();
	ActStack::g_backStack.pop();
}

void Redo()
{
	ActStack::g_backStack.push(ActStack::g_foreStack.top());
	ActStack::g_foreStack.top()->Redo();
	ActStack::g_foreStack.pop();
}
