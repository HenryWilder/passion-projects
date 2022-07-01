#pragma once
#include <unordered_map>
#include <raylib.h>

using HalfLong_t = std::conditional_t<sizeof(size_t) == 8, int, short>;

struct IVec2
{
	IVec2() = default;
	IVec2(int x, int y);
	IVec2(HalfLong_t x, HalfLong_t y);
	explicit IVec2(Vector2 vec);
	explicit operator Vector2() const;

	HalfLong_t x, y;
};

IVec2 Vector2FloorToIVec(Vector2 vec);

namespace std
{
	template<>
	struct hash<IVec2>
	{
		static_assert((sizeof(IVec2::x) * 2) == sizeof(size_t));
		_NODISCARD size_t operator()(const IVec2 _Keyval) const noexcept {

			size_t kv =
				static_cast<size_t>(_Keyval.x) << (8 * sizeof(IVec2::x)) |
				static_cast<size_t>(_Keyval.y);
			return _Hash_representation(kv);
		}
	};
}

// Base class for anything that can be on a gridspace
class Space
{
	static std::unordered_map<IVec2, Space*> world;

	// Todo
};
