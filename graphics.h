
// Microsoft being picky on standard C
#define _CRT_SECURE_NO_WARNINGS

// For UTF-8 strings across WINAPI functions (does it apply to standard libraries too?)
#define UNICODE

#include <windows.h>

typedef struct shape_impl shape;

extern shape mengerL1;
extern shape mengerL2;
extern shape* currentShape;

void InitD3D(HWND window, int width, int height);

void resizeD3D(int width, int height);

void CALLBACK RenderFrame(HWND window, UINT message, UINT_PTR timerID, DWORD currentTime);

void CleanD3D();
