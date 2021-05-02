// Microsoft being picky on standard C
#define _CRT_SECURE_NO_WARNINGS

// For UTF-8 strings across WINAPI functions (does it apply to standard libraries too?)
#define UNICODE

#define _USE_MATH_DEFINES //to get math constants
#include <math.h>
#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <ScrnSave.h>
#include <CommCtrl.h>

#include "graphics.h"

#pragma comment(lib, "user32.lib")

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "ScrnSavW.lib")
#pragma comment(linker, "/subsystem:windows")


// Logging stuff: The file is for general purpose, the info queue is used to dump d3d messages to another file
#define LOG_MAIN_FILENAME "application.log"
#define LOG_D3D_FILENAME "d3d.log"


FILE* instanceLog = NULL;




// RATIONALE: The window is set to fullscreen at native resolution, and the viewport is set as square. Hence, viewport size is bound by the smallest dimension
// This is effectively the custom screensaver's "side" entry point
// int i = 0;
LRESULT WINAPI ScreenSaverProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    // static HDC          hdc;      // device-context handle  
    // static RECT         rc;       // RECT structure  
    // static UINT         uTimer;   // timer identifier

    switch(message) {

    case WM_CREATE:

        // AP210501: Need to study these more
        // TODO: Add error checking to verify LoadString success for both calls
        // Retrieve the application name from the .rc file.
        LoadString(hMainInstance, idsAppName, szAppName, sizeof szAppName);
        // Retrieve the .ini (or registry) file name.
        LoadString(hMainInstance, idsIniFile, szIniFile, sizeof szIniFile);
        // Get configuration from registry
        // GetConfig();
        // Set a timer for the screen saver window using the redraw rate stored in Regedit.ini
        // uTimer = SetTimer(hWnd, 1, 10, NULL); 

        // Log construction
        instanceLog = fopen(LOG_MAIN_FILENAME, "w");
        setvbuf(instanceLog, NULL, _IONBF, 0);
        SYSTEMTIME st;
        GetLocalTime(&st);
        fprintf(instanceLog, "Start logging at %04d-%02d-%02d %02d:%02d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);

        // Get window info, and start the engines
        CREATESTRUCT* wInfo = (CREATESTRUCT*) lParam;
        InitD3D(hWnd, wInfo->cx, wInfo->cy, instanceLog);
 
        break; 

    // case WM_ERASEBKGND: 

    //     // The WM_ERASEBKGND message is issued before the 
    //     // WM_TIMER message, allowing the screen saver to 
    //     // paint the background as appropriate. 

    //     hdc = GetDC(hWnd); 
    //     GetClientRect (hWnd, &rc); 
    //     FillRect (hdc, &rc, GetStockObject(BLACK_BRUSH)); 
    //     ReleaseDC(hWnd,hdc); 
    //     break; 

    // case WM_TIMER: 
    case WM_PAINT: 

        // The WM_TIMER message is issued at (lSpeed * 1000) 
        // intervals, where lSpeed == .001 seconds. This 
        // code repaints the entire desktop with a white, 
        // light gray, dark gray, or black brush each 
        // time a WM_TIMER message is issued. 

        // hdc = GetDC(hWnd); 
        // GetClientRect(hWnd, &rc); 
        // if (i++ <= 4) 
        //     FillRect(hdc, &rc, GetStockObject(i)); 
        // else 
        //     (i = 0); 
        // ReleaseDC(hWnd,hdc); 

        RenderFrame();
        return 0;
        // break;

    // case WM_SIZE:
    //     int width = LOWORD(lParam), height = HIWORD(lParam);
    //     fprintf(instanceLog, "Resizing to %d x %d\n", width, height);
    //     resizeD3D(width, height);
    //     return 0;
 
    case WM_DESTROY: 

        // When the WM_DESTROY message is issued, the screen saver must destroy any of the timers that were set at WM_CREATE time
        fprintf(instanceLog, "Received WM_DESTROY message. Terminating\n");
        CleanD3D(LOG_D3D_FILENAME);
        fclose(instanceLog);

        // if (uTimer) 
        //     KillTimer(hWnd, uTimer); 
        // break; 
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
            EndDialog( hDlg, LOWORD( wParam ) == IDOK ); 
            return TRUE; 

        case IDCANCEL: 
            EndDialog( hDlg, LOWORD( wParam ) == IDOK ); 
            return TRUE;   
        }
    }
    
    return FALSE; 
}



// needed for SCRNSAVE.LIB
BOOL WINAPI RegisterDialogClasses(HANDLE hInst) {
    (void) hInst;
    return TRUE;
}
