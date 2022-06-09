#include "Engine.h"

using namespace Engine;
using namespace Events;
using namespace Shapes;
using namespace InternalEvents;

namespace Engine
{
	namespace Shapes
	{
		void Rectangle2D::Draw(Color tint) const
		{
			DrawRectangleRec(data, tint);
		}
	}

	namespace InternalEvents
	{
		Event TickEvent;
		Event DrawEvent;

		Event LeftMousePressEvent;
		Event LeftMouseReleaseEvent;
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

	void Draggable::PressVerifier(void* sender, void* e)
	{
		if (shape->CheckCollision(e.point))
			OnPressEvent(this, e);
	}
	void Draggable::ReleaseVerifier(void* sender, void* e)
	{
		if (shape->CheckCollision(e.point))
			OnReleaseEvent(this, e);
	}
	void Draggable::OnPress(void* sender, void args)
	{
		held = true;
	}
	void Draggable::OnRelease(void* sender, void args)
	{
		held = false;
	}

	void Draggable::OnEnable()
	{
		Delegate::FromMethod<Draggable, &Draggable::PressVerifier>(this);
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
		shape->Draw(RAYWHITE);
	}

	void Draggable::OnDisable()
	{
		LeftMousePressEvent.Disconnect(listener);
		LeftMouseReleaseEvent.Disconnect(listener);
		OnPressEvent.Disconnect(listener);
		OnReleaseEvent.Disconnect(listener);
	}
}
