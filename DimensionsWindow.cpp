#include "DimensionsWindow.h"

HRESULT DimensionsWindow::CreateGraphicsResources()
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
    return hr;
}

void DimensionsWindow::DiscardGraphicsResources()
{
    SafeRelease(&pRenderTarget);
    SafeRelease(&pBrush);
}

void DimensionsWindow::OnPaint()
{
    HRESULT hr = CreateGraphicsResources();
    if (SUCCEEDED(hr))
    {
        PAINTSTRUCT ps;
        BeginPaint(m_hwnd, &ps);
        pRenderTarget->BeginDraw();

        // draw the canvas
        pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

        hr = pRenderTarget->EndDraw();
        if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
        {
            DiscardGraphicsResources();
        }
        EndPaint(m_hwnd, &ps);
    }
}

void DimensionsWindow::OnKeyDown(UINT vkey)
{
}

LRESULT DimensionsWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
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
        width_hwnd = CreateWindowEx(NULL, L"EDIT", L"xxxx",
            WS_BORDER | WS_CHILD | WS_VISIBLE | WS_EX_TOPMOST,
            10, 10, 200, 20,
            m_hwnd, NULL, NULL, NULL);
        apply_hwnd = CreateWindowEx(NULL, L"BUTTON", L"Apply",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            4, 52, 100, 30,
            m_hwnd, NULL, NULL, NULL);
        break;
    break;
    case WM_COMMAND:
        break;
    case WM_PAINT:
        OnPaint();
        break;
    case WM_KEYDOWN:
        OnKeyDown((UINT)wParam);
        break;
    case WM_CLOSE:
        ShowWindow(m_hwnd, SW_HIDE);
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
