#include "ScreenInfo.h"

// private helper function that checks if a is between b and c
BOOL isBetween(float a, float b, float c);

BOOL ScreenInfo::HitTest(float testX, float testY)
{
	// rotate input point around (x,y)
	const float sin = sinf(angle * static_cast<float>(M_PI) / -180);
	const float cos = cosf(angle * static_cast<float>(M_PI) / -180);
	const float dx = testX - x;
	const float dy = testY - y;
	const float rotX = dx * cos - dy * sin;
	const float rotY = dx * sin + dy * cos;

	// check if the point is within the bounds of the rectangle
	return isBetween(rotX, 0, width) && isBetween(rotY, 0, height);
}

D2D1_RECT_F ScreenInfo::GetBoundRect(float testX, float testY)
{
    // code adapted from https://stackoverflow.com/a/624082
    const float sin = sinf(angle * static_cast<float>(M_PI) / 180);
    const float cos = cosf(angle * static_cast<float>(M_PI) / 180);

    const float hct = height * cos;
    const float wct = width * cos;
    const float hst = height * sin;
    const float wst = width * sin;

    float x_min, x_max, y_min, y_max;
    if (angle < 90)
    {
        y_min = testY;
        y_max = testY + hct + wst;
        x_min = testX - hst;
        x_max = testX + wct;
    }
    else if (angle < 180)
    {
        y_min = testY + hct;
        y_max = testY + wst;
        x_min = testX - hst + wct;
        x_max = testX;
    }
    else if (angle < 270)
    {
        y_min = testY + wst + hct;
        y_max = testY;
        x_min = testX + wct;
        x_max = testX;
    }
    else
    {
        y_min = testY + wst;
        y_max = testY + hct;
        x_min = testX;
        x_max = testX + wct - hst;
    }
    return D2D1::RectF(x_min, y_min, x_max, y_max);
}

BOOL isBetween(float a, float b, float c) {
	return (b <= a && a <= c) || (c <= a && a <= b);
}
