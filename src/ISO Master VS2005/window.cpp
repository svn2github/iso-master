#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

#include "window.h"

extern HINSTANCE GBLhInstance;

HWND createSimpleToolbar(HWND hWndParent)
{
    const int ImageListID = 0;
    const int numButtons = 3;
    const DWORD buttonStyles = BTNS_AUTOSIZE;
    const int bitmapSize = 16;

    HWND hWndToolbar = CreateWindowEx(0, L"ISOMasterTopToolbar", NULL, 
                                      WS_CHILD | TBSTYLE_WRAPABLE,
                                      0, 0, 0, 0,
                                      hWndParent, NULL, GBLhInstance, NULL);
    if (hWndToolbar == NULL)
    {
        return NULL;
    }
    
    HIMAGELIST hImageList = ImageList_Create(
                    bitmapSize, bitmapSize,
                    ILC_COLOR32,
                    numButtons, 0);

    // Set the image list.
    //SendMessage(hWndToolbar, TB_SETIMAGELIST, (WPARAM)ImageListID, (LPARAM)hImageList);
    SendMessage(hWndToolbar, TB_SETIMAGELIST, NULL, (LPARAM)hImageList);

    //// Load the button images.
    //SendMessage(hWndToolbar, TB_LOADIMAGES, (WPARAM)IDB_STD_SMALL_COLOR, 
    //    (LPARAM)HINST_COMMCTRL);

    //// Initialize button info.
    //// IDM_NEW, IDM_OPEN, and IDM_SAVE are application-defined command constants.
    //TBBUTTON tbButtons[numButtons] = 
    //{
    //    { MAKELONG(STD_FILENEW, ImageListID), IDM_NEW, TBSTATE_ENABLED, 
    //      buttonStyles, {0}, 0, (INT_PTR)L"New" },
    //    { MAKELONG(STD_FILEOPEN, ImageListID), IDM_OPEN, TBSTATE_ENABLED, 
    //      buttonStyles, {0}, 0, (INT_PTR)L"Open"},
    //    { MAKELONG(STD_FILESAVE, ImageListID), IDM_SAVE, 0, 
    //      buttonStyles, {0}, 0, (INT_PTR)L"Save"}
    //};

    // Add buttons.
    SendMessage(hWndToolbar, TB_BUTTONSTRUCTSIZE, 
        (WPARAM)sizeof(TBBUTTON), 0);
    SendMessage(hWndToolbar, TB_ADDBUTTONS, (WPARAM)numButtons, 
        (LPARAM)&tbButtons);

    // Tell the toolbar to resize itself, and show it.
    SendMessage(hWndToolbar, TB_AUTOSIZE, 0, 0); 
    ShowWindow(hWndToolbar, TRUE);

    return hWndToolbar;
}