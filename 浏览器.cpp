#include <windows.h>
#include <string>
#include <vector>
#include <Shlwapi.h>
#include <shellapi.h>

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Shell32.lib")

// 全局变量
HWND hEdit, hButton, hCloseBtn;
const WCHAR* CLASS_NAME = L"BrowserWindowClass";

// 颜色定义
const COLORREF BG_COLOR = RGB(45, 45, 65);
const COLORREF BTN_COLOR = RGB(0, 120, 215);
const COLORREF BTN_HOVER_COLOR = RGB(0, 90, 180);
const COLORREF TEXT_COLOR = RGB(255, 255, 255);
const COLORREF EDIT_BG_COLOR = RGB(30, 30, 40);
const COLORREF EDIT_TEXT_COLOR = RGB(255, 255, 255);

// 手动定义 EM_SETCUEBANNER（MinGW 可能没有）
#if !defined(EM_SETCUEBANNER)
#define EM_SETCUEBANNER (ECM_FIRST + 1)
#endif

// 启动浏览器
void LaunchBrowser(const std::wstring& url) {
    ShellExecuteW(NULL, L"open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

// URL编码函数
std::wstring UrlEncode(const std::wstring& value) {
    std::wstring encoded;
    const wchar_t* hex = L"0123456789ABCDEF";
    
    for (wchar_t c : value) {
        if (iswalnum(c) || c == L'-' || c == L'_' || c == L'.' || c == L'~') {
            encoded += c;
        } else if (c == L' ') {
            encoded += L'+';
        } else {
            encoded += L'%';
            encoded += hex[(c >> 4) & 0x0F];
            encoded += hex[c & 0x0F];
        }
    }
    
    return encoded;
}

// 处理访问逻辑
void HandleAccess() {
    WCHAR buffer[1024] = {0};
    GetWindowTextW(hEdit, buffer, 1023);
    std::wstring input = buffer;

    if (!input.empty()) {
        std::wstring url;
        
        if (input.substr(0, 3) == L"ss:") {
            // Bing搜索
            std::wstring query = input.substr(3);
            url = L"https://www.bing.com/search?q=" + UrlEncode(query);
        } else if (input.find(L".") != std::wstring::npos) {
            // 直接访问URL
            if (input.find(L"://") == std::wstring::npos) {
                url = L"https://" + input;
            } else {
                url = input;
            }
        } else {
            // 普通搜索
            url = L"https://www.bing.com/search?q=" + UrlEncode(input);
        }
        
        LaunchBrowser(url);
        SetWindowTextW(hEdit, L"");
        SetFocus(hEdit);
    }
}

// 绘制圆角矩形
void DrawRoundRect(HDC hdc, int x, int y, int width, int height, int radius, HBRUSH brush) {
    HRGN hRgn = CreateRoundRectRgn(x, y, x + width, y + height, radius, radius);
    FillRgn(hdc, hRgn, brush);
    DeleteObject(hRgn);
}

// 绘制渐变背景
void DrawGradientBackground(HWND hwnd, HDC hdc) {
    RECT rc;
    GetClientRect(hwnd, &rc);
    
    TRIVERTEX vertex[2];
    vertex[0].x = 0;
    vertex[0].y = 0;
    vertex[0].Red = 30 << 8;
    vertex[0].Green = 30 << 8;
    vertex[0].Blue = 50 << 8;
    vertex[0].Alpha = 0;
    
    vertex[1].x = rc.right;
    vertex[1].y = rc.bottom;
    vertex[1].Red = 60 << 8;
    vertex[1].Green = 60 << 8;
    vertex[1].Blue = 80 << 8;
    vertex[1].Alpha = 0;
    
    GRADIENT_RECT gRect;
    gRect.UpperLeft = 0;
    gRect.LowerRight = 1;
    
    GdiGradientFill(hdc, vertex, 2, &gRect, 1, GRADIENT_FILL_RECT_V);
}

// 窗口过程
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static bool btnHover = false;
    
    switch (uMsg) {
        case WM_CREATE: {
            // 创建关闭按钮
            hCloseBtn = CreateWindowW(
                L"BUTTON",
                L"X",
                WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                475, 10, 35, 35,
                hwnd,
                (HMENU)3,
                ((LPCREATESTRUCT)lParam)->hInstance,
                nullptr
            );
            
            // 创建输入框
            hEdit = CreateWindowExW(
                WS_EX_CLIENTEDGE,
                L"EDIT",
                L"",
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_LEFT | WS_TABSTOP,
                20, 60, 460, 40,
                hwnd,
                (HMENU)1,
                ((LPCREATESTRUCT)lParam)->hInstance,
                nullptr
            );
            
            // 创建访问按钮
            hButton = CreateWindowW(
                L"BUTTON",
                L"Access / Search",
                WS_CHILD | WS_VISIBLE | BS_OWNERDRAW | WS_TABSTOP,
                20, 110, 460, 45,
                hwnd,
                (HMENU)2,
                ((LPCREATESTRUCT)lParam)->hInstance,
                nullptr
            );
            
            // 设置字体
            HFONT hFont = CreateFontW(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, 
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
            
            HFONT hIconFont = CreateFontW(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, 
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
            
            SendMessageW(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageW(hButton, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageW(hCloseBtn, WM_SETFONT, (WPARAM)hIconFont, TRUE);
            
            // 设置窗口样式
            SetLayeredWindowAttributes(hwnd, 0, 245, LWA_ALPHA);
            
            SetFocus(hEdit);
            break;
        }

        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, EDIT_TEXT_COLOR);
            SetBkColor(hdc, EDIT_BG_COLOR);
            static HBRUSH hEditBrush = CreateSolidBrush(EDIT_BG_COLOR);
            return (LRESULT)hEditBrush;
        }

        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT pDraw = (LPDRAWITEMSTRUCT)lParam;
            
            if (pDraw->CtlID == 2) { // 访问按钮
                HBRUSH hBrush = CreateSolidBrush(btnHover ? BTN_HOVER_COLOR : BTN_COLOR);
                DrawRoundRect(pDraw->hDC, pDraw->rcItem.left, pDraw->rcItem.top, 
                             pDraw->rcItem.right - pDraw->rcItem.left,
                             pDraw->rcItem.bottom - pDraw->rcItem.top, 20, hBrush);
                DeleteObject(hBrush);
                
                SetTextColor(pDraw->hDC, TEXT_COLOR);
                SetBkMode(pDraw->hDC, TRANSPARENT);
                
                DrawTextW(pDraw->hDC, L"Access / Search", -1, &pDraw->rcItem, 
                         DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                
                if (pDraw->itemState & ODS_FOCUS) {
                    DrawFocusRect(pDraw->hDC, &pDraw->rcItem);
                }
                return TRUE;
            }
            else if (pDraw->CtlID == 3) { // 关闭按钮
                HBRUSH hBrush = CreateSolidBrush(btnHover ? RGB(200, 50, 50) : RGB(150, 40, 40));
                FillRect(pDraw->hDC, &pDraw->rcItem, hBrush);
                DeleteObject(hBrush);
                
                SetTextColor(pDraw->hDC, TEXT_COLOR);
                SetBkMode(pDraw->hDC, TRANSPARENT);
                
                DrawTextW(pDraw->hDC, L"X", -1, &pDraw->rcItem, 
                         DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                return TRUE;
            }
            break;
        }

        case WM_MOUSEMOVE: {
            POINT pt = { LOWORD(lParam), HIWORD(lParam) };
            RECT btnRect;
            GetWindowRect(hButton, &btnRect);
            MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&btnRect, 2);
            
            bool oldHover = btnHover;
            btnHover = PtInRect(&btnRect, pt);
            
            if (oldHover != btnHover) {
                InvalidateRect(hButton, NULL, TRUE);
            }
            
            // 设置手型光标
            if (btnHover) {
                SetCursor(LoadCursorW(NULL, (LPCWSTR)IDC_HAND));
            }
            break;
        }

        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case 2:
                    HandleAccess();
                    break;
                case 3:
                    SendMessageW(hwnd, WM_CLOSE, 0, 0);
                    break;
            }
            break;
        }

        case WM_KEYDOWN: {
            if (wParam == VK_RETURN) {
                HandleAccess();
            }
            else if (wParam == VK_ESCAPE) {
                SendMessageW(hwnd, WM_CLOSE, 0, 0);
            }
            break;
        }

        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // 绘制渐变背景
            DrawGradientBackground(hwnd, hdc);
            
            // 绘制标题
            SetTextColor(hdc, TEXT_COLOR);
            SetBkMode(hdc, TRANSPARENT);
            HFONT hTitleFont = CreateFontW(24, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, 
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
            HFONT hOldFont = (HFONT)SelectObject(hdc, hTitleFont);
            
            RECT titleRect = {20, 15, 500, 50};
            DrawTextW(hdc, L"Devin 浏览器", -1, &titleRect, DT_LEFT | DT_SINGLELINE);
            
            SelectObject(hdc, hOldFont);
            DeleteObject(hTitleFont);
            
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_NCHITTEST: {
            // 允许通过拖动标题栏移动窗口
            LRESULT hit = DefWindowProcW(hwnd, uMsg, wParam, lParam);
            if (hit == HTCLIENT) hit = HTCAPTION;
            return hit;
        }

        default:
            return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

// 注册窗口类
BOOL RegisterWindowClass(HINSTANCE hInstance) {
    WNDCLASSEXW wc = {0};
    wc.cbSize        = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.hCursor       = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
    wc.hIcon         = LoadIconW(NULL, (LPCWSTR)IDI_APPLICATION);
    
    return RegisterClassExW(&wc) != 0;
}

// 居中窗口
void CenterWindow(HWND hwnd) {
    RECT rc;
    GetWindowRect(hwnd, &rc);
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;
    
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    
    int x = (screenWidth - width) / 2;
    int y = (screenHeight - height) / 3;
    
    SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 检查是否已有实例运行
    HANDLE hMutex = CreateMutexW(NULL, TRUE, L"BrowserLauncherMutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBoxW(NULL, L"Browser Launcher is already running!", L"Info", MB_ICONINFORMATION);
        return 0;
    }

    if (!RegisterWindowClass(hInstance)) {
        MessageBoxW(NULL, L"Window class registration failed!", L"Error", MB_ICONERROR);
        return 1;
    }

    // 创建窗口
    HWND hwnd = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TOPMOST,
        CLASS_NAME,
        L"Devin 浏览器",
        WS_POPUP | WS_VISIBLE,
        0, 0, 520, 180,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        MessageBoxW(NULL, L"Window creation failed!", L"Error", MB_ICONERROR);
        return 1;
    }

    // 设置圆角窗口
    HRGN hRgn = CreateRoundRectRgn(0, 0, 520, 180, 20, 20);
    SetWindowRgn(hwnd, hRgn, TRUE);

    CenterWindow(hwnd);
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // 消息循环
    MSG msg = {0};
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
    
    return 0;
}