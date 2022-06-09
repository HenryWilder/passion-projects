#pragma once
#include <raylib.h>
#include "Event.h"

namespace Engine
{
	namespace Shapes
	{
		__interface IPointCollidable
		{
			virtual bool CheckCollision(Vector2 pt) const = 0;
		};

		struct UIStyle
		{
			Vector2 offset = { 0 };
			Vector2 scale = { 1 };
			Color tint = WHITE;
		};
		__interface IDrawable
		{
			virtual void Draw(UIStyle style = {}) const = 0;
		};

		__interface Shape2D : IDrawable, IPointCollidable {};

		class Rectangle2D : public Shape2D
		{
			Rectangle data;

			// Todo: UIStyle not currently being stored

		public:
			Rectangle2D(const Rectangle& rec) : data{ rec } {}
			Rectangle2D(float x, float y, float width, float height) : data{ x, y, width, height } {}

			void Draw(UIStyle style) const final;
			inline bool CheckCollision(Vector2 pt) const final
			{
				return CheckCollisionPointRec(pt, data);
			}
		};
	}

	struct Transform
	{
		Vector2 position;
		Vector2 scale;
		float rotation;

		Vector2 anchors[2]; // 0..1
		Vector2 extents;
	};

	namespace InternalEvents
	{
		struct TickEventArgs { size_t tickNumber; };
		extern Events::Event<TickEventArgs> TickEvent;
		struct DrawEventArgs {};
		extern Events::Event<DrawEventArgs> DrawEvent;

		struct MouseEventArgs { Vector2 point; };
		extern Events::Event<MouseEventArgs> LeftMousePressEvent;
		extern Events::Event<MouseEventArgs> LeftMouseReleaseEvent;

	}

	class Object
	{
	private:	bool active = false;
	protected:	Events::Listener listener;
	public:		Transform transform;

	public:
		inline Object(bool enabledByDefault = true) { SetActive(enabledByDefault); }
		inline ~Object() { SetActive(false); }

		void SetActive(bool value);

	protected:
		// Called when the object is enabled
		virtual void OnEnable();
		// Called every tick
		virtual void Update();
		// Called every frame
		virtual void Draw();
		// Called when the object is disabled
		virtual void OnDisable();
	};

	
	class Draggable : public Object
	{
		Events::Event<InternalEvents::MouseEventArgs> OnPressEvent;
		Events::Event<InternalEvents::MouseEventArgs> OnReleaseEvent;
		Shapes::Shape2D* shape = nullptr;
		bool held = false;

	public:
		inline Draggable(Shapes::Shape2D* shape, bool enabledByDefault = true) : Object(enabledByDefault), shape(shape) {}
		inline ~Draggable() { _ASSERT_EXPR(shape, L"Draggable shape cannot be null"); delete shape; }

	private:
		void PressVerifier(void* sender, InternalEvents::MouseEventArgs e);
		void ReleaseVerifier(void* sender, InternalEvents::MouseEventArgs e);

		void OnPress(void* sender, InternalEvents::MouseEventArgs e);
		void OnRelease(void* sender, InternalEvents::MouseEventArgs e);

	protected:
		// Called when the object is enabled
		virtual void OnEnable() override;
		// Called every tick
		virtual void Update() override;
		// Called every Frame
		virtual void Draw() override;
		// Called when the object is disabled
		virtual void OnDisable() override;
	};
}
