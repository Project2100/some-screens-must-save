// Microsoft being picky on standard C
#define _CRT_SECURE_NO_WARNINGS

// For UTF-8 strings across WINAPI functions (does it apply to standard libraries too?)
#define UNICODE

#include <windows.h>

#ifdef DEBUG
#include "debug.h"
#endif

#include <ScrnSave.h>
#include <CommCtrl.h>

#include "graphics.h"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "ScrnSavW.lib")
#pragma comment(linker, "/subsystem:windows")



// RATIONALE: The window is set to fullscreen at native resolution, and the viewport is set as square. Hence, viewport size is bound by the smallest dimension
// This is effectively the custom screensaver's "side" entry point
// int i = 0;
LRESULT WINAPI ScreenSaverProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

    static UINT_PTR uTimer;

    switch (message) {

    case WM_CREATE:
        // Log construction
#ifdef DEBUG
        instanceLog = fopen(LOG_MAIN_FILENAME, "w");
        setvbuf(instanceLog, NULL, _IONBF, 0);
        SYSTEMTIME st;
        GetLocalTime(&st);
        fprintf(instanceLog, "Start logging at %04d-%02d-%02d %02d:%02d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
#endif
        // AP210501: Need to study these more
        // TODO: Add error checking to verify LoadString success for both calls
        // Retrieve the application name from the .rc file, and the .ini (or registry) file name
        LoadString(hMainInstance, idsAppName, szAppName, sizeof szAppName);
        LoadString(hMainInstance, idsIniFile, szIniFile, sizeof szIniFile);
        // Get configuration from registry
        // GetConfig();
        // Set a timer for the screen saver window using the redraw rate stored in Regedit.ini
        uTimer = SetTimer(hWnd, 1, 13, RenderFrame); 
        // Get window info, and start the engines
        CREATESTRUCT* wInfo = (CREATESTRUCT*) lParam;
        InitD3D(hWnd, wInfo->cx, wInfo->cy);
        break;


    case WM_SIZE:
        int width = LOWORD(lParam), height = HIWORD(lParam);
#ifdef DEBUG
        fprintf(instanceLog, "Resizing to %d x %d\n", width, height);
#endif
        resizeD3D(width, height);
        break;
 

    case WM_DESTROY: 
#ifdef DEBUG
        fprintf(instanceLog, "Received WM_DESTROY message. Terminating\n");
#endif
        CleanD3D();
        if (uTimer) KillTimer(hWnd, uTimer);
#ifdef DEBUG
        fprintf(instanceLog, "Exiting\n");
        fclose(instanceLog);
#endif
        break;

        
    }

    return DefScreenSaverProc(hWnd, message, wParam, lParam);

}




BOOL WINAPI ScreenSaverConfigureDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    (void) lParam;

    // would need this for slider bars or other common controls
    InitCommonControls();

    // HWND aCheck;

    switch (message) {

    case WM_INITDIALOG:
        LoadString(hMainInstance, IDS_DESCRIPTION, szAppName, APPNAMEBUFFERLEN);

        // GetConfig();

        // aCheck = GetDlgItem( hDlg, IDC_TUMBLE );
        // SendMessage( aCheck, BM_SETCHECK, bTumble ? BST_CHECKED : BST_UNCHECKED, 0 );

        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) { 

        // case IDC_TUMBLE:
        //     bTumble = (IsDlgButtonChecked( hDlg, IDC_TUMBLE ) == BST_CHECKED);
        //     return TRUE;

        //     //cases for other controls would go here

        case IDOK:
            // WriteConfig(hDlg);      //get info from controls
            EndDialog(hDlg, LOWORD(wParam) == IDOK); 
            return TRUE; 

        case IDCANCEL: 
            EndDialog(hDlg, LOWORD(wParam) == IDOK); 
            return TRUE;   
        }
    }
    
    return FALSE; 
}




BOOL WINAPI RegisterDialogClasses(HANDLE hInst) {
    (void) hInst;
    return TRUE;
}
