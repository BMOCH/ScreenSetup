#pragma once

#include "resource.h"
#include "HelperFunctions.h"
#include "windowsx.h"

// maximum percentage of the window width the canvas will take up
const float MAX_CANVAS_WIDTH_PERCENT = 0.75;

// screen fill, border, and selection border color when drawn on the canvas
const D2D1_COLOR_F fillColor = D2D1::ColorF(D2D1::ColorF::Gray);
const D2D1_COLOR_F borderColor = D2D1::ColorF(D2D1::ColorF::Black);
const D2D1_COLOR_F selectColor = D2D1::ColorF(D2D1::ColorF::Red);

const int DEFAULT_CANVAS_WIDTH = 7680;
const int DEFAULT_CANVAS_HEIGHT = 4320;
