/*
 * Defines the window for setting the canvas dimensions.
 */
#pragma once
#include "framework.h"
#include "BaseWindow.h"
#include "HelperFunctions.h"

class DimensionsWindow : public BaseWindow<DimensionsWindow>
{
    HCURSOR                 hCursor;        // cursor
    ID2D1Factory*           pFactory;       // drawing factory
    ID2D1HwndRenderTarget*  pRenderTarget;  // render target
    ID2D1SolidColorBrush*   pBrush;         // drawing brush

    HWND     height_hwnd;
    HWND     width_hwnd;
    HWND     apply_hwnd;

    HRESULT  CreateGraphicsResources();
    void     DiscardGraphicsResources();
    void     OnPaint();
    void     OnKeyDown(UINT vkey);

public:

    DimensionsWindow() : hCursor(LoadCursor(NULL, IDC_ARROW)), pFactory(NULL),
        pRenderTarget(NULL), pBrush(NULL), height_hwnd(NULL), width_hwnd(NULL)
    {
    }

    // size of the window in dp
    const INT WindowWidth  = 500;
    const INT WindowHeight = 400;

    PCWSTR  ClassName() const { return L"Canvas Dimension Window Class"; }
    INT     MenuName() const { return NULL; }
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};