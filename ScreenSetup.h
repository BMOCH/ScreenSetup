/*
 * Settings.
*/
#pragma once
#include "resource.h"
#include "windowsx.h"

// maximum percentage of the window width the canvas will take up
const float MAX_CANVAS_WIDTH_PERCENT = 0.75;

// default canvas dimensions
const int DEFAULT_CANVAS_WIDTH = 7680;
const int DEFAULT_CANVAS_HEIGHT = 4320;

// screen fill, border, and selection border color when drawn on the canvas
const D2D1_COLOR_F fillColor = D2D1::ColorF(D2D1::ColorF::Gray);
const D2D1_COLOR_F borderColor = D2D1::ColorF(D2D1::ColorF::Black);
const D2D1_COLOR_F selectColor = D2D1::ColorF(D2D1::ColorF::Red);

// font and text size for side panel
const WCHAR msc_fontName[] = L"Verdana";
const FLOAT msc_fontSize = 30;

// side panel padding
const FLOAT SIDE_PADDING = 0.025f;
const D2D1_COLOR_F fontColor = D2D1::ColorF(D2D1::ColorF::Black);

// class to safely release pointers
template <class T> void SafeRelease(T** ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}
