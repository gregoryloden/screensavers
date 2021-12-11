#include "main.h"
#include <scrnsave.h>
#include <commctrl.h>

void getScreenSize(HWND hWnd);
void setupOpengl(HWND hWnd);
void teardownOpengl(HWND hWnd);

int screenWidth;
int screenHeight;
LARGE_INTEGER ticksPerSecond;
LARGE_INTEGER startTime;
HDC hdc;
HGLRC hglrc;

LONG WINAPI ScreenSaverProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
		case WM_CREATE:
			QueryPerformanceCounter(&startTime);
			QueryPerformanceFrequency(&ticksPerSecond);
			getScreenSize(hWnd);
			setupOpengl(hWnd);
			Init();
			return 0;
		case WM_DESTROY:
			teardownOpengl(hWnd);
			break;
		case WM_PAINT:
			LARGE_INTEGER currentTime;
			QueryPerformanceCounter(&currentTime);
			float timeElapsed = (float)(currentTime.QuadPart - startTime.QuadPart) / (float)ticksPerSecond.QuadPart;
			glClear(GL_COLOR_BUFFER_BIT);
			Draw(hdc, timeElapsed);
			glFlush();
			SwapBuffers(hdc);
			return 0;
	}
	return DefScreenSaverProc(hWnd, message, wParam, lParam);
}

BOOL WINAPI ScreenSaverConfigureDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
		case WM_INITDIALOG:
			return DialogInit(hDlg);
		case WM_COMMAND:
			WORD command = LOWORD(wParam);
			BOOL wasProcessed = DialogCommand(hDlg, command);
			switch (command) {
				case IDOK: EndDialog(hDlg, true); return TRUE;
				case IDCANCEL: EndDialog(hDlg, false); return TRUE;
			}
			return wasProcessed;
	}
	return FALSE;
}

//Needed for scrnsave.lib
BOOL WINAPI RegisterDialogClasses(HANDLE hInst) { return TRUE; }

void getScreenSize(HWND hWnd) {
	RECT rect;
	GetClientRect(hWnd, &rect);
	screenWidth = rect.right;
	screenHeight = rect.bottom;
}

void setupOpengl(HWND hWnd) {
	PIXELFORMATDESCRIPTOR pfd = {};
	pfd.nSize = sizeof pfd;
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.cColorBits = 24;

	hdc = GetDC(hWnd);
	SetPixelFormat(hdc, ChoosePixelFormat(hdc, &pfd), &pfd);

	hglrc = wglCreateContext(hdc);
	wglMakeCurrent(hdc, hglrc);

	//Render this box of the world to the window
	glOrtho(0, screenWidth, screenHeight, 0, -1, 1);
}

void teardownOpengl(HWND hWnd) {
	wglMakeCurrent(nullptr, nullptr);
	wglDeleteContext(hglrc);
	ReleaseDC(hWnd, hdc);
}
