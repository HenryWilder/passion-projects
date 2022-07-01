#pragma once

class Panel;

// Contents that gets put inside a SubWindow
class Frame
{
protected:
	Panel* panel; // The owning panel

public:
	void SetPanel(Panel* panel);

	virtual void TickPassive() = 0; // Tick without any interaction
	virtual void TickActive() = 0; // Tick with interaction
	virtual void Draw() const = 0;
	virtual const char* GetName() const = 0;
};
