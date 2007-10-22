#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

#include "window.h"
#include "resource.h"

extern HINSTANCE GBLhInstance;

#define IDM_NEW                         1
#define IDM_OPEN                        2
#define IDM_SAVE                        3

TBBUTTON tbButtons[] = {
	{STD_FILENEW,  IDM_NEW,  TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
	{STD_FILEOPEN, IDM_OPEN, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
	{STD_FILESAVE, IDM_SAVE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
};

HWND createSimpleToolbar(HWND hWndParent)
{
    HWND hWndToolBar;

    hWndToolBar = CreateToolbarEx(hWndParent,
                                  WS_VISIBLE | 
                                  WS_CHILD | 
                                  TBSTYLE_FLAT |
                                  CCS_NODIVIDER | 
                                  CCS_NORESIZE | 
                                  CCS_NOPARENTALIGN,
                                  IDR_TOOLBAR, 2, 
                                  (HINSTANCE)HINST_COMMCTRL, IDB_STD_LARGE_COLOR,
                                  (LPCTBBUTTON)&tbButtons, 2, 
                                  GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 
                                  GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 
                                  sizeof (TBBUTTON));

    return hWndToolBar;
}

//HWND createSimpleToolbar(HWND hWndParent)
//{
//    HWND hwnd;
//    
//    hwnd = CreateWindowEx(0, TOOLBARCLASSNAME, NULL, WS_CHILD, 
//                          0, 0, 100, 100, hWndParent, NULL, GBLhInstance, NULL);
//    if(hwnd == NULL)
//        return NULL;
//    
//    const int ImageListID = 0;
//    const int numButtons = 3;
//    const DWORD buttonStyles = BTNS_AUTOSIZE;
//    const int bitmapSize = 16;
//
//    HIMAGELIST hImageList = ImageList_Create(
//        bitmapSize, bitmapSize,
//        ILC_COLOR | ILC_MASK,
//        numButtons, 0);
//    
//    SendMessage(hwnd, TB_SETIMAGELIST, (WPARAM)ImageListID, 
//        (LPARAM)hImageList);
//       
//    SendMessage(hwnd, TB_LOADIMAGES, (WPARAM)IDB_STD_SMALL_COLOR, 
//        (LPARAM)HINST_COMMCTRL);
//    
//    TBBUTTON tbButtons[numButtons] = 
//    {
//        { MAKELONG(STD_FILENEW, ImageListID), IDM_NEW, TBSTATE_ENABLED, 
//          buttonStyles, {0}, 0, (INT_PTR)L"New" },
//        { MAKELONG(STD_FILEOPEN, ImageListID), IDM_OPEN, TBSTATE_ENABLED, 
//          buttonStyles, {0}, 0, (INT_PTR)L"Open"},
//        { MAKELONG(STD_FILESAVE, ImageListID), IDM_SAVE, 0, 
//          buttonStyles, {0}, 0, (INT_PTR)L"Save"}
//    };
//    
//    SendMessage(hwnd, TB_BUTTONSTRUCTSIZE, 
//        (WPARAM)sizeof(TBBUTTON), 0);
//    SendMessage(hwnd, TB_ADDBUTTONS, (WPARAM)numButtons, 
//        (LPARAM)&tbButtons);
//    
//    SendMessage(hwnd, TB_AUTOSIZE, 0, 0); 
//    ShowWindow(hwnd, TRUE);
//    
//    return hwnd;
//}
