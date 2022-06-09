#include "Engine.h"

namespace Engine
{
	namespace Shapes
	{
		void Rectangle2D::Draw(UIStyle style) const
		{
			Rectangle temp = {
				data.x + style.offset.x,
				data.y + style.offset.y,
				data.width * style.scale.x,
				data.height * style.scale.y
			};
			DrawRectangleRec(temp, style.tint);
		}
	}

	namespace InternalEvents
	{
		Events::Event<TickEventArgs> TickEvent;
		Events::Event<DrawEventArgs> DrawEvent;

		Events::Event<MouseEventArgs> LeftMousePressEvent;
		Events::Event<MouseEventArgs> LeftMouseReleaseEvent;
	}

	using namespace InternalEvents;

	void Object::SetActive(bool value)
	{
		if (active != value)
		{
			active = value;
			if (active)
			{
				TickEvent.Connect(listener, [this](void* sender, TickEventArgs args) { Update(); });
				DrawEvent.Connect(listener, [this](void* sender, DrawEventArgs args) { Draw(); });
				OnEnable();
			}
			else
			{
				TickEvent.Disconnect(listener);
				DrawEvent.Disconnect(listener);
				OnDisable();
			}
		}
	}

	void Object::OnEnable() {}
	void Object::Update() {}
	void Object::Draw() {}
	void Object::OnDisable() {}

	void Draggable::PressVerifier(void* sender, MouseEventArgs e)
	{
		if (shape->CheckCollision(e.point))
			OnPressEvent(this, e);
	}
	void Draggable::ReleaseVerifier(void* sender, InternalEvents::MouseEventArgs e)
	{
		if (shape->CheckCollision(e.point))
			OnReleaseEvent(this, e);
	}
	void Draggable::OnPress(void* sender, InternalEvents::MouseEventArgs args)
	{
		held = true;
	}
	void Draggable::OnRelease(void* sender, InternalEvents::MouseEventArgs args)
	{
		held = false;
	}

	void Draggable::OnEnable()
	{
		LeftMousePressEvent.Connect(listener, [this](void* sender, MouseEventArgs args) { PressVerifier(sender, args); });
		LeftMouseReleaseEvent.Connect(listener, [this](void* sender, MouseEventArgs args) { ReleaseVerifier(sender, args); });
		OnPressEvent.Connect(listener, [this](void* sender, MouseEventArgs args) { OnPress(sender, args); });
		OnReleaseEvent.Connect(listener, [this](void* sender, MouseEventArgs args) { OnRelease(sender, args); });
	}

	void Draggable::Update()
	{

	}

	void Draggable::Draw()
	{
		shape->Draw();
	}

	void Draggable::OnDisable()
	{
		LeftMousePressEvent.Disconnect(listener);
		LeftMouseReleaseEvent.Disconnect(listener);
		OnPressEvent.Disconnect(listener);
		OnReleaseEvent.Disconnect(listener);
	}
}
