#include "rayex.h"

void DrawRectangleMinMax(float xMin, float yMin, float xMax, float yMax, Color color)
{
	Vector2 topLeft     = { xMin, yMin };
	Vector2 topRight    = { xMax, yMin };
	Vector2 bottomLeft  = { xMin, yMax };
	Vector2 bottomRight = { xMax, yMax };

#if defined(SUPPORT_QUADS_DRAW_MODE)
	rlCheckRenderBatchLimit(4);

	rlSetTexture(1u);

	rlBegin(RL_QUADS);

	rlNormal3f(0.0f, 0.0f, 1.0f);
	rlColor4ub(color.r, color.g, color.b, color.a);
	rlTexCoord2f(0, 0);
	rlVertexV2(topLeft);
	rlVertexV2(bottomLeft);
	rlVertexV2(bottomRight);
	rlVertexV2(topRight);

	rlEnd();

	rlSetTexture(0);
#else
	rlCheckRenderBatchLimit(6);

	rlBegin(RL_TRIANGLES);

	rlColor4ub(color.r, color.g, color.b, color.a);

	rlVertex2f(topLeft.x, topLeft.y);
	rlVertex2f(bottomLeft.x, bottomLeft.y);
	rlVertex2f(topRight.x, topRight.y);

	rlVertex2f(topRight.x, topRight.y);
	rlVertex2f(bottomLeft.x, bottomLeft.y);
	rlVertex2f(bottomRight.x, bottomRight.y);

	rlEnd();
#endif
}

void DrawRectangleLinesMinMax(float xMin, float yMin, float xMax, float yMax, Color color)
{
	float width = xMax - xMin;
	float height = yMax - yMin;
#if defined(SUPPORT_QUADS_DRAW_MODE)

	DrawRectangleMinMax(xMin, yMin, xMax, 1, color);
	DrawRectangleMinMax(xMin - 1, yMin + 1, 1, yMax - 1, color);
	DrawRectangleMinMax(xMin, yMin + 1, xMax, 1, color);
	DrawRectangleMinMax(xMin, yMin + 1, 1, yMax - 1, color);
#else
	rlBegin(RL_LINES);
	rlColor4ub(color.r, color.g, color.b, color.a);
	rlVertex2i(xMin + 1, yMin + 1);
	rlVertex2i(xMax, yMin + 1);

	rlVertex2i(xMax, yMin + 1);
	rlVertex2i(xMax, yMax);

	rlVertex2i(xMax, yMax);
	rlVertex2i(xMin + 1, yMax);

	rlVertex2i(xMin + 1, yMax);
	rlVertex2i(xMin + 1, yMin + 1);
	rlEnd();
#endif
}

Texture2D texShapes;
Rectangle texShapesRec = { 0,0,1,1 };
