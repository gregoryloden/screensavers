#include "main.h"
#include <scrnsave.h>
#include <commctrl.h>

void SetupOpengl(HWND hWnd);
void TeardownOpengl(HWND hWnd);

HDC hdc;
HGLRC hglrc;
int screenWidth;
int screenHeight;
LARGE_INTEGER ticksPerSecond;
LARGE_INTEGER startTime;

LONG WINAPI ScreenSaverProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
		case WM_CREATE:
			QueryPerformanceFrequency(&ticksPerSecond);
			QueryPerformanceCounter(&startTime);
			SetupOpengl(hWnd);
			return 0;
		case WM_DESTROY:
			TeardownOpengl(hWnd);
			break;
		case WM_PAINT:
			LARGE_INTEGER currentTime;
			QueryPerformanceCounter(&currentTime);
			float timeElapsed = (float)(currentTime.QuadPart - startTime.QuadPart) / (float)ticksPerSecond.QuadPart;
			Draw(hdc, timeElapsed, screenWidth, screenHeight);
			return 0;
	}
	return DefScreenSaverProc(hWnd, message, wParam, lParam);
}

//Needed for scrnsave.lib
BOOL WINAPI ScreenSaverConfigureDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) { return FALSE; }

//Needed for scrnsave.lib
BOOL WINAPI RegisterDialogClasses(HANDLE hInst) { return TRUE; }

void SetupOpengl(HWND hWnd) {
	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory(&pfd, sizeof pfd);
	pfd.nSize = sizeof pfd;
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.cColorBits = 24;

	hdc = GetDC(hWnd);
	SetPixelFormat(hdc, ChoosePixelFormat(hdc, &pfd), &pfd);

	hglrc = wglCreateContext(hdc);
	wglMakeCurrent(hdc, hglrc);

	//Enable vsync
	((BOOL(WINAPI*)(int))wglGetProcAddress("wglSwapIntervalEXT"))(1);

	RECT rect;
	GetClientRect(hWnd, &rect);
	screenWidth = rect.right;
	screenHeight = rect.bottom;

	//Render this box of the world to the window
	glOrtho(0, screenWidth, screenHeight, 0, -1, 1);
}

void TeardownOpengl(HWND hWnd) {
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hglrc);
	ReleaseDC(hWnd, hdc);
}
