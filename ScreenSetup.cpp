/*
* Defines the entry point for the application.
*/

#include "framework.h"
#include "MainWindow.h"
#include "ScreenSetup.h"

/*******************************************************************************
 * Function Declarations :
 ******************************************************************************/
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

/*******************************************************************************
* Global Variables :
*******************************************************************************/
float canvasWidthDP  = -1.f; // display pixel height of the canvas the monitors will be placed on
float canvasHeightDP = -1.f; // display pixel width of the canvas the monitors will be placed on

/*******************************************************************************
* 
*******************************************************************************/

HRESULT MainWindow::CreateGraphicsResources()
{
    HRESULT hr = S_OK;
    if (pRenderTarget == NULL)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

        hr = pFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &pRenderTarget);

        if (SUCCEEDED(hr))
        {
            const D2D1_COLOR_F color = D2D1::ColorF(1.0f, 1.0f, 0);
            hr = pRenderTarget->CreateSolidColorBrush(color, &pBrush);
        }
    }

    // text code based on https://docs.microsoft.com/en-us/windows/win32/Direct2D/how-to--draw-text
    if (pTextFormat == NULL)
    {
        // Create a DirectWrite factory.
        hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(pDWriteFactory),
            reinterpret_cast<IUnknown**>(&pDWriteFactory)
        );
        if (SUCCEEDED(hr))
        {
            // Create a DirectWrite text format object.
            hr = pDWriteFactory->CreateTextFormat(
                msc_fontName,
                NULL,
                DWRITE_FONT_WEIGHT_NORMAL,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                msc_fontSize,
                L"", //locale
                &pTextFormat
            );
        }
        if (SUCCEEDED(hr))
        {
            // Center the text horizontally and vertically.
            pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
            pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
        }
    }
    return hr;
}

void MainWindow::DiscardGraphicsResources()
{
    SafeRelease(&pRenderTarget);
    SafeRelease(&pBrush);
    SafeRelease(&pTextFormat);
    SafeRelease(&pDWriteFactory);
}

void MainWindow::DrawScreen(ID2D1RenderTarget* pRT, ID2D1SolidColorBrush* pBrush, ScreenInfo screen, boolean sel)
{
    // get scaling factor
    FLOAT scaleX = static_cast<float>(canvasWidthDP) / canvasWidth;
    FLOAT scaleY = static_cast<float>(canvasHeightDP) / canvasHeight;

    // create rectangle for drawing scaled to canvas pixels
    D2D1_RECT_F rect = D2D1::RectF(
        scaleX * screen.x,
        scaleY * screen.y,
        scaleX * (screen.x + screen.width),
        scaleY * (screen.y + screen.height)
    );

    // set rotation
    pRT->SetTransform(D2D1::Matrix3x2F::Rotation(screen.angle, D2D1::Point2F(screen.x, screen.y)));

    pBrush->SetColor(fillColor);
    pRT->FillRectangle(rect, pBrush); // draw rectangle
    if (sel)
    {
        pBrush->SetColor(selectColor);
    }
    else
    {
        pBrush->SetColor(borderColor);
    }
    pRT->DrawRectangle(rect, pBrush, 1.0f); // draw border
}

int MainWindow::GetSidePanelText(wchar_t* buf, int len)
{
    int n;
    if (Selection())
    {
        n  = swprintf(buf    , len    , L"Selected Screen:\n");
        n += swprintf(buf + n, len - n, L"x: %f\ny: %f\n", Selection()->x, Selection()->y);
        n += swprintf(buf + n, len - n, L"width: %d\nheight: %d\n", Selection()->width, Selection()->height);
        n += swprintf(buf + n, len - n, L"rotation:%f\n", Selection()->angle);
    }
    else
    {
        n = swprintf(buf, len, L"Selected Screen:\nx:\ny:\nwidth:\nheight:\nrotation:\n");
    }
    
    return n;
}

void MainWindow::OnPaint()
{
    HRESULT hr = CreateGraphicsResources();
    if (SUCCEEDED(hr))
    {
        // make sure canvas DP dimensions are set
        if (canvasWidthDP < 0 || canvasHeightDP < 0)
        {
            Resize();
        }

        PAINTSTRUCT ps;
        BeginPaint(m_hwnd, &ps);
        pRenderTarget->BeginDraw();

        // draw the canvas
        pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
        D2D1_RECT_F canvas = D2D1::RectF(0, 0, canvasWidthDP, canvasHeightDP);
        pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::SkyBlue));
        pRenderTarget->FillRectangle(&canvas, pBrush);

        // draw screens
        for (auto i = screens.begin(); i != screens.end(); ++i)
        {
            DrawScreen(pRenderTarget, pBrush, **i, false);
        }

        if (Selection())
        {
            DrawScreen(pRenderTarget, pBrush, *Selection(), true);
        }

        // draw side panel
        
        // calculate side panel dimensions
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_RECT_F sidePanel = D2D1::RectF(
            rc.right * (MAX_CANVAS_WIDTH_PERCENT + sidePadding),
            rc.bottom * sidePadding,
            rc.right * (1 - sidePadding),
            rc.bottom * (1 - sidePadding)
        );

        // get text to draw
        const int max_len = 2048;
        wchar_t text[max_len];
        int len = GetSidePanelText(text, max_len);

        // draw the text
        pBrush->SetColor(fontColor);
        pRenderTarget->DrawText(text, len, pTextFormat, sidePanel, pBrush);

        hr = pRenderTarget->EndDraw();
        if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
        {
            DiscardGraphicsResources();
        }
        EndPaint(m_hwnd, &ps);
    }
}

void MainWindow::Resize()
{
    if (pRenderTarget != NULL)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

        pRenderTarget->Resize(size);

        // resize the canvas
        D2D1_SIZE_F rtSize = pRenderTarget->GetSize();
        if ((MAX_CANVAS_WIDTH_PERCENT * rtSize.width / rtSize.height) < (canvasWidth / static_cast<float>(canvasHeight)))
        {
            canvasWidthDP = MAX_CANVAS_WIDTH_PERCENT * rtSize.width;
            canvasHeightDP = canvasWidthDP * (canvasHeight / static_cast<float>(canvasWidth));
        }
        else
        {
            canvasHeightDP = rtSize.height;
            canvasWidthDP = canvasHeightDP * (canvasWidth / static_cast<float>(canvasHeight));
        }

        InvalidateRect(m_hwnd, NULL, FALSE);
    }
}

void MainWindow::OnLButtonDown(int pixelX, int pixelY, DWORD flags)
{
    if (pixelX <= canvasWidthDP && pixelY <= canvasHeightDP)
    {
        // get scaling factor
        FLOAT scaleX = static_cast<float>(canvasWidth) / canvasWidthDP;
        FLOAT scaleY = static_cast<float>(canvasHeight) / canvasHeightDP;

        // get coords in terms of canvas pixel
        FLOAT x = scaleX * pixelX;
        FLOAT y = scaleY * pixelY;

        ClearSelection();

        if (mode == Mode::Select && HitTest(x, y))
        {
            SetMode(Mode::Move);
            SetCapture(m_hwnd);

            ptMouse = D2D1::Point2F(Selection()->x, Selection()->y);
            ptMouse.x -= x;
            ptMouse.y -= y;
        }
        else
        {
            InsertScreen(x, y);
        }
    }
    InvalidateRect(m_hwnd, NULL, FALSE);
}

void MainWindow::OnLButtonUp()
{
    SetMode(Mode::Select);
    ReleaseCapture();
}

void MainWindow::OnMouseMove(int pixelX, int pixelY, DWORD flags)
{
    // get scaling factor
    FLOAT scaleX = static_cast<float>(canvasWidth) / canvasWidthDP;
    FLOAT scaleY = static_cast<float>(canvasHeight) / canvasHeightDP;

    // get coords in terms of canvas pixel
    FLOAT x = scaleX * pixelX;
    FLOAT y = scaleY * pixelY;

    if (mode == Mode::Move && (flags & MK_LBUTTON) && Selection())
    { 
        // Move the screen.
        SetLocation(x + ptMouse.x, y + ptMouse.y);
        InvalidateRect(m_hwnd, NULL, FALSE);
    }
}

void MainWindow::OnKeyDown(UINT vkey)
{
    switch (vkey)
    {
    case VK_ESCAPE:
        if (Selection())
        {
            ClearSelection();
        }
    case VK_BACK:
    case VK_DELETE:
        if (Selection())
        {
            screens.erase(selection);
            ClearSelection();
        };
        break;
    case VK_LEFT:
        MoveSelection(-1, 0);
        break;
    case VK_RIGHT:
        MoveSelection(1, 0);
        break;

    case VK_UP:
        MoveSelection(0, -1);
        break;

    case VK_DOWN:
        MoveSelection(0, 1);
        break;
    }
    InvalidateRect(m_hwnd, NULL, FALSE);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize window
    MainWindow win;

    // Perform application initialization:
    if (!win.Create(L"Set Screen Setup", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN))
    {
        return FALSE;
    }

    UpdateWindow(win.Window());

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SCREENSETUP));

    ShowWindow(win.Window(), nCmdShow);

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(win.Window(), hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        if (FAILED(D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory)))
        {
            OutputDebugString(L"Failed creating factory.");
            return -1;  // Fail CreateWindowEx.
        }
        SetMode(Mode::Select);

        wchar_t buf[10];
        swprintf(buf, 10, L"%d", canvasWidth);
        canvasWidth_hwnd = CreateWindowEx(NULL, L"EDIT", buf,
            WS_BORDER | WS_CHILD | WS_VISIBLE | WS_EX_TOPMOST,
            10, 10, 200, 20,
            m_hwnd, NULL, NULL, NULL);
        swprintf(buf, 10, L"%d", canvasHeight);
        canvasHeight_hwnd = CreateWindowEx(NULL, L"EDIT", buf,
            WS_BORDER | WS_CHILD | WS_VISIBLE | WS_EX_TOPMOST,
            10, 40, 200, 20,
            m_hwnd, NULL, NULL, NULL);
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUTBOX), m_hwnd, About);
                break;
            case IDM_EXIT:
                DiscardGraphicsResources();
                SafeRelease(&pFactory);
                PostQuitMessage(0);
                break;
            default:
                return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        OnPaint();
        break;
    case WM_SIZE:
        Resize();
        break;
    case WM_LBUTTONDOWN:
        OnLButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam);
        break;
    case WM_LBUTTONUP:
        OnLButtonUp();
        break;
    case WM_MOUSEMOVE:
        OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam);
        break;
    case WM_SETCURSOR:
        if (LOWORD(lParam) == HTCLIENT)
        {
            SetCursor(hCursor);
            return TRUE;
        }
        break;
    case WM_KEYDOWN:
        OnKeyDown((UINT)wParam);
        break;
    case WM_DESTROY:
        DiscardGraphicsResources();
        SafeRelease(&pFactory);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}