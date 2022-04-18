#include "Actions.h"
#include "Wire.h"
#include "Node.h"
#include "NodeWorld.h"

void ActionHandler::Undo()
{
	if (m_index >= 0)
		m_actions[m_index--]->Undo();
}

void ActionHandler::Redo()
{
	if (m_index < m_actions.size())
		m_actions[m_index++]->Redo();
}

void ActionHandler::Push(IAction* action)
{
	while (m_index < m_actions.size())
	{
		m_actions.pop_back();
	}
	m_actions.push_back(action);
	m_index++;
}

ActionHandler g_actionHandler;

void Action_ChangeGate::Undo()
{
	target->SetGate(prev);
}

void Action_ChangeGate::Redo()
{
	target->SetGate(post);
}

void Action_MoveNode::Undo()
{
	target->SetPosition(prev);
}

void Action_MoveNode::Redo()
{
	target->SetPosition(post);
}
