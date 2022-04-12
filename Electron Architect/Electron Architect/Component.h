#pragma once
#include "HUtility.h"
#include "IVec.h"
#include "Node.h"

// A scriptable black-box blueprint with inputs and outputs
class Component
{
private:
	Rectangle m_rec;
	std::vector<Node*> m_inputs;
	std::vector<Node*> m_outputs;

#pragma region Script
	const char* m_script;
	std::unordered_map<std::string, char> m_stack; // Read/write memory for the script to use

	// Accessible by the script interpreter
	bool Input(uint16_t index) const
	{
		try
		{
			return m_inputs[index]->GetState();
		}
		catch (...)
		{
			return false;
		}
	}
	// Accessible by the script interpreter
	void Output(uint16_t index, bool value)
	{
		try
		{
			m_outputs[index]->SetState(value);
		}
		catch (...)
		{
			return;
		}
	}
#pragma endregion

public:
};

