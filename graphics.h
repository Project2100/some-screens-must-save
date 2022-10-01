
// Microsoft being picky on standard C
#define _CRT_SECURE_NO_WARNINGS

// For UTF-8 strings across WINAPI functions (does it apply to standard libraries too?)
#define UNICODE

#include <windows.h>


void InitD3D(HWND window);

void resizeD3D(int width, int height);

void CALLBACK RenderFrame(HWND window, UINT message, UINT_PTR timerID, DWORD currentTime);

void CleanD3D();
