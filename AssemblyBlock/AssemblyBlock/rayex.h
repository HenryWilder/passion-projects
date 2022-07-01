#pragma once
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#pragma region Vector operators

inline Vector2 operator+(Vector2 a, Vector2 b)      { return Vector2Add(a, b);               }
inline Vector2 operator-(Vector2 a, Vector2 b)      { return Vector2Subtract(a, b);          }
inline Vector2 operator*(Vector2 a, Vector2 b)      { return Vector2Multiply(a, b);          }
inline Vector2 operator/(Vector2 a, Vector2 b)      { return Vector2Divide(a, b);            }
inline Vector2 operator+(Vector2 a, float b)        { return Vector2AddValue(a, b);          }
inline Vector2 operator-(Vector2 a, float b)        { return Vector2SubtractValue(a, b);     }
inline Vector2 operator*(Vector2 a, float b)        { return Vector2Scale(a, b);             }
inline Vector2 operator/(Vector2 a, float b)        { return Vector2Scale(a, 1.0f/b);        }
inline Vector2 operator+(float a, Vector2 b)        { return Vector2AddValue(b, a);          }
inline Vector2 operator-(float a, Vector2 b)        { return Vector2Subtract({ a,a }, b);    }
inline Vector2 operator*(float a, Vector2 b)        { return Vector2Scale(b, a);             }
inline Vector2 operator/(float a, Vector2 b)        { return Vector2Divide({ a,a }, b);      }

inline Vector2& operator+=(Vector2& a, Vector2 b)   { return a = Vector2Add(a, b);           }
inline Vector2& operator-=(Vector2& a, Vector2 b)   { return a = Vector2Subtract(a, b);      }
inline Vector2& operator*=(Vector2& a, Vector2 b)   { return a = Vector2Multiply(a, b);      }
inline Vector2& operator/=(Vector2& a, Vector2 b)   { return a = Vector2Divide(a, b);        }
inline Vector2& operator+=(Vector2& a, float b)     { return a = Vector2AddValue(a, b);      }
inline Vector2& operator-=(Vector2& a, float b)     { return a = Vector2SubtractValue(a, b); }
inline Vector2& operator*=(Vector2& a, float b)     { return a = Vector2Scale(a, b);         }
inline Vector2& operator/=(Vector2& a, float b)     { return a = Vector2Scale(a, 1.0f/b);    }

inline Vector2 operator-(Vector2 vec)               { return Vector2Negate(vec);             }
inline Vector2 operator+(Vector2 vec)               { return vec;                            }

#pragma endregion

inline void rlVertexV2(Vector2 vert)
{
	rlVertex2f(vert.x, vert.y);
}

inline Vector2 Vector2Floor(Vector2 vec)
{
	Vector2 ret = { floorf(vec.x), floorf(vec.y) };
	return ret;
}

void DrawRectangleMinMax     (float xMin, float yMin, float xMax, float yMax, Color color);
void DrawRectangleLinesMinMax(float xMin, float yMin, float xMax, float yMax, Color color);

// Draws a square centered around the point
// Size in screenspace
inline void DrawPoint2D(Vector2 point, float size, Color color, Camera2D camera)
{
	float sizeWS = size / camera.zoom;
	float halfSize = sizeWS * 0.5f;
	Vector2 position = point - halfSize;
	DrawRectangleV(position, { sizeWS, sizeWS }, color);
}

extern Texture2D texShapes;
extern Rectangle texShapesRec;
inline void InitShapeTexture()
{
	Image img = GenImageColor(1,1,WHITE);
	texShapes = LoadTextureFromImage(img);
	UnloadImage(img);
}
