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
#include "dynamenger.h"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "ScrnSavW.lib")
#pragma comment(linker, "/subsystem:windows")

#include "controls.h"


#define REG_SCR_ROOT_PATH TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Screensavers\\SomeScreensMustSave")

HKEY rootKey = NULL;
DWORD regTypeSpongeLevel;
DWORD spongeLevel;
DWORD slSize = sizeof spongeLevel;




void GetScrConfig() {
    // Open (or create) the screensaver's registry key
    RegCreateKeyEx(HKEY_CURRENT_USER, REG_SCR_ROOT_PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &rootKey, NULL);

    // Read the sponge level from the registry, defaults to 1
    // DOWNCAST
    switch (RegQueryValueEx(rootKey, REGNAME_SPONGE_LEVEL, NULL, &regTypeSpongeLevel, (LPVOID) &spongeLevel, &slSize)) {
    case ERROR_SUCCESS:
        if (regTypeSpongeLevel != REG_DWORD || spongeLevel < 1 || spongeLevel > 2) {
            spongeLevel = 1;
        }
        break;
    case ERROR_FILE_NOT_FOUND:
    case ERROR_MORE_DATA:
    default:
        // Some error occurred, and we don't care
        spongeLevel = 1;
        break;
    }
}




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
        // Retrieve the application name from the .rc file
        LoadString(hMainInstance, idsAppName, szAppName, sizeof szAppName);

        // Get configuration from registry
        GetScrConfig();

#ifdef DEBUG
        fprintf(instanceLog, "Sponge level: %lu\n", spongeLevel);
#endif
        
        switch (spongeLevel) {
        case 0:
            currentShape = &mengerL0;
            break;
        case 1:
            currentShape = &mengerL1;
            L1_completeLayers();
            break;
        case 2:
            currentShape = &mengerL2;
            L2_completeLayers();
            break;
        }

        buildShape(currentShape);

        // Get window info, and start the engines
        CREATESTRUCT* wInfo = (CREATESTRUCT*) lParam;
        InitD3D(hWnd, wInfo->cx, wInfo->cy);
        
        // Set a timer for the screen saver window using the redraw rate stored in Regedit.ini
        uTimer = SetTimer(hWnd, 1, 13, RenderFrame);
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




// NOTE: when invoking ssms /c, the exit code will be the one given to th EndDialog function
BOOL WINAPI ScreenSaverConfigureDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    (void) lParam;

    // would need this for slider bars or other common controls
    //InitCommonControlsEx();

    switch (message) {

    case WM_INITDIALOG:

        LoadString(hMainInstance, idsAppName, szAppName, sizeof szAppName);
        GetScrConfig();
        
        // Set the dialog with the current seting
        switch (spongeLevel) {
        case 1:
            CheckRadioButton(hDlg, CTRL_SPONGE_LEVEL_1, CTRL_SPONGE_LEVEL_2, CTRL_SPONGE_LEVEL_1);
            break;
        case 2:
            CheckRadioButton(hDlg, CTRL_SPONGE_LEVEL_1, CTRL_SPONGE_LEVEL_2, CTRL_SPONGE_LEVEL_2);
            break;
        }
        
        return TRUE;


    case WM_COMMAND:

        switch (LOWORD(wParam)) {
        case CTRL_SPONGE_LEVEL_1:
            spongeLevel = 1;
            return TRUE;
        case CTRL_SPONGE_LEVEL_2:
            spongeLevel = 2;
            return TRUE;
        case IDOK:
            RegSetValueEx(rootKey, REGNAME_SPONGE_LEVEL, 0, REG_DWORD, (LPVOID) &spongeLevel, sizeof spongeLevel);
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam) == IDOK);
            RegCloseKey(rootKey);
            return TRUE;
        }

        return FALSE;


    }
    
    return FALSE;
}




BOOL WINAPI RegisterDialogClasses(HANDLE hInst) {
    (void) hInst;
    return TRUE;
}
