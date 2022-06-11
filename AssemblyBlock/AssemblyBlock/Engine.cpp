#include "Engine.h"

Vector2 LocalFromAnchor(Vector2 extents, Vector2 anchor)
{
	return extents * anchor;
}

Vector2 AnchorFromLocal(Vector2 extents, Vector2 localPos)
{
	Vector2 ret;
	ret.x = ((extents.x == 0) ? (0) : (localPos.x / extents.x));
	ret.y = ((extents.y == 0) ? (0) : (localPos.y / extents.y));
	return ret;
}

Vector2 RectanglePosition(Rectangle rec)
{
	return { rec.x, rec.y };
}

Vector2 RectangleExtents(Rectangle rec)
{
	return { rec.width, rec.height };
}

void SetRectanglePosition(Rectangle& rec, Vector2 pos)
{
	rec.x = pos.x;
	rec.y = pos.y;
}

void AddRectanglePosition(Rectangle& rec, Vector2 offset)
{
	rec.x += offset.x;
	rec.y += offset.y;
}

void SetRectangleExtents(Rectangle& rec, Vector2 ext)
{
	rec.width = ext.x;
	rec.height = ext.y;
}


void ObjectTransform::_RemoveSelfFromParent()
{
	auto it = parent->children.find(this);
	_ASSERT_EXPR(it != parent->children.end(), L"Parent must have this as a child");
	parent->children.erase(it);
}
void ObjectTransform::_AddSelfToParent()
{
	parent->children.insert(this);
}

void ObjectTransform::RemoveParent()
{
	_RemoveSelfFromParent();
	parent = nullptr;
}
void ObjectTransform::SetParent(ObjectTransform& newParent)
{
	if (parent) _RemoveSelfFromParent();
	parent = &newParent;
	_AddSelfToParent();
}
void ObjectTransform::SetParent_KeepWorld(ObjectTransform& newParent)
{
	Vector2 worldPosition = GetWorldPosition();
	SetParent(newParent);
	SetWorldPosition(worldPosition);
}

const ObjectTransform* ObjectTransform::Parent() const
{
	return parent;
}

void ObjectTransform::SetObject(Object* object)
{
	this->object = object;
}
const Object* ObjectTransform::MyObject() const
{
	return object;
}


Vector2 ObjectTransform::_GetAnchorlessWorldPosition() const
{
	Vector2 local = RectanglePosition(bounds);
	if (parent)
		return local + parent->_GetAnchorlessWorldPosition();
	else
		return local;
}

Rectangle ObjectTransform::WorldBounds() const
{
	Rectangle ret = bounds;
	SetRectanglePosition(ret, _GetAnchorlessWorldPosition());
	return ret;
}
Rectangle ObjectTransform::LocalBounds() const
{
	return bounds;
}
void ObjectTransform::SetExtents(Vector2 extents)
{
	SetRectangleExtents(bounds, extents);
}

Vector2 ObjectTransform::GetLocalPosition(Vector2 anchor) const
{
	return RectanglePosition(bounds) + LocalFromAnchor(RectangleExtents(bounds), anchor);
}

Vector2 ObjectTransform::GetWorldPosition(Vector2 anchor) const
{
	Vector2 local = GetLocalPosition(anchor);
	if (parent)
		return local + parent->GetWorldPosition();
	else
		return local;
}

void ObjectTransform::SetLocalPosition(Vector2 position, Vector2 anchor)
{
	SetRectanglePosition(bounds, position - LocalFromAnchor(RectangleExtents(bounds), anchor));
}

void ObjectTransform::SetWorldPosition(Vector2 position, Vector2 anchor)
{
	Vector2 localPos;
	if (parent)
		localPos = position - parent->GetWorldPosition();
	else
		localPos = position;
	SetLocalPosition(localPos, anchor);
}


using namespace Data;


Object::Object(ObjectTransform trans) : transform(trans) { transform.SetObject(this); }

// Todo: make a function to get the anchor from a point on the rectangle
bool Object::CheckPointSimpleCollision(Vector2 point) const
{
	return CheckCollisionPointRec(point, transform.WorldBounds());
}
// Make sure to specialize "IsComplexCollisionDifferentFromSimpleCollision"
// too if you specialize this function, or it will be skipped!!
bool Object::CheckPointComplexCollision(Vector2 point) const
{
	return CheckPointSimpleCollision(point);
}

void Destroy(Object* object)
{
	delete object;
}


Hoverable::Hoverable(ObjectTransform trans) : Object(trans), hovered() {}

void Hoverable::Update()
{
	hovered = CheckPointSimpleCollision(Frame::cursor);
	if (!hovered) return;
	if (IsComplexCollisionDifferentFromSimpleCollision())
		hovered &= CheckPointComplexCollision(Frame::cursor);
}


FocusableBase::FocusableBase(ObjectTransform trans) : Hoverable(trans) {}

void FocusableBase::OnFocus() {}
void FocusableBase::OnLoseFocus() {}
bool FocusableBase::IsFocusable() const
{
	return b_focusable;
}
void FocusableBase::SetFocusable(bool value)
{
	b_focusable = value;
	if (value == false)
		focused = false;
}


Focusable::Focusable(ObjectTransform trans) : FocusableBase(trans) {}

void Focusable::Update()
{
	Hoverable::Update();
	if (!IsFocusable()) return;
	if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
	{
		if (focused = hovered)
			OnFocus();
		else
			OnLoseFocus();
	}
}


ADDFocusable::ADDFocusable(ObjectTransform trans) : FocusableBase(trans) {}

void ADDFocusable::Update()
{
	Hoverable::Update();
	if (!IsFocusable()) return;
	if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
	{
		focused = false;
		OnLoseFocus();
	}
	else if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && hovered)
	{
		focused = true;
		OnFocus();
	}
}


Draggable::Draggable(ObjectTransform trans) : ADDFocusable(trans) {}

void Draggable::OnFocus()
{
	if (!IsDraggable()) return;
	beingDragged = true;
	OnStartDragging();
}
void Draggable::OnLoseFocus()
{
	if (!IsDraggable()) return;
	beingDragged = false;
	OnStopDragging();
}

void Draggable::OnStartDragging() {}
void Draggable::OnStopDragging() {}

void Draggable::Update()
{
	ADDFocusable::Update();
	beingDragged = focused && b_draggable;
	if (beingDragged)
		transform.Offset(GetMouseDelta());
}

bool Draggable::IsDraggable() const
{
	return b_draggable;
}
void Draggable::SetDraggable(bool value)
{
	b_draggable = value;
	if (value == false)
		beingDragged = false;
}
