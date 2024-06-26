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

#include "controls.h"


#define SSMS_REGISTRY_ROOT_PATH TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Screensavers\\SomeScreensMustSave")

HKEY rootKey = NULL;

#define SSMS_SPONGELEVEL_MIN 1
#define SSMS_SPONGELEVEL_MAX 3
#define SSMS_SPONGELEVEL_DEFAULT 1
DWORD spongeLevel;
DWORD slSize = sizeof spongeLevel;

#define SSMS_COLOURMODE_MIN 0
#define SSMS_COLOURMODE_MAX 1
#define SSMS_COLOURMODE_DEFAULT 0
DWORD colourMode;
DWORD cmSize = sizeof colourMode;

#define SSMS_RAINBOWSPEED_MIN 10
#define SSMS_RAINBOWSPEED_MAX 1000
#define SSMS_RAINBOWSPEED_DEFAULT 40
DWORD rainbowSpeed;
DWORD rsSize = sizeof rainbowSpeed;


// Reads the screensaver's configuration from its registry values
// Is (and must be) called when setting the screensaver options, or actually running the screensaver itself
//
// Overview:
// - Open the root key for the screensaver (creates it, if it does not exist)
// - Read each value one by one
// - On each read, use hardcoded default value when one of the conditions below apply:
//   - If read goes wrong (value does not exist)
//   - If read value is of unexpected type
//   - If value is outside bounds

void GetScrConfig() {
    
    RegCreateKeyEx(HKEY_CURRENT_USER, SSMS_REGISTRY_ROOT_PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &rootKey, NULL);
    DWORD regValueType;

    // Sponge level
    // DOWNCAST
    switch (RegQueryValueEx(rootKey, REGNAME_SPONGE_LEVEL, NULL, &regValueType, (LPVOID) &spongeLevel, &slSize)) {
    case ERROR_SUCCESS:
        if (regValueType != REG_DWORD || spongeLevel < SSMS_SPONGELEVEL_MIN || spongeLevel > SSMS_SPONGELEVEL_MAX) {
            spongeLevel = SSMS_SPONGELEVEL_DEFAULT;
        }
        break;
    case ERROR_FILE_NOT_FOUND:
    case ERROR_MORE_DATA:
    default:
        spongeLevel = SSMS_SPONGELEVEL_DEFAULT;
        break;
    }

    // Colour mode
    // DOWNCAST
    switch (RegQueryValueEx(rootKey, REGNAME_COLOUR_MODE, NULL, &regValueType, (LPVOID) &colourMode, &cmSize)) {
    case ERROR_SUCCESS:
        if (regValueType != REG_DWORD || colourMode < SSMS_COLOURMODE_MIN || colourMode > SSMS_COLOURMODE_MAX) {
            colourMode = SSMS_COLOURMODE_DEFAULT;
        }
        break;
    case ERROR_FILE_NOT_FOUND:
    case ERROR_MORE_DATA:
    default:
        colourMode = SSMS_COLOURMODE_DEFAULT;
        break;
    }
    
    // Rainbow shift speed
    // DOWNCAST
    switch (RegQueryValueEx(rootKey, REGNAME_RAINBOW_SPEED, NULL, &regValueType, (LPVOID) &rainbowSpeed, &rsSize)) {
    case ERROR_SUCCESS:
        if (regValueType != REG_DWORD || rainbowSpeed < SSMS_RAINBOWSPEED_MIN || rainbowSpeed > SSMS_RAINBOWSPEED_MAX) {
            rainbowSpeed = SSMS_RAINBOWSPEED_DEFAULT;
        }
        break;
    case ERROR_FILE_NOT_FOUND:
    case ERROR_MORE_DATA:
    default:
        rainbowSpeed = SSMS_RAINBOWSPEED_DEFAULT;
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
#ifdef DEBUG
        // Log construction
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
        
        // Let Dynamenger do the hard work
        buildShape(spongeLevel);

        // Get window info, and start the engines
        //CREATESTRUCT* wInfo = (CREATESTRUCT*) lParam;

        // int fsWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        // int fsHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);

        HDC hdc = GetDC(hWnd);
        RECT rc = (RECT) {
            .bottom = GetSystemMetrics(SM_CYVIRTUALSCREEN),
            .left = GetSystemMetrics(SM_XVIRTUALSCREEN),
            .right = GetSystemMetrics(SM_CXVIRTUALSCREEN),
            .top = GetSystemMetrics(SM_YVIRTUALSCREEN),
        };
        // GetClientRect (hWnd, &rc); 
        FillRect (hdc, &rc, GetStockObject(BLACK_BRUSH)); 
        ReleaseDC(hWnd, hdc); 



        InitD3D(hWnd, colourMode, (float) rainbowSpeed / 10000);
        
        // Set a timer for the screen saver window using the redraw rate stored in Regedit.ini
        uTimer = SetTimer(hWnd, 1, 13, RenderFrame);
        break;

    // Not handling SIZE messages, assuming window size remains unchanged (i.e. fullscreen on primary)
//     case WM_SIZE:
//         int width = LOWORD(lParam), height = HIWORD(lParam);
// #ifdef DEBUG
//         fprintf(instanceLog, "Resizing to %d x %d\n", width, height);
// #endif
//         resizeD3D(width, height);
//         break;


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




// NOTE: when invoking ssms /c, the exit code will be the one given to the EndDialog function
BOOL WINAPI ScreenSaverConfigureDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    (void) lParam;

    // would need this for slider bars or other common controls
    //InitCommonControlsEx();

    switch (message) {

    case WM_INITDIALOG:
#ifdef DEBUG
        // Log construction
        configLog = fopen(LOG_CONFIG_FILENAME, "w");
        setvbuf(configLog, NULL, _IONBF, 0);
        SYSTEMTIME st;
        GetLocalTime(&st);
        fprintf(configLog, "Start logging at %04d-%02d-%02d %02d:%02d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
#endif

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
        case 3:
            CheckRadioButton(hDlg, CTRL_SPONGE_LEVEL_3, CTRL_SPONGE_LEVEL_3, CTRL_SPONGE_LEVEL_3);
            break;
        }
        
        return TRUE;


    case WM_COMMAND:

#ifdef DEBUG
            fprintf(configLog, "Command received - wparam: %08x - lparam: %016lx - loword: %04x\n", wParam, lParam, LOWORD(wParam));
#endif

        switch (LOWORD(wParam)) {
        case CTRL_SPONGE_LEVEL_1:
            spongeLevel = 1;
            return TRUE;
        case CTRL_SPONGE_LEVEL_2:
            spongeLevel = 2;
            return TRUE;
        case CTRL_SPONGE_LEVEL_3:
            spongeLevel = 3;
            return TRUE;
        case IDOK:
            RegSetValueEx(rootKey, REGNAME_SPONGE_LEVEL, 0, REG_DWORD, (LPVOID) &spongeLevel, sizeof spongeLevel);
#ifdef DEBUG
            fprintf(configLog, "IDOK command processed\n");
#endif
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam) == IDOK);
            RegCloseKey(rootKey);
#ifdef DEBUG
            fprintf(configLog, "Closing dialog\n");
            fclose(configLog);
#endif
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
