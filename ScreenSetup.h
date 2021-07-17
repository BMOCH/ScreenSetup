/*
 * Settings.
 * 
 * For colors use RGB values or defaults here:
 * https://developer.mozilla.org/en-US/docs/Web/CSS/color_value
*/
#pragma once
#include "resource.h"
#include "windowsx.h"

// maximum percentage of the window width the canvas will take up
const FLOAT MAX_CANVAS_WIDTH_PERCENT = 0.75;

// default canvas dimensions
const INT DEFAULT_CANVAS_WIDTH = 7680;
const INT DEFAULT_CANVAS_HEIGHT = 4320;

// background color
const D2D1_COLOR_F backgroundColor = D2D1::ColorF(D2D1::ColorF::WhiteSmoke);

// canvas color
const D2D1_COLOR_F canvasColor = D2D1::ColorF(D2D1::ColorF::DarkGray);

// screen fill, border, and selection border color when drawn on the canvas
const D2D1_COLOR_F fillColor = D2D1::ColorF(D2D1::ColorF::Gray);
const D2D1_COLOR_F borderColor = D2D1::ColorF(D2D1::ColorF::Black);
const D2D1_COLOR_F selectColor = D2D1::ColorF(D2D1::ColorF::Red);

// side panel font settings
const WCHAR msc_fontName[] = L"Verdana";
const FLOAT msc_fontSize = 20;
const D2D1_COLOR_F fontColor = D2D1::ColorF(D2D1::ColorF::Black);

// side panel padding
const FLOAT SIDE_PADDING = 0.025f;

// side panel textbox max width
const INT MAX_TEXTBOX_WIDTH = 200;

// class to safely release pointers
template <class T> void SafeRelease(T** ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}
