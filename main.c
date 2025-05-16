#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <stdbool.h>
#include <winuser.h>
#include <windowsx.h>
#include <wingdi.h>

#define CLAMP(x) ((x) < 0 ? 0 : ((x) > 255 ? 255 : (x))) 

typedef enum {
    FREEHAND,
    ERASE,
    RECTANGLE,
    ELLIPSE
} editorSetting; // uh no

HWND subwinHdl;

void freehandDraw(HDC hdc, HWND hwnd, POINT prevPoint, LPARAM lparam) {
    MoveToEx(hdc, prevPoint.x, prevPoint.y, NULL);
    LineTo(hdc, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
    ReleaseDC(hwnd, hdc);
}

HWND createTextBox(HWND parent, HINSTANCE hInstance, int x, int y, const char *name) {
    return CreateWindowExA(0, "Edit", name, WS_CHILD | WS_VISIBLE | ES_NUMBER, x, y, 100, 30, parent, NULL, hInstance, NULL);
}

void setupSubWindow(HINSTANCE hInstance) {
    HWND widthTextBox = createTextBox(subwinHdl, hInstance, CW_USEDEFAULT, CW_USEDEFAULT, "Width");

    HWND redTextBox = createTextBox(subwinHdl, hInstance, 100, 50, "R");
    HWND greenTextBox = createTextBox(subwinHdl, hInstance, 200, 50, "G");
    HWND blueTextBox = createTextBox(subwinHdl, hInstance, 300, 50, "B");
}

COLORREF getColor() {
    HWND hwndRed = FindWindowExA(subwinHdl, NULL, NULL, "R");
    HWND hwndGreen = FindWindowExA(subwinHdl, NULL, NULL, "G");
    HWND hwndBlue = FindWindowExA(subwinHdl, NULL, NULL, "B");

    if (hwndRed && hwndGreen && hwndBlue) {
        char rbuf[4], gbuf[4], bbuf[4];
        GetWindowTextA(hwndRed, rbuf, 4);
        GetWindowTextA(hwndGreen, gbuf, 4);
        GetWindowTextA(hwndBlue, bbuf, 4);
        
        int r = CLAMP(atoi(rbuf));
        int g = CLAMP(atoi(gbuf));
        int b = CLAMP(atoi(bbuf));

        return RGB(r, g, b);
    }

    return RGB(0, 0, 0);
    
}

int getPenWidth() {
    HWND hwndWidth = FindWindowExA(subwinHdl, NULL, NULL, "Width");
    if (hwndWidth) {
        char buf[3];
        GetWindowTextA(hwndWidth, buf, 3);
        return CLAMP(atoi(buf));
    }

    return 1;
}

HPEN setupPen() {
    COLORREF penColor = getColor();
    int penWidth = getPenWidth();

    HPEN pen = CreatePen(PS_SOLID, penWidth, penColor);
    return pen;
}

LRESULT CALLBACK windProc(HWND hwnd, UINT uMsg, WPARAM wparam, LPARAM lparam) {
    static POINT prevPoint = {0, 0};
    static bool drawing = false;

    switch (uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        case WM_LBUTTONDOWN: {
            prevPoint.x = GET_X_LPARAM(lparam);
            prevPoint.y = GET_Y_LPARAM(lparam);
            drawing = true;
            break;
        }
        case WM_LBUTTONUP:
            drawing = false;
            break;
        case WM_MOUSEMOVE: {
            if (drawing) {
                RECT rect;
                GetWindowRect(subwinHdl, &rect);

                POINT mousePos = {GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam)};
                ClientToScreen(hwnd, &mousePos);

                if (mousePos.x >= rect.left && mousePos.x <= rect.right && mousePos.y >= rect.top && mousePos.y <= rect.bottom) {
                    break;
                }

                HDC hdc = GetDC(hwnd);
                HPEN pen = setupPen();

                SelectObject(hdc, pen);
                
                //MoveToEx(hdc, prevPoint.x, prevPoint.y, NULL);
                //LineTo(hdc, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
                //ReleaseDC(hwnd, hdc);

                freehandDraw(hdc, hwnd, prevPoint, lparam);

                prevPoint.x = GET_X_LPARAM(lparam);
                prevPoint.y = GET_Y_LPARAM(lparam);
            }

            break;
        }
        default:
            return DefWindowProc(hwnd, uMsg, wparam, lparam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEXA winCInfo = {0};
    winCInfo.cbSize = sizeof(WNDCLASSEXA);
    winCInfo.lpfnWndProc = windProc;
    winCInfo.hInstance = hInstance;
    winCInfo.lpszClassName = "WindowClass";

    if (!RegisterClassExA(&winCInfo)) {
        MessageBox(NULL, "FUCK YOUUU", "ERROR", MB_ICONERROR);
        return -1;
    }

    HWND windowHdl = CreateWindowExA(
        WS_EX_CLIENTEDGE, "WindowClass", "SCanvas", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 
        800, 600, NULL, NULL, hInstance, NULL
    );

    subwinHdl = CreateWindowExA(
        0, "Static", "Tools", WS_CHILD | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 
        800, 100, windowHdl, NULL, hInstance, NULL
    );

    setupSubWindow(hInstance);

    ShowWindow(windowHdl, nCmdShow);
    UpdateWindow(windowHdl);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}
