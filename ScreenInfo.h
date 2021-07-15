/* 
 * Defines the ScreenInfo struct which will be used to store all screen information.
 */

#pragma once
#include "framework.h"
#pragma comment(lib, "d2d1")
#define _USE_MATH_DEFINES
#include <math.h> // pi, trig functions

using namespace std;

// screen's default height and width.
const int DEFAULT_HEIGHT = 1080;
const int DEFAULT_WIDTH = 1920;

struct ScreenInfo
{
	FLOAT x;      // x coord on wall
	FLOAT y;      // y coord on wall
	FLOAT angle;  // screen angle (around (x,y) clockwise)
	INT   height; // screen height
	INT   width;  // screen width

	// used when working with the screens on the panel
	BOOL HitTest(float testX, float testY);
	D2D1_RECT_F GetBoundRect();
};
