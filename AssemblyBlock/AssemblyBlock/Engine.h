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

		__interface IDrawable
		{
			virtual void Draw(Color tint) const = 0;
		};

		__interface Shape2D : IDrawable, IPointCollidable {};

		class Rectangle2D : public Shape2D
		{
			Rectangle data;

			// Todo: UIStyle not currently being stored

		public:
			Rectangle2D(const Rectangle& rec) : data{ rec } {}
			Rectangle2D(float x, float y, float width, float height) : data{ x, y, width, height } {}

			void Draw(Color tint) const final;
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
	public:
		// Called every tick
		virtual void Update();
		// Called every frame
		virtual void Draw();
	private:
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
		template<class TShape>
		inline Draggable(TShape&& shape, bool enabledByDefault = true) requires std::derived_from<TShape, Shapes::Shape2D>
			: Object(enabledByDefault)
		{
			TShape* _shape = new TShape;
			*_shape = shape;
			this->shape = _shape;
		}
		inline ~Draggable() { _ASSERT_EXPR(shape, L"Draggable shape cannot be null"); delete shape; }

	private:
		void PressVerifier(void* sender, void* e);
		void ReleaseVerifier(void* sender, void* e);

		void OnPress(void* sender, void* e);
		void OnRelease(void* sender, void* e);

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
