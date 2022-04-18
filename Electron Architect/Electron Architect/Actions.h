#pragma once
#include "HUtility.h"
#include "IVec.h"

__interface IAction
{
	// Should have a prev
	// Should have a post
	// Should have a target
public:
	virtual void Undo() = 0;
	virtual void Redo() = 0;
};

class ActionHandler
{
	int m_index;
	std::vector<IAction*> m_actions;
public:
	void Undo();
	void Redo();
	void Push(IAction* action);
};
extern ActionHandler g_actionHandler;

class Node;
class Action_ChangeGate : public IAction
{
	Gate prev;
	Gate post;
	Node* target;
public:
	void Undo() override;
	void Redo() override;
};
class Action_MoveNode : public IAction
{
	IVec2 prev;
	IVec2 post;
	Node* target;
public:
	void Undo() override;
	void Redo() override;
};
