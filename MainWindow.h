/*
 * Defines the main application window.
 */
#pragma once
#include "framework.h"
#include "BaseWindow.h"
#include "ScreenInfo.h"
#include "ScreenSetup.h"

class MainWindow : public BaseWindow<MainWindow>
{
    enum class Mode
    {
        Select,
        Move
    };

    int canvasHeight; // pixel height of the canvas the monitors will be placed on
    int canvasWidth; // pixel width of the canvas the monitors will be placed on

    HCURSOR                 hCursor;

    ID2D1Factory*           pFactory;
    ID2D1HwndRenderTarget*  pRenderTarget;
    ID2D1SolidColorBrush*   pBrush;
    D2D1_POINT_2F           ptMouse; // point offset of mouse click from screen top left corner

    Mode                    mode; // current mode

    list<shared_ptr<ScreenInfo>>             screens; // list of screens
    list<shared_ptr<ScreenInfo>>::iterator   selection; // iterator for screens

    /********************************************************************************
    * Screen Selection and Movement Functions (defined in ScreenFunctions.cpp)
    ********************************************************************************/

    // gets a pointer to the selected screen
    shared_ptr<ScreenInfo> Selection()
    {
        if (selection == screens.end())
        {
            return nullptr;
        }
        else
        {
            return (*selection);
        }
    }

    // sets Selection() to the end of the list
    void    ClearSelection();

    // inserts a screen into the end of the list and sets it to Selection()
    HRESULT InsertScreen(float x, float y);

    // iterates over the list to test if any screens have (x,y) and returns true
    // and sets selection to that screen if found
    BOOL    HitTest(float x, float y);

    // sets the selection screen's top left corner to (x,y)
    void    SetLocation(float x, float y);

    // offsets the selection screen by (x,y)
    void    MoveSelection(float x, float y);

    /********************************************************************************
    * Input Functions (defined in ScreenSetup.cpp)
    ********************************************************************************/
    HRESULT CreateGraphicsResources();
    void    DiscardGraphicsResources();
    void    OnPaint();
    void    Resize();
    void    OnLButtonDown(int pixelX, int pixelY, DWORD flags);
    void    OnLButtonUp();
    void    OnMouseMove(int pixelX, int pixelY, DWORD flags);
    void    OnKeyDown(UINT vkey);

    // draws inputted screen to canvas/wall
    void    DrawScreen(ID2D1RenderTarget* pRT, ID2D1SolidColorBrush* pBrush, ScreenInfo screen, boolean sel);

    /********************************************************************************
    * Application State Functions
    ********************************************************************************/
    void    SetMode(Mode m)
    {
        mode = m;
    }

public:

    MainWindow() : hCursor(NULL), pFactory(NULL), pRenderTarget(NULL), pBrush(NULL),
        ptMouse(D2D1::Point2F()), mode(Mode::Select), selection(screens.end()),
        canvasHeight(DEFAULT_CANVAS_HEIGHT), canvasWidth(DEFAULT_CANVAS_WIDTH)
    {
    }

    PCWSTR  ClassName() const { return L"Screen Setup Window Class"; }
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};