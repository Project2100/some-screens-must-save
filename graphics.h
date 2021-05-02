
// Microsoft being picky on standard C
#define _CRT_SECURE_NO_WARNINGS

// For UTF-8 strings across WINAPI functions (does it apply to standard libraries too?)
#define UNICODE

#include <windows.h>
#include <stdio.h>

void InitD3D(HWND hWnd, int width, int height, FILE* log);

void resizeD3D(int width, int height);

void RenderFrame();

void CleanD3D(char* logFilename);
