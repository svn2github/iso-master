/******************************************************************************
* Windows main for ISO Master
* */

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdio.h>

#include "resource.h"
#include "error.h"
#include "window.h"

HINSTANCE GBLhInstance;

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    MSG msg;
	WNDCLASSEX  windowClass;
	HWND hwnd;
    
    clearWarningLog();

    GBLhInstance = hInstance;
    
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WndProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GBLhInstance;
	windowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	windowClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	windowClass.lpszClassName = L"ISOMasterWindowClass";
	windowClass.hIconSm = NULL;
	if (!RegisterClassEx(&windowClass))
		return 1;
	
    logWarning("window class registered");
    
    hwnd = CreateWindowEx(NULL, 
                          L"ISOMasterWindowClass", 
                          L"ISO Master",
                          WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_VISIBLE | WS_CAPTION | WS_SYSMENU, 
                          100,100,
                          400,400,
                          NULL,
                          NULL,
                          GBLhInstance,
                          NULL);
    if(!hwnd)
        return 2;
    
    ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

    logWarning("window created");
    
    while(GetMessage(&msg, NULL, 0, 0))
    {
        if(msg.message == WM_QUIT)
            break;
        else
        {
            TranslateMessage(&msg); 
            DispatchMessage(&msg);
        }
    }
    
    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static DWORD dwSplitterPos;
    static HWND topPane;
    static HWND bottomPane;
    static HCURSOR hCursor;
    static BOOL bSplitterMoving;
    RECT rect;
    
    switch(message)
    {
    case WM_CREATE:
        
        //createSimpleToolbar(hwnd);
        
        topPane = CreateWindowEx(WS_EX_CLIENTEDGE,
                                 L"edit", NULL,  
                                 WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | ES_MULTILINE | WS_VSCROLL, 
                                 0, 0, 0, 0, 
                                 hwnd, (HMENU)1,
                                 GBLhInstance, NULL);
        
        bottomPane = CreateWindowEx(WS_EX_CLIENTEDGE,
                                    L"edit", NULL,  
                                    WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | ES_MULTILINE | WS_VSCROLL, 
                                    0, 0, 0, 0, 
                                    hwnd, (HMENU)2,
                                    GBLhInstance, NULL);
        
        hCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENS));
        
        bSplitterMoving = FALSE;
        dwSplitterPos = 130;
        
        return 0;
    
    case WM_SIZE:
        
        //!! need some smarter way to deal with this to have minimum bottom height
        //!! and increase the size when window made bigger
        
        // If window is shrunk so that splitter now lies outside the 
        //  window boundaries, move the splitter within the window.
        if ((wParam != SIZE_MINIMIZED) && (HIWORD(lParam) < dwSplitterPos))
        {
            // but not if it would go above the top of the top pane
            if(HIWORD(lParam) - 10 > 20)
                dwSplitterPos = HIWORD(lParam) - 10;
        }
        
        // Adjust the children's size and position
        MoveWindow(topPane, 0, 0, LOWORD(lParam), dwSplitterPos - 1, TRUE);
        MoveWindow(bottomPane, 0, dwSplitterPos + 2, LOWORD(lParam) , HIWORD(lParam) - dwSplitterPos - 2, TRUE);
		
        return 0;

    case WM_COMMAND:
        
        if(wParam == ID_IMAGE_QUIT)
        {
            logWarning("quit");
            PostQuitMessage(1);
        }
        else if(wParam == ID_IMAGE_OPEN)
        {
            logWarning("open");
        }
        break;
        
    case WM_MOUSEMOVE:

        if (HIWORD(lParam) > 20) // do not allow above this mark
        {
            SetCursor(hCursor);
            if ((wParam == MK_LBUTTON) && bSplitterMoving)
            {
                GetClientRect(hwnd, &rect);
                if (HIWORD(lParam) > rect.bottom - 20) // or below this mark
                    return 0;

                dwSplitterPos = HIWORD(lParam);
                SendMessage(hwnd, WM_SIZE, 0, MAKELPARAM(rect.right, rect.bottom));
            }
        }
        return 0;
        
    case WM_LBUTTONDOWN:
    
        SetCursor(hCursor);
        bSplitterMoving = TRUE;
        SetCapture(hwnd);
        return 0;

    case WM_LBUTTONUP:
    
        ReleaseCapture();
        bSplitterMoving = FALSE;
        return 0;
		
    case WM_CLOSE: 

        PostQuitMessage(0);
        return 0;

    default:
        break;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}
