/*
 * Defines the screen selection and movement functions.
 */
#include "MainWindow.h"

void MainWindow::ClearSelection()
{
    selection = screens.end();
}

HRESULT MainWindow::InsertScreen(float x, float y)
{
    try
    {
        selection = screens.insert(
            screens.end(),
            shared_ptr<ScreenInfo>(new ScreenInfo()));

        Selection()->angle = 0; // default angle is 0
        Selection()->height = DEFAULT_HEIGHT;
        Selection()->width = DEFAULT_WIDTH;
        SetLocation(x, y);
    }
    catch (std::bad_alloc)
    {
        return E_OUTOFMEMORY;
    }
    return S_OK;
}

BOOL MainWindow::HitTest(float x, float y)
{
    for (auto i = screens.rbegin(); i != screens.rend(); ++i)
    {
        if ((*i)->HitTest(x, y))
        {
            selection = (++i).base();
            return TRUE;
        }
    }
    return FALSE;
}

void MainWindow::SetLocation(float x, float y)
{
    float boundedX = max(0, min(x, canvasWidth - Selection()->width));
    float boundedY = max(0, min(y, canvasHeight - Selection()->height));
    Selection()->x = boundedX;
    Selection()->y = boundedY;
}

void MainWindow::MoveSelection(float x, float y)
{
    SetLocation(Selection()->x + x, Selection()->y + y);
}
