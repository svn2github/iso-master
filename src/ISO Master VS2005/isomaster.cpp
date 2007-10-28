/******************************************************************************
* Windows main for ISO Master
* */

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <shlobj.h>
#include <stdio.h>

#include "resource.h"
#include "error.h"
#include "window.h"

HINSTANCE GBLhInstance;
HWND GBLfsBrowser;
HWND GBLisoBrowser;

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
    static HCURSOR hCursor;
    static BOOL bSplitterMoving;
    RECT rect;
    
    switch(message)
    {
    case WM_CREATE:
        
        logWarning("WM_CREATE");
        
        //createSimpleToolbar(hwnd);
        
        // for WC_LISTVIEW
        InitCommonControls();
        
        GBLfsBrowser = CreateWindowEx(WS_EX_CLIENTEDGE,
                                      WC_LISTVIEW, NULL,  
                                      WS_CHILD | WS_VISIBLE | LVS_LIST | LVS_REPORT | LVS_EDITLABELS, 
                                      0, 0, 0, 0, 
                                      hwnd, NULL,
                                      GBLhInstance, NULL);
        
        LVITEM lvitem;
        lvitem.mask = LVIF_TEXT | /*LVIF_IMAGE |*/ LVIF_PARAM | LVIF_STATE; 
        lvitem.state = 0; 
        lvitem.stateMask = 0; 
        for(int count = 0; count < 10; count++)
        {
            lvitem.iItem = count;
            //lvitem.iImage = index;
            lvitem.iSubItem = 0;
            //lvitem.lParam = (LPARAM) &rgPetInfo[index];
            //lvitem.pszText = LPSTR_TEXTCALLBACK; // sends an LVN_GETDISP message.
            lvitem.pszText = L"asd";

            if(ListView_InsertItem(GBLfsBrowser, &lvitem) == -1)
                break;
        }
        
        GBLisoBrowser = CreateWindowEx(WS_EX_CLIENTEDGE,
                                       WC_LISTVIEW, NULL,  
                                       WS_CHILD | WS_VISIBLE | LVS_LIST | LVS_REPORT | LVS_EDITLABELS, 
                                       0, 0, 0, 0, 
                                       hwnd, NULL,
                                       GBLhInstance, NULL);
        
        hCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENS));
        
        bSplitterMoving = FALSE;
        dwSplitterPos = 130;
        
        return 0;
    
    case WM_SIZE:
        
        logWarning("WM_SIZE");
        
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
        MoveWindow(GBLfsBrowser, 0, 0, LOWORD(lParam), dwSplitterPos - 1, TRUE);
        MoveWindow(GBLisoBrowser, 0, dwSplitterPos + 2, LOWORD(lParam) , HIWORD(lParam) - dwSplitterPos - 2, TRUE);
		
        return 0;

    case WM_COMMAND:
        
        logWarning("WM_COMMAND");
        
        switch(wParam)
        {
        case ID_IMAGE_QUIT:
            logWarning("quit");
            PostQuitMessage(1);
        break;
        case ID_IMAGE_OPEN:
            logWarning("open");
            
            OPENFILENAME openFileName;
            LPTSTR filePathAndName = new TCHAR[1024];
            filePathAndName[0] = NULL;
            
            memset(&openFileName, 0, sizeof(openFileName));
            openFileName.lStructSize = sizeof(OPENFILENAME);
            openFileName.hwndOwner = hwnd;
            openFileName.hInstance = NULL;
            openFileName.lpstrFilter = NULL;
            openFileName.lpstrCustomFilter = NULL;
            openFileName.nMaxCustFilter = 0;
            openFileName.nFilterIndex = 0;
            openFileName.lpstrFile = filePathAndName;
            openFileName.nMaxFile = 1024;
            openFileName.lpstrFileTitle = NULL;
            openFileName.nMaxFileTitle = 0;
            openFileName.lpstrInitialDir = NULL;
            openFileName.lpstrTitle = NULL;
            openFileName.Flags = 0;
            
            GetOpenFileName(&openFileName);
            
            // see msdn OPENFILENAME->lpstrFile for how to get the actual filename
            
            delete filePathAndName;
        break;
        }
        return 0;
        
    case WM_MOUSEMOVE:
        
        logWarning("WM_MOUSEMOVE");
        
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
        
        logWarning("WM_LBUTTONDOWN");
        
        SetCursor(hCursor);
        bSplitterMoving = TRUE;
        SetCapture(hwnd);
        return 0;

    case WM_LBUTTONUP:
        
        logWarning("WM_LBUTTONUP");
        
        ReleaseCapture();
        bSplitterMoving = FALSE;
        return 0;
    
    case WM_NOTIFY:
        
        switch (((LPNMHDR) lParam)->code)
        {
        case LVN_GETDISPINFO:
            NMLVDISPINFO* plvdi = (NMLVDISPINFO*)lParam;
            switch (((NMLVDISPINFO*)lParam)->item.iSubItem)
            {
            case 0:
            logWarning("WM_NOTIFY:LVN_GETDISPINFO0");
                plvdi->item.pszText = L"asd";
                break;
        	
            case 1:
            logWarning("WM_NOTIFY:LVN_GETDISPINFO1");
                plvdi->item.pszText = L"qwe";
                break;
            
            case 2:
            logWarning("WM_NOTIFY:LVN_GETDISPINFO2");
                plvdi->item.pszText = L"zxc";
                break;
            
            default:
                break;
            }
            // NOTE: in addition to setting pszText to point to the item text, you could 
            // copy the item text into pszText using StringCchCopy. For example:

            // StringCchCopy(rgPetInfo[plvdi->item.iItem].szKind, 
            //                         sizeof(rgPetInfo[plvdi->item.iItem].szKind), 
            //                         plvdi->item.pszText);
        }
        return 0;
        
    case WM_CLOSE: 
        
        logWarning("WM_CLOSE");
        
        PostQuitMessage(0);
        return 0;

    default:
        break;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}
