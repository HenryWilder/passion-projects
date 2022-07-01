#include "GridWorld.h"

#pragma region IVec2

IVec2::IVec2(int x, int y) : x((HalfLong_t)x), y((HalfLong_t)y) {}
IVec2::IVec2(HalfLong_t x, HalfLong_t y) : x(x), y(y) {}
IVec2::IVec2(Vector2 vec) : x((HalfLong_t)vec.x), y((HalfLong_t)vec.y) {}
IVec2::operator Vector2() const { return { (float)x, (float)y }; }

#pragma endregion

IVec2 Vector2FloorToIVec(Vector2 vec)
{
	IVec2 ret = { (HalfLong_t)floorf(vec.x), (HalfLong_t)floorf(vec.y) };
	return ret;
}

// Base class for anything that can be on a gridspace
#pragma region Space

std::unordered_map<IVec2, Space*> Space::world;

// Todo

#pragma endregion
