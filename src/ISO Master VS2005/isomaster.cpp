/******************************************************************************
* Windows main for ISO Master
* */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    MSG msg;
	WNDCLASSEX  windowClass;
	HWND hwnd;
    
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WndProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = hInstance;
	windowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	windowClass.lpszMenuName = NULL;
	windowClass.lpszClassName = L"ISOMasterWindowClass";
	windowClass.hIconSm = NULL;
	if (!RegisterClassEx(&windowClass))
		return 1;
	
    hwnd = CreateWindowEx(NULL, 
                          L"ISOMasterWindowClass", 
                          L"ISO Master",
                          WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CAPTION | WS_SYSMENU, 
                          100,100,
                          400,400,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);
    if(!hwnd)
        return 2;
    
    while(1)
    {
        PeekMessage(&msg, hwnd, NULL, NULL, PM_REMOVE);
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
    PAINTSTRUCT paintStruct;

    switch(message)
    {
        case WM_CREATE: 
            return 0;
            break;
        case WM_CLOSE: 
            PostQuitMessage(0);
            return 0;
            break;
        case WM_PAINT: 
            BeginPaint(hwnd,&paintStruct);
            EndPaint(hwnd, &paintStruct);
            return 0;
            break;
        default:
            break;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}
