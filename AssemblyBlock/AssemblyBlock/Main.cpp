#include <vector>
#include "raylib.h"
#include "raymath.h"
#include "rayex.h"
#include "Properties.h"
#include "Rect.h"
#include "Window.h"
#include "Frame.h"
#include "Panel.h"
#include "Viewport.h"
#include "Console.h"
#include "Inspector.h"
#include "Toolbox.h"

int main()
{
	SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
	Vector2 windowSize = { 1280, 720 };
	InitWindow((int)windowSize.x, (int)windowSize.y, "Assembly Block v0.0.1");
	InitShapeTexture();
	Panel::gripShader = LoadShader(0, "grip.frag");
	Panel::gripShaderSizeLoc = GetShaderLocation(Panel::gripShader, "size");
	SetTargetFPS(60);

	// Prep phase
	bool invertScrolling = false;
	UseInvertedScoll(invertScrolling);

	Panel::CreatePanel(Rect(0,0,1280,80), new Toolbox());
	Panel::CreatePanel(Rect(0,80,200,640), new Inspector());
	Panel::CreatePanel(Rect(200,80,1080,440), new Viewport());
	Panel::CreatePanel(Rect(200,520,1080,200), new Console());

	while (!WindowShouldClose())
	{
		// Sim phase

		Window::Update();

		// Order
		for (size_t i = 0; i < Panel::allPanels.size(); ++i)
		{
			Panel* panel = Panel::allPanels[i];

			// Focused
			if (panel->PanelContains(Window::mousePos) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
			{
				Panel::SetActivePanel(i);
				break;
			}
		}

		Window::cursor = MOUSE_CURSOR_DEFAULT;

		Panel::allPanels[0]->TickActive();
		for (size_t i = 1; i < Panel::allPanels.size(); ++i)
		{
			Panel::allPanels[i]->TickPassive();
		}

		// Add and remove panels
		while (!Panel::panelsToRemove.empty())
		{
			Panel* panel = Panel::panelsToRemove.front();
			auto it = std::find(Panel::allPanels.begin(), Panel::allPanels.end(), panel);
			Panel::allPanels.erase(it);
			delete panel;
			Panel::panelsToRemove.pop();
		}
		while (!Panel::panelsToAdd.empty())
		{
			// @Speed: this is gonna copy a lot if multiple panels are added in a tick
			Panel::allPanels.insert(Panel::allPanels.begin(), Panel::panelsToAdd.front());
			Panel::panelsToAdd.pop();
		}

		SetMouseCursor(Window::cursor);

		// Draw phase
		BeginDrawing();
		{
			ClearBackground({ 80,80,80,255 });

			for (int i = Panel::allPanels.size() - 1; i >= 0; i--)
			{
				Panel::allPanels[i]->Draw();
			}
		}
		EndDrawing();
	}

	// Cleanup phase

	UnloadShader(Panel::gripShader);
	UnloadTexture(texShapes);

	CloseWindow();

	for (Panel* panel : Panel::allPanels)
	{
		delete panel;
	}

	return 0;
}
