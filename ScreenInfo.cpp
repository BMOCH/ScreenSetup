#include "ScreenInfo.h"

// private helper function that checks if a is between b and c
BOOL isBetween(float a, float b, float c);

BOOL ScreenInfo::HitTest(float testX, float testY)
{
	// rotate input point around (x,y)
	const float s = sinf(angle * static_cast<float>(M_PI) / 180);
	const float c = cosf(angle * static_cast<float>(M_PI) / 180);
	const float dx = testX - x;
	const float dy = testY - y;
	const float rotatedX = dx * c - dy * s;
	const float rotatedY = dx * s + dy * c;

	// check if the point is bounded by the rectangle
	//return isBetween(rotatedX, x, x + width) && isBetween(rotatedY, y, y + height);
	return isBetween(testX, x, x + width) && isBetween(testY, y, y + height);
}

D2D1_RECT_F ScreenInfo::GetBoundRect()
{
	// rotate bottom right corner around (x,y)
	const float s = sinf(angle * static_cast<float>(M_PI) / 180.0f);
	const float c = cosf(angle * static_cast<float>(M_PI) / 180.0f);
	const float dx = width - x;
	const float dy = height - y;
	const float cx = dx * c - dy * s;
	const float cy = dx * s + dy * c;
	// construct rectangle using top left and bottom right points
	return D2D1::RectF(min(x,cx),min(y,cy),max(x,cx),max(y,cy));
}

BOOL isBetween(float a, float b, float c) {
	return (b <= a && a <= c) || (c <= a && a <= c);
}
