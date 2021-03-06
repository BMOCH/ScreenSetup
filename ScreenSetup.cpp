/*
* Defines the entry point for the application.
*/
#include "MainWindow.h"
#include "ScreenSetup.h"

/*******************************************************************************
 * Function Declarations :
 ******************************************************************************/

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

// creates a custom textbox used for the sidebar
HWND CreateCustomTextbox(HINSTANCE hInstance, LPCWSTR text, HWND hParent, DWORD dwStyle, UINT id);

/*******************************************************************************
* Global Variables :
*******************************************************************************/
static WNDPROC OriginalEditCtrlProc = NULL;


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
    pRT->SetTransform(D2D1::Matrix3x2F::Rotation(screen.angle, D2D1::Point2F(scaleX * screen.x, scaleY * screen.y)));

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
    n  = swprintf(buf    , len    , L"Canvas Dimensions:\n");
    n += swprintf(buf + n, len - n, L"width:\nheight:\n");
    n += swprintf(buf + n, len - n, L"Selected Screen:\nwidth:\nheight:\nx:\ny:\nrotation:\n");
    
    return n;
}

void MainWindow::UpdateSidePanel()
{
    if (Selection())
    {
        EnableWindow(screenWidth_hwnd, true);
        EnableWindow(screenHeight_hwnd, true);
        EnableWindow(screenX_hwnd, true);
        EnableWindow(screenY_hwnd, true);
        EnableWindow(screenRot_hwnd, true);

        SetWindowText(screenWidth_hwnd, to_wstring(Selection()->width).c_str());
        SetWindowText(screenHeight_hwnd, to_wstring(Selection()->height).c_str());
        SetWindowText(screenX_hwnd, to_wstring(static_cast<int>(roundf(Selection()->x))).c_str());
        SetWindowText(screenY_hwnd, to_wstring(static_cast<int>(roundf(Selection()->y))).c_str());
        SetWindowText(screenRot_hwnd, to_wstring(static_cast<int>(roundf(Selection()->angle))).c_str());
    }
    else
    {
        EnableWindow(screenWidth_hwnd, false);
        EnableWindow(screenHeight_hwnd, false);
        EnableWindow(screenX_hwnd, false);
        EnableWindow(screenY_hwnd, false);
        EnableWindow(screenRot_hwnd, false);

        SetWindowText(screenWidth_hwnd, L"");
        SetWindowText(screenHeight_hwnd, L"");
        SetWindowText(screenX_hwnd, L"");
        SetWindowText(screenY_hwnd, L"");
        SetWindowText(screenRot_hwnd, L"");
    }
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

        // canvas
        pRenderTarget->Clear(backgroundColor);
        pRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation(0));
        D2D1_RECT_F canvas = D2D1::RectF(0, 0, canvasWidthDP, canvasHeightDP);
        pBrush->SetColor(canvasColor);
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
        pRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation(0));

        /* side panel */
        
        // calculate side panel dimensions
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_RECT_F sidePanel = D2D1::RectF(
            rc.right * (MAX_CANVAS_WIDTH_PERCENT + SIDE_PADDING),
            rc.bottom * SIDE_PADDING,
            rc.right * (1 - SIDE_PADDING),
            rc.bottom * (1 - SIDE_PADDING)
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

        // move side panel boxes
        int paddingWidth = static_cast<int>(SIDE_PADDING * rtSize.width);
        int paddingHeight = static_cast<int>(SIDE_PADDING * rtSize.height);
        int betweenHeight = 5;
        int boxHeight = static_cast<int>(msc_fontSize);
        int boxWidth = min(MAX_TEXTBOX_WIDTH, static_cast<int>((1 - MAX_CANVAS_WIDTH_PERCENT) * rtSize.width) - 2 * paddingWidth);

        MoveWindow(canvasWidth_hwnd, static_cast<int>(rtSize.width) - paddingWidth - boxWidth, paddingHeight + 1 * (boxHeight + betweenHeight), boxWidth, 20, false);
        InvalidateRect(canvasWidth_hwnd , NULL, FALSE);

        MoveWindow(canvasHeight_hwnd, static_cast<int>(rtSize.width) - paddingWidth - boxWidth, paddingHeight + 2 * (boxHeight + betweenHeight), boxWidth, 20, false);
        InvalidateRect(canvasHeight_hwnd, NULL, FALSE);

        MoveWindow(screenWidth_hwnd, static_cast<int>(rtSize.width) - paddingWidth - boxWidth, paddingHeight + 4 * (boxHeight + betweenHeight), boxWidth, 20, false);
        InvalidateRect(screenWidth_hwnd, NULL, FALSE);

        MoveWindow(screenHeight_hwnd, static_cast<int>(rtSize.width) - paddingWidth - boxWidth, paddingHeight + 5 * (boxHeight + betweenHeight), boxWidth, 20, false);
        InvalidateRect(screenHeight_hwnd, NULL, FALSE);

        MoveWindow(screenX_hwnd, static_cast<int>(rtSize.width) - paddingWidth - boxWidth, paddingHeight + 6 * (boxHeight + betweenHeight), boxWidth, 20, false);
        InvalidateRect(screenX_hwnd, NULL, FALSE);

        MoveWindow(screenY_hwnd, static_cast<int>(rtSize.width) - paddingWidth - boxWidth, paddingHeight + 7 * (boxHeight + betweenHeight), boxWidth, 20, false);
        InvalidateRect(screenY_hwnd, NULL, FALSE);

        MoveWindow(screenRot_hwnd, static_cast<int>(rtSize.width) - paddingWidth - boxWidth, paddingHeight + 8 * (boxHeight + betweenHeight), boxWidth, 20, false);
        InvalidateRect(screenRot_hwnd, NULL, FALSE);

        InvalidateRect(m_hwnd, NULL, FALSE);
    }
}

void MainWindow::OnLButtonDown(int dpX, int dpY, DWORD flags)
{
    SetFocus(m_hwnd);
    if (dpX <= canvasWidthDP && dpY <= canvasHeightDP)
    {
        // get scaling factor
        FLOAT scaleX = static_cast<float>(canvasWidth) / canvasWidthDP;
        FLOAT scaleY = static_cast<float>(canvasHeight) / canvasHeightDP;

        // get coords in terms of canvas pixel
        FLOAT x = scaleX * dpX;
        FLOAT y = scaleY * dpY;

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
    UpdateSidePanel();
    InvalidateRect(m_hwnd, NULL, FALSE);
}

void MainWindow::OnLButtonUp()
{
    SetMode(Mode::Select);
    ReleaseCapture();
    UpdateSidePanel();
}

void MainWindow::OnMouseMove(int dpX, int dpY, DWORD flags)
{
    // get scaling factor
    FLOAT scaleX = static_cast<float>(canvasWidth) / canvasWidthDP;
    FLOAT scaleY = static_cast<float>(canvasHeight) / canvasHeightDP;

    // get coords in terms of canvas pixel
    FLOAT x = scaleX * dpX;
    FLOAT y = scaleY * dpY;

    if (mode == Mode::Move && (flags & MK_LBUTTON) && Selection())
    { 
        // Move the screen.
        int prevX = static_cast<int>(round(Selection()->x));
        int prevY = static_cast<int>(round(Selection()->y));
        SetLocation(x + ptMouse.x, y + ptMouse.y);
        if (prevX != round(Selection()->x))
        {
            SetWindowText(screenX_hwnd, to_wstring(static_cast<int>(roundf(Selection()->x))).c_str());
        }
        if (prevY != round(Selection()->y))
        {
            SetWindowText(screenY_hwnd, to_wstring(static_cast<int>(roundf(Selection()->y))).c_str());
        }
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
        }
        break;
    case VK_LEFT:
        if (Selection())
        {
            MoveSelection(-1, 0);
        }
        break;
    case VK_RIGHT:
        if (Selection())
        {
            MoveSelection(1, 0);
        }
        break;

    case VK_UP:
        if (Selection())
        {
            MoveSelection(0, -1);
        }
        break;

    case VK_DOWN:
        if (Selection())
        {
            MoveSelection(0, 1);
        }
        break;
    }
    UpdateSidePanel();
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
        if (!TranslateAccelerator(win.Window(), hAccelTable, &msg) && !IsDialogMessage(win.Window(), &msg))
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
        {
        if (FAILED(D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory)))
        {
            OutputDebugString(L"Failed creating factory.");
            return -1;  // Fail CreateWindowEx.
        }
        SetMode(Mode::Select);

        wchar_t buf[10];
        swprintf(buf, 10, L"%d", canvasWidth);
        canvasWidth_hwnd = CreateCustomTextbox(GetModuleHandle(NULL), buf, m_hwnd, WS_GROUP, ID_CANVAS_WIDTH);
        swprintf(buf, 10, L"%d", canvasHeight);
        canvasHeight_hwnd = CreateCustomTextbox(GetModuleHandle(NULL), buf, m_hwnd, 0, ID_CANVAS_HEIGHT);
        screenWidth_hwnd = CreateCustomTextbox(GetModuleHandle(NULL), L"", m_hwnd, WS_DISABLED, ID_SCREEN_WIDTH);
        screenHeight_hwnd = CreateCustomTextbox(GetModuleHandle(NULL), L"", m_hwnd, WS_DISABLED, ID_SCREEN_HEIGHT);
        screenX_hwnd = CreateCustomTextbox(GetModuleHandle(NULL), L"", m_hwnd, WS_DISABLED, ID_SCREEN_X);
        screenY_hwnd = CreateCustomTextbox(GetModuleHandle(NULL), L"", m_hwnd, WS_DISABLED, ID_SCREEN_Y);
        screenRot_hwnd = CreateCustomTextbox(GetModuleHandle(NULL), L"", m_hwnd, WS_DISABLED, ID_SCREEN_ROTATION);
        }
        break;
    case WM_INITDIALOG:
        {
        LOGFONT logfont;
            ZeroMemory(&logfont, sizeof(LOGFONT));
            logfont.lfCharSet = DEFAULT_CHARSET;
            logfont.lfHeight = static_cast<LONG>(msc_fontSize);
            HFONT hFont = CreateFontIndirect(&logfont);
            SendMessage(m_hwnd, WM_SETFONT, (WPARAM)hFont, TRUE);
        }
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
                SendMessage(m_hwnd, WM_DESTROY, TRUE, TRUE);
                break;
            case ID_CANVAS_WIDTH: // canvas width
                if (HIWORD(wParam) == EN_UPDATE)
                {
                    vector<wchar_t> buf(GetWindowTextLength(canvasWidth_hwnd) + 1);
                    GetWindowText(canvasWidth_hwnd, &buf[0], 10);
                    if (wcslen(&buf[0]) != 0)
                    {
                        canvasWidth = _wtoi(&buf[0]);
                        Resize();
                    }
                }
                break;
            case ID_CANVAS_HEIGHT: // canvas height
                if (HIWORD(wParam) == EN_CHANGE)
                {
                    vector<wchar_t> buf(GetWindowTextLength(canvasHeight_hwnd) + 1);
                    GetWindowText(canvasHeight_hwnd, &buf[0], 10);
                    if (wcslen(&buf[0]) != 0)
                    {
                        canvasHeight = _wtoi(&buf[0]);
                        Resize();
                    }
                }
                break;
            case ID_SCREEN_WIDTH: // screen width
                if (HIWORD(wParam) == EN_CHANGE)
                {
                    vector<wchar_t> buf(GetWindowTextLength(screenWidth_hwnd) + 1);
                    GetWindowText(screenWidth_hwnd, &buf[0], 10);
                    if (wcslen(&buf[0]) != 0)
                    {
                        Selection()->width = _wtoi(&buf[0]);
                        Resize();
                    }
                }
                break;
            case ID_SCREEN_HEIGHT: // screen height
                if (HIWORD(wParam) == EN_CHANGE)
                {
                    vector<wchar_t> buf(GetWindowTextLength(screenHeight_hwnd) + 1);
                    GetWindowText(screenHeight_hwnd, &buf[0], 10);
                    if (wcslen(&buf[0]) != 0)
                    {
                        Selection()->height = _wtoi(&buf[0]);
                        Resize();
                    }
                }
                break;
            case ID_SCREEN_X: // screen x
                if (HIWORD(wParam) == EN_CHANGE)
                {
                    vector<wchar_t> buf(GetWindowTextLength(screenX_hwnd) + 1);
                    GetWindowText(screenX_hwnd, &buf[0], 10);
                    if (wcslen(&buf[0]) != 0)
                    {
                        SetLocation(static_cast<float>(_wtof(&buf[0])), Selection()->y);
                        Resize();
                    }
                }
                break;
            case ID_SCREEN_Y: // screen y
                if (HIWORD(wParam) == EN_CHANGE)
                {
                    vector<wchar_t> buf(GetWindowTextLength(screenY_hwnd) + 1);
                    GetWindowText(screenY_hwnd, &buf[0], 10);
                    if (wcslen(&buf[0]) != 0)
                    {
                        SetLocation(Selection()->x, static_cast<float>(_wtof(&buf[0])));
                        Resize();
                    }
                }
                break;
            case ID_SCREEN_ROTATION: // screen rotation
                if (HIWORD(wParam) == EN_CHANGE)
                {
                    vector<wchar_t> buf(GetWindowTextLength(screenRot_hwnd) + 1);
                    GetWindowText(screenRot_hwnd, &buf[0], 10);
                    if (wcslen(&buf[0]) != 0)
                    {
                        SetAngle(static_cast<float>(_wtof(&buf[0])));
                        Resize();
                    }
                }
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
    case WM_GETDLGCODE:
        return DLGC_WANTARROWS | DLGC_WANTMESSAGE;
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

// custom textbox based off https://stackoverflow.com/a/16639814
LRESULT CALLBACK CustomTextboxProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    if (uMsg == WM_CHAR)
    {
        // Make sure we only allow specific characters
        if (!((wParam >= '0' && wParam <= '9')
            || !(wParam == 'a' && GetAsyncKeyState(VK_CONTROL))
            || wParam == VK_TAB
            || wParam == VK_RETURN
            || wParam == VK_DELETE
            || wParam == VK_BACK))
        {
            return 0;
        }
    }

    return CallWindowProc(OriginalEditCtrlProc, hwnd, uMsg, wParam, lParam);
}

HWND CreateCustomTextbox(HINSTANCE hInstance, LPCWSTR text, HWND hParent, DWORD dwStyle, UINT id)
{
    HWND hwnd;

    hwnd = CreateWindowEx(
        NULL,
        _T("EDIT"),
        text,
        WS_BORDER | WS_CHILD | WS_VISIBLE | WS_EX_TOPMOST | WS_TABSTOP | dwStyle,
        0, 0, 0, 0,
        hParent,
        reinterpret_cast<HMENU>(id),
        hInstance,
        NULL);
    if (hwnd != NULL)
    {
        // Subclass the window so we can filter keystrokes
        WNDPROC oldProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(
            hwnd,
            GWLP_WNDPROC,
            reinterpret_cast<LONG_PTR>(CustomTextboxProc)));
        if (OriginalEditCtrlProc == NULL)
        {
            OriginalEditCtrlProc = oldProc;
        }
    }
    return hwnd;
}
